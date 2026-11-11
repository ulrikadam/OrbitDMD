//
// weathericons.h - icones meteo 32x32 dessinees (soleil, lune, nuages, pluie, neige...)
//
#ifndef _weathericons_h
#define _weathericons_h

#include <circle/types.h>
#include "canvas.h"

// dessine l'icone du code meteo OMM dans le carre de 32 pixels en (x0, y0) ;
// nMillis fait bouger la pluie, la neige, les rayons... (0 : image fixe)
void DrawWeatherIcon (CCanvas *pCanvas, int x0, int y0, int nCode, boolean bDay, unsigned nMillis);

#endif
