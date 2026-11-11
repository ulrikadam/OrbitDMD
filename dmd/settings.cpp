//
// settings.cpp
//
#include "settings.h"
#include "common.h"
#include "configedit.h"
#include "netutil.h"
#include "network.h"
#include "fileutil.h"
#include "remote.h"
#include "clock.h"
#include <fatfs/ff.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "reglages";

#define MAX_CONFIG_SIZE		16384
#define MAX_CHANGES		48

enum TKind
{
	KindNumber,	// entier entre nMin et nMax
	KindBool,	// 0 ou 1
	KindChoice,	// une valeur de pChoices (liste separee par des virgules)
	KindZone,	// fuseau horaire de s_Zones
	KindHostname,	// lettres, chiffres, tiret
	KindSSID,	// nom de reseau Wi-Fi
	KindPSK,	// mot de passe Wi-Fi (vide = inchange)
	KindCountry,	// code pays a 2 lettres
	KindPassword,	// mot de passe de la page web (vide = inchange)
	KindIP,		// adresse IPv4 (vide autorise : adresse automatique)
	KindFont,	// "Pixel" ou police TTF/OTF existante de SD:/fonts
	KindImage,	// "None" ou image PNG/GIF existante de SD:/fonts
	KindColor,	// couleur RRGGBB
	KindFormat,	// format d'heure ou de date (%H:%M...)
	KindCoord,	// latitude ou longitude (vide autorise : pas de lieu)
	KindText	// nom de lieu (UTF-8, sans guillemet)
};

enum TApply
{
	ApplyLive,	// pris en compte tout de suite
	ApplyNextBoot,	// au prochain demarrage
	ApplyReboot,	// redemarrage necessaire
	ApplyWifi	// redemarrage avec essai du nouveau Wi-Fi
};

struct TSetting
{
	const char *pKey;
	const char *pLabel;
	TKind	    Kind;
	int	    nMin;
	int	    nMax;
	const char *pChoices;
	TApply	    Apply;
};

static const TSetting s_Settings[] =
{
	{"Gif_Active",		"GIF affichees",	KindBool,	0, 1,	  0,			ApplyLive},
	{"Gif_Min_Duration",	"Duree minimale",	KindNumber,	1000, 60000, 0,			ApplyLive},
	{"Clock_Active",	"Horloge",		KindBool,	0, 1,	  0,			ApplyLive},
	{"Clock_Display_Time",	"Duree de l'horloge",	KindNumber,	1000, 60000, 0,			ApplyLive},
	{"Clock_TZ",		"Fuseau horaire",	KindZone,	0, 0,	  0,			ApplyLive},
	{"Clock_Format",	"Format de l'heure",	KindFormat,	0, 0,	  0,			ApplyLive},
	{"Clock_Font",		"Police de l'heure",	KindFont,	0, 0,	  0,			ApplyLive},
	{"Clock_Font_Size",	"Taille de l'heure",	KindNumber,	0, CLOCK_MAX_SIZE, 0,		ApplyLive},
	{"Clock_Color",		"Couleur de l'heure",	KindColor,	0, 0,	  0,			ApplyLive},
	{"Clock_Font_Pattern",	"Motif de l'heure",	KindImage,	0, 0,	  0,			ApplyLive},
	{"Clock_Background",	"Fond",			KindImage,	0, 0,	  0,			ApplyLive},
	{"Clock_Blink",		"Deux-points clignotants", KindBool,	0, 1,	  0,			ApplyLive},
	{"Clock_Smooth",	"Bords adoucis",	KindBool,	0, 1,	  0,			ApplyLive},
	{"Date_Active",		"Date",			KindNumber,	0, 2,	  0,			ApplyLive},
	{"Date_Display_Time",	"Duree de la date",	KindNumber,	1000, 60000, 0,			ApplyLive},
	{"Date_Format",		"Format de la date",	KindFormat,	0, 0,	  0,			ApplyLive},
	{"Date_Font",		"Police de la date",	KindFont,	0, 0,	  0,			ApplyLive},
	{"Date_Font_Size",	"Taille de la date",	KindNumber,	0, CLOCK_MAX_SIZE, 0,		ApplyLive},
	{"Date_Color",		"Couleur de la date",	KindColor,	0, 0,	  0,			ApplyLive},
	{"Date_Font_Pattern",	"Motif de la date",	KindImage,	0, 0,	  0,			ApplyLive},
	{"Weather_Active",	"Meteo",		KindBool,	0, 1,	  0,			ApplyLive},
	{"Weather_Display_Time", "Duree de la meteo",	KindNumber,	1000, 60000, 0,			ApplyLive},
	{"Weather_Every",	"Frequence de la meteo", KindNumber,	1, 20,	  0,			ApplyLive},
	{"Weather_Place",	"Lieu",			KindText,	0, 0,	  0,			ApplyLive},
	{"Weather_Latitude",	"Latitude",		KindCoord,	0, 0,	  0,			ApplyLive},
	{"Weather_Longitude",	"Longitude",		KindCoord,	0, 0,	  0,			ApplyLive},
	{"Weather_Font",	"Police de la meteo",	KindFont,	0, 0,	  0,			ApplyLive},
	{"Weather_Font_Size",	"Taille de la temperature", KindNumber, 0, CLOCK_MAX_SIZE, 0,		ApplyLive},
	{"Weather_Font_Pattern", "Motif de la temperature", KindImage,	0, 0,	  0,			ApplyLive},
	{"Weather_Text_color",	"Couleur de la temperature", KindColor, 0, 0,	  0,			ApplyLive},
	{"Weather_Info",	"Deuxieme ligne",	KindNumber,	0, WEATHER_INFO_MAX, 0,		ApplyLive},
	{"Weather_Info_Color",	"Couleur de la deuxieme ligne", KindColor, 0, 0, 0,			ApplyLive},
	{"Weather_Icon",	"Icone meteo",		KindBool,	0, 1,	  0,			ApplyLive},
	{"Weather_Animate",	"Icone animee",		KindBool,	0, 1,	  0,			ApplyLive},
	{"Test_Pattern",	"Mire de test",		KindBool,	0, 1,	  0,			ApplyNextBoot},
	{"Journal_Ecran",	"Journal sur l'ecran",	KindBool,	0, 1,	  0,			ApplyNextBoot},
	{"Panel_XSize",		"Largeur d'un panneau",	KindChoice,	0, 0,	  "16,32,64,128",	ApplyReboot},
	{"Panel_YSize",		"Hauteur d'un panneau",	KindChoice,	0, 0,	  "16,32,64",		ApplyReboot},
	{"Panel_XNumber",	"Panneaux a la suite",	KindNumber,	1, 8,	  0,			ApplyReboot},
	{"Panel_YNumber",	"Sorties en parallele",	KindNumber,	1, 3,	  0,			ApplyReboot},
	{"GPIO_Slowdown",	"Ralentissement GPIO",	KindNumber,	0, 4,	  0,			ApplyReboot},
	{"RGB_Order",		"Ordre des couleurs",	KindChoice,	0, 0,	  "RGB,RBG,GRB,GBR,BRG,BGR", ApplyReboot},
	{"PWM_Bits",		"Bits de couleur",	KindNumber,	1, 11,	  0,			ApplyReboot},
	{"PWM_LSB_Nanoseconds",	"Duree LSB",		KindNumber,	50, 3000, 0,			ApplyReboot},
	{"Network_name",	"Nom reseau",		KindHostname,	0, 0,	  0,			ApplyReboot},
	{"Wifi_ssid",		"Reseau Wi-Fi",		KindSSID,	0, 0,	  0,			ApplyWifi},
	{"Wifi_psk",		"Mot de passe Wi-Fi",	KindPSK,	0, 0,	  0,			ApplyWifi},
	{"Wifi_country",	"Pays Wi-Fi",		KindCountry,	0, 0,	  0,			ApplyWifi},
	{"Web_Password",	"Mot de passe web",	KindPassword,	0, 0,	  0,			ApplyLive},
	{"IP_Fixe",		"Adresse IP fixe",	KindIP,		0, 0,	  0,			ApplyWifi},
	{"IP_Masque",		"Masque",		KindIP,		0, 0,	  0,			ApplyWifi},
	{"IP_Passerelle",	"Passerelle",		KindIP,		0, 0,	  0,			ApplyWifi},
	{"IP_DNS",		"Serveur DNS",		KindIP,		0, 0,	  0,			ApplyWifi},
};

// fuseaux proposes (heure d'Europe centrale par defaut, voir localtime.cpp)
static const char *s_Zones[] =
{
	"Europe/Paris", "Europe/Brussels", "Europe/Luxembourg", "Europe/Zurich", "Europe/Monaco",
	"Europe/Berlin", "Europe/Amsterdam", "Europe/Madrid", "Europe/Rome", "Europe/Vienna",
	"Europe/London", "Europe/Dublin", "Europe/Lisbon",
	"Europe/Helsinki", "Europe/Athens", "Europe/Bucharest", "Europe/Sofia", "Europe/Kyiv",
	"Europe/Istanbul", "Europe/Moscow", "UTC"
};

static const TSetting s_Brightness =
	{"Panel_Brightness_", "Luminosite", KindNumber, 0, 100, 0, ApplyLive};

// --- outils ---

static void AppendJSONString (CString *pJSON, const char *pText)
{
	pJSON->Append ('"');
	for (; *pText != '\0'; pText++)
	{
		if (*pText == '"' || *pText == '\\')
		{
			pJSON->Append ('\\');
			pJSON->Append (*pText);
		}
		else if ((unsigned char) *pText >= ' ')
		{
			pJSON->Append (*pText);
		}
	}
	pJSON->Append ('"');
}

static boolean ParseInt (const char *pValue, int *pResult)
{
	if (*pValue == '\0' || strlen (pValue) > 9)
	{
		return FALSE;
	}

	int nValue = 0;
	for (const char *p = pValue; *p != '\0'; p++)
	{
		if (*p < '0' || *p > '9')
		{
			return FALSE;
		}
		nValue = nValue * 10 + (*p - '0');
	}

	*pResult = nValue;

	return TRUE;
}

static boolean InList (const char *pList, const char *pValue)
{
	size_t nLength = strlen (pValue);
	const char *p = pList;
	while (*p != '\0')
	{
		const char *pComma = strchr (p, ',');
		size_t nItem = pComma != 0 ? (size_t) (pComma - p) : strlen (p);
		if (nItem == nLength && strncmp (p, pValue, nLength) == 0)
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

static boolean IsPrintable (const char *pValue, boolean bAllowSpace)
{
	for (; *pValue != '\0'; pValue++)
	{
		unsigned char c = (unsigned char) *pValue;
		if (c < ' ' || c > '~' || c == '"' || (!bAllowSpace && c == ' '))
		{
			return FALSE;
		}
	}

	return TRUE;
}

static const TSetting *FindSetting (const char *pKey)
{
	for (unsigned i = 0; i < sizeof s_Settings / sizeof s_Settings[0]; i++)
	{
		if (strcasecmp (pKey, s_Settings[i].pKey) == 0)
		{
			return &s_Settings[i];
		}
	}

	// Panel_Brightness_0 a Panel_Brightness_23
	int nHour;
	if (   strncasecmp (pKey, s_Brightness.pKey, 17) == 0
	    && ParseInt (pKey + 17, &nHour) && nHour < 24)
	{
		return &s_Brightness;
	}

	return 0;
}

// valeur actuelle d'un reglage, en texte
static void GetCurrent (CConfig *pConfig, const char *pKey, CString *pValue)
{
	unsigned n = 0;
	const char *pText = 0;

	if      (strcasecmp (pKey, "Gif_Active") == 0)		n = pConfig->GifActive ? 1 : 0;
	else if (strcasecmp (pKey, "Gif_Min_Duration") == 0)	n = pConfig->GifMinDuration;
	else if (strcasecmp (pKey, "Clock_Active") == 0)	n = pConfig->ClockActive ? 1 : 0;
	else if (strcasecmp (pKey, "Clock_Display_Time") == 0)	n = pConfig->ClockDisplayTime;
	else if (strcasecmp (pKey, "Clock_TZ") == 0)		pText = pConfig->TimeZone;
	else if (strncasecmp (pKey, "Weather_", 8) == 0)
	{
		const TWeatherStyle &S = pConfig->WeatherStyle;
		const char *pName = pKey + 8;
		const u8 *pColor = 0;

		if      (strcasecmp (pName, "Active") == 0)		n = S.bActive ? 1 : 0;
		else if (strcasecmp (pName, "Display_Time") == 0)	n = S.nDisplayTime;
		else if (strcasecmp (pName, "Every") == 0)		n = S.nEvery;
		else if (strcasecmp (pName, "Place") == 0)		pText = S.Place;
		else if (strcasecmp (pName, "Latitude") == 0)		pText = S.Latitude;
		else if (strcasecmp (pName, "Longitude") == 0)		pText = S.Longitude;
		else if (strcasecmp (pName, "Font") == 0)		pText = S.Text.Font;
		else if (strcasecmp (pName, "Font_Size") == 0)		n = S.Text.nSize;
		else if (strcasecmp (pName, "Font_Pattern") == 0)	pText = S.Text.Pattern;
		else if (strcasecmp (pName, "Info") == 0)		n = S.Info;
		else if (strcasecmp (pName, "Icon") == 0)		n = S.bIcon ? 1 : 0;
		else if (strcasecmp (pName, "Animate") == 0)		n = S.bAnimate ? 1 : 0;
		else if (strcasecmp (pName, "Text_color") == 0)	pColor = S.Text.Color;
		else if (strcasecmp (pName, "Info_Color") == 0)		pColor = S.InfoColor;

		if (pColor != 0)
		{
			pValue->Format ("%02X%02X%02X", pColor[0], pColor[1], pColor[2]);
			return;
		}
	}
	else if (   strncasecmp (pKey, "Clock_", 6) == 0
		 || strncasecmp (pKey, "Date_", 5) == 0)
	{
		const TClockStyle &S = pConfig->ClockStyle;
		boolean bDate = strncasecmp (pKey, "Date_", 5) == 0;
		const TTextStyle &T = bDate ? S.Date : S.Time;
		const char *pName = pKey + (bDate ? 5 : 6);

		if      (strcasecmp (pName, "Format") == 0)		pText = T.Format;
		else if (strcasecmp (pName, "Font") == 0)		pText = T.Font;
		else if (strcasecmp (pName, "Font_Size") == 0)		n = T.nSize;
		else if (strcasecmp (pName, "Font_Pattern") == 0)	pText = T.Pattern;
		else if (strcasecmp (pName, "Background") == 0)		pText = S.Background;
		else if (strcasecmp (pName, "Blink") == 0)		n = S.bBlink ? 1 : 0;
		else if (strcasecmp (pName, "Smooth") == 0)		n = S.bSmooth ? 1 : 0;
		else if (strcasecmp (pName, "Active") == 0)		n = S.DateMode;
		else if (strcasecmp (pName, "Display_Time") == 0)	n = S.nDateDisplayTime;
		else if (strcasecmp (pName, "Color") == 0)
		{
			pValue->Format ("%02X%02X%02X", T.Color[0], T.Color[1], T.Color[2]);
			return;
		}
	}
	else if (strcasecmp (pKey, "Test_Pattern") == 0)	n = pConfig->TestPattern ? 1 : 0;
	else if (strcasecmp (pKey, "Journal_Ecran") == 0)	n = pConfig->LogToScreen ? 1 : 0;
	else if (strcasecmp (pKey, "Panel_XSize") == 0)		n = pConfig->PanelCols;
	else if (strcasecmp (pKey, "Panel_YSize") == 0)		n = pConfig->PanelRows;
	else if (strcasecmp (pKey, "Panel_XNumber") == 0)	n = pConfig->Chain;
	else if (strcasecmp (pKey, "Panel_YNumber") == 0)	n = pConfig->Parallel;
	else if (strcasecmp (pKey, "GPIO_Slowdown") == 0)	n = pConfig->Slowdown;
	else if (strcasecmp (pKey, "RGB_Order") == 0)		pText = pConfig->RGBOrder;
	else if (strcasecmp (pKey, "PWM_Bits") == 0)		n = pConfig->PWMBits;
	else if (strcasecmp (pKey, "PWM_LSB_Nanoseconds") == 0)	n = pConfig->PWMLSBNanos;
	else if (strcasecmp (pKey, "Network_name") == 0)	pText = pConfig->NetworkName;
	else if (strcasecmp (pKey, "Wifi_ssid") == 0)		pText = pConfig->WifiSSID;
	else if (strcasecmp (pKey, "Wifi_psk") == 0)		pText = pConfig->WifiPSK;
	else if (strcasecmp (pKey, "Wifi_country") == 0)	pText = pConfig->WifiCountry;
	else if (strcasecmp (pKey, "Web_Password") == 0)	pText = pConfig->WebPassword;
	else if (strcasecmp (pKey, "IP_Fixe") == 0)		pText = pConfig->IPAddress;
	else if (strcasecmp (pKey, "IP_Masque") == 0)		pText = pConfig->IPMask;
	else if (strcasecmp (pKey, "IP_Passerelle") == 0)	pText = pConfig->IPGateway;
	else if (strcasecmp (pKey, "IP_DNS") == 0)		pText = pConfig->IPDNS;
	else if (strncasecmp (pKey, "Panel_Brightness_", 17) == 0)
	{
		n = pConfig->GetBrightness ((unsigned) atoi (pKey + 17));
	}

	if (pText != 0)
	{
		*pValue = pText;
	}
	else
	{
		pValue->Format ("%u", n);
	}
}

// verifie et normalise une valeur ; FALSE si elle est invalide
static boolean Normalize (const TSetting *pSetting, const char *pIn, CString *pOut)
{
	int nValue;
	size_t nLength = strlen (pIn);

	switch (pSetting->Kind)
	{
	case KindNumber:
		if (!ParseInt (pIn, &nValue) || nValue < pSetting->nMin || nValue > pSetting->nMax)
		{
			return FALSE;
		}
		pOut->Format ("%d", nValue);
		return TRUE;

	case KindBool:
		if (   strcmp (pIn, "1") == 0 || strcasecmp (pIn, "on") == 0
		    || strcasecmp (pIn, "true") == 0 || strcasecmp (pIn, "oui") == 0)
		{
			*pOut = "1";
			return TRUE;
		}
		if (   strcmp (pIn, "0") == 0 || strcasecmp (pIn, "off") == 0
		    || strcasecmp (pIn, "false") == 0 || strcasecmp (pIn, "non") == 0)
		{
			*pOut = "0";
			return TRUE;
		}
		return FALSE;

	case KindChoice: {
		char Upper[16];
		if (nLength == 0 || nLength >= sizeof Upper)
		{
			return FALSE;
		}
		for (size_t i = 0; i <= nLength; i++)
		{
			char c = pIn[i];
			Upper[i] = ('a' <= c && c <= 'z') ? c - 'a' + 'A' : c;
		}
		if (!InList (pSetting->pChoices, Upper))
		{
			return FALSE;
		}
		*pOut = Upper;
		return TRUE; }

	case KindZone:
		for (unsigned i = 0; i < sizeof s_Zones / sizeof s_Zones[0]; i++)
		{
			if (strcmp (pIn, s_Zones[i]) == 0)
			{
				*pOut = pIn;
				return TRUE;
			}
		}
		return FALSE;

	case KindHostname:
		if (nLength == 0 || nLength > 31 || pIn[0] == '-' || pIn[nLength-1] == '-')
		{
			return FALSE;
		}
		for (const char *p = pIn; *p != '\0'; p++)
		{
			char c = *p;
			if (!(('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '-'))
			{
				return FALSE;
			}
		}
		*pOut = pIn;
		return TRUE;

	case KindSSID:
		if (   nLength == 0 || nLength > 32 || !IsPrintable (pIn, TRUE)
		    || pIn[0] == ' ' || pIn[nLength-1] == ' ')
		{
			return FALSE;
		}
		*pOut = pIn;
		return TRUE;

	case KindPSK:
		if (nLength < 8 || nLength > 63 || !IsPrintable (pIn, TRUE) || pIn[0] == ' ' || pIn[nLength-1] == ' ')
		{
			return FALSE;
		}
		*pOut = pIn;
		return TRUE;

	case KindCountry:
		if (   nLength != 2
		    || !(('A' <= (pIn[0] & ~0x20) && (pIn[0] & ~0x20) <= 'Z'))
		    || !(('A' <= (pIn[1] & ~0x20) && (pIn[1] & ~0x20) <= 'Z')))
		{
			return FALSE;
		}
		{
			char Code[3] = {(char) (pIn[0] & ~0x20), (char) (pIn[1] & ~0x20), '\0'};
			*pOut = Code;
		}
		return TRUE;

	case KindPassword:
		if (nLength < 8 || nLength > 63 || !IsPrintable (pIn, FALSE))
		{
			return FALSE;
		}
		*pOut = pIn;
		return TRUE;

	case KindIP: {
		if (nLength == 0)
		{
			*pOut = "";
			return TRUE;
		}
		u8 Address[4];
		if (!ParseIPv4 (pIn, Address))
		{
			return FALSE;
		}
		pOut->Format ("%u.%u.%u.%u", Address[0], Address[1], Address[2], Address[3]);
		return TRUE; }

	case KindFont:
	case KindImage: {
		boolean bFont = pSetting->Kind == KindFont;
		if (bFont && strcasecmp (pIn, CLOCK_FONT_PIXEL) == 0)
		{
			*pOut = CLOCK_FONT_PIXEL;
			return TRUE;
		}
		if (!bFont && (nLength == 0 || strcasecmp (pIn, CLOCK_NONE) == 0))
		{
			*pOut = CLOCK_NONE;
			return TRUE;
		}
		if (!IsValidAssetName (pIn, bFont ? "ttf,otf" : "png,gif"))
		{
			return FALSE;
		}
		CString Path;
		Path.Format ("%s/%s", CLOCK_FONT_DIR, pIn);
		FILINFO Info;
		if (f_stat (Path, &Info) != FR_OK || (Info.fattrib & AM_DIR))
		{
			return FALSE;			// fichier absent de la carte
		}
		*pOut = pIn;
		return TRUE; }

	case KindColor: {
		u8 RGB[3];
		if (!ParseHexColor (pIn, RGB))
		{
			return FALSE;
		}
		pOut->Format ("%02X%02X%02X", RGB[0], RGB[1], RGB[2]);
		return TRUE; }

	case KindFormat:
		if (!IsValidClockFormat (pIn))
		{
			return FALSE;
		}
		*pOut = pIn;
		return TRUE;

	case KindCoord: {
		if (nLength == 0)
		{
			*pOut = "";
			return TRUE;
		}
		char Coord[16];
		if (!NormalizeCoordinate (pIn, strcasecmp (pSetting->pKey, "Weather_Latitude") == 0,
					  Coord, sizeof Coord))
		{
			return FALSE;
		}
		*pOut = Coord;
		return TRUE; }

	case KindText:
		if (!IsValidPlaceName (pIn))
		{
			return FALSE;
		}
		*pOut = pIn;
		return TRUE;
	}

	return FALSE;
}

// controle d'ensemble de l'adresse fixe (valeurs actuelles + changements demandes)
static const char *CheckIPChanges (CConfig *pConfig, const TConfigChange *pChanges, unsigned nChanges)
{
	static const char *Keys[4] = {"IP_Fixe", "IP_Masque", "IP_Passerelle", "IP_DNS"};
	const char *Values[4] = {pConfig->IPAddress, pConfig->IPMask, pConfig->IPGateway, pConfig->IPDNS};

	for (unsigned i = 0; i < nChanges; i++)
	{
		for (unsigned k = 0; k < 4; k++)
		{
			if (strcasecmp (pChanges[i].pKey, Keys[k]) == 0)
			{
				Values[k] = pChanges[i].pValue;
			}
		}
	}

	if (Values[0][0] == '\0')
	{
		return 0;		// adresse automatique : rien a verifier
	}

	u8 Address[4][4];
	for (unsigned k = 0; k < 4; k++)
	{
		if (!ParseIPv4 (Values[k], Address[k]))
		{
			return "masque, passerelle et DNS sont obligatoires";
		}
	}

	return CheckStaticIP (Address[0], Address[1], Address[2], Address[3]);
}

// --- lecture ---

void CSettings::BuildJSON (CConfig *pConfig, CString *pJSON)
{
	CString Value;

	pJSON->Append ("{\"luminosite\":[");
	for (unsigned h = 0; h < 24; h++)
	{
		Value.Format (h == 0 ? "%u" : ",%u", pConfig->GetBrightness (h));
		pJSON->Append (Value);
	}
	pJSON->Append ("]");

	TDateTime Time;
	Value.Format (",\"heure_actuelle\":%d", CClock::Get ()->GetLocalTime (&Time) ? (int) Time.Hour : -1);
	pJSON->Append (Value);

	for (unsigned i = 0; i < sizeof s_Settings / sizeof s_Settings[0]; i++)
	{
		const TSetting *pSetting = &s_Settings[i];
		if (pSetting->Kind == KindPSK || pSetting->Kind == KindPassword)
		{
			continue;		// les secrets ne sortent jamais
		}

		GetCurrent (pConfig, pSetting->pKey, &Value);
		pJSON->Append (",\"");
		pJSON->Append (pSetting->pKey);
		pJSON->Append ("\":");
		if (pSetting->Kind == KindNumber || pSetting->Kind == KindBool)
		{
			pJSON->Append (Value);
		}
		else
		{
			AppendJSONString (pJSON, Value);
		}
	}

	// configuration reseau en service (pour preremplir l'adresse fixe)
	CString IP, Mask, Gateway, DNS, MAC;
	CNetwork *pNetwork = CNetwork::Get ();
	if (pNetwork != 0)
	{
		pNetwork->GetCurrentConfig (&IP, &Mask, &Gateway, &DNS, &MAC);
	}
	pJSON->Append (",\"reseau_actuel\":{\"ip\":");
	AppendJSONString (pJSON, IP);
	pJSON->Append (",\"masque\":");
	AppendJSONString (pJSON, Mask);
	pJSON->Append (",\"passerelle\":");
	AppendJSONString (pJSON, Gateway);
	pJSON->Append (",\"dns\":");
	AppendJSONString (pJSON, DNS);
	pJSON->Append (",\"mac\":");
	AppendJSONString (pJSON, MAC);
	Value.Format (",\"fixe\":%s}", pNetwork != 0 && pNetwork->IsStaticIP () ? "true" : "false");
	pJSON->Append (Value);

	Value.Format (",\"wifi_psk_defini\":%s,\"redemarrage_en_attente\":%s,\"fuseaux\":[",
		      pConfig->WifiPSK[0] != '\0' ? "true" : "false",
		      CRemoteControl::Get ()->IsRebootPending () ? "true" : "false");
	pJSON->Append (Value);
	for (unsigned i = 0; i < sizeof s_Zones / sizeof s_Zones[0]; i++)
	{
		if (i > 0)
		{
			pJSON->Append (",");
		}
		AppendJSONString (pJSON, s_Zones[i]);
	}
	pJSON->Append ("]}\n");
}

// --- modification ---

boolean CSettings::Apply (CConfig *pConfig, const char * const *ppNames, const char * const *ppValues,
			  unsigned nFields, CString *pReply)
{
	// 1. verifier toutes les valeurs, garder celles qui changent
	static CString Keys[MAX_CHANGES];
	static CString Values[MAX_CHANGES];
	TConfigChange Changes[MAX_CHANGES];
	unsigned nChanges = 0;
	boolean bReboot = FALSE, bNextBoot = FALSE, bWifi = FALSE;

	for (unsigned i = 0; i < nFields; i++)
	{
		const TSetting *pSetting = FindSetting (ppNames[i]);
		if (pSetting == 0)
		{
			pReply->Format ("ERREUR : reglage inconnu : %s\n", ppNames[i]);
			return FALSE;
		}

		// mot de passe vide : on garde l'actuel
		if (   (pSetting->Kind == KindPSK || pSetting->Kind == KindPassword)
		    && ppValues[i][0] == '\0')
		{
			continue;
		}

		CString Normalized;
		if (!Normalize (pSetting, ppValues[i], &Normalized))
		{
			pReply->Format ("ERREUR : valeur refusee pour %s\n", pSetting->pLabel);
			return FALSE;
		}

		CString Current;
		GetCurrent (pConfig, ppNames[i], &Current);
		if (strcmp (Current, Normalized) == 0)
		{
			continue;
		}

		if (nChanges >= MAX_CHANGES)
		{
			*pReply = "ERREUR : trop de reglages a la fois\n";
			return FALSE;
		}

		// cle ecrite avec sa forme officielle (Panel_Brightness_N garde son numero)
		Keys[nChanges] = pSetting == &s_Brightness ? ppNames[i] : pSetting->pKey;
		Values[nChanges] = Normalized;
		Changes[nChanges].pKey = Keys[nChanges];
		Changes[nChanges].pValue = Values[nChanges];
		nChanges++;

		switch (pSetting->Apply)
		{
		case ApplyReboot:	bReboot = TRUE;		break;
		case ApplyNextBoot:	bNextBoot = TRUE;	break;
		case ApplyWifi:		bWifi = TRUE;		break;
		default:					break;
		}
	}

	if (nChanges == 0)
	{
		*pReply = "OK : aucun changement\n";
		return TRUE;
	}

	const char *pIPError = CheckIPChanges (pConfig, Changes, nChanges);
	if (pIPError != 0)
	{
		pReply->Format ("ERREUR : adresse fixe : %s\n", pIPError);
		return FALSE;
	}

	// 2. modifier config.txt (ancienne version gardee en config.bak)
	char *pOld = new char[MAX_CONFIG_SIZE + 1];
	char *pNew = new char[MAX_CONFIG_SIZE + 4096];
	unsigned nOldLength = MAX_CONFIG_SIZE;
	boolean bOK = FALSE;
	int nNewLength = -1;

	if (ReadWholeFile (CONFIG_FILE, pOld, &nOldLength))
	{
		pOld[nOldLength] = '\0';
		nNewLength = EditConfigText (pOld, Changes, nChanges, pNew, MAX_CONFIG_SIZE + 4096);
		bOK =    nNewLength > 0
		      && SafeWriteFile (CONFIG_FILE, DRIVE "/Config/config.new", DRIVE "/Config/config.bak",
					pNew, (unsigned) nNewLength);
	}

	delete [] pOld;
	delete [] pNew;

	if (!bOK)
	{
		*pReply = "ERREUR : ecriture de config.txt impossible\n";
		return FALSE;
	}

	// 3. appliquer
	pConfig->Load (CONFIG_FILE);
	CRemoteControl *pRemote = CRemoteControl::Get ();
	pRemote->RequestReload ();

	for (unsigned i = 0; i < nChanges; i++)
	{
		boolean bSecret =    strcasecmp (Changes[i].pKey, "Wifi_psk") == 0
				  || strcasecmp (Changes[i].pKey, "Web_Password") == 0
				  || strcasecmp (Changes[i].pKey, "Weather_Latitude") == 0	// lieu : pas
				  || strcasecmp (Changes[i].pKey, "Weather_Longitude") == 0;	// dans le journal
		CLogger::Get ()->Write (From, LogNotice, "%s = %s", Changes[i].pKey,
					bSecret ? "(modifie)" : Changes[i].pValue);
	}

	if (bWifi)
	{
		// essai du nouveau Wi-Fi : retour a l'ancien s'il ne se connecte pas (dmdapp.cpp)
		FIL File;
		if (f_open (&File, WIFI_TRIAL_FILE, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
		{
			UINT nWritten;
			f_write (&File, "essai\n", 6, &nWritten);
			f_close (&File);
		}

		pRemote->Submit (RemoteReboot, 0, 0);
		pReply->Format ("OK : %u reglage(s) reseau enregistre(s). Le panneau redemarre ; "
				"s'il n'est pas joignable en %u s, il revient aux anciens reglages.\n",
				nChanges, WIFI_TRIAL_SECONDS);
		return TRUE;
	}

	if (bReboot)
	{
		pRemote->SetRebootPending ();
		pReply->Format ("OK : %u reglage(s) enregistre(s) ; redemarrage necessaire pour les appliquer\n",
				nChanges);
	}
	else if (bNextBoot)
	{
		pReply->Format ("OK : %u reglage(s) enregistre(s), effectif(s) au prochain demarrage\n", nChanges);
	}
	else
	{
		pReply->Format ("OK : %u reglage(s) enregistre(s)\n", nChanges);
	}

	return TRUE;
}
