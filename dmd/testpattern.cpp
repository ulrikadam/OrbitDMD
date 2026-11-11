//
// testpattern.cpp
//
#include "testpattern.h"

static const TTestScreen s_Screens[] =
{
	{"accueil",	3000},
	{"rouge",	2000},
	{"vert",	2000},
	{"bleu",	2000},
	{"blanc",	2000},
	{"degrades",	4000},
	{"geometrie",	6000}
};

unsigned TestPatternGetCount (void)
{
	return sizeof s_Screens / sizeof s_Screens[0];
}

const TTestScreen *TestPatternGetInfo (unsigned nScreen)
{
	return &s_Screens[nScreen];
}

void TestPatternDraw (CCanvas *pCanvas, unsigned nScreen,
		      unsigned nPanelCols, unsigned nPanelRows,
		      unsigned nChain, unsigned nParallel)
{
	CCanvas &C = *pCanvas;
	const int W = (int) C.GetWidth ();
	const int H = (int) C.GetHeight ();

	C.Clear ();

	switch (nScreen)
	{
	case 0:		// accueil
		C.DrawTextCentered (0, W, 2, "ORBITDMD", 255, 255, 255, 2);
		C.DrawTextCentered (0, W, H - 9, "SANS LINUX", 0, 170, 255);
		break;

	case 1:		// couleurs pleines : ordre des couleurs et alimentation
	case 2:
	case 3:
	case 4: {
		static const struct
		{
			const char *pName;
			u8 r, g, b;
		}
		Colors[] =
		{
			{"ROUGE", 255,   0,   0},
			{"VERT",    0, 255,   0},
			{"BLEU",    0,   0, 255},
			{"BLANC", 255, 255, 255}
		};

		unsigned i = nScreen - 1;
		C.Clear (Colors[i].r, Colors[i].g, Colors[i].b);
		C.DrawTextCentered (0, W, (H - 7) / 2, Colors[i].pName, 0, 0, 0);
		} break;

	case 5:		// degrades rouge, vert, bleu, gris : finesse des nuances
		for (int y = 0; y < H; y++)
		{
			int nBand = y * 4 / H;
			for (int x = 0; x < W; x++)
			{
				u8 v = (u8) (x * 255 / (W - 1));
				switch (nBand)
				{
				case 0:	 C.SetPixel (x, y, v, 0, 0);	break;
				case 1:	 C.SetPixel (x, y, 0, v, 0);	break;
				case 2:	 C.SetPixel (x, y, 0, 0, v);	break;
				default: C.SetPixel (x, y, v, v, v);	break;
				}
			}
		}
		break;

	case 6:		// geometrie : cadre, numero de chaque panneau, coins de couleur
		C.DrawRect (0, 0, W, H, 255, 255, 255);
		for (unsigned p = 0; p < nParallel; p++)
		{
			for (unsigned c = 0; c < nChain; c++)
			{
				char Number[2];
				Number[0] = (char) ('0' + (p * nChain + c + 1) % 10);
				Number[1] = '\0';

				int nScale = nPanelRows >= 32 ? 3 : 2;
				C.DrawTextCentered ((int) (c * nPanelCols), (int) nPanelCols,
						    (int) (p * nPanelRows) + ((int) nPanelRows - 7 * nScale) / 2,
						    Number, 0, 255, 255, nScale);
			}
		}
		C.FillRect (1, 1, 4, 4, 255, 0, 0);		// haut gauche : rouge
		C.FillRect (W - 5, 1, 4, 4, 0, 255, 0);		// haut droite : vert
		C.FillRect (1, H - 5, 4, 4, 0, 0, 255);		// bas gauche : bleu
		C.FillRect (W - 5, H - 5, 4, 4, 255, 255, 0);	// bas droite : jaune
		break;

	default:
		break;
	}
}
