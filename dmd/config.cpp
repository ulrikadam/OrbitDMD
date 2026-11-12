//
// config.cpp
//
#include "config.h"
#include "lang.h"
#include <fatfs/ff.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "config";

#define MAX_FILE_SIZE	16384

static void CopyString (char *pDest, const char *pSrc, unsigned nSize)
{
	strncpy (pDest, pSrc, nSize-1);
	pDest[nSize-1] = '\0';
}

static char *Trim (char *pString)
{
	while (*pString == ' ' || *pString == '\t')
	{
		pString++;
	}

	char *pEnd = pString + strlen (pString);
	while (   pEnd > pString
	       && (pEnd[-1] == ' ' || pEnd[-1] == '\t' || pEnd[-1] == '\r'))
	{
		*--pEnd = '\0';
	}

	return pString;
}

CConfig::CConfig (void)
:	PanelCols (64),
	PanelRows (32),
	Chain (2),
	Parallel (1),
	Slowdown (2),
	PWMBits (11),
	PWMLSBNanos (130),
	GifActive (TRUE),
	nCategories (0),
	GifMinDuration (4000),
	IPScreenTime (10000),
	ClockActive (TRUE),
	ClockDisplayTime (3000),
	TestPattern (TRUE),
	LogToScreen (FALSE)
{
	CopyString (RGBOrder, "RGB", sizeof RGBOrder);
	CopyString (TimeZone, "Europe/Paris", sizeof TimeZone);
	CopyString (NetMode, "auto", sizeof NetMode);
	WifiSSID[0] = '\0';
	WifiPSK[0] = '\0';
	CopyString (WifiCountry, "FR", sizeof WifiCountry);
	CopyString (NetworkName, "OrbitDMD", sizeof NetworkName);
	WebPassword[0] = '\0';
	IPAddress[0] = '\0';
	IPMask[0] = '\0';
	IPGateway[0] = '\0';
	IPDNS[0] = '\0';

	for (unsigned i = 0; i < 24; i++)
	{
		Brightness[i] = 60;
	}

	for (unsigned i = 0; i < CONFIG_MAX_CATEGORIES; i++)
	{
		m_RawCategory[i][0] = '\0';
		Categories[i][0] = '\0';
	}

	ClockStyleDefaults (&ClockStyle);
	WeatherStyleDefaults (&WeatherStyle);
	BannerStyleDefaults (&Banner);
	CountdownStyleDefaults (&Countdown);
	GifInOrder = FALSE;
	ScheduleDefaults (&Schedule);
	memset (&Mqtt, 0, sizeof Mqtt);
	Mqtt.nPort = 1883;
}

boolean CConfig::Load (const char *pFileName)
{
	FIL File;
	if (f_open (&File, pFileName, FA_READ | FA_OPEN_EXISTING) != FR_OK)
	{
		CLogger::Get ()->Write (From, LogWarning, "%s introuvable, reglages par defaut",
					pFileName);
		return FALSE;
	}

	char *pBuffer = new char[MAX_FILE_SIZE+1];
	UINT nBytesRead = 0;
	FRESULT Result = f_read (&File, pBuffer, MAX_FILE_SIZE, &nBytesRead);
	f_close (&File);
	if (Result != FR_OK)
	{
		delete [] pBuffer;
		CLogger::Get ()->Write (From, LogError, "Erreur de lecture de %s", pFileName);
		return FALSE;
	}
	pBuffer[nBytesRead] = '\0';

	// en cas de rechargement : oublier les anciennes categories et l'ancienne apparence de l'heure
	for (unsigned i = 0; i < CONFIG_MAX_CATEGORIES; i++)
	{
		m_RawCategory[i][0] = '\0';
	}
	ClockStyleDefaults (&ClockStyle);
	WeatherStyleDefaults (&WeatherStyle);
	BannerStyleDefaults (&Banner);
	CountdownStyleDefaults (&Countdown);
	GifInOrder = FALSE;
	ScheduleDefaults (&Schedule);
	memset (&Mqtt, 0, sizeof Mqtt);
	Mqtt.nPort = 1883;

	char *pLine = pBuffer;
	while (pLine != 0 && *pLine != '\0')
	{
		char *pNext = strchr (pLine, '\n');
		if (pNext != 0)
		{
			*pNext++ = '\0';
		}

		char *pText = Trim (pLine);
		if (*pText != '\0' && *pText != '#')
		{
			char *pEqual = strchr (pText, '=');
			if (pEqual != 0)
			{
				*pEqual = '\0';
				SetValue (Trim (pText), Trim (pEqual+1));
			}
		}

		pLine = pNext;
	}

	delete [] pBuffer;

	// categories actives, dans l'ordre Gif_Cat_1, Gif_Cat_2, ...
	nCategories = 0;
	for (unsigned i = 0; i < CONFIG_MAX_CATEGORIES; i++)
	{
		if (   m_RawCategory[i][0] != '\0'
		    && strcasecmp (m_RawCategory[i], "disabled") != 0)
		{
			CopyString (Categories[nCategories++], m_RawCategory[i], CONFIG_NAME_LEN);
		}
	}

	return TRUE;
}

void CConfig::SetValue (const char *pKey, const char *pValue)
{
	unsigned nValue = (unsigned) atoi (pValue);

	if      (strcasecmp (pKey, "Panel_XSize") == 0)		PanelCols = nValue;
	else if (strcasecmp (pKey, "Panel_YSize") == 0)		PanelRows = nValue;
	else if (strcasecmp (pKey, "Panel_XNumber") == 0)	Chain = nValue;
	else if (strcasecmp (pKey, "Panel_YNumber") == 0)	Parallel = nValue;
	else if (strcasecmp (pKey, "GPIO_Slowdown") == 0)	Slowdown = nValue;
	else if (strcasecmp (pKey, "PWM_Bits") == 0)		PWMBits = nValue;
	else if (strcasecmp (pKey, "PWM_LSB_Nanoseconds") == 0)	PWMLSBNanos = nValue;
	else if (strcasecmp (pKey, "RGB_Order") == 0)		CopyString (RGBOrder, pValue, sizeof RGBOrder);
	else if (strcasecmp (pKey, "Langue") == 0)
	{
		unsigned nLanguage;
		if (ParseLanguage (pValue, &nLanguage))
		{
			SetLanguage (nLanguage);
		}
	}
	else if (strcasecmp (pKey, "Gif_Active") == 0)		GifActive = nValue != 0;
	else if (strcasecmp (pKey, "Gif_Min_Duration") == 0)	GifMinDuration = nValue;
	else if (strcasecmp (pKey, "IP_Ecran") == 0)		IPScreenTime = nValue;
	else if (strcasecmp (pKey, "Clock_Active") == 0)	ClockActive = nValue != 0;
	else if (strcasecmp (pKey, "Clock_Display_Time") == 0)	ClockDisplayTime = nValue;
	else if (strcasecmp (pKey, "Clock_TZ") == 0)		CopyString (TimeZone, pValue, sizeof TimeZone);
	else if (strcasecmp (pKey, "Test_Pattern") == 0)	TestPattern = nValue != 0;
	else if (strcasecmp (pKey, "Journal_Ecran") == 0)	LogToScreen = nValue != 0;
	else if (strcasecmp (pKey, "Reseau") == 0)		CopyString (NetMode, pValue, sizeof NetMode);
	else if (strcasecmp (pKey, "Wifi_ssid") == 0)		CopyString (WifiSSID, pValue, sizeof WifiSSID);
	else if (strcasecmp (pKey, "Wifi_psk") == 0)		CopyString (WifiPSK, pValue, sizeof WifiPSK);
	else if (strcasecmp (pKey, "Wifi_country") == 0)	CopyString (WifiCountry, pValue, sizeof WifiCountry);
	else if (strcasecmp (pKey, "Network_name") == 0)
	{
		if (*pValue != '\0')
		{
			CopyString (NetworkName, pValue, sizeof NetworkName);
		}
	}
	else if (strcasecmp (pKey, "Web_Password") == 0)	CopyString (WebPassword, pValue, sizeof WebPassword);
	else if (strcasecmp (pKey, "IP_Fixe") == 0)		CopyString (IPAddress, pValue, sizeof IPAddress);
	else if (strcasecmp (pKey, "IP_Masque") == 0)		CopyString (IPMask, pValue, sizeof IPMask);
	else if (strcasecmp (pKey, "IP_Passerelle") == 0)	CopyString (IPGateway, pValue, sizeof IPGateway);
	else if (strcasecmp (pKey, "IP_DNS") == 0)		CopyString (IPDNS, pValue, sizeof IPDNS);
	else if (strncasecmp (pKey, "Panel_Brightness_", 17) == 0)
	{
		unsigned nHour = (unsigned) atoi (pKey + 17);
		if (nHour < 24)
		{
			Brightness[nHour] = nValue > 100 ? 100 : nValue;
		}
	}
	else if (strncasecmp (pKey, "Gif_Cat_", 8) == 0)
	{
		unsigned nIndex = (unsigned) atoi (pKey + 8);
		if (1 <= nIndex && nIndex <= CONFIG_MAX_CATEGORIES)
		{
			CopyString (m_RawCategory[nIndex-1], pValue, CONFIG_NAME_LEN);
		}
	}
	// police, couleurs, formats de l'heure et de la date
	else if (SetClockStyleValue (&ClockStyle, pKey, pValue))
	{
	}
	else if (SetWeatherStyleValue (&WeatherStyle, pKey, pValue))
	{
	}
	else if (SetBannerStyleValue (&Banner, pKey, pValue))
	{
	}
	else if (SetCountdownStyleValue (&Countdown, pKey, pValue))
	{
	}
	else if (strcasecmp (pKey, "HA_Active") == 0)		Mqtt.bActive = nValue != 0;
	else if (strcasecmp (pKey, "HA_Serveur") == 0)		CopyString (Mqtt.Host, pValue, sizeof Mqtt.Host);
	else if (strcasecmp (pKey, "HA_Port") == 0)		{ if (1 <= nValue && nValue <= 65535) Mqtt.nPort = nValue; }
	else if (strcasecmp (pKey, "HA_Utilisateur") == 0)	CopyString (Mqtt.User, pValue, sizeof Mqtt.User);
	else if (strcasecmp (pKey, "HA_MotDePasse") == 0)	CopyString (Mqtt.Password, pValue, sizeof Mqtt.Password);
	else if (SetScheduleValue (&Schedule, pKey, pValue))
	{
	}
	else if (strcasecmp (pKey, "Gif_Ordre") == 0)
	{
		GifInOrder = strcasecmp (pValue, "ordre") == 0;
	}
	// les autres reglages (meteo...) seront geres plus tard
}

unsigned CConfig::GetBrightness (unsigned nHour) const
{
	return Brightness[nHour % 24];
}

void CConfig::Log (void) const
{
	CLogger *pLogger = CLogger::Get ();

	pLogger->Write (From, LogNotice, "Panneaux %ux%u, chaine %u, parallele %u, slowdown %u",
			PanelCols, PanelRows, Chain, Parallel, Slowdown);
	pLogger->Write (From, LogNotice, "PWM %u bits, LSB %u ns, ordre %s",
			PWMBits, PWMLSBNanos, RGBOrder);
	pLogger->Write (From, LogNotice, "GIF %s, %u categorie(s), duree mini %u ms",
			GifActive ? "oui" : "non", nCategories, GifMinDuration);
	for (unsigned i = 0; i < nCategories; i++)
	{
		pLogger->Write (From, LogNotice, "  categorie %u : %s", i+1, Categories[i]);
	}
	pLogger->Write (From, LogNotice, "Horloge %s (%u ms), fuseau %s, mire %s",
			ClockActive ? "oui" : "non", ClockDisplayTime, TimeZone,
			TestPattern ? "oui" : "non");
	pLogger->Write (From, LogNotice, "Heure : police %s (%u), motif %s, fond %s ; date : %s",
			ClockStyle.Time.Font, ClockStyle.Time.nSize, ClockStyle.Time.Pattern,
			ClockStyle.Background,
			ClockStyle.DateMode == DateOff ? "non" : ClockStyle.DateMode == DateScreen ? "apres l'heure" : "sous l'heure");
	pLogger->Write (From, LogNotice, "Meteo %s, %s", WeatherStyle.bActive ? "oui" : "non",
			WeatherStyle.Latitude[0] != '\0' ? "lieu choisi" : "lieu pas encore choisi");
	pLogger->Write (From, LogNotice, "Wi-Fi %s, nom reseau %s, mot de passe web %s",
			WifiSSID[0] != '\0' ? "configure" : "non configure", NetworkName,
			WebPassword[0] != '\0' ? "defini" : "NON DEFINI");
	pLogger->Write (From, LogNotice, "Adresse IP : %s", IPAddress[0] != '\0' ? IPAddress : "automatique (DHCP)");
}
