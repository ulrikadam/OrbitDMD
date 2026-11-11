//
// clockstyle.cpp
//
#include "clockstyle.h"
#include <circle/util.h>

static const char *s_DayShort[7] = {"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"};
static const char *s_DayLong[7] =
	{"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"};
static const char *s_MonthShort[12] =
	{"JAN", "FEV", "MARS", "AVR", "MAI", "JUIN", "JUIL", "AOUT", "SEPT", "OCT", "NOV", "DEC"};
static const char *s_MonthLong[12] =
	{"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT",
	 "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"};

static const char s_FormatLetters[] = "HIMSpdemygYaAbB%";

static void CopyText (char *pDest, const char *pSource, unsigned nSize)
{
	unsigned i = 0;
	while (pSource[i] != '\0' && i < nSize - 1)
	{
		pDest[i] = pSource[i];
		i++;
	}
	pDest[i] = '\0';
}

static boolean ParseNumber (const char *pText, unsigned nMax, unsigned *pValue)
{
	if (*pText == '\0')
	{
		return FALSE;
	}

	unsigned nValue = 0;
	for (; *pText != '\0'; pText++)
	{
		if (*pText < '0' || *pText > '9')
		{
			return FALSE;
		}
		nValue = nValue * 10 + (*pText - '0');
		if (nValue > nMax)
		{
			return FALSE;
		}
	}

	*pValue = nValue;

	return TRUE;
}

static boolean IsNone (const char *pValue)
{
	return *pValue == '\0' || strcasecmp (pValue, CLOCK_NONE) == 0;
}

void ClockStyleDefaults (TClockStyle *pStyle)
{
	memset (pStyle, 0, sizeof *pStyle);

	TTextStyle *Texts[2] = {&pStyle->Time, &pStyle->Date};
	for (unsigned i = 0; i < 2; i++)
	{
		CopyText (Texts[i]->Font, CLOCK_FONT_PIXEL, CLOCK_NAME_LEN);
		Texts[i]->nSize = 0;
		CopyText (Texts[i]->Pattern, CLOCK_NONE, CLOCK_NAME_LEN);
	}

	pStyle->Time.Color[0] = 0xFF; pStyle->Time.Color[1] = 0xAA; pStyle->Time.Color[2] = 0x00;
	pStyle->Date.Color[0] = 0x01; pStyle->Date.Color[1] = 0xA7; pStyle->Date.Color[2] = 0xC5;
	CopyText (pStyle->Time.Format, "%H:%M:%S", CLOCK_FORMAT_LEN);
	CopyText (pStyle->Date.Format, "%a %d %b", CLOCK_FORMAT_LEN);

	pStyle->bSmooth = TRUE;
	pStyle->bBlink = FALSE;
	CopyText (pStyle->Background, CLOCK_NONE, CLOCK_NAME_LEN);
	pStyle->DateMode = DateOff;
	pStyle->nDateDisplayTime = 2500;
}

boolean SetClockStyleValue (TClockStyle *pStyle, const char *pKey, const char *pValue)
{
	TTextStyle *pText;
	const char *pName;

	if (strncasecmp (pKey, "Clock_", 6) == 0)
	{
		pText = &pStyle->Time;
		pName = pKey + 6;
	}
	else if (strncasecmp (pKey, "Date_", 5) == 0)
	{
		pText = &pStyle->Date;
		pName = pKey + 5;
	}
	else
	{
		return FALSE;
	}

	unsigned nValue;

	if (strcasecmp (pName, "Font") == 0)
	{
		if (   strcasecmp (pValue, CLOCK_FONT_PIXEL) == 0
		    || IsValidAssetName (pValue, "ttf,otf"))
		{
			CopyText (pText->Font, pValue, CLOCK_NAME_LEN);
		}
	}
	else if (strcasecmp (pName, "Font_Size") == 0)
	{
		if (ParseNumber (pValue, CLOCK_MAX_SIZE, &nValue))
		{
			pText->nSize = nValue;
		}
	}
	else if (strcasecmp (pName, "Font_Pattern") == 0)
	{
		if (IsNone (pValue))
		{
			CopyText (pText->Pattern, CLOCK_NONE, CLOCK_NAME_LEN);
		}
		else if (IsValidAssetName (pValue, "png,gif"))
		{
			CopyText (pText->Pattern, pValue, CLOCK_NAME_LEN);
		}
	}
	else if (strcasecmp (pName, "Color") == 0)
	{
		ParseHexColor (pValue, pText->Color);
	}
	else if (strcasecmp (pName, "Format") == 0)
	{
		if (IsValidClockFormat (pValue))
		{
			CopyText (pText->Format, pValue, CLOCK_FORMAT_LEN);
		}
	}
	else if (pText == &pStyle->Time && strcasecmp (pName, "Background") == 0)
	{
		if (IsNone (pValue))
		{
			CopyText (pStyle->Background, CLOCK_NONE, CLOCK_NAME_LEN);
		}
		else if (IsValidAssetName (pValue, "png,gif"))
		{
			CopyText (pStyle->Background, pValue, CLOCK_NAME_LEN);
		}
	}
	else if (pText == &pStyle->Time && strcasecmp (pName, "Blink") == 0)
	{
		if (ParseNumber (pValue, 1, &nValue))
		{
			pStyle->bBlink = nValue != 0;
		}
	}
	else if (pText == &pStyle->Time && strcasecmp (pName, "Smooth") == 0)
	{
		if (ParseNumber (pValue, 1, &nValue))
		{
			pStyle->bSmooth = nValue != 0;
		}
	}
	else if (pText == &pStyle->Date && strcasecmp (pName, "Active") == 0)
	{
		if (ParseNumber (pValue, 2, &nValue))
		{
			pStyle->DateMode = nValue;
		}
	}
	else if (pText == &pStyle->Date && strcasecmp (pName, "Display_Time") == 0)
	{
		if (ParseNumber (pValue, 60000, &nValue) && nValue >= 500)
		{
			pStyle->nDateDisplayTime = nValue;
		}
	}
	else
	{
		return FALSE;		// Clock_Active, Clock_TZ... : geres par CConfig
	}

	return TRUE;
}

static unsigned WeekDay (const TDateTime &rTime)
{
	// 1er janvier 1970 : un jeudi
	return (DateTimeToUnix (rTime) / 86400 + 4) % 7;
}

void FormatDateTime (const char *pFormat, const TDateTime &rTime, char *pOut, unsigned nOutSize)
{
	unsigned n = 0;

	for (const char *p = pFormat; *p != '\0' && n < nOutSize - 1; p++)
	{
		char Part[16];
		const char *pPart = Part;
		Part[0] = '\0';

		if (*p != '%' || p[1] == '\0')
		{
			Part[0] = *p;
			Part[1] = '\0';
		}
		else
		{
			unsigned nHour12 = rTime.Hour % 12 == 0 ? 12 : rTime.Hour % 12;
			unsigned nMonth = (rTime.Month >= 1 && rTime.Month <= 12) ? rTime.Month - 1 : 0;

			switch (*++p)
			{
			case 'H':	Part[0] = '0' + rTime.Hour / 10;   Part[1] = '0' + rTime.Hour % 10;   Part[2] = '\0'; break;
			case 'I':	Part[0] = '0' + nHour12 / 10;      Part[1] = '0' + nHour12 % 10;      Part[2] = '\0'; break;
			case 'M':	Part[0] = '0' + rTime.Minute / 10; Part[1] = '0' + rTime.Minute % 10; Part[2] = '\0'; break;
			case 'S':	Part[0] = '0' + rTime.Second / 10; Part[1] = '0' + rTime.Second % 10; Part[2] = '\0'; break;
			case 'd':	Part[0] = '0' + rTime.Day / 10;    Part[1] = '0' + rTime.Day % 10;    Part[2] = '\0'; break;
			case 'm':	Part[0] = '0' + rTime.Month / 10;  Part[1] = '0' + rTime.Month % 10;  Part[2] = '\0'; break;
			case 'y':
			case 'g':	Part[0] = '0' + rTime.Year / 10 % 10; Part[1] = '0' + rTime.Year % 10; Part[2] = '\0'; break;
			case 'e':
				if (rTime.Day >= 10)
				{
					Part[0] = '0' + rTime.Day / 10; Part[1] = '0' + rTime.Day % 10; Part[2] = '\0';
				}
				else
				{
					Part[0] = '0' + rTime.Day; Part[1] = '\0';
				}
				break;
			case 'Y':
				Part[0] = '0' + rTime.Year / 1000 % 10; Part[1] = '0' + rTime.Year / 100 % 10;
				Part[2] = '0' + rTime.Year / 10 % 10;   Part[3] = '0' + rTime.Year % 10;
				Part[4] = '\0';
				break;
			case 'p':	pPart = rTime.Hour < 12 ? "AM" : "PM";	break;
			case 'a':	pPart = s_DayShort[WeekDay (rTime)];	break;
			case 'A':	pPart = s_DayLong[WeekDay (rTime)];	break;
			case 'b':	pPart = s_MonthShort[nMonth];		break;
			case 'B':	pPart = s_MonthLong[nMonth];		break;
			case '%':	Part[0] = '%'; Part[1] = '\0';		break;
			default:	break;		// code inconnu : ignore
			}
		}

		for (; *pPart != '\0' && n < nOutSize - 1; pPart++)
		{
			pOut[n++] = *pPart;
		}
	}

	pOut[n] = '\0';
}

boolean IsValidClockFormat (const char *pFormat)
{
	size_t nLength = strlen (pFormat);
	if (nLength == 0 || nLength >= CLOCK_FORMAT_LEN)
	{
		return FALSE;
	}

	for (const char *p = pFormat; *p != '\0'; p++)
	{
		unsigned char c = (unsigned char) *p;
		if (c < ' ' || c > '~' || c == '"' || c == '\\')
		{
			return FALSE;
		}

		if (c == '%')
		{
			p++;
			if (*p == '\0' || strchr (s_FormatLetters, *p) == 0)
			{
				return FALSE;
			}
		}
	}

	return TRUE;
}

static int HexDigit (char c)
{
	if ('0' <= c && c <= '9')	return c - '0';
	if ('a' <= c && c <= 'f')	return c - 'a' + 10;
	if ('A' <= c && c <= 'F')	return c - 'A' + 10;
	return -1;
}

boolean ParseHexColor (const char *pText, u8 *pRGB)
{
	if (*pText == '#')
	{
		pText++;
	}

	if (strlen (pText) != 6)
	{
		return FALSE;
	}

	u8 RGB[3];
	for (unsigned i = 0; i < 3; i++)
	{
		int nHigh = HexDigit (pText[2*i]);
		int nLow = HexDigit (pText[2*i+1]);
		if (nHigh < 0 || nLow < 0)
		{
			return FALSE;
		}
		RGB[i] = (u8) (nHigh * 16 + nLow);
	}

	pRGB[0] = RGB[0];
	pRGB[1] = RGB[1];
	pRGB[2] = RGB[2];

	return TRUE;
}

// --- meteo ---

void WeatherStyleDefaults (TWeatherStyle *pStyle)
{
	memset (pStyle, 0, sizeof *pStyle);

	pStyle->bActive = TRUE;
	pStyle->nDisplayTime = 5000;
	pStyle->nEvery = 2;
	CopyText (pStyle->Text.Font, CLOCK_FONT_PIXEL, CLOCK_NAME_LEN);
	CopyText (pStyle->Text.Pattern, CLOCK_NONE, CLOCK_NAME_LEN);
	CopyText (pStyle->Text.Format, "-", CLOCK_FORMAT_LEN);
	pStyle->Text.Color[0] = 0xFF; pStyle->Text.Color[1] = 0xAA; pStyle->Text.Color[2] = 0x00;
	pStyle->InfoColor[0] = 0x01; pStyle->InfoColor[1] = 0xA7; pStyle->InfoColor[2] = 0xC5;
	pStyle->Info = InfoDescription;
	pStyle->bIcon = TRUE;
	pStyle->bAnimate = TRUE;
}

boolean SetWeatherStyleValue (TWeatherStyle *pStyle, const char *pKey, const char *pValue)
{
	if (strncasecmp (pKey, "Weather_", 8) != 0)
	{
		return FALSE;
	}
	const char *pName = pKey + 8;
	unsigned nValue;

	if (strcasecmp (pName, "Active") == 0)
	{
		if (ParseNumber (pValue, 1, &nValue))	pStyle->bActive = nValue != 0;
	}
	else if (strcasecmp (pName, "Display_Time") == 0)
	{
		if (ParseNumber (pValue, 60000, &nValue) && nValue >= 1000)	pStyle->nDisplayTime = nValue;
	}
	else if (strcasecmp (pName, "Every") == 0)
	{
		if (ParseNumber (pValue, 20, &nValue) && nValue >= 1)		pStyle->nEvery = nValue;
	}
	else if (strcasecmp (pName, "Latitude") == 0)
	{
		if (*pValue == '\0')	pStyle->Latitude[0] = '\0';
		else			NormalizeCoordinate (pValue, TRUE, pStyle->Latitude, sizeof pStyle->Latitude);
	}
	else if (strcasecmp (pName, "Longitude") == 0)
	{
		if (*pValue == '\0')	pStyle->Longitude[0] = '\0';
		else			NormalizeCoordinate (pValue, FALSE, pStyle->Longitude, sizeof pStyle->Longitude);
	}
	else if (strcasecmp (pName, "Place") == 0)
	{
		if (IsValidPlaceName (pValue))	CopyText (pStyle->Place, pValue, sizeof pStyle->Place);
	}
	else if (strcasecmp (pName, "Font") == 0)
	{
		if (strcasecmp (pValue, CLOCK_FONT_PIXEL) == 0 || IsValidAssetName (pValue, "ttf,otf"))
		{
			CopyText (pStyle->Text.Font, pValue, CLOCK_NAME_LEN);
		}
	}
	else if (strcasecmp (pName, "Font_Size") == 0)
	{
		if (ParseNumber (pValue, CLOCK_MAX_SIZE, &nValue))	pStyle->Text.nSize = nValue;
	}
	else if (strcasecmp (pName, "Font_Pattern") == 0)
	{
		if (IsNone (pValue))					CopyText (pStyle->Text.Pattern, CLOCK_NONE, CLOCK_NAME_LEN);
		else if (IsValidAssetName (pValue, "png,gif"))		CopyText (pStyle->Text.Pattern, pValue, CLOCK_NAME_LEN);
	}
	else if (strcasecmp (pName, "Text_color") == 0)
	{
		ParseAnyColor (pValue, pStyle->Text.Color);
	}
	else if (strcasecmp (pName, "Info_Color") == 0)
	{
		ParseAnyColor (pValue, pStyle->InfoColor);
	}
	else if (strcasecmp (pName, "Info") == 0)
	{
		if (ParseNumber (pValue, WEATHER_INFO_MAX, &nValue))	pStyle->Info = nValue;
	}
	else if (strcasecmp (pName, "Icon") == 0)
	{
		if (ParseNumber (pValue, 1, &nValue))	pStyle->bIcon = nValue != 0;
	}
	else if (strcasecmp (pName, "Animate") == 0)
	{
		if (ParseNumber (pValue, 1, &nValue))	pStyle->bAnimate = nValue != 0;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

boolean NormalizeCoordinate (const char *pText, boolean bLatitude, char *pOut, unsigned nOutSize)
{
	const char *p = pText;
	boolean bNegative = FALSE;
	if (*p == '-' || *p == '+')
	{
		bNegative = *p == '-';
		p++;
	}

	unsigned nInteger = 0, nDigits = 0;
	while ('0' <= *p && *p <= '9')
	{
		nInteger = nInteger * 10 + (*p++ - '0');
		if (++nDigits > 3)
		{
			return FALSE;
		}
	}
	if (nDigits == 0)
	{
		return FALSE;
	}

	char Decimals[5] = "";
	unsigned nDecimals = 0;
	boolean bNonZero = FALSE;
	if (*p == '.')
	{
		p++;
		while ('0' <= *p && *p <= '9')
		{
			if (nDecimals < 4)
			{
				Decimals[nDecimals++] = *p;
				bNonZero = bNonZero || *p != '0';
			}
			p++;
		}
		Decimals[nDecimals] = '\0';
	}
	if (*p != '\0')
	{
		return FALSE;
	}

	unsigned nLimit = bLatitude ? 90 : 180;
	if (nInteger > nLimit || (nInteger == nLimit && bNonZero))
	{
		return FALSE;
	}

	char Text[16];
	unsigned n = 0;
	if (bNegative && (nInteger != 0 || bNonZero))
	{
		Text[n++] = '-';
	}
	if (nInteger >= 100)	Text[n++] = '0' + nInteger / 100;
	if (nInteger >= 10)	Text[n++] = '0' + nInteger / 10 % 10;
	Text[n++] = '0' + nInteger % 10;
	if (nDecimals > 0)
	{
		Text[n++] = '.';
		for (unsigned i = 0; i < nDecimals; i++)
		{
			Text[n++] = Decimals[i];
		}
	}
	Text[n] = '\0';

	if (n >= nOutSize)
	{
		return FALSE;
	}
	CopyText (pOut, Text, nOutSize);

	return TRUE;
}

boolean IsValidPlaceName (const char *pName)
{
	size_t nLength = strlen (pName);
	if (nLength >= 48)
	{
		return FALSE;
	}

	for (const char *p = pName; *p != '\0'; p++)
	{
		unsigned char c = (unsigned char) *p;
		if (c < ' ' || c == '"' || c == '\\' || c == 0x7F)
		{
			return FALSE;		// UTF-8 (accents) accepte
		}
	}

	return TRUE;
}

boolean ParseAnyColor (const char *pText, u8 *pRGB)
{
	if (strchr (pText, ',') == 0)
	{
		return ParseHexColor (pText, pRGB);
	}

	// ancien format "255,255,0"
	unsigned Values[3];
	const char *p = pText;
	for (unsigned i = 0; i < 3; i++)
	{
		while (*p == ' ')
		{
			p++;
		}
		char Number[4];
		unsigned n = 0;
		while ('0' <= *p && *p <= '9' && n < 3)
		{
			Number[n++] = *p++;
		}
		Number[n] = '\0';
		while (*p == ' ')
		{
			p++;
		}
		if (n == 0 || !ParseNumber (Number, 255, &Values[i]) || *p != (i < 2 ? ',' : '\0'))
		{
			return FALSE;
		}
		if (i < 2)
		{
			p++;
		}
	}

	for (unsigned i = 0; i < 3; i++)
	{
		pRGB[i] = (u8) Values[i];
	}

	return TRUE;
}

boolean IsValidAssetName (const char *pName, const char *pExtensions)
{
	size_t nLength = strlen (pName);
	if (nLength < 5 || nLength >= CLOCK_NAME_LEN || pName[0] == '/' || pName[0] == '.')
	{
		return FALSE;
	}

	unsigned nSlashes = 0;
	for (const char *p = pName; *p != '\0'; p++)
	{
		unsigned char c = (unsigned char) *p;
		if (c < ' ' || c > '~' || c == '\\' || c == ':' || c == '"' || c == '*' || c == '?'
		    || c == '<' || c == '>' || c == '|')
		{
			return FALSE;
		}
		if (c == '/')
		{
			if (++nSlashes > 1 || p[1] == '\0' || p[1] == '/' || p[1] == '.')
			{
				return FALSE;
			}
		}
		if (c == '.' && p[1] == '.')
		{
			return FALSE;
		}
	}

	// extension (apres le dernier point)
	const char *pDot = 0;
	for (const char *p = pName; *p != '\0'; p++)
	{
		if (*p == '.')
		{
			pDot = p;
		}
	}
	if (pDot == 0 || strchr (pDot, '/') != 0)
	{
		return FALSE;
	}
	pDot++;
	size_t nExt = strlen (pDot);

	const char *p = pExtensions;
	while (*p != '\0')
	{
		const char *pComma = strchr (p, ',');
		size_t nItem = pComma != 0 ? (size_t) (pComma - p) : strlen (p);
		if (nItem == nExt && strncasecmp (p, pDot, nExt) == 0)
		{
			return TRUE;
		}
		p += nItem;
		if (*p == ',')
		{
			p++;
		}
	}

	return FALSE;
}
