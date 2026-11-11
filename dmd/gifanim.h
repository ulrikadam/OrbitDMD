//
// gifanim.h - decodeur de GIF animes
//
// Decode toutes les images d'un GIF (LZW, palettes, transparence, modes de
// disposition, entrelacement) en images RGB888 deja composees, centrees sur
// une zone de la taille des panneaux.
//
#ifndef _gifanim_h
#define _gifanim_h

#include <circle/types.h>

class CGifAnimation
{
public:
	static const unsigned MaxFrames = 4096;
	static const unsigned MaxScreenPixels = 1024 * 1024;

public:
	CGifAnimation (unsigned nWidth, unsigned nHeight);
	~CGifAnimation (void);

	boolean Decode (const u8 *pData, unsigned nSize);
	void Clear (void);

	// fonction appelee regulierement pendant le decodage (pour laisser tourner le reseau)
	static void SetYieldHandler (void (*pHandler) (void))	{ s_pYieldHandler = pHandler; }

	unsigned GetFrameCount (void) const		{ return m_nFrames; }
	const u8 *GetFrame (unsigned nIndex) const	{ return m_ppFrame[nIndex]; }
	unsigned GetDelay (unsigned nIndex) const	{ return m_pDelay[nIndex]; }	// ms
	unsigned GetTotalDuration (void) const;						// ms

private:
	boolean AddFrame (const u8 *pScreen, unsigned nScreenWidth, unsigned nScreenHeight,
			  unsigned nDelay);
	unsigned DecodeLZW (const u8 *pIn, unsigned nInLength, unsigned nMinCodeSize,
			    u8 *pOut, unsigned nOutLength);

private:
	unsigned m_nWidth;
	unsigned m_nHeight;

	u8 **m_ppFrame;
	unsigned *m_pDelay;
	unsigned m_nFrames;

	// tampons alloues une seule fois (les gros blocs liberes ne sont pas reutilises par Circle)
	u8 *m_pScreen;		// image logique du GIF en cours de composition
	u8 *m_pPrevious;	// sauvegarde pour la disposition "restaurer l'image precedente"
	u8 *m_pIndex;		// indices de couleur d'une image
	u8 *m_pLZWData;		// donnees LZW rassemblees
	unsigned m_nLZWDataSize;

	u16 *m_pPrefix;
	u8 *m_pSuffix;
	u8 *m_pStack;

	static void (*s_pYieldHandler) (void);
};

#endif
