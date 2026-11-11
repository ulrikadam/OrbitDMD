//
// schedule.cpp
//
#include "schedule.h"
#include <circle/util.h>

static boolean ParseSmall (const char *p, unsigned nDigits, unsigned nMax, unsigned *pValue)
{
	unsigned n = 0, nValue = 0;
	for (; *p != '\0' && n < nDigits; p++, n++)
	{
		if (*p < '0' || *p > '9')
		{
			return FALSE;
		}
		nValue = nValue * 10 + (*p - '0');
	}
	if (n == 0 || *p != '\0' || nValue > nMax)
	{
		return FALSE;
	}

	*pValue = nValue;
	return TRUE;
}

void ScheduleDefaults (TSchedule *pSchedule)
{
	memset (pSchedule, 0, sizeof *pSchedule);
	pSchedule->nOffFrom = 1;
	pSchedule->nOffTo = 7;
	pSchedule->nClockFrom = 23;
	pSchedule->nClockTo = 7;
}

boolean SetScheduleValue (TSchedule *pSchedule, const char *pKey, const char *pValue)
{
	unsigned n;

	if (strcasecmp (pKey, "Gif_Cat_Mois") == 0)
	{
		ParseCategoryMonths (pValue, pSchedule);
		return TRUE;
	}
	if (strncasecmp (pKey, "Prog_", 5) != 0)
	{
		return FALSE;
	}

	const char *pName = pKey + 5;
	boolean bNumber = ParseSmall (pValue, 2, 23, &n);
	if      (strcasecmp (pName, "Eteint_Active") == 0)	{ if (bNumber) pSchedule->bOff = n != 0; }
	else if (strcasecmp (pName, "Eteint_Debut") == 0)	{ if (bNumber) pSchedule->nOffFrom = n; }
	else if (strcasecmp (pName, "Eteint_Fin") == 0)		{ if (bNumber) pSchedule->nOffTo = n; }
	else if (strcasecmp (pName, "Heure_Active") == 0)	{ if (bNumber) pSchedule->bClockOnly = n != 0; }
	else if (strcasecmp (pName, "Heure_Debut") == 0)	{ if (bNumber) pSchedule->nClockFrom = n; }
	else if (strcasecmp (pName, "Heure_Fin") == 0)		{ if (bNumber) pSchedule->nClockTo = n; }
	else
	{
		return FALSE;
	}

	return TRUE;
}

boolean InHourWindow (unsigned nFrom, unsigned nTo, unsigned nHour)
{
	if (nFrom == nTo)
	{
		return FALSE;
	}

	return nFrom < nTo ? nFrom <= nHour && nHour < nTo
			   : nHour >= nFrom || nHour < nTo;	// passe minuit
}

// "12", "11,12", "6-8", "11-2" -> bits des mois
static boolean ParseMonths (const char *pText, size_t nLength, u16 *pMask)
{
	u16 nMask = 0;
	size_t i = 0;
	while (i < nLength)
	{
		char Part[8];
		size_t n = 0;
		while (i < nLength && pText[i] != ',' && n < sizeof Part - 1)
		{
			Part[n++] = pText[i++];
		}
		Part[n] = '\0';
		if (i < nLength && pText[i] != ',')
		{
			return FALSE;
		}
		i++;

		unsigned nFirst, nLast;
		char *pDash = strchr (Part, '-');
		if (pDash != 0)
		{
			*pDash = '\0';
			if (!ParseSmall (Part, 2, 12, &nFirst) || !ParseSmall (pDash + 1, 2, 12, &nLast))
			{
				return FALSE;
			}
		}
		else if (!ParseSmall (Part, 2, 12, &nFirst))
		{
			return FALSE;
		}
		else
		{
			nLast = nFirst;
		}
		if (nFirst == 0 || nLast == 0)
		{
			return FALSE;
		}

		for (unsigned m = nFirst; ; m = m % 12 + 1)	// "11-2" : novembre a fevrier
		{
			nMask |= 1 << m;
			if (m == nLast)
			{
				break;
			}
		}
	}

	*pMask = nMask;
	return nMask != 0;
}

boolean ParseCategoryMonths (const char *pValue, TSchedule *pSchedule)
{
	if (strlen (pValue) >= SCHEDULE_RAW_LEN)
	{
		return FALSE;
	}

	TSchedule New = *pSchedule;
	New.nRules = 0;

	const char *p = pValue;
	while (*p != '\0')
	{
		const char *pEnd = strchr (p, ';');
		size_t nLength = pEnd != 0 ? (size_t) (pEnd - p) : strlen (p);

		const char *pColon = 0;
		for (size_t i = 0; i < nLength && pColon == 0; i++)
		{
			if (p[i] == ':')
			{
				pColon = p + i;
			}
		}
		size_t nName = pColon != 0 ? (size_t) (pColon - p) : 0;
		if (nLength > 0)
		{
			if (   pColon == 0 || nName == 0 || nName > 32
			    || New.nRules >= SCHEDULE_MAX_RULES)
			{
				return FALSE;
			}
			for (size_t i = 0; i < nName; i++)
			{
				char c = p[i];
				if (!(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')
				      || c == '_' || c == '-'))
				{
					return FALSE;
				}
			}

			u16 nMask;
			if (!ParseMonths (pColon + 1, nLength - nName - 1, &nMask))
			{
				return FALSE;
			}
			memcpy (New.RuleCategory[New.nRules], p, nName);
			New.RuleCategory[New.nRules][nName] = '\0';
			New.RuleMonths[New.nRules] = nMask;
			New.nRules++;
		}

		p += nLength;
		if (*p == ';')
		{
			p++;
		}
	}

	strcpy (New.MonthsRaw, pValue);
	*pSchedule = New;

	return TRUE;
}

u16 GetCategoryMonths (const TSchedule &rSchedule, const char *pCategory)
{
	for (unsigned i = 0; i < rSchedule.nRules; i++)
	{
		if (strcasecmp (rSchedule.RuleCategory[i], pCategory) == 0)
		{
			return rSchedule.RuleMonths[i];
		}
	}

	return SCHEDULE_ALL_MONTHS;
}

boolean IsValidMonthDay (const char *pValue)
{
	if (*pValue == '\0')
	{
		return TRUE;
	}

	static const unsigned Days[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	char Month[3] = {pValue[0], pValue[1], '\0'};
	unsigned nMonth, nDay;

	return    strlen (pValue) == 5 && pValue[2] == '-'
	       && ParseSmall (Month, 2, 12, &nMonth) && nMonth >= 1
	       && ParseSmall (pValue + 3, 2, 31, &nDay) && nDay >= 1 && nDay <= Days[nMonth];
}

boolean InDateWindow (const char *pFrom, const char *pTo, const TDateTime &rNow)
{
	if (   (*pFrom == '\0' && *pTo == '\0')
	    || !IsValidMonthDay (pFrom) || !IsValidMonthDay (pTo))
	{
		return TRUE;
	}

	// MMJJ en nombre : 1225 pour le 25 decembre
	unsigned nNow = rNow.Month * 100 + rNow.Day;
	unsigned nFrom = *pFrom != '\0' ? (unsigned) ((pFrom[0] - '0') * 1000 + (pFrom[1] - '0') * 100
						      + (pFrom[3] - '0') * 10 + (pFrom[4] - '0')) : 101;
	unsigned nTo = *pTo != '\0' ? (unsigned) ((pTo[0] - '0') * 1000 + (pTo[1] - '0') * 100
					       + (pTo[3] - '0') * 10 + (pTo[4] - '0')) : 1231;

	return nFrom <= nTo ? nFrom <= nNow && nNow <= nTo
			    : nNow >= nFrom || nNow <= nTo;		// passe le 1er janvier
}
