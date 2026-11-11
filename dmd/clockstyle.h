//
// clockstyle.h - apparence de l'heure et de la date (page Horloge)
//
// Les cles de config.txt reprennent celles de l'ancien RPI2DMD (Clock_Font,
// Clock_Font_Size, Clock_Font_Pattern, Clock_Format, Clock_Background, Date_...),
// plus quelques nouvelles (Clock_Color, Clock_Blink, Clock_Smooth, Date_Color).
// Polices et images sont des fichiers de SD:/fonts.
//
#ifndef _clockstyle_h
#define _clockstyle_h

#include <circle/types.h>
#include "localtime.h"

#define CLOCK_FONT_DIR		"SD:/fonts"
#define CLOCK_FONT_PIXEL	"Pixel"		// police 5x7 integree au programme
#define CLOCK_NONE		"None"		// pas de motif / pas de fond
#define CLOCK_NAME_LEN		80
#define CLOCK_FORMAT_LEN	40
#define CLOCK_MAX_SIZE		48		// taille de police maximale (pixels)

struct TTextStyle
{
	char	 Font[CLOCK_NAME_LEN];		// "Pixel", ou fichier de SD:/fonts ("Polices/X.ttf")
	unsigned nSize;				// taille en pixels ; 0 = la plus grande qui tient
	char	 Pattern[CLOCK_NAME_LEN];	// "None" : couleur unie ; sinon image PNG ou GIF
	u8	 Color[3];
	char	 Format[CLOCK_FORMAT_LEN];	// %H %M %S %d %b ... (voir FormatDateTime)
};

enum TDateMode
{
	DateOff    = 0,
	DateScreen = 1,		// la date seule, apres l'heure (comme l'ancien RPI2DMD)
	DateBelow  = 2		// la date sous l'heure
};

struct TClockStyle
{
	TTextStyle Time;
	TTextStyle Date;
	boolean	   bSmooth;			// bords adoucis (sinon pixels nets)
	boolean	   bBlink;			// deux-points clignotants
	char	   Background[CLOCK_NAME_LEN];	// "None" ou image PNG/GIF de SD:/fonts
	unsigned   DateMode;			// TDateMode
	unsigned   nDateDisplayTime;		// ms (DateScreen)
};

// --- meteo (page Meteo) : cles Weather_* (Weather_Active, Weather_Display_Time et
// Weather_Text_color viennent de l'ancien RPI2DMD) ---

enum TWeatherInfo		// deuxieme ligne de l'ecran meteo
{
	InfoNone	= 0,
	InfoDescription	= 1,	// PLUIE FAIBLE
	InfoMinMax	= 2,	// 4° 11°
	InfoFeels	= 3,	// RESS. 3°
	InfoHumidity	= 4,	// 92 %
	InfoWind	= 5,	// 11 KM/H
	InfoTomorrow	= 6	// DEMAIN 4° 11°
};

#define WEATHER_INFO_MAX	6

struct TWeatherStyle
{
	boolean	   bActive;
	unsigned   nDisplayTime;		// ms
	unsigned   nEvery;			// montree toutes les N GIF (1 a 20)
	char	   Latitude[16];		// "" : lieu pas encore choisi
	char	   Longitude[16];
	char	   Place[48];			// nom du lieu (affiche sur la page web)
	TTextStyle Text;			// temperature : police, taille, motif, couleur
	u8	   InfoColor[3];		// deuxieme ligne
	unsigned   Info;			// TWeatherInfo
	boolean	   bIcon;
	boolean	   bAnimate;		// icones animees
};

void WeatherStyleDefaults (TWeatherStyle *pStyle);
boolean SetWeatherStyleValue (TWeatherStyle *pStyle, const char *pKey, const char *pValue);

// latitude ou longitude "49.9214" (4 decimales au plus) ; FALSE si invalide
boolean NormalizeCoordinate (const char *pText, boolean bLatitude, char *pOut, unsigned nOutSize);
boolean IsValidPlaceName (const char *pName);
// couleur "FFAA00", "#FFAA00" ou "255,170,0" (ancien format)
boolean ParseAnyColor (const char *pText, u8 *pRGB);

// valeurs par defaut (police Pixel ambre, comme avant la page Horloge)
void ClockStyleDefaults (TClockStyle *pStyle);

// applique une cle de config.txt (ou de l'apercu web) ; FALSE si la cle n'est pas une
// cle d'horloge. Une valeur invalide est ignoree (la valeur actuelle est gardee).
boolean SetClockStyleValue (TClockStyle *pStyle, const char *pKey, const char *pValue);

// texte selon un format du type strftime ; noms de jours et de mois en francais, majuscules
//   %H %I heure (24 h / 12 h)  %M minutes  %S secondes  %p AM/PM
//   %d %e jour  %m mois  %y %g annee sur 2 chiffres  %Y annee  %a %A jour  %b %B mois  %%
void FormatDateTime (const char *pFormat, const TDateTime &rTime, char *pOut, unsigned nOutSize);

boolean IsValidClockFormat (const char *pFormat);
boolean ParseHexColor (const char *pText, u8 *pRGB);		// "FFAA00" ou "#FFAA00"

// nom de fichier de SD:/fonts : chemin relatif (un sous-dossier au plus), sans "..",
// avec une des extensions de la liste ("ttf,otf")
boolean IsValidAssetName (const char *pName, const char *pExtensions);

#endif
