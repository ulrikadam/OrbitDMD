//
// canvas.h - image RGB888 en memoire, avec dessin simple
//
#ifndef _canvas_h
#define _canvas_h

#include <circle/types.h>

class CCanvas
{
public:
	CCanvas (unsigned nWidth, unsigned nHeight);
	~CCanvas (void);

	unsigned GetWidth (void) const		{ return m_nWidth; }
	unsigned GetHeight (void) const		{ return m_nHeight; }
	const u8 *GetData (void) const		{ return m_pData; }
	u8 *GetPixels (void)			{ return m_pData; }	// RGB888, ligne par ligne

	void Clear (u8 r = 0, u8 g = 0, u8 b = 0);
	void SetPixel (int x, int y, u8 r, u8 g, u8 b);
	void FillRect (int x, int y, int w, int h, u8 r, u8 g, u8 b);
	void DrawRect (int x, int y, int w, int h, u8 r, u8 g, u8 b);

	// texte en police 5x7, agrandi nScale fois ; renvoie la largeur dessinee
	int DrawText (int x, int y, const char *pText, u8 r, u8 g, u8 b, unsigned nScale = 1);
	// texte centre horizontalement dans [x0, x0 + nAreaWidth[
	void DrawTextCentered (int x0, int nAreaWidth, int y, const char *pText,
			       u8 r, u8 g, u8 b, unsigned nScale = 1);

	static int GetTextWidth (const char *pText, unsigned nScale = 1);

private:
	unsigned m_nWidth;
	unsigned m_nHeight;
	u8 *m_pData;
};

#endif
