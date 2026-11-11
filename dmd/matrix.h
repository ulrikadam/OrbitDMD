//
// matrix.h - pilote des panneaux LED HUB75 (sans Linux)
//
// Reprend le principe de la bibliotheque hzeller/rpi-rgb-led-matrix :
// modulation par plans de bits (Binary Code Modulation), correction de
// luminance CIE1931 et brochage "regular" (carte 3 sorties TOP-P0 / Middle-P1 /
// BOTTOM-P3). Le rafraichissement tourne en boucle sur un coeur dedie.
//
#ifndef _matrix_h
#define _matrix_h

#include <circle/types.h>

struct TMatrixOptions
{
	unsigned nRows;		// lignes par panneau (16, 32 ou 64)
	unsigned nCols;		// colonnes par panneau
	unsigned nChain;	// panneaux a la suite sur une sortie
	unsigned nParallel;	// sorties utilisees en parallele (1 a 3)
	unsigned nSlowdown;	// repetition des ecritures GPIO (0 a 4)
	unsigned nPWMBits;	// 1 a 11
	unsigned nPWMLSBNanos;	// duree du plan de bits le plus faible
	char	 RGBOrder[4];	// "RGB", "RBG", "GRB", ...
};

class CMatrix
{
public:
	static const unsigned BitPlanes = 11;
	static const unsigned MaxDoubleRows = 32;

public:
	CMatrix (void);
	~CMatrix (void);

	boolean Initialize (const TMatrixOptions &rOptions);

	unsigned GetWidth (void) const		{ return m_nWidth; }
	unsigned GetHeight (void) const		{ return m_nHeight; }

	// luminosite en % (0 = eteint), prise en compte au prochain Show()
	void SetBrightness (unsigned nPercent);
	unsigned GetBrightness (void) const	{ return m_nBrightness; }

	// affiche une image RGB888 de GetWidth() x GetHeight() pixels
	// (attend au plus un rafraichissement si l'image precedente n'a pas encore ete prise)
	void Show (const u8 *pRGB);

	// boucle de rafraichissement, ne revient jamais (coeur dedie)
	void RefreshLoop (void);

	// nombre d'images completes envoyees aux panneaux depuis le demarrage
	unsigned GetRefreshCount (void) const	{ return m_nRefreshCount; }

	// copie de la derniere image affichee (apercu du tableau de bord), 0 avant Initialize()
	const u8 *GetLastFrame (void) const	{ return m_pLastFrame; }

	static CMatrix *Get (void)		{ return s_pThis; }

private:
	inline void WriteSet (u32 nBits);
	inline void WriteClear (u32 nBits);

private:
	TMatrixOptions m_Options;
	unsigned m_nWidth;
	unsigned m_nHeight;
	unsigned m_nDoubleRows;
	unsigned m_nSlowdown;

	u32 m_nColorMask;
	u32 m_nClock;
	u32 m_nStrobe;
	u32 m_nOutputEnable;
	u32 m_nRowMask;
	u32 m_RowBits[MaxDoubleRows];
	unsigned m_ColorMap[3];		// canal physique (R, G, B) -> composante de l'image

	u64 m_PulseTicks[BitPlanes];	// duree d'allumage par plan de bits (compteur ARM)

	unsigned m_nBrightness;
	u16 m_Lookup[256];		// 8 bits -> niveau sur 11 bits (luminosite + CIE1931)

	unsigned m_nBufferWords;
	u32 *m_pBuffer[2];
	volatile unsigned m_nFront;		// tampon affiche
	volatile boolean m_bSwapPending;	// le tampon arriere est pret

	volatile unsigned m_nRefreshCount;

	u8 *m_pLastFrame;

	static CMatrix *s_pThis;
};

#endif
