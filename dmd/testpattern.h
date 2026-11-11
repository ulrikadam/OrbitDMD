//
// testpattern.h - ecrans de la mire de test (partages avec le banc de test PC)
//
#ifndef _testpattern_h
#define _testpattern_h

#include <circle/types.h>
#include "canvas.h"

struct TTestScreen
{
	const char *pName;
	unsigned    nDuration;	// ms
};

// ecrans : accueil, rouge, vert, bleu, blanc, degrades, geometrie
unsigned TestPatternGetCount (void);
const TTestScreen *TestPatternGetInfo (unsigned nScreen);
void TestPatternDraw (CCanvas *pCanvas, unsigned nScreen,
		      unsigned nPanelCols, unsigned nPanelRows,
		      unsigned nChain, unsigned nParallel);

#endif
