//
// weathericons.cpp
//
// Formes simples, bords adoucis (4 x 4 echantillons par pixel), sans fonction mathematique.
//
#include "weathericons.h"
#include "weatherdata.h"

#define ICON	32

struct TColor { u8 r, g, b; };

static const TColor Sun		= {255, 196, 0};
static const TColor Moon	= {235, 228, 170};
static const TColor Cloud	= {205, 212, 225};
static const TColor DarkCloud	= {115, 118, 135};
static const TColor Rain	= {70, 150, 255};
static const TColor Snow	= {240, 248, 255};
static const TColor Flash	= {255, 230, 60};
static const TColor Fog		= {150, 155, 165};

// 16 directions (cos, sin) pour les rayons du soleil
static const float s_Dir[16][2] =
{
	{1.000f, 0.000f}, {0.924f, 0.383f}, {0.707f, 0.707f}, {0.383f, 0.924f},
	{0.000f, 1.000f}, {-0.383f, 0.924f}, {-0.707f, 0.707f}, {-0.924f, 0.383f},
	{-1.000f, 0.000f}, {-0.924f, -0.383f}, {-0.707f, -0.707f}, {-0.383f, -0.924f},
	{0.000f, -1.000f}, {0.383f, -0.924f}, {0.707f, -0.707f}, {0.924f, -0.383f}
};

class CIconPainter
{
public:
	CIconPainter (CCanvas *pCanvas, int x0, int y0)
	:	m_pPixels (pCanvas->GetPixels ()), m_nWidth ((int) pCanvas->GetWidth ()),
		m_nHeight ((int) pCanvas->GetHeight ()), m_x0 (x0), m_y0 (y0)
	{
	}

	// melange d'une couleur dans un pixel de l'icone (a : 0..255)
	void Blend (int x, int y, TColor c, unsigned a)
	{
		if (x < 0 || y < 0 || x >= ICON || y >= ICON || a == 0)
		{
			return;
		}
		int px = m_x0 + x, py = m_y0 + y;
		if (px < 0 || py < 0 || px >= m_nWidth || py >= m_nHeight)
		{
			return;
		}
		u8 *p = m_pPixels + (py * m_nWidth + px) * 3;
		p[0] = (u8) ((p[0] * (255 - a) + c.r * a) / 255);
		p[1] = (u8) ((p[1] * (255 - a) + c.g * a) / 255);
		p[2] = (u8) ((p[2] * (255 - a) + c.b * a) / 255);
	}

	void Circle (float cx, float cy, float r, TColor c, unsigned nAlpha = 255)
	{
		for (int y = (int) (cy - r) - 1; y <= (int) (cy + r) + 1; y++)
		{
			for (int x = (int) (cx - r) - 1; x <= (int) (cx + r) + 1; x++)
			{
				unsigned n = 0;
				for (int sy = 0; sy < 4; sy++)
				{
					for (int sx = 0; sx < 4; sx++)
					{
						float dx = x + (sx + 0.5f) / 4.0f - cx;
						float dy = y + (sy + 0.5f) / 4.0f - cy;
						if (dx * dx + dy * dy <= r * r)
						{
							n++;
						}
					}
				}
				Blend (x, y, c, n * nAlpha / 16);
			}
		}
	}

	void Rect (int x, int y, int w, int h, TColor c, unsigned nAlpha = 255)
	{
		for (int j = y; j < y + h; j++)
		{
			for (int i = x; i < x + w; i++)
			{
				Blend (i, j, c, nAlpha);
			}
		}
	}

	// trait d'un pixel d'epaisseur, de (x1, y1) a (x2, y2)
	void Line (float x1, float y1, float x2, float y2, TColor c, unsigned nAlpha = 255)
	{
		float dx = x2 - x1, dy = y2 - y1;
		float fLength = (dx < 0 ? -dx : dx) > (dy < 0 ? -dy : dy) ? (dx < 0 ? -dx : dx) : (dy < 0 ? -dy : dy);
		int nSteps = (int) (fLength * 2) + 1;
		int xLast = -1000, yLast = -1000;
		for (int i = 0; i <= nSteps; i++)
		{
			int x = (int) (x1 + dx * i / nSteps + 0.5f);
			int y = (int) (y1 + dy * i / nSteps + 0.5f);
			if (x != xLast || y != yLast)
			{
				Blend (x, y, c, nAlpha);
				xLast = x;
				yLast = y;
			}
		}
	}

	// nuage centre en (cx, cy) (bas du nuage a cy + 5)
	void CloudShape (float cx, float cy, TColor c, float s = 1.0f)
	{
		Circle (cx - 6 * s, cy + 1 * s, 4.5f * s, c);
		Circle (cx + 0 * s, cy - 2 * s, 6.5f * s, c);
		Circle (cx + 6.5f * s, cy + 1 * s, 4.5f * s, c);
		Rect ((int) (cx - 6 * s), (int) (cy + 1 * s), (int) (12.5f * s), (int) (4.6f * s), c);
	}

	void SunShape (float cx, float cy, float r, unsigned nMillis)
	{
		unsigned nStep = (nMillis / 250) % 2;		// les rayons tournent doucement
		for (unsigned i = nStep; i < 16; i += 2)
		{
			float fx = s_Dir[i][0], fy = s_Dir[i][1];
			Line (cx + fx * (r + 2.0f), cy + fy * (r + 2.0f),
			      cx + fx * (r + 4.5f), cy + fy * (r + 4.5f), Sun);
		}
		Circle (cx, cy, r, Sun);
	}

	void MoonShape (float cx, float cy, float r, unsigned nMillis)
	{
		Circle (cx, cy, r, Moon);
		Circle (cx + r * 0.55f, cy - r * 0.45f, r * 0.85f, TColor {0, 0, 0});	// croissant

		// etoiles qui scintillent
		static const int Stars[3][2] = {{25, 6}, {28, 14}, {22, 22}};
		for (unsigned i = 0; i < 3; i++)
		{
			unsigned a = ((nMillis / 200 + i * 3) % 6) < 4 ? 255 : 90;
			Blend (Stars[i][0], Stars[i][1], Snow, a);
		}
	}

private:
	u8 *m_pPixels;
	int m_nWidth, m_nHeight;
	int m_x0, m_y0;
};

void DrawWeatherIcon (CCanvas *pCanvas, int x0, int y0, int nCode, boolean bDay, unsigned nMillis)
{
	CIconPainter P (pCanvas, x0, y0);

	switch (WeatherIconFor (nCode))
	{
	case IconClear:
		if (bDay)
		{
			P.SunShape (16, 16, 7, nMillis);
		}
		else
		{
			P.MoonShape (14, 16, 9, nMillis);
		}
		break;

	case IconPartly:
		if (bDay)
		{
			P.SunShape (11, 11, 5, nMillis);
		}
		else
		{
			P.MoonShape (10, 11, 7, nMillis);
		}
		P.CloudShape (19, 20, Cloud, 0.9f);
		break;

	case IconCloudy:
		P.CloudShape (13, 13, DarkCloud, 0.75f);
		P.CloudShape (17, 19, Cloud);
		break;

	case IconFog: {
		P.CloudShape (16, 10, Cloud, 0.8f);
		int nShift = (int) ((nMillis / 400) % 4);
		for (int i = 0; i < 3; i++)
		{
			int dx = ((i % 2) ? nShift : -nShift) / 2;
			P.Line (4 + dx + (i % 2) * 3, 19 + i * 4, 27 + dx - (i % 2) * 3, 19 + i * 4, Fog);
		}
		break; }

	case IconDrizzle:
	case IconRain:
	case IconStorm: {
		boolean bStorm = WeatherIconFor (nCode) == IconStorm;
		boolean bDrizzle = WeatherIconFor (nCode) == IconDrizzle;
		P.CloudShape (16, 9, bStorm ? DarkCloud : Cloud);

		unsigned nDrops = bDrizzle ? 3 : 5;
		for (unsigned i = 0; i < nDrops; i++)
		{
			int x = 7 + (int) i * (bDrizzle ? 8 : 5);
			int y = 17 + (int) ((nMillis / 70 + i * 5) % 13);
			if (bDrizzle)
			{
				P.Blend (x, y, Rain, 255);
				P.Blend (x, y + 1, Rain, 140);
			}
			else
			{
				P.Line ((float) x + 1, (float) y, (float) x, (float) y + 3, Rain);
			}
		}

		if (bStorm && (nMillis / 120) % 10 < 7)	// eclair qui clignote
		{
			P.Line (18, 14, 14, 21, Flash);
			P.Line (14, 21, 19, 21, Flash);
			P.Line (19, 21, 15, 29, Flash);
			P.Line (19, 14, 15, 21, Flash);
		}
		break; }

	case IconSnow:
		P.CloudShape (16, 9, Cloud);
		for (unsigned i = 0; i < 5; i++)
		{
			int x = 7 + (int) i * 5 + (int) (((nMillis / 300 + i) % 3)) - 1;
			int y = 17 + (int) ((nMillis / 160 + i * 4) % 14);
			P.Blend (x, y, Snow, 255);
			P.Blend (x - 1, y, Snow, 110);
			P.Blend (x + 1, y, Snow, 110);
			P.Blend (x, y - 1, Snow, 110);
			P.Blend (x, y + 1, Snow, 110);
		}
		break;
	}
}
