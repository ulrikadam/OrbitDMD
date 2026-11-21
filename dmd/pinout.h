//
// pinout.h
//
// Brochage de la carte adaptatrice HUB75 (reglage "Brochage" de config.txt).
// Les numeros de broches sont dans matrix.cpp ; ici seulement les noms, pour la configuration
// et la page Reglages (compile aussi sur le PC, sans materiel).
//
#ifndef _orbitdmd_pinout_h
#define _orbitdmd_pinout_h

enum TMatrixPinout
{
	MatrixPinoutRegular	= 0,	// carte RPI2DMD, Electrodragon... ("regular" de rpi-rgb-led-matrix), 3 sorties
	MatrixPinoutAdafruit	= 1,	// Adafruit RGB Matrix Bonnet / HAT, 1 sortie
	MatrixPinoutAdafruitPWM	= 2,	// Adafruit modifie (fil entre GPIO 4 et 18) : OE sur GPIO 18
	MatrixPinoutCount
};

// "rpi2dmd" (ou "regular"), "adafruit", "adafruit-pwm" ; -1 si inconnu
int MatrixPinoutFromName (const char *pName);

// nom du brochage, pour config.txt et le journal
const char *MatrixPinoutName (unsigned nPinout);

// nombre de sorties HUB75 de la carte (3 pour RPI2DMD, 1 pour Adafruit)
unsigned MatrixPinoutOutputs (unsigned nPinout);

#endif
