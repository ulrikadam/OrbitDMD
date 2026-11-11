//
// rtc.cpp
//
#include "rtc.h"

#define DS1307_ADDRESS	0x68

static unsigned FromBCD (u8 uchValue)
{
	return (uchValue >> 4) * 10 + (uchValue & 0x0F);
}

static u8 ToBCD (unsigned nValue)
{
	return (u8) (((nValue / 10) << 4) | (nValue % 10));
}

CRtcDS1307::CRtcDS1307 (CI2CMaster *pI2CMaster)
:	m_pI2CMaster (pI2CMaster),
	m_bAvailable (FALSE)
{
}

boolean CRtcDS1307::Read (TDateTime *pUTC)
{
	if (!m_bAvailable)
	{
		return FALSE;
	}

	u8 uchRegister = 0;
	if (m_pI2CMaster->Write (DS1307_ADDRESS, &uchRegister, 1) != 1)
	{
		return FALSE;
	}

	u8 Buffer[7];
	if (m_pI2CMaster->Read (DS1307_ADDRESS, Buffer, sizeof Buffer) != (int) sizeof Buffer)
	{
		return FALSE;
	}

	if (Buffer[0] & 0x80)		// bit CH : horloge arretee
	{
		return FALSE;
	}

	pUTC->Second = FromBCD (Buffer[0] & 0x7F);
	pUTC->Minute = FromBCD (Buffer[1] & 0x7F);

	if (Buffer[2] & 0x40)		// mode 12 h
	{
		unsigned nHour = FromBCD (Buffer[2] & 0x1F) % 12;
		pUTC->Hour = (Buffer[2] & 0x20) ? nHour + 12 : nHour;
	}
	else
	{
		pUTC->Hour = FromBCD (Buffer[2] & 0x3F);
	}

	pUTC->Day   = FromBCD (Buffer[4] & 0x3F);
	pUTC->Month = FromBCD (Buffer[5] & 0x1F);
	pUTC->Year  = 2000 + FromBCD (Buffer[6]);

	return    pUTC->Second < 60 && pUTC->Minute < 60 && pUTC->Hour < 24
	       && pUTC->Day >= 1 && pUTC->Day <= 31
	       && pUTC->Month >= 1 && pUTC->Month <= 12;
}

boolean CRtcDS1307::Write (const TDateTime &rUTC)
{
	if (!m_bAvailable)
	{
		return FALSE;
	}

	u8 Buffer[8];
	Buffer[0] = 0;				// premier registre
	Buffer[1] = ToBCD (rUTC.Second);	// CH = 0 : horloge en marche
	Buffer[2] = ToBCD (rUTC.Minute);
	Buffer[3] = ToBCD (rUTC.Hour);		// mode 24 h
	Buffer[4] = 1;				// jour de la semaine (non utilise)
	Buffer[5] = ToBCD (rUTC.Day);
	Buffer[6] = ToBCD (rUTC.Month);
	Buffer[7] = ToBCD (rUTC.Year % 100);

	return m_pI2CMaster->Write (DS1307_ADDRESS, Buffer, sizeof Buffer) == (int) sizeof Buffer;
}
