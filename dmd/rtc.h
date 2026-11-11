//
// rtc.h - horloge a pile DS1307 de la carte d'adaptation (I2C, adresse 0x68)
//
// L'ancien systeme Linux y enregistrait l'heure UTC (hwclock, /etc/adjtime = UTC).
//
#ifndef _rtc_h
#define _rtc_h

#include <circle/i2cmaster.h>
#include <circle/types.h>
#include "localtime.h"

class CRtcDS1307
{
public:
	CRtcDS1307 (CI2CMaster *pI2CMaster);

	void SetAvailable (boolean bAvailable)	{ m_bAvailable = bAvailable; }

	// FALSE si l'horloge est absente, arretee ou incoherente
	boolean Read (TDateTime *pUTC);
	boolean Write (const TDateTime &rUTC);

private:
	CI2CMaster *m_pI2CMaster;
	boolean m_bAvailable;
};

#endif
