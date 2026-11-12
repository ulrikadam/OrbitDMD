//
// weatherdata.cpp
//
#include "weatherdata.h"
#include "lang.h"
#include <circle/util.h>

// debut de l'objet "nom":{ ... } (0 si absent)
static const char *FindObject (const char *pJSON, const char *pName)
{
	char Key[40];
	unsigned n = 0;
	Key[n++] = '"';
	for (const char *p = pName; *p != '\0' && n < sizeof Key - 4; p++)
	{
		Key[n++] = *p;
	}
	Key[n++] = '"';
	Key[n++] = ':';
	Key[n++] = '{';
	Key[n] = '\0';

	return strstr (pJSON, Key);
}

// nombre decimal -> dixiemes ("6.5" -> 65, "-0.4" -> -4, "92" -> 920)
static boolean ParseTenths (const char *p, int *pValue)
{
	while (*p == ' ')
	{
		p++;
	}

	boolean bNegative = FALSE;
	if (*p == '-')
	{
		bNegative = TRUE;
		p++;
	}
	if (*p < '0' || *p > '9')
	{
		return FALSE;			// null, ou autre
	}

	int nValue = 0;
	while ('0' <= *p && *p <= '9')
	{
		nValue = nValue * 10 + (*p++ - '0');
		if (nValue > 100000)
		{
			return FALSE;
		}
	}
	nValue *= 10;
	if (*p == '.' && '0' <= p[1] && p[1] <= '9')
	{
		int nDigit = p[1] - '0';
		if ('5' <= p[2] && p[2] <= '9')
		{
			nDigit++;		// arrondi au dixieme
		}
		nValue += nDigit;
	}

	*pValue = bNegative ? -nValue : nValue;

	return TRUE;
}

// valeur de "cle": dans l'objet qui commence en pObject (jusqu'a la '}' correspondante)
static boolean GetValue (const char *pObject, const char *pName, unsigned nIndex, int *pValue)
{
	if (pObject == 0)
	{
		return FALSE;
	}

	const char *pEnd = strchr (pObject, '}');
	char Key[40];
	unsigned n = 0;
	Key[n++] = '"';
	for (const char *p = pName; *p != '\0' && n < sizeof Key - 3; p++)
	{
		Key[n++] = *p;
	}
	Key[n++] = '"';
	Key[n++] = ':';
	Key[n] = '\0';

	const char *p = strstr (pObject, Key);
	if (p == 0 || (pEnd != 0 && p > pEnd))
	{
		return FALSE;
	}
	p += n;

	if (*p == '[')			// tableau : element nIndex
	{
		p++;
		for (unsigned i = 0; i < nIndex; i++)
		{
			while (*p != ',' && *p != ']' && *p != '\0')
			{
				p++;
			}
			if (*p != ',')
			{
				return FALSE;
			}
			p++;
		}
	}
	else if (nIndex != 0)
	{
		return FALSE;
	}

	return ParseTenths (p, pValue);
}

boolean ParseOpenMeteo (const char *pJSON, TWeatherData *pData)
{
	TWeatherData D;
	memset (&D, 0, sizeof D);

	const char *pCurrent = FindObject (pJSON, "current");
	const char *pDaily = FindObject (pJSON, "daily");

	int nHumidity10, nCode10, nDay10, nTomorrowCode10;
	if (   !GetValue (pCurrent, "temperature_2m", 0, &D.nTemp10)
	    || !GetValue (pCurrent, "weather_code", 0, &nCode10))
	{
		return FALSE;
	}
	D.nCode = nCode10 / 10;

	if (!GetValue (pCurrent, "apparent_temperature", 0, &D.nFeels10))	D.nFeels10 = D.nTemp10;
	if (!GetValue (pCurrent, "relative_humidity_2m", 0, &nHumidity10))	nHumidity10 = 0;
	if (!GetValue (pCurrent, "wind_speed_10m", 0, &D.nWind10))		D.nWind10 = 0;
	if (!GetValue (pCurrent, "is_day", 0, &nDay10))				nDay10 = 10;
	D.nHumidity = nHumidity10 / 10;
	D.bDay = nDay10 != 0;

	if (   !GetValue (pDaily, "temperature_2m_min", 0, &D.nMin10)
	    || !GetValue (pDaily, "temperature_2m_max", 0, &D.nMax10))
	{
		D.nMin10 = D.nMax10 = D.nTemp10;
	}
	if (   GetValue (pDaily, "weather_code", 1, &nTomorrowCode10)
	    && GetValue (pDaily, "temperature_2m_min", 1, &D.nTomorrowMin10)
	    && GetValue (pDaily, "temperature_2m_max", 1, &D.nTomorrowMax10))
	{
		D.nTomorrowCode = nTomorrowCode10 / 10;
	}
	else
	{
		D.nTomorrowCode = -1;
	}

	D.bValid = TRUE;
	*pData = D;

	return TRUE;
}

const char *WeatherDescription (int nCode, boolean bDay)
{
	int n;		// index des textes de lang.cpp
	switch (nCode)
	{
	case 0:			n = bDay ? 0 : 1;	break;
	case 1:			n = 2;		break;
	case 2:			n = 3;		break;
	case 3:			n = 4;		break;
	case 45: case 48:	n = 5;		break;
	case 51: case 53:	n = 6;		break;
	case 55:		n = 7;		break;
	case 56: case 57:	n = 8;		break;
	case 61:		n = 9;		break;
	case 63:		n = 10;		break;
	case 65:		n = 11;		break;
	case 66: case 67:	n = 12;		break;
	case 71:		n = 13;		break;
	case 73: case 77:	n = 14;		break;
	case 75:		n = 15;		break;
	case 80: case 81:	n = 16;		break;
	case 82:		n = 17;		break;
	case 85: case 86:	n = 18;		break;
	case 95:		n = 19;		break;
	case 96: case 99:	n = 20;		break;
	default:		return "?";
	}

	return WeatherText ((unsigned) n);
}

TWeatherIcon WeatherIconFor (int nCode)
{
	if (nCode == 0)				return IconClear;
	if (nCode == 1 || nCode == 2)		return IconPartly;
	if (nCode == 3)				return IconCloudy;
	if (nCode == 45 || nCode == 48)		return IconFog;
	if (51 <= nCode && nCode <= 57)		return IconDrizzle;
	if ((61 <= nCode && nCode <= 67) || (80 <= nCode && nCode <= 82))	return IconRain;
	if ((71 <= nCode && nCode <= 77) || nCode == 85 || nCode == 86)		return IconSnow;
	if (nCode >= 95)			return IconStorm;
	return IconCloudy;
}
