//
// localtime.cpp
//
#include "localtime.h"
#include <circle/util.h>

struct TZone
{
	const char *pName;
	int	    nOffsetHours;	// decalage d'hiver par rapport a UTC
	boolean	    bEUSummerTime;
};

static const TZone s_Zones[] =
{
	{"Europe/London",	0, TRUE},
	{"Europe/Dublin",	0, TRUE},
	{"Europe/Lisbon",	0, TRUE},
	{"Europe/Helsinki",	2, TRUE},
	{"Europe/Athens",	2, TRUE},
	{"Europe/Bucharest",	2, TRUE},
	{"Europe/Sofia",	2, TRUE},
	{"Europe/Kiev",		2, TRUE},
	{"Europe/Kyiv",		2, TRUE},
	{"Europe/Istanbul",	3, FALSE},
	{"Europe/Moscow",	3, FALSE},
	{"UTC",			0, FALSE},
	{"Etc/UTC",		0, FALSE},
};

static boolean IsLeapYear (unsigned nYear)
{
	return (nYear % 4 == 0 && nYear % 100 != 0) || nYear % 400 == 0;
}

static unsigned DaysInMonth (unsigned nYear, unsigned nMonth)
{
	static const unsigned Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return nMonth == 2 && IsLeapYear (nYear) ? 29 : Days[(nMonth - 1) % 12];
}

static unsigned DayOfWeek (unsigned y, unsigned m, unsigned d)	// 0 = dimanche
{
	static const unsigned t[12] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	if (m < 3)
	{
		y--;
	}
	return (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
}

static unsigned LastSunday (unsigned nYear, unsigned nMonth)
{
	unsigned nLast = DaysInMonth (nYear, nMonth);
	return nLast - DayOfWeek (nYear, nMonth, nLast);
}

static boolean IsEUSummerTime (const TDateTime &rUTC)
{
	if (rUTC.Month < 3 || rUTC.Month > 10)
	{
		return FALSE;
	}
	if (rUTC.Month > 3 && rUTC.Month < 10)
	{
		return TRUE;
	}

	unsigned nSunday = LastSunday (rUTC.Year, rUTC.Month);
	if (rUTC.Month == 3)
	{
		return rUTC.Day > nSunday || (rUTC.Day == nSunday && rUTC.Hour >= 1);
	}

	return rUTC.Day < nSunday || (rUTC.Day == nSunday && rUTC.Hour < 1);
}

static void AddHours (TDateTime *pTime, int nHours)
{
	int nHour = (int) pTime->Hour + nHours;

	while (nHour >= 24)
	{
		nHour -= 24;
		if (++pTime->Day > DaysInMonth (pTime->Year, pTime->Month))
		{
			pTime->Day = 1;
			if (++pTime->Month > 12)
			{
				pTime->Month = 1;
				pTime->Year++;
			}
		}
	}

	while (nHour < 0)
	{
		nHour += 24;
		if (--pTime->Day < 1)
		{
			if (--pTime->Month < 1)
			{
				pTime->Month = 12;
				pTime->Year--;
			}
			pTime->Day = DaysInMonth (pTime->Year, pTime->Month);
		}
	}

	pTime->Hour = (unsigned) nHour;
}

u32 DateTimeToUnix (const TDateTime &rTime)
{
	u32 nDays = 0;
	for (unsigned nYear = 1970; nYear < rTime.Year; nYear++)
	{
		nDays += IsLeapYear (nYear) ? 366 : 365;
	}
	for (unsigned nMonth = 1; nMonth < rTime.Month; nMonth++)
	{
		nDays += DaysInMonth (rTime.Year, nMonth);
	}
	nDays += rTime.Day - 1;

	return ((nDays * 24 + rTime.Hour) * 60 + rTime.Minute) * 60 + rTime.Second;
}

void UnixToDateTime (u32 nTime, TDateTime *pTime)
{
	pTime->Second = nTime % 60;
	nTime /= 60;
	pTime->Minute = nTime % 60;
	nTime /= 60;
	pTime->Hour = nTime % 24;
	u32 nDays = nTime / 24;

	unsigned nYear = 1970;
	while (nDays >= (IsLeapYear (nYear) ? 366U : 365U))
	{
		nDays -= IsLeapYear (nYear) ? 366 : 365;
		nYear++;
	}

	unsigned nMonth = 1;
	while (nDays >= DaysInMonth (nYear, nMonth))
	{
		nDays -= DaysInMonth (nYear, nMonth);
		nMonth++;
	}

	pTime->Year = nYear;
	pTime->Month = nMonth;
	pTime->Day = nDays + 1;
}

void UTCToLocal (const TDateTime &rUTC, const char *pTimeZone, TDateTime *pLocal)
{
	// par defaut : heure d'Europe centrale (Paris, Bruxelles, Berlin, Madrid, Rome...)
	int nOffset = 1;
	boolean bSummerTime = TRUE;

	for (unsigned i = 0; i < sizeof s_Zones / sizeof s_Zones[0]; i++)
	{
		if (strcasecmp (pTimeZone, s_Zones[i].pName) == 0)
		{
			nOffset = s_Zones[i].nOffsetHours;
			bSummerTime = s_Zones[i].bEUSummerTime;
			break;
		}
	}

	if (bSummerTime && IsEUSummerTime (rUTC))
	{
		nOffset++;
	}

	*pLocal = rUTC;
	AddHours (pLocal, nOffset);
}
