//
// pinout.cpp
//
// Noms des brochages de carte adaptatrice (voir pinout.h)
//
#include "pinout.h"
#include <circle/util.h>

static const char *const s_Names[MatrixPinoutCount] = {"rpi2dmd", "adafruit", "adafruit-pwm"};

int MatrixPinoutFromName (const char *pName)
{
	if (strcasecmp (pName, "regular") == 0)		// nom de rpi-rgb-led-matrix
	{
		return MatrixPinoutRegular;
	}
	for (unsigned i = 0; i < MatrixPinoutCount; i++)
	{
		if (strcasecmp (pName, s_Names[i]) == 0)
		{
			return (int) i;
		}
	}
	return -1;
}

const char *MatrixPinoutName (unsigned nPinout)
{
	return s_Names[nPinout < MatrixPinoutCount ? nPinout : 0];
}

unsigned MatrixPinoutOutputs (unsigned nPinout)
{
	return nPinout == MatrixPinoutAdafruit || nPinout == MatrixPinoutAdafruitPWM ? 1 : 3;
}
