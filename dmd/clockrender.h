//
// clockrender.h - dessin de l'heure et de la date
//
// Polices : "Pixel" (5x7 integree) ou fichiers TTF/OTF de SD:/fonts (stb_truetype).
// Remplissage du texte : couleur unie, ou motif PNG/GIF (anime) de SD:/fonts.
// Fond : noir, ou image PNG/GIF (animee) de SD:/fonts.
// Les chiffres ont tous la meme largeur : l'heure ne bouge pas quand les secondes changent.
//
// Un objet par tache qui dessine (boucle d'affichage, apercu de la page web) : les
// chargements depuis la carte rendent la main au reseau en cours de route.
//
#ifndef _clockrender_h
#define _clockrender_h

#include <circle/types.h>
#include "clockstyle.h"
#include "weatherdata.h"
#include "canvas.h"

#define DEGREE_CHAR	0xB0		// signe degre (Latin-1) dans les textes a dessiner

struct stbtt_fontinfo;
class CGifAnimation;

class CClockRenderer
{
public:
	CClockRenderer (unsigned nWidth, unsigned nHeight);
	~CClockRenderer (void);

	// charge les polices et images du style si elles ont change (lecture de la carte SD)
	void Prepare (const TClockStyle &rStyle);

	// cle qui change quand l'image a montrer change (seconde, clignotement, animations)
	u32 GetFrameKey (const TClockStyle &rStyle, const TDateTime &rTime, unsigned nMillis,
			 boolean bColonOn, boolean bDateScreen) const;
	boolean IsAnimated (const TClockStyle &rStyle, boolean bDateScreen) const;

	// dessine l'heure (et la date dessous si demande), ou la date seule (bDateScreen) ;
	// nMillis : temps ecoule, pour les motifs et fonds animes
	void Render (const TClockStyle &rStyle, const TDateTime &rTime, unsigned nMillis,
		     boolean bColonOn, boolean bDateScreen, CCanvas *pCanvas);

	// ecran meteo : icone, temperature, une ligne d'information (Pixel)
	void PrepareWeather (const TWeatherStyle &rStyle);
	void RenderWeather (const TWeatherStyle &rStyle, const TWeatherData &rData, unsigned nMillis,
			    CCanvas *pCanvas);

private:
	enum { SlotTime, SlotDate, SlotWeather, SlotCount };

	struct TFont
	{
		char		Name[CLOCK_NAME_LEN];
		u8		*pData;		// fichier TTF (garde en memoire)
		stbtt_fontinfo	*pInfo;		// 0 : police Pixel
	};

	struct TImage
	{
		char		Name[CLOCK_NAME_LEN];
		u8		*pRGBA;		// image fixe (PNG), m_nWidth x m_nHeight
		CGifAnimation	*pAnim;		// GIF (cree au premier besoin, garde ensuite)
		boolean		bAnimValid;
	};

	struct TMetrics			// encre du texte, par rapport a l'origine (ligne de base)
	{
		int nLeft, nRight, nTop, nBottom;
	};

	struct TDigits			// chiffres d'une police a une taille : largeur commune
	{
		const TFont *pFont;
		unsigned nSize;
		float fCell;		// largeur d'un chiffre
		int nTop, nBottom;	// hauteur commune de tous les chiffres
	};

	struct TColon			// deux-points dessines (police sans ce caractere)
	{
		int x;
		int y[2];		// haut de chaque point, par rapport a la ligne de base
		int nDot;		// cote d'un point
		float fAdvance;
	};

	struct TFitCache		// derniere taille automatique calculee
	{
		const TFont *pFont;
		char Text[64];
		int nMaxWidth, nMaxHeight;
		unsigned nSize;
	};

	void LoadFont (TFont *pFont, const char *pName);
	void LoadImage (TImage *pImage, const char *pName);
	u8 *ReadAsset (const char *pName, unsigned *pSize);

	const TDigits &GetDigits (const TFont &rFont, unsigned nSize);
	boolean Measure (const TFont &rFont, unsigned nSize, const char *pText, TMetrics *pMetrics);
	void Draw (const TFont &rFont, unsigned nSize, const char *pText, int x, int yBaseline,
		   boolean bColonOn, boolean bSmooth, u8 *pMask);
	unsigned FitSize (const TFont &rFont, const char *pText, int nMaxWidth, int nMaxHeight,
			  TFitCache *pCache);
	unsigned ChooseSize (const TFont &rFont, const char *pText, unsigned nSize,
			     int nMaxWidth, int nMaxHeight, TFitCache *pCache);
	static void GetColon (const TDigits &rDigits, float fX, TColon *pColon);
	static int DegreeSize (const TDigits &rDigits);

	void Composite (const u8 *pMask, const TTextStyle &rText, const TImage &rPattern,
			unsigned nMillis, u8 *pOut);

	static unsigned FrameAt (const CGifAnimation *pAnim, unsigned nMillis);
	static unsigned PixelScale (unsigned nSize);

private:
	unsigned m_nWidth;
	unsigned m_nHeight;

	TFont m_Font[SlotCount];
	TImage m_Pattern[SlotCount];
	TImage m_Background;

	TDigits m_Digits;
	TFitCache m_Fit[SlotCount];
	TFitCache m_FitInfo;		// deuxieme ligne de la meteo
	TFont m_Pixel;			// police Pixel (ligne d'information de la meteo)
	TImage m_NoImage;		// pas de motif

	u8 *m_pMask[SlotCount];		// texte (0..255) a la taille de l'ecran
	u8 *m_pGlyph;			// un caractere TTF

	static const unsigned GlyphMax = 160;
};

#endif
