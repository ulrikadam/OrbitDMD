//
// canvas.cpp
//
#include "canvas.h"
#include "font5x7.h"
#include <circle/util.h>

CCanvas::CCanvas (unsigned nWidth, unsigned nHeight)
:	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_pData (new u8[nWidth * nHeight * 3])
{
	Clear ();
}

CCanvas::~CCanvas (void)
{
	delete [] m_pData;
}

void CCanvas::Clear (u8 r, u8 g, u8 b)
{
	if (r == 0 && g == 0 && b == 0)
	{
		memset (m_pData, 0, m_nWidth * m_nHeight * 3);
		return;
	}

	u8 *p = m_pData;
	for (unsigned i = 0; i < m_nWidth * m_nHeight; i++)
	{
		*p++ = r;
		*p++ = g;
		*p++ = b;
	}
}

void CCanvas::SetPixel (int x, int y, u8 r, u8 g, u8 b)
{
	if (x < 0 || y < 0 || x >= (int) m_nWidth || y >= (int) m_nHeight)
	{
		return;
	}

	u8 *p = m_pData + (y * m_nWidth + x) * 3;
	p[0] = r;
	p[1] = g;
	p[2] = b;
}

void CCanvas::FillRect (int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	for (int j = y; j < y + h; j++)
	{
		for (int i = x; i < x + w; i++)
		{
			SetPixel (i, j, r, g, b);
		}
	}
}

void CCanvas::DrawRect (int x, int y, int w, int h, u8 r, u8 g, u8 b)
{
	FillRect (x, y, w, 1, r, g, b);
	FillRect (x, y + h - 1, w, 1, r, g, b);
	FillRect (x, y, 1, h, r, g, b);
	FillRect (x + w - 1, y, 1, h, r, g, b);
}

int CCanvas::DrawText (int x, int y, const char *pText, u8 r, u8 g, u8 b, unsigned nScale)
{
	int x0 = x;

	while (*pText != '\0')
	{
		const char * const *pRows = FontGetGlyph (Utf8Next (&pText));	// texte UTF-8
		if (pRows == 0)
		{
			pRows = FontGetGlyph ('?');
		}

		for (int nRow = 0; nRow < FONT_HEIGHT; nRow++)
		{
			for (int nCol = 0; nCol < FONT_WIDTH; nCol++)
			{
				if (pRows[nRow][nCol] == '#')
				{
					FillRect (x + nCol * nScale, y + nRow * nScale,
						  nScale, nScale, r, g, b);
				}
			}
		}

		x += FONT_ADVANCE * nScale;
	}

	return x - x0;
}

void CCanvas::DrawTextCentered (int x0, int nAreaWidth, int y, const char *pText,
				u8 r, u8 g, u8 b, unsigned nScale)
{
	int nWidth = GetTextWidth (pText, nScale);
	DrawText (x0 + (nAreaWidth - nWidth) / 2, y, pText, r, g, b, nScale);
}

int CCanvas::GetTextWidth (const char *pText, unsigned nScale)
{
	int nLength = (int) Utf8Length (pText);
	if (nLength == 0)
	{
		return 0;
	}

	// pas d'espace apres le dernier caractere
	return (nLength * FONT_ADVANCE - (FONT_ADVANCE - FONT_WIDTH)) * nScale;
}
