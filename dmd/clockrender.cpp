//
// clockrender.cpp
//
#include "clockrender.h"
#include "font5x7.h"
#include "gifanim.h"
#include "fileutil.h"
#include "weathericons.h"
#include <circle/logger.h>
#include <circle/string.h>
#include <circle/util.h>
#include <fatfs/ff.h>

#ifndef NULL
	#define NULL	0
#endif
#include "stb/stb_truetype.h"		// declarations seulement (code dans stbimpl.cpp)
#define STBI_NO_STDIO
#include "stb/stb_image.h"

static const char From[] = "horloge";

#define MAX_ASSET_SIZE		(4 * 1024 * 1024)
#define MIN_FONT_SIZE		6
#define DATE_SHARE_PERCENT	38	// date sous l'heure : 38 % de la hauteur au plus (taille auto)
#define LINE_GAP		1

static inline boolean IsDigit (char c)
{
	return '0' <= c && c <= '9';
}

CClockRenderer::CClockRenderer (unsigned nWidth, unsigned nHeight)
:	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_pGlyph (new u8[GlyphMax * GlyphMax])
{
	for (unsigned i = 0; i < SlotCount; i++)
	{
		m_Font[i].Name[0] = '\0';
		m_Font[i].pData = 0;
		m_Font[i].pInfo = 0;

		m_Pattern[i].Name[0] = '\0';
		m_Pattern[i].pRGBA = 0;
		m_Pattern[i].pAnim = 0;
		m_Pattern[i].bAnimValid = FALSE;

		m_Fit[i].pFont = 0;
		m_pMask[i] = new u8[nWidth * nHeight];
	}

	m_Background.Name[0] = '\0';
	m_Background.pRGBA = 0;
	m_Background.pAnim = 0;
	m_Background.bAnimValid = FALSE;

	strcpy (m_NoImage.Name, CLOCK_NONE);
	m_NoImage.pRGBA = 0;
	m_NoImage.pAnim = 0;
	m_NoImage.bAnimValid = FALSE;

	strcpy (m_Pixel.Name, CLOCK_FONT_PIXEL);
	m_Pixel.pData = 0;
	m_Pixel.pInfo = 0;

	m_Digits.pFont = 0;
	m_FitInfo.pFont = 0;
}

CClockRenderer::~CClockRenderer (void)
{
	for (unsigned i = 0; i < SlotCount; i++)
	{
		delete [] m_Font[i].pData;
		delete m_Font[i].pInfo;
		delete [] m_Pattern[i].pRGBA;
		delete m_Pattern[i].pAnim;
		delete [] m_pMask[i];
	}
	delete [] m_Background.pRGBA;
	delete m_Background.pAnim;
	delete [] m_pGlyph;
}

// --- chargement des fichiers de SD:/fonts ---

void CClockRenderer::Prepare (const TClockStyle &rStyle)
{
	LoadFont (&m_Font[SlotTime], rStyle.Time.Font);
	LoadImage (&m_Pattern[SlotTime], rStyle.Time.Pattern);
	LoadImage (&m_Background, rStyle.Background);

	if (rStyle.DateMode != DateOff)
	{
		LoadFont (&m_Font[SlotDate], rStyle.Date.Font);
		LoadImage (&m_Pattern[SlotDate], rStyle.Date.Pattern);
	}
}

u8 *CClockRenderer::ReadAsset (const char *pName, unsigned *pSize)
{
	CString Path;
	Path.Format ("%s/%s", CLOCK_FONT_DIR, pName);

	FILINFO Info;
	if (   f_stat (Path, &Info) != FR_OK
	    || Info.fsize == 0 || Info.fsize > MAX_ASSET_SIZE)
	{
		CLogger::Get ()->Write (From, LogWarning, "%s introuvable ou trop gros", (const char *) Path);
		return 0;
	}

	unsigned nSize = (unsigned) Info.fsize;
	u8 *pData = new u8[nSize];
	if (pData == 0 || !ReadWholeFile (Path, pData, &nSize))
	{
		delete [] pData;
		CLogger::Get ()->Write (From, LogWarning, "Lecture de %s impossible", (const char *) Path);
		return 0;
	}

	*pSize = nSize;

	return pData;
}

void CClockRenderer::LoadFont (TFont *pFont, const char *pName)
{
	if (strcmp (pFont->Name, pName) == 0)
	{
		return;				// deja chargee (ou deja essayee)
	}

	strncpy (pFont->Name, pName, sizeof pFont->Name - 1);
	pFont->Name[sizeof pFont->Name - 1] = '\0';
	delete [] pFont->pData;
	pFont->pData = 0;
	delete pFont->pInfo;
	pFont->pInfo = 0;
	m_Digits.pFont = 0;
	for (unsigned i = 0; i < SlotCount; i++)
	{
		m_Fit[i].pFont = 0;
	}

	if (strcasecmp (pName, CLOCK_FONT_PIXEL) == 0)
	{
		return;
	}

	unsigned nSize;
	u8 *pData = ReadAsset (pName, &nSize);
	if (pData == 0)
	{
		return;				// police Pixel a la place
	}

	stbtt_fontinfo *pInfo = new stbtt_fontinfo;
	int nOffset = stbtt_GetFontOffsetForIndex (pData, 0);
	if (pInfo == 0 || nOffset < 0 || !stbtt_InitFont (pInfo, pData, nOffset))
	{
		CLogger::Get ()->Write (From, LogWarning, "Police illisible : %s", pName);
		delete pInfo;
		delete [] pData;
		return;
	}

	pFont->pData = pData;
	pFont->pInfo = pInfo;
	CLogger::Get ()->Write (From, LogNotice, "Police %s chargee", pName);
}

void CClockRenderer::LoadImage (TImage *pImage, const char *pName)
{
	if (strcmp (pImage->Name, pName) == 0)
	{
		return;
	}

	strncpy (pImage->Name, pName, sizeof pImage->Name - 1);
	pImage->Name[sizeof pImage->Name - 1] = '\0';
	delete [] pImage->pRGBA;
	pImage->pRGBA = 0;
	pImage->bAnimValid = FALSE;
	if (pImage->pAnim != 0)
	{
		pImage->pAnim->Clear ();
	}

	if (strcasecmp (pName, CLOCK_NONE) == 0)
	{
		return;
	}

	unsigned nSize;
	u8 *pData = ReadAsset (pName, &nSize);
	if (pData == 0)
	{
		return;
	}

	if (nSize >= 6 && memcmp (pData, "GIF8", 4) == 0)
	{
		if (pImage->pAnim == 0)
		{
			pImage->pAnim = new CGifAnimation (m_nWidth, m_nHeight);
		}
		pImage->bAnimValid =    pImage->pAnim != 0
				     && pImage->pAnim->Decode (pData, nSize)
				     && pImage->pAnim->GetFrameCount () > 0;
	}
	else
	{
		// PNG : repete (en mosaique) pour couvrir l'ecran
		int w, h, n;
		u8 *pPixels = stbi_load_from_memory (pData, (int) nSize, &w, &h, &n, 4);
		if (pPixels != 0 && w > 0 && h > 0)
		{
			pImage->pRGBA = new u8[m_nWidth * m_nHeight * 4];
			for (unsigned y = 0; y < m_nHeight; y++)
			{
				for (unsigned x = 0; x < m_nWidth; x++)
				{
					memcpy (pImage->pRGBA + (y * m_nWidth + x) * 4,
						pPixels + ((y % h) * w + (x % w)) * 4, 4);
				}
			}
		}
		stbi_image_free (pPixels);
	}

	delete [] pData;

	if (pImage->pRGBA == 0 && !pImage->bAnimValid)
	{
		CLogger::Get ()->Write (From, LogWarning, "Image illisible : %s", pName);
	}
}

// --- texte ---

unsigned CClockRenderer::PixelScale (unsigned nSize)
{
	unsigned nScale = (nSize + 3) / FONT_HEIGHT;
	return nScale > 0 ? nScale : 1;
}

const CClockRenderer::TDigits &CClockRenderer::GetDigits (const TFont &rFont, unsigned nSize)
{
	if (m_Digits.pFont == &rFont && m_Digits.nSize == nSize)
	{
		return m_Digits;
	}

	m_Digits.pFont = &rFont;
	m_Digits.nSize = nSize;
	m_Digits.fCell = 0.0f;
	m_Digits.nTop = 0;
	m_Digits.nBottom = 0;

	float fScale = stbtt_ScaleForMappingEmToPixels (rFont.pInfo, (float) nSize);
	for (char c = '0'; c <= '9'; c++)
	{
		int nGlyph = stbtt_FindGlyphIndex (rFont.pInfo, c);
		int nAdvance, nBearing;
		stbtt_GetGlyphHMetrics (rFont.pInfo, nGlyph, &nAdvance, &nBearing);
		if (nAdvance * fScale > m_Digits.fCell)
		{
			m_Digits.fCell = nAdvance * fScale;
		}

		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox (rFont.pInfo, nGlyph, fScale, fScale, &x0, &y0, &x1, &y1);
		if (y1 > y0)
		{
			if (y0 < m_Digits.nTop)		m_Digits.nTop = y0;
			if (y1 > m_Digits.nBottom)	m_Digits.nBottom = y1;
		}
	}

	return m_Digits;
}

boolean CClockRenderer::Measure (const TFont &rFont, unsigned nSize, const char *pText,
				 TMetrics *pMetrics)
{
	TMetrics M = {0x7FFF, -0x7FFF, 0x7FFF, -0x7FFF};

	if (rFont.pInfo == 0)
	{
		// police Pixel : 5x7, avance de 6, agrandie
		unsigned nScale = PixelScale (nSize);
		int nLength = (int) strlen (pText);
		if (nLength == 0)
		{
			return FALSE;
		}
		M.nLeft = 0;
		M.nRight = nLength * FONT_ADVANCE * nScale - nScale;
		M.nTop = -(int) (FONT_HEIGHT * nScale);
		M.nBottom = 0;
		*pMetrics = M;
		return TRUE;
	}

	const TDigits &Digits = GetDigits (rFont, nSize);
	float fScale = stbtt_ScaleForMappingEmToPixels (rFont.pInfo, (float) nSize);
	float fX = 0.0f;
	int nPrevious = 0;
	char chPrevious = 0;

	for (const char *p = pText; *p != '\0'; p++)
	{
		char c = *p;
		int nGlyph = stbtt_FindGlyphIndex (rFont.pInfo, (unsigned char) c);

		TColon Colon;
		if (c == ':' && (nGlyph == 0 || stbtt_IsGlyphEmpty (rFont.pInfo, nGlyph)))
		{
			GetColon (Digits, fX, &Colon);	// police sans deux-points : on les dessine
			if (Colon.x < M.nLeft)				M.nLeft = Colon.x;
			if (Colon.x + Colon.nDot > M.nRight)		M.nRight = Colon.x + Colon.nDot;
			if (Colon.y[0] < M.nTop)			M.nTop = Colon.y[0];
			if (Colon.y[1] + Colon.nDot > M.nBottom)	M.nBottom = Colon.y[1] + Colon.nDot;
			fX += Colon.fAdvance;
			nPrevious = 0;
			chPrevious = c;
			continue;
		}

		if ((unsigned char) c == DEGREE_CHAR && (nGlyph == 0 || stbtt_IsGlyphEmpty (rFont.pInfo, nGlyph)))
		{
			int nSize = DegreeSize (Digits);	// police sans signe degre : petit anneau
			int x = (int) (fX + 0.5f) + 1;
			if (x < M.nLeft)			M.nLeft = x;
			if (x + nSize > M.nRight)		M.nRight = x + nSize;
			if (Digits.nTop < M.nTop)		M.nTop = Digits.nTop;
			if (Digits.nTop + nSize > M.nBottom)	M.nBottom = Digits.nTop + nSize;
			fX += nSize + 2;
			nPrevious = 0;
			chPrevious = c;
			continue;
		}

		int nAdvance, nBearing;
		stbtt_GetGlyphHMetrics (rFont.pInfo, nGlyph, &nAdvance, &nBearing);

		if (nPrevious != 0 && !IsDigit (c) && !IsDigit (chPrevious))
		{
			fX += fScale * stbtt_GetGlyphKernAdvance (rFont.pInfo, nPrevious, nGlyph);
		}

		float fAdvance = nAdvance * fScale;
		float fOffset = 0.0f;
		if (IsDigit (c))
		{
			fOffset = (Digits.fCell - fAdvance) / 2.0f;
			fAdvance = Digits.fCell;
		}

		int x0, y0, x1, y1;
		stbtt_GetGlyphBitmapBox (rFont.pInfo, nGlyph, fScale, fScale, &x0, &y0, &x1, &y1);
		if (x1 > x0 && y1 > y0)
		{
			int nX = (int) (fX + fOffset);
			if (IsDigit (c))
			{
				// tous les chiffres occupent la meme place : pas de saut d'une seconde a l'autre
				x0 = (int) (fX - nX);
				x1 = (int) (fX + Digits.fCell - nX + 0.999f);
				y0 = Digits.nTop;
				y1 = Digits.nBottom;
			}
			if (nX + x0 < M.nLeft)		M.nLeft = nX + x0;
			if (nX + x1 > M.nRight)		M.nRight = nX + x1;
			if (y0 < M.nTop)		M.nTop = y0;
			if (y1 > M.nBottom)		M.nBottom = y1;
		}

		fX += fAdvance;
		nPrevious = nGlyph;
		chPrevious = c;
	}

	if (M.nRight <= M.nLeft || M.nBottom <= M.nTop)
	{
		return FALSE;			// rien de visible (police sans ces caracteres ?)
	}

	*pMetrics = M;

	return TRUE;
}

void CClockRenderer::Draw (const TFont &rFont, unsigned nSize, const char *pText, int x,
			   int yBaseline, boolean bColonOn, boolean bSmooth, u8 *pMask)
{
	const int W = (int) m_nWidth, H = (int) m_nHeight;

	if (rFont.pInfo == 0)
	{
		unsigned nScale = PixelScale (nSize);
		int yTop = yBaseline - (int) (FONT_HEIGHT * nScale);
		for (const char *p = pText; *p != '\0'; p++, x += FONT_ADVANCE * nScale)
		{
			const char * const *ppGlyph = FontGetGlyph (*p);
			if (ppGlyph == 0 || (*p == ':' && !bColonOn))
			{
				continue;
			}
			for (int row = 0; row < FONT_HEIGHT; row++)
			{
				for (int col = 0; col < FONT_WIDTH; col++)
				{
					if (ppGlyph[row][col] != '#')
					{
						continue;
					}
					for (unsigned dy = 0; dy < nScale; dy++)
					{
						int py = yTop + row * (int) nScale + (int) dy;
						for (unsigned dx = 0; dx < nScale; dx++)
						{
							int px = x + col * (int) nScale + (int) dx;
							if (0 <= px && px < W && 0 <= py && py < H)
							{
								pMask[py * W + px] = 255;
							}
						}
					}
				}
			}
		}
		return;
	}

	const TDigits &Digits = GetDigits (rFont, nSize);
	float fScale = stbtt_ScaleForMappingEmToPixels (rFont.pInfo, (float) nSize);
	float fX = (float) x;
	int nPrevious = 0;
	char chPrevious = 0;

	for (const char *p = pText; *p != '\0'; p++)
	{
		char c = *p;
		int nGlyph = stbtt_FindGlyphIndex (rFont.pInfo, (unsigned char) c);

		if (c == ':' && (nGlyph == 0 || stbtt_IsGlyphEmpty (rFont.pInfo, nGlyph)))
		{
			TColon Colon;
			GetColon (Digits, fX, &Colon);	// police sans deux-points : deux carres
			for (unsigned k = 0; k < 2 && bColonOn; k++)
			{
				for (int dy = 0; dy < Colon.nDot; dy++)
				{
					int py = yBaseline + Colon.y[k] + dy;
					for (int dx = 0; dx < Colon.nDot; dx++)
					{
						int px = Colon.x + dx;
						if (0 <= px && px < W && 0 <= py && py < H)
						{
							pMask[py * W + px] = 255;
						}
					}
				}
			}
			fX += Colon.fAdvance;
			nPrevious = 0;
			chPrevious = c;
			continue;
		}

		if ((unsigned char) c == DEGREE_CHAR && (nGlyph == 0 || stbtt_IsGlyphEmpty (rFont.pInfo, nGlyph)))
		{
			// petit anneau aux coins arrondis, en haut des chiffres
			int nSize = DegreeSize (Digits);
			int nThick = nSize >= 8 ? 2 : 1;
			int x = (int) (fX + 0.5f) + 1;
			for (int dy = 0; dy < nSize; dy++)
			{
				for (int dx = 0; dx < nSize; dx++)
				{
					boolean bCorner = (dx == 0 || dx == nSize - 1) && (dy == 0 || dy == nSize - 1);
					boolean bBorder =    dx < nThick || dy < nThick
							  || dx >= nSize - nThick || dy >= nSize - nThick;
					int px = x + dx, py = yBaseline + Digits.nTop + dy;
					if (bBorder && !(bCorner && nSize >= 4) && 0 <= px && px < W && 0 <= py && py < H)
					{
						pMask[py * W + px] = 255;
					}
				}
			}
			fX += nSize + 2;
			nPrevious = 0;
			chPrevious = c;
			continue;
		}

		int nAdvance, nBearing;
		stbtt_GetGlyphHMetrics (rFont.pInfo, nGlyph, &nAdvance, &nBearing);

		if (nPrevious != 0 && !IsDigit (c) && !IsDigit (chPrevious))
		{
			fX += fScale * stbtt_GetGlyphKernAdvance (rFont.pInfo, nPrevious, nGlyph);
		}

		float fAdvance = nAdvance * fScale;
		float fOffset = 0.0f;
		if (IsDigit (c))
		{
			fOffset = (Digits.fCell - fAdvance) / 2.0f;
			fAdvance = Digits.fCell;
		}

		if (c != ' ' && !(c == ':' && !bColonOn))
		{
			float fPos = fX + fOffset;
			int nX = (int) fPos;
			if (nX > fPos)
			{
				nX--;			// arrondi vers le bas (positions negatives)
			}
			float fShift = fPos - nX;

			int x0, y0, x1, y1;
			stbtt_GetGlyphBitmapBoxSubpixel (rFont.pInfo, nGlyph, fScale, fScale, fShift, 0.0f,
							 &x0, &y0, &x1, &y1);
			int w = x1 - x0, h = y1 - y0;
			if (w > 0 && h > 0 && w <= (int) GlyphMax && h <= (int) GlyphMax)
			{
				stbtt_MakeGlyphBitmapSubpixel (rFont.pInfo, m_pGlyph, w, h, w, fScale, fScale,
							       fShift, 0.0f, nGlyph);
				for (int gy = 0; gy < h; gy++)
				{
					int py = yBaseline + y0 + gy;
					if (py < 0 || py >= H)
					{
						continue;
					}
					for (int gx = 0; gx < w; gx++)
					{
						int px = nX + x0 + gx;
						if (px < 0 || px >= W)
						{
							continue;
						}
						u8 v = m_pGlyph[gy * w + gx];
						if (!bSmooth)
						{
							v = v >= 128 ? 255 : 0;
						}
						u8 *pDest = &pMask[py * W + px];
						if (v > *pDest)
						{
							*pDest = v;
						}
					}
				}
			}
		}

		fX += fAdvance;
		nPrevious = nGlyph;
		chPrevious = c;
	}
}

// taille choisie (0 = automatique) ; une taille choisie trop large pour l'ecran est reduite
unsigned CClockRenderer::ChooseSize (const TFont &rFont, const char *pText, unsigned nSize,
				     int nMaxWidth, int nMaxHeight, TFitCache *pCache)
{
	if (nSize == 0)
	{
		return FitSize (rFont, pText, nMaxWidth, nMaxHeight, pCache);
	}

	TMetrics M;
	if (!Measure (rFont, nSize, pText, &M) || M.nRight - M.nLeft <= nMaxWidth)
	{
		return nSize;
	}

	unsigned nFit = FitSize (rFont, pText, nMaxWidth, 1000, pCache);	// largeur seulement

	return nFit < nSize ? nFit : nSize;
}

// cote du signe degre dessine : environ un tiers de la hauteur des chiffres
int CClockRenderer::DegreeSize (const TDigits &rDigits)
{
	int nSize = (rDigits.nBottom - rDigits.nTop) * 3 / 10;
	return nSize < 3 ? 3 : nSize;
}

// deux-points dessines (police sans ce caractere) : deux carres a 30 % et 70 % des chiffres
void CClockRenderer::GetColon (const TDigits &rDigits, float fX, TColon *pColon)
{
	int nHeight = rDigits.nBottom - rDigits.nTop;
	int nDot = nHeight / 6;
	if (nDot < 1)
	{
		nDot = 1;
	}

	pColon->nDot = nDot;
	pColon->x = (int) (fX + 0.5f) + nDot;
	pColon->y[0] = rDigits.nTop + nHeight * 3 / 10 - nDot / 2;
	pColon->y[1] = rDigits.nTop + nHeight * 7 / 10 - nDot / 2;
	pColon->fAdvance = (float) (3 * nDot);
}

unsigned CClockRenderer::FitSize (const TFont &rFont, const char *pText, int nMaxWidth,
				  int nMaxHeight, TFitCache *pCache)
{
	// les chiffres ont tous la meme largeur : "12:34:56" et "00:00:00" ont la meme taille
	char Key[sizeof pCache->Text];
	unsigned i;
	for (i = 0; pText[i] != '\0' && i < sizeof Key - 1; i++)
	{
		Key[i] = IsDigit (pText[i]) ? '0' : pText[i];
	}
	Key[i] = '\0';

	if (   pCache->pFont == &rFont && strcmp (pCache->Text, Key) == 0
	    && pCache->nMaxWidth == nMaxWidth && pCache->nMaxHeight == nMaxHeight)
	{
		return pCache->nSize;
	}

	unsigned nSize = MIN_FONT_SIZE;
	for (unsigned s = CLOCK_MAX_SIZE; s >= MIN_FONT_SIZE; s--)
	{
		TMetrics M;
		if (   Measure (rFont, s, Key, &M)
		    && M.nRight - M.nLeft <= nMaxWidth
		    && M.nBottom - M.nTop <= nMaxHeight)
		{
			nSize = s;
			break;
		}

		if (rFont.pInfo == 0 && s > MIN_FONT_SIZE)
		{
			s = (PixelScale (s) - 1) * FONT_HEIGHT + 1;	// police Pixel : taille suivante
		}
	}

	pCache->pFont = &rFont;
	strcpy (pCache->Text, Key);
	pCache->nMaxWidth = nMaxWidth;
	pCache->nMaxHeight = nMaxHeight;
	pCache->nSize = nSize;

	return nSize;
}

// --- image ---

unsigned CClockRenderer::FrameAt (const CGifAnimation *pAnim, unsigned nMillis)
{
	unsigned nTotal = pAnim->GetTotalDuration ();
	unsigned nFrames = pAnim->GetFrameCount ();
	if (nTotal == 0 || nFrames <= 1)
	{
		return 0;
	}

	unsigned t = nMillis % nTotal;
	for (unsigned i = 0; i < nFrames; i++)
	{
		unsigned nDelay = pAnim->GetDelay (i);
		if (t < nDelay)
		{
			return i;
		}
		t -= nDelay;
	}

	return nFrames - 1;
}

void CClockRenderer::Composite (const u8 *pMask, const TTextStyle &rText, const TImage &rPattern,
				unsigned nMillis, u8 *pOut)
{
	const u8 *pFrame = 0;
	if (rPattern.bAnimValid)
	{
		pFrame = rPattern.pAnim->GetFrame (FrameAt (rPattern.pAnim, nMillis));
	}

	unsigned nPixels = m_nWidth * m_nHeight;
	for (unsigned i = 0; i < nPixels; i++)
	{
		unsigned a = pMask[i];
		if (a == 0)
		{
			continue;
		}

		const u8 *pFill = rText.Color;
		if (pFrame != 0)
		{
			pFill = pFrame + i * 3;
		}
		else if (rPattern.pRGBA != 0)
		{
			pFill = rPattern.pRGBA + i * 4;
			a = a * pFill[3] / 255;
		}

		u8 *pDest = pOut + i * 3;
		for (unsigned k = 0; k < 3; k++)
		{
			pDest[k] = (u8) ((pDest[k] * (255 - a) + pFill[k] * a) / 255);
		}
	}
}

// --- ensemble ---

boolean CClockRenderer::IsAnimated (const TClockStyle &rStyle, boolean bDateScreen) const
{
	boolean bTime = !bDateScreen;
	boolean bDate = bDateScreen || rStyle.DateMode == DateBelow;

	return    (m_Background.bAnimValid && m_Background.pAnim->GetFrameCount () > 1)
	       || (bTime && m_Pattern[SlotTime].bAnimValid && m_Pattern[SlotTime].pAnim->GetFrameCount () > 1)
	       || (bDate && m_Pattern[SlotDate].bAnimValid && m_Pattern[SlotDate].pAnim->GetFrameCount () > 1);
}

u32 CClockRenderer::GetFrameKey (const TClockStyle &rStyle, const TDateTime &rTime, unsigned nMillis,
				 boolean bColonOn, boolean bDateScreen) const
{
	u32 nKey = ((rTime.Day * 24 + rTime.Hour) * 60 + rTime.Minute) * 60 + rTime.Second;
	nKey = nKey * 4 + (bColonOn ? 2 : 0) + (bDateScreen ? 1 : 0);

	const TImage *Images[3] = {&m_Background, &m_Pattern[SlotTime], &m_Pattern[SlotDate]};
	for (unsigned i = 0; i < 3; i++)
	{
		if (Images[i]->bAnimValid)
		{
			nKey = nKey * 4099 + FrameAt (Images[i]->pAnim, nMillis);
		}
	}

	return nKey;
}

void CClockRenderer::Render (const TClockStyle &rStyle, const TDateTime &rTime, unsigned nMillis,
			     boolean bColonOn, boolean bDateScreen, CCanvas *pCanvas)
{
	const int W = (int) m_nWidth, H = (int) m_nHeight;
	const int nMaxWidth = W - 2;

	// textes
	char TimeText[64], DateText[64];
	FormatDateTime (rStyle.Time.Format, rTime, TimeText, sizeof TimeText);
	FormatDateTime (rStyle.Date.Format, rTime, DateText, sizeof DateText);

	boolean bShowTime = !bDateScreen;
	boolean bShowDate = bDateScreen || rStyle.DateMode == DateBelow;
	boolean bTwoLines = bShowTime && bShowDate;

	memset (m_pMask[SlotTime], 0, W * H);
	memset (m_pMask[SlotDate], 0, W * H);

	// tailles et place de chaque ligne
	const TFont &TimeFont = m_Font[SlotTime];
	const TFont &DateFont = bShowDate ? m_Font[SlotDate] : m_Font[SlotTime];
	unsigned nTimeSize = 0, nDateSize = 0;
	TMetrics TM = {0, 0, 0, 0}, DM = {0, 0, 0, 0};
	boolean bTimeOK = FALSE, bDateOK = FALSE;

	if (bShowDate)
	{
		int nMaxHeight = bTwoLines ? H * DATE_SHARE_PERCENT / 100 : H;
		if (bTwoLines && rStyle.Time.nSize != 0)
		{
			// heure de taille imposee : la date prend la place qui reste
			TMetrics M;
			if (Measure (TimeFont, rStyle.Time.nSize, TimeText, &M))
			{
				int nLeft = H - (M.nBottom - M.nTop) - LINE_GAP;
				nMaxHeight = nLeft > MIN_FONT_SIZE ? nLeft : MIN_FONT_SIZE;
			}
		}
		nDateSize = ChooseSize (DateFont, DateText, rStyle.Date.nSize, nMaxWidth, nMaxHeight, &m_Fit[SlotDate]);
		bDateOK = Measure (DateFont, nDateSize, DateText, &DM);
	}

	if (bShowTime)
	{
		int nMaxHeight = H;
		if (bTwoLines && bDateOK)
		{
			nMaxHeight = H - (DM.nBottom - DM.nTop) - LINE_GAP;
		}
		nTimeSize = ChooseSize (TimeFont, TimeText, rStyle.Time.nSize, nMaxWidth, nMaxHeight, &m_Fit[SlotTime]);
		bTimeOK = Measure (TimeFont, nTimeSize, TimeText, &TM);
	}

	// placement : centre, le bloc des deux lignes centre verticalement
	int nTimeHeight = bTimeOK ? TM.nBottom - TM.nTop : 0;
	int nDateHeight = bDateOK ? DM.nBottom - DM.nTop : 0;
	int nTotal = nTimeHeight + nDateHeight + (bTimeOK && bDateOK ? LINE_GAP : 0);
	int yTop = (H - nTotal) / 2;

	if (bTimeOK)
	{
		int x = (W - (TM.nRight - TM.nLeft)) / 2 - TM.nLeft;
		Draw (TimeFont, nTimeSize, TimeText, x, yTop - TM.nTop, bColonOn, rStyle.bSmooth,
		      m_pMask[SlotTime]);
		yTop += nTimeHeight + LINE_GAP;
	}

	if (bDateOK)
	{
		int x = (W - (DM.nRight - DM.nLeft)) / 2 - DM.nLeft;
		Draw (DateFont, nDateSize, DateText, x, yTop - DM.nTop, TRUE, rStyle.bSmooth,
		      m_pMask[SlotDate]);
	}

	// fond
	u8 *pOut = pCanvas->GetPixels ();
	if (m_Background.bAnimValid)
	{
		memcpy (pOut, m_Background.pAnim->GetFrame (FrameAt (m_Background.pAnim, nMillis)), W * H * 3);
	}
	else if (m_Background.pRGBA != 0)
	{
		for (int i = 0; i < W * H; i++)
		{
			const u8 *pSource = m_Background.pRGBA + i * 4;
			unsigned a = pSource[3];
			pOut[i * 3 + 0] = (u8) (pSource[0] * a / 255);
			pOut[i * 3 + 1] = (u8) (pSource[1] * a / 255);
			pOut[i * 3 + 2] = (u8) (pSource[2] * a / 255);
		}
	}
	else
	{
		pCanvas->Clear ();
	}

	// textes, remplis de leur couleur ou motif
	if (bTimeOK)
	{
		Composite (m_pMask[SlotTime], rStyle.Time, m_Pattern[SlotTime], nMillis, pOut);
	}
	if (bDateOK)
	{
		Composite (m_pMask[SlotDate], rStyle.Date, bShowDate ? m_Pattern[SlotDate] : m_Pattern[SlotTime],
			   nMillis, pOut);
	}
}

// --- meteo ---

void CClockRenderer::PrepareWeather (const TWeatherStyle &rStyle)
{
	LoadFont (&m_Font[SlotWeather], rStyle.Text.Font);
	LoadImage (&m_Pattern[SlotWeather], rStyle.Text.Pattern);
}

// "7°", "-3°" (arrondi au degre)
static void FormatDegrees (int nTenths, char *pOut)
{
	int nValue = nTenths >= 0 ? (nTenths + 5) / 10 : -((-nTenths + 5) / 10);
	unsigned n = 0;
	if (nValue < 0)
	{
		pOut[n++] = '-';
		nValue = -nValue;
	}
	if (nValue >= 100)	pOut[n++] = '0' + nValue / 100 % 10;
	if (nValue >= 10)	pOut[n++] = '0' + nValue / 10 % 10;
	pOut[n++] = '0' + nValue % 10;
	pOut[n++] = (char) DEGREE_CHAR;
	pOut[n] = '\0';
}

void CClockRenderer::RenderWeather (const TWeatherStyle &rStyle, const TWeatherData &rData,
				    unsigned nMillis, CCanvas *pCanvas)
{
	const int W = (int) m_nWidth, H = (int) m_nHeight;

	// textes
	char Temp[16], Info[40], A[16], B[16];
	FormatDegrees (rData.nTemp10, Temp);
	Info[0] = '\0';
	switch (rStyle.Info)
	{
	case InfoDescription:
		strcpy (Info, WeatherDescription (rData.nCode, rData.bDay));
		break;

	case InfoMinMax:
		FormatDegrees (rData.nMin10, A);
		FormatDegrees (rData.nMax10, B);
		strcpy (Info, A); strcat (Info, " / "); strcat (Info, B);
		break;

	case InfoFeels:
		FormatDegrees (rData.nFeels10, A);
		strcpy (Info, "RESSENTI "); strcat (Info, A);
		break;

	case InfoHumidity: {
		CString Text;
		Text.Format ("HUMIDITE %d%%", rData.nHumidity);
		strcpy (Info, Text);
		break; }

	case InfoWind: {
		CString Text;
		Text.Format ("VENT %d KM/H", (rData.nWind10 + 5) / 10);
		strcpy (Info, Text);
		break; }

	case InfoTomorrow:
		if (rData.nTomorrowCode >= 0)
		{
			FormatDegrees (rData.nTomorrowMin10, A);
			FormatDegrees (rData.nTomorrowMax10, B);
			strcpy (Info, "DEMAIN "); strcat (Info, A); strcat (Info, "/"); strcat (Info, B);
		}
		break;

	default:
		break;
	}

	// place : icone a gauche, textes centres dans le reste
	int xText = rStyle.bIcon ? 33 : 0;
	int nAreaWidth = W - xText;
	const TFont &Font = m_Font[SlotWeather];

	memset (m_pMask[SlotWeather], 0, W * H);
	memset (m_pMask[SlotDate], 0, W * H);

	TMetrics IM = {0, 0, 0, 0}, TM = {0, 0, 0, 0};
	unsigned nInfoSize = 0;
	boolean bInfo = FALSE;
	if (Info[0] != '\0')
	{
		nInfoSize = FitSize (m_Pixel, Info, nAreaWidth - 2, H * 30 / 100, &m_FitInfo);
		bInfo = Measure (m_Pixel, nInfoSize, Info, &IM);
	}

	int nInfoHeight = bInfo ? IM.nBottom - IM.nTop : 0;
	int nMaxHeight = H - (bInfo ? nInfoHeight + 3 : 0);
	unsigned nTempSize = ChooseSize (Font, Temp, rStyle.Text.nSize, nAreaWidth - 2, nMaxHeight,
					 &m_Fit[SlotWeather]);
	boolean bTemp = Measure (Font, nTempSize, Temp, &TM);

	int nTempHeight = bTemp ? TM.nBottom - TM.nTop : 0;
	int nTotal = nTempHeight + nInfoHeight + (bTemp && bInfo ? 3 : 0);
	int yTop = (H - nTotal) / 2;

	if (bTemp)
	{
		int x = xText + (nAreaWidth - (TM.nRight - TM.nLeft)) / 2 - TM.nLeft;
		Draw (Font, nTempSize, Temp, x, yTop - TM.nTop, TRUE, TRUE, m_pMask[SlotWeather]);
		yTop += nTempHeight + 3;
	}
	if (bInfo)
	{
		int x = xText + (nAreaWidth - (IM.nRight - IM.nLeft)) / 2 - IM.nLeft;
		Draw (m_Pixel, nInfoSize, Info, x, yTop - IM.nTop, TRUE, TRUE, m_pMask[SlotDate]);
	}

	// fond noir, icone, puis textes
	pCanvas->Clear ();
	if (rStyle.bIcon)
	{
		DrawWeatherIcon (pCanvas, 0, (H - 32) / 2, rData.nCode, rData.bDay,
				 rStyle.bAnimate ? nMillis : 0);
	}

	u8 *pOut = pCanvas->GetPixels ();
	if (bTemp)
	{
		Composite (m_pMask[SlotWeather], rStyle.Text, m_Pattern[SlotWeather], nMillis, pOut);
	}
	if (bInfo)
	{
		TTextStyle InfoText = rStyle.Text;
		InfoText.Color[0] = rStyle.InfoColor[0];
		InfoText.Color[1] = rStyle.InfoColor[1];
		InfoText.Color[2] = rStyle.InfoColor[2];
		Composite (m_pMask[SlotDate], InfoText, m_NoImage, nMillis, pOut);
	}
}
