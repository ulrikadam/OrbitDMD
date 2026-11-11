//
// gifanim.cpp
//
#include "gifanim.h"
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "gif";

#define LZW_DATA_SIZE	(16 * 1024 * 1024)
#define MIN_DELAY	20	// ms ; en dessous, les navigateurs affichent 100 ms
#define DEFAULT_DELAY	100
#define YIELD_FRAMES	16	// passer la main toutes les 16 images

void (*CGifAnimation::s_pYieldHandler) (void) = 0;

static unsigned InterlacedRow (unsigned nRow, unsigned nHeight)
{
	static const unsigned Start[4] = {0, 4, 2, 1};
	static const unsigned Step[4]  = {8, 8, 4, 2};

	for (unsigned nPass = 0; nPass < 4; nPass++)
	{
		unsigned nRows = nHeight > Start[nPass]
			       ? (nHeight - Start[nPass] + Step[nPass] - 1) / Step[nPass] : 0;
		if (nRow < nRows)
		{
			return Start[nPass] + nRow * Step[nPass];
		}

		nRow -= nRows;
	}

	return 0;
}

CGifAnimation::CGifAnimation (unsigned nWidth, unsigned nHeight)
:	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_ppFrame (new u8 *[MaxFrames]),
	m_pDelay (new unsigned[MaxFrames]),
	m_nFrames (0),
	m_pScreen (new u8[MaxScreenPixels * 3]),
	m_pPrevious (new u8[MaxScreenPixels * 3]),
	m_pIndex (new u8[MaxScreenPixels]),
	m_pLZWData (new u8[LZW_DATA_SIZE]),
	m_nLZWDataSize (LZW_DATA_SIZE),
	m_pPrefix (new u16[4096]),
	m_pSuffix (new u8[4096]),
	m_pStack (new u8[4097])
{
}

CGifAnimation::~CGifAnimation (void)
{
	Clear ();

	delete [] m_ppFrame;
	delete [] m_pDelay;
	delete [] m_pScreen;
	delete [] m_pPrevious;
	delete [] m_pIndex;
	delete [] m_pLZWData;
	delete [] m_pPrefix;
	delete [] m_pSuffix;
	delete [] m_pStack;
}

void CGifAnimation::Clear (void)
{
	for (unsigned i = 0; i < m_nFrames; i++)
	{
		delete [] m_ppFrame[i];
	}

	m_nFrames = 0;
}

unsigned CGifAnimation::GetTotalDuration (void) const
{
	unsigned nTotal = 0;
	for (unsigned i = 0; i < m_nFrames; i++)
	{
		nTotal += m_pDelay[i];
	}

	return nTotal;
}

boolean CGifAnimation::Decode (const u8 *pData, unsigned nSize)
{
	Clear ();

	if (   nSize < 13
	    || memcmp (pData, "GIF8", 4) != 0)
	{
		return FALSE;
	}

	unsigned nScreenWidth  = pData[6] | (pData[7] << 8);
	unsigned nScreenHeight = pData[8] | (pData[9] << 8);
	u8 uchFlags = pData[10];

	if (   nScreenWidth == 0 || nScreenHeight == 0
	    || nScreenWidth * nScreenHeight > MaxScreenPixels)
	{
		CLogger::Get ()->Write (From, LogWarning, "Taille non geree : %ux%u",
					nScreenWidth, nScreenHeight);
		return FALSE;
	}

	unsigned nPos = 13;

	const u8 *pGlobalColors = 0;
	unsigned nGlobalColors = 0;
	if (uchFlags & 0x80)
	{
		nGlobalColors = 2 << (uchFlags & 7);
		pGlobalColors = pData + nPos;
		nPos += 3 * nGlobalColors;
		if (nPos > nSize)
		{
			return FALSE;
		}
	}

	const unsigned nScreenBytes = nScreenWidth * nScreenHeight * 3;
	memset (m_pScreen, 0, nScreenBytes);

	// controle graphique de la prochaine image
	unsigned nDelay = DEFAULT_DELAY;
	unsigned nDisposal = 0;
	int nTransparent = -1;

	while (nPos < nSize)
	{
		u8 uchBlock = pData[nPos++];

		if (uchBlock == 0x3B)		// fin du fichier
		{
			break;
		}

		if (uchBlock == 0x21)		// extension
		{
			if (nPos >= nSize)
			{
				break;
			}

			u8 uchLabel = pData[nPos++];
			if (   uchLabel == 0xF9		// Graphic Control Extension
			    && nPos + 5 < nSize
			    && pData[nPos] == 4)
			{
				u8 uchPacked = pData[nPos+1];
				nDelay = (pData[nPos+2] | (pData[nPos+3] << 8)) * 10;
				nDisposal = (uchPacked >> 2) & 7;
				nTransparent = (uchPacked & 1) ? pData[nPos+4] : -1;
			}

			while (nPos < nSize && pData[nPos] != 0)	// sous-blocs ignores
			{
				nPos += pData[nPos] + 1;
			}
			nPos++;

			continue;
		}

		if (uchBlock != 0x2C)		// bloc inconnu : on garde ce qui est decode
		{
			break;
		}

		// descripteur d'image
		if (nPos + 9 > nSize)
		{
			break;
		}

		unsigned nLeft   = pData[nPos+0] | (pData[nPos+1] << 8);
		unsigned nTop    = pData[nPos+2] | (pData[nPos+3] << 8);
		unsigned nWidth  = pData[nPos+4] | (pData[nPos+5] << 8);
		unsigned nHeight = pData[nPos+6] | (pData[nPos+7] << 8);
		u8 uchImageFlags = pData[nPos+8];
		nPos += 9;

		const u8 *pColors = pGlobalColors;
		unsigned nColors = nGlobalColors;
		if (uchImageFlags & 0x80)
		{
			nColors = 2 << (uchImageFlags & 7);
			pColors = pData + nPos;
			nPos += 3 * nColors;
		}
		boolean bInterlaced = !!(uchImageFlags & 0x40);

		if (nPos >= nSize)
		{
			break;
		}
		unsigned nMinCodeSize = pData[nPos++];

		// rassembler les sous-blocs de donnees LZW
		unsigned nLZWLength = 0;
		while (nPos < nSize && pData[nPos] != 0)
		{
			unsigned nBlockSize = pData[nPos];
			if (nPos + 1 + nBlockSize > nSize)
			{
				nBlockSize = nSize - nPos - 1;
			}
			if (nLZWLength + nBlockSize > m_nLZWDataSize)
			{
				break;
			}

			memcpy (m_pLZWData + nLZWLength, pData + nPos + 1, nBlockSize);
			nLZWLength += nBlockSize;
			nPos += pData[nPos] + 1;
		}
		nPos++;

		if (   nWidth > 0 && nHeight > 0
		    && nWidth * nHeight <= MaxScreenPixels)
		{
			const unsigned nPixels = nWidth * nHeight;

			// pixels absents (fichier tronque) = transparents
			memset (m_pIndex, nTransparent >= 0 ? nTransparent : 0, nPixels);
			DecodeLZW (m_pLZWData, nLZWLength, nMinCodeSize, m_pIndex, nPixels);

			if (nDisposal == 3)
			{
				memcpy (m_pPrevious, m_pScreen, nScreenBytes);
			}

			// composer l'image sur l'ecran logique
			for (unsigned nRow = 0; nRow < nHeight; nRow++)
			{
				unsigned y = nTop + (bInterlaced ? InterlacedRow (nRow, nHeight) : nRow);
				if (y >= nScreenHeight)
				{
					continue;
				}

				const u8 *pIndex = m_pIndex + nRow * nWidth;
				u8 *pDest = m_pScreen + (y * nScreenWidth + nLeft) * 3;

				for (unsigned nCol = 0; nCol < nWidth; nCol++, pDest += 3)
				{
					if (nLeft + nCol >= nScreenWidth)
					{
						break;
					}

					unsigned nIndex = pIndex[nCol];
					if (   (int) nIndex == nTransparent
					    || pColors == 0
					    || nIndex >= nColors)
					{
						continue;
					}

					pDest[0] = pColors[nIndex*3 + 0];
					pDest[1] = pColors[nIndex*3 + 1];
					pDest[2] = pColors[nIndex*3 + 2];
				}
			}

			if (nDelay < MIN_DELAY)
			{
				nDelay = DEFAULT_DELAY;
			}

			if (!AddFrame (m_pScreen, nScreenWidth, nScreenHeight, nDelay))
			{
				break;
			}

			// disposition avant l'image suivante
			if (nDisposal == 2)		// effacer la zone (fond = noir)
			{
				for (unsigned nRow = 0; nRow < nHeight; nRow++)
				{
					unsigned y = nTop + nRow;
					if (y >= nScreenHeight || nLeft >= nScreenWidth)
					{
						continue;
					}

					unsigned nCount = nWidth;
					if (nLeft + nCount > nScreenWidth)
					{
						nCount = nScreenWidth - nLeft;
					}

					memset (m_pScreen + (y * nScreenWidth + nLeft) * 3, 0, nCount * 3);
				}
			}
			else if (nDisposal == 3)	// revenir a l'image precedente
			{
				memcpy (m_pScreen, m_pPrevious, nScreenBytes);
			}
		}

		// le controle graphique ne vaut que pour une image
		nDelay = DEFAULT_DELAY;
		nDisposal = 0;
		nTransparent = -1;
	}

	return m_nFrames > 0;
}

boolean CGifAnimation::AddFrame (const u8 *pScreen, unsigned nScreenWidth, unsigned nScreenHeight,
				 unsigned nDelay)
{
	if (m_nFrames >= MaxFrames)
	{
		return FALSE;
	}

	u8 *pFrame = new u8[m_nWidth * m_nHeight * 3];
	if (pFrame == 0)
	{
		return FALSE;
	}
	memset (pFrame, 0, m_nWidth * m_nHeight * 3);

	// centrer l'ecran logique du GIF sur la zone d'affichage
	int nOffsetX = ((int) m_nWidth - (int) nScreenWidth) / 2;
	int nOffsetY = ((int) m_nHeight - (int) nScreenHeight) / 2;

	for (unsigned y = 0; y < m_nHeight; y++)
	{
		int sy = (int) y - nOffsetY;
		if (sy < 0 || sy >= (int) nScreenHeight)
		{
			continue;
		}

		int x0 = nOffsetX > 0 ? nOffsetX : 0;
		int sx0 = x0 - nOffsetX;
		int nCount = (int) nScreenWidth - sx0;
		if (nCount > (int) m_nWidth - x0)
		{
			nCount = (int) m_nWidth - x0;
		}
		if (nCount <= 0)
		{
			continue;
		}

		memcpy (pFrame + (y * m_nWidth + x0) * 3,
			pScreen + (sy * nScreenWidth + sx0) * 3, nCount * 3);
	}

	m_ppFrame[m_nFrames] = pFrame;
	m_pDelay[m_nFrames] = nDelay;
	m_nFrames++;

	if (s_pYieldHandler != 0 && m_nFrames % YIELD_FRAMES == 0)
	{
		(*s_pYieldHandler) ();
	}

	return TRUE;
}

unsigned CGifAnimation::DecodeLZW (const u8 *pIn, unsigned nInLength, unsigned nMinCodeSize,
				   u8 *pOut, unsigned nOutLength)
{
	if (nMinCodeSize < 1 || nMinCodeSize > 11)
	{
		return 0;
	}

	const unsigned nClear = 1U << nMinCodeSize;
	const unsigned nEnd = nClear + 1;

	unsigned nCodeSize = nMinCodeSize + 1;
	unsigned nCodeMask = (1U << nCodeSize) - 1;
	unsigned nNext = nEnd + 1;

	for (unsigned i = 0; i < nClear; i++)
	{
		m_pPrefix[i] = 0;
		m_pSuffix[i] = (u8) i;
	}

	u32 nBits = 0;
	unsigned nBitCount = 0;
	unsigned nInPos = 0;
	unsigned nOutPos = 0;
	int nOld = -1;
	u8 uchFirst = 0;

	while (nOutPos < nOutLength)
	{
		while (nBitCount < nCodeSize)
		{
			if (nInPos >= nInLength)
			{
				return nOutPos;
			}

			nBits |= (u32) pIn[nInPos++] << nBitCount;
			nBitCount += 8;
		}

		unsigned nCode = nBits & nCodeMask;
		nBits >>= nCodeSize;
		nBitCount -= nCodeSize;

		if (nCode == nClear)
		{
			nCodeSize = nMinCodeSize + 1;
			nCodeMask = (1U << nCodeSize) - 1;
			nNext = nEnd + 1;
			nOld = -1;
			continue;
		}

		if (nCode == nEnd)
		{
			break;
		}

		if (nOld < 0)
		{
			if (nCode >= nClear)
			{
				return nOutPos;		// donnees invalides
			}

			uchFirst = (u8) nCode;
			pOut[nOutPos++] = uchFirst;
			nOld = (int) nCode;
			continue;
		}

		unsigned nInCode = nCode;
		unsigned nStack = 0;

		if (nCode > nNext)
		{
			return nOutPos;			// donnees invalides
		}

		if (nCode == nNext)			// cas KwKwK
		{
			m_pStack[nStack++] = uchFirst;
			nCode = (unsigned) nOld;
		}

		while (nCode >= nClear)
		{
			if (nCode <= nEnd || nStack >= 4096)
			{
				return nOutPos;		// donnees invalides
			}

			m_pStack[nStack++] = m_pSuffix[nCode];
			nCode = m_pPrefix[nCode];
		}

		uchFirst = m_pSuffix[nCode];
		m_pStack[nStack++] = uchFirst;

		while (nStack > 0 && nOutPos < nOutLength)
		{
			pOut[nOutPos++] = m_pStack[--nStack];
		}

		if (nNext < 4096)
		{
			m_pPrefix[nNext] = (u16) nOld;
			m_pSuffix[nNext] = uchFirst;
			nNext++;

			if (nNext == (1U << nCodeSize) && nCodeSize < 12)
			{
				nCodeSize++;
				nCodeMask = (1U << nCodeSize) - 1;
			}
		}

		nOld = (int) nInCode;
	}

	return nOutPos;
}
