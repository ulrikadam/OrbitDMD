//
// matrix.cpp
//
#include "matrix.h"
#include "fileutil.h"		// YieldToNetwork ()
#include <circle/bcm2835.h>
#include <circle/memio.h>
#include <circle/gpiopin.h>
#include <circle/synchronize.h>
#include <circle/logger.h>
#include <circle/util.h>

#if AARCH != 64
	#error "Ce pilote utilise le compteur generique ARM en mode 64 bits"
#endif

static const char From[] = "matrix";

#define GPIO_BIT(n)	(1U << (n))

// Brochage "regular" (hzeller/rpi-rgb-led-matrix, lib/hardware-mapping.c)
#define PIN_OE		18
#define PIN_CLOCK	17
#define PIN_STROBE	4

static const unsigned s_AddressPins[5] = {22, 23, 24, 25, 15};	// A B C D E

static const unsigned s_ColorPins[3][6] =	// R1 G1 B1 R2 G2 B2
{
	{11, 27,  7,  8,  9, 10},	// sortie TOP-P0
	{12,  5,  6, 19, 13, 20},	// sortie Middle-P1
	{14,  2,  3, 26, 16, 21}	// sortie BOTTOM-P3 (masque I2C et UART)
};

static inline u64 ReadCounter (void)
{
	u64 nValue;
	asm volatile ("isb\n\tmrs %0, cntpct_el0" : "=r" (nValue));
	return nValue;
}

static inline u64 CounterFrequency (void)
{
	u64 nValue;
	asm volatile ("mrs %0, cntfrq_el0" : "=r" (nValue));
	return nValue;
}

CMatrix *CMatrix::s_pThis = 0;

CMatrix::CMatrix (void)
:	m_nWidth (0),
	m_nHeight (0),
	m_nBrightness (101),
	m_nBufferWords (0),
	m_nFront (0),
	m_bSwapPending (FALSE),
	m_nRefreshCount (0),
	m_pLastFrame (0)
{
	m_pBuffer[0] = 0;
	m_pBuffer[1] = 0;
	s_pThis = this;
}

CMatrix::~CMatrix (void)
{
	delete [] m_pBuffer[0];
	delete [] m_pBuffer[1];
	delete [] m_pLastFrame;
}

boolean CMatrix::Initialize (const TMatrixOptions &rOptions)
{
	m_Options = rOptions;
	CLogger *pLogger = CLogger::Get ();

	if (   (m_Options.nRows != 16 && m_Options.nRows != 32 && m_Options.nRows != 64)
	    || m_Options.nCols == 0 || m_Options.nCols > 256
	    || m_Options.nChain == 0 || m_Options.nChain > 8
	    || m_Options.nParallel == 0 || m_Options.nParallel > 3)
	{
		pLogger->Write (From, LogError, "Geometrie non geree : %ux%u, chaine %u, parallele %u",
				m_Options.nRows, m_Options.nCols, m_Options.nChain,
				m_Options.nParallel);
		return FALSE;
	}

	if (m_Options.nPWMBits < 1 || m_Options.nPWMBits > BitPlanes)
	{
		m_Options.nPWMBits = BitPlanes;
	}
	if (m_Options.nPWMLSBNanos < 50 || m_Options.nPWMLSBNanos > 3000)
	{
		m_Options.nPWMLSBNanos = 130;
	}
	m_nSlowdown = m_Options.nSlowdown > 4 ? 4 : m_Options.nSlowdown;

	m_nDoubleRows = m_Options.nRows / 2;
	m_nWidth = m_Options.nCols * m_Options.nChain;
	m_nHeight = m_Options.nRows * m_Options.nParallel;

	// masques des broches
	m_nColorMask = 0;
	for (unsigned nChain = 0; nChain < m_Options.nParallel; nChain++)
	{
		for (unsigned i = 0; i < 6; i++)
		{
			m_nColorMask |= GPIO_BIT (s_ColorPins[nChain][i]);
		}
	}

	m_nClock = GPIO_BIT (PIN_CLOCK);
	m_nStrobe = GPIO_BIT (PIN_STROBE);
	m_nOutputEnable = GPIO_BIT (PIN_OE);

	unsigned nAddressBits = 0;
	while ((1U << nAddressBits) < m_nDoubleRows)
	{
		nAddressBits++;
	}

	m_nRowMask = 0;
	for (unsigned i = 0; i < nAddressBits; i++)
	{
		m_nRowMask |= GPIO_BIT (s_AddressPins[i]);
	}

	for (unsigned nRow = 0; nRow < m_nDoubleRows; nRow++)
	{
		u32 nBits = 0;
		for (unsigned i = 0; i < nAddressBits; i++)
		{
			if (nRow & (1U << i))
			{
				nBits |= GPIO_BIT (s_AddressPins[i]);
			}
		}
		m_RowBits[nRow] = nBits;
	}

	// ordre des couleurs : le canal physique i recoit la composante RGBOrder[i]
	for (unsigned i = 0; i < 3; i++)
	{
		m_ColorMap[i] = i;
	}
	if (strlen (m_Options.RGBOrder) == 3)
	{
		for (unsigned i = 0; i < 3; i++)
		{
			switch (m_Options.RGBOrder[i])
			{
			case 'R': case 'r':	m_ColorMap[i] = 0;	break;
			case 'G': case 'g':	m_ColorMap[i] = 1;	break;
			case 'B': case 'b':	m_ColorMap[i] = 2;	break;
			default:					break;
			}
		}
	}

	// tampons d'image (plans de bits deja traduits en etats GPIO)
	m_nBufferWords = m_nDoubleRows * BitPlanes * m_nWidth;
	for (unsigned i = 0; i < 2; i++)
	{
		m_pBuffer[i] = new u32[m_nBufferWords];
		if (m_pBuffer[i] == 0)
		{
			pLogger->Write (From, LogError, "Memoire insuffisante");
			return FALSE;
		}
		memset (m_pBuffer[i], 0, m_nBufferWords * sizeof (u32));
	}

	// copie de l'image affichee, pour l'apercu du tableau de bord
	m_pLastFrame = new u8[m_nWidth * m_nHeight * 3];
	if (m_pLastFrame == 0)
	{
		pLogger->Write (From, LogError, "Memoire insuffisante");
		return FALSE;
	}
	memset (m_pLastFrame, 0, m_nWidth * m_nHeight * 3);

	// durees d'allumage : LSB, 2 x LSB, 4 x LSB, ...
	u64 nFrequency = CounterFrequency ();
	for (unsigned b = 0; b < BitPlanes; b++)
	{
		u64 nNanos = (u64) m_Options.nPWMLSBNanos << b;
		u64 nTicks = (nNanos * nFrequency + 500000000U) / 1000000000U;
		m_PulseTicks[b] = nTicks > 0 ? nTicks : 1;
	}

	SetBrightness (100);

	// toutes les broches en sortie, panneaux eteints (OE est actif a l'etat bas)
	u32 nOutputs = m_nColorMask | m_nClock | m_nStrobe | m_nOutputEnable | m_nRowMask;
	CGPIOPin::SetModeAll (0, nOutputs);
	write32 (ARM_GPIO_GPCLR0, nOutputs & ~m_nOutputEnable);
	write32 (ARM_GPIO_GPSET0, m_nOutputEnable);

	pLogger->Write (From, LogNotice, "%ux%u pixels, %u plans de bits, compteur %lu Hz",
			m_nWidth, m_nHeight, m_Options.nPWMBits, (unsigned long) nFrequency);

	return TRUE;
}

void CMatrix::SetBrightness (unsigned nPercent)
{
	if (nPercent > 100)
	{
		nPercent = 100;
	}

	if (nPercent == m_nBrightness)
	{
		return;
	}

	// correction de luminance CIE1931 (comme hzeller : luminance_cie1931)
	const float fOutFactor = (float) ((1 << BitPlanes) - 1);
	for (unsigned c = 0; c < 256; c++)
	{
		float v = (float) c * nPercent / 255.0f;
		float fLuminance;
		if (v <= 8.0f)
		{
			fLuminance = v / 902.3f;
		}
		else
		{
			float t = (v + 16.0f) / 116.0f;
			fLuminance = t * t * t;
		}

		m_Lookup[c] = (u16) (fOutFactor * fLuminance + 0.5f);
	}

	m_nBrightness = nPercent;
}

void CMatrix::Show (const u8 *pRGB)
{
	// attendre que le coeur de rafraichissement ait pris l'image precedente
	while (m_bSwapPending)
	{
		YieldToNetwork ();	// laisser tourner le Wi-Fi pendant l'attente (coeur 0)
	}
	DataMemBarrier ();

	memcpy (m_pLastFrame, pRGB, m_nWidth * m_nHeight * 3);	// 12 Ko : negligeable

	u32 *pBuffer = m_pBuffer[1 - m_nFront];
	memset (pBuffer, 0, m_nBufferWords * sizeof (u32));

	for (unsigned y = 0; y < m_nHeight; y++)
	{
		unsigned nChain = y / m_Options.nRows;
		unsigned yy = y % m_Options.nRows;
		boolean bBottom = yy >= m_nDoubleRows;
		unsigned nRow = yy % m_nDoubleRows;

		const unsigned *pPins = s_ColorPins[nChain] + (bBottom ? 3 : 0);
		const u32 PinBit[3] = {GPIO_BIT (pPins[0]), GPIO_BIT (pPins[1]), GPIO_BIT (pPins[2])};

		u32 *pRow = pBuffer + nRow * BitPlanes * m_nWidth;
		const u8 *pPixel = pRGB + y * m_nWidth * 3;

		for (unsigned x = 0; x < m_nWidth; x++, pPixel += 3)
		{
			u16 Level[3];
			for (unsigned i = 0; i < 3; i++)
			{
				Level[i] = m_Lookup[pPixel[m_ColorMap[i]]];
			}

			if ((Level[0] | Level[1] | Level[2]) == 0)
			{
				continue;
			}

			u32 *pBits = pRow + x;
			for (unsigned b = 0; b < BitPlanes; b++, pBits += m_nWidth)
			{
				u32 nMask = 1U << b;
				u32 nValue = 0;
				if (Level[0] & nMask)	nValue |= PinBit[0];
				if (Level[1] & nMask)	nValue |= PinBit[1];
				if (Level[2] & nMask)	nValue |= PinBit[2];
				*pBits |= nValue;
			}
		}
	}

	DataMemBarrier ();
	m_bSwapPending = TRUE;
}

inline void CMatrix::WriteSet (u32 nBits)
{
	if (nBits == 0)
	{
		return;
	}

	for (unsigned i = 0; i <= m_nSlowdown; i++)
	{
		write32 (ARM_GPIO_GPSET0, nBits);
	}
}

inline void CMatrix::WriteClear (u32 nBits)
{
	if (nBits == 0)
	{
		return;
	}

	for (unsigned i = 0; i <= m_nSlowdown; i++)
	{
		write32 (ARM_GPIO_GPCLR0, nBits);
	}
}

void CMatrix::RefreshLoop (void)
{
	const unsigned nStartBit = BitPlanes - m_Options.nPWMBits;
	const u32 nColorClockMask = m_nColorMask | m_nClock;
	unsigned nLastRow = MaxDoubleRows;	// force le premier reglage d'adresse

	while (TRUE)
	{
		if (m_bSwapPending)
		{
			DataMemBarrier ();
			m_nFront = 1 - m_nFront;
			DataMemBarrier ();
			m_bSwapPending = FALSE;
		}

		const u32 *pBuffer = m_pBuffer[m_nFront];

		for (unsigned nRow = 0; nRow < m_nDoubleRows; nRow++)
		{
			for (unsigned b = nStartBit; b < BitPlanes; b++)
			{
				// envoi d'une ligne double (haut + bas) pour ce plan de bits
				const u32 *pData = pBuffer + (nRow * BitPlanes + b) * m_nWidth;
				for (unsigned x = 0; x < m_nWidth; x++)
				{
					u32 nValue = pData[x];
					WriteClear (~nValue & nColorClockMask);	// donnees + horloge basse
					WriteSet (nValue);
					WriteSet (m_nClock);			// front montant
				}
				WriteClear (nColorClockMask);

				// adresse de ligne et verrouillage pendant que tout est eteint
				if (nRow != nLastRow)
				{
					WriteClear (~m_RowBits[nRow] & m_nRowMask);
					WriteSet (m_RowBits[nRow]);
					nLastRow = nRow;
				}

				WriteSet (m_nStrobe);
				WriteClear (m_nStrobe);

				// allumage pendant la duree de ce plan de bits
				write32 (ARM_GPIO_GPCLR0, m_nOutputEnable);
				u64 nStart = ReadCounter ();
				while (ReadCounter () - nStart < m_PulseTicks[b])
				{
					// attente active
				}
				write32 (ARM_GPIO_GPSET0, m_nOutputEnable);
			}
		}

		m_nRefreshCount++;
	}
}
