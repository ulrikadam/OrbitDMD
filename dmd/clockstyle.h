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
#define CLOCK_RAINBOW		"Arc-en-ciel"	// motif dessine : arc-en-ciel anime
#define CLOCK_RANDOM		"Random"	// motif ou fond tire au hasard a chaque affichage
					// (meme valeur que l'ancien RPI2DMD pour le fond)
boolean IsRandomChoice (const char *pValue);	// "Random" ou "Hasard"
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

// --- texte defilant (page Messages) : cles Texte_* ---

#define BANNER_TEXT_LEN		201		// texte UTF-8 (200 octets au plus)
#define BANNER_SPEED_MAX	10

enum TBannerEffect		// arrivee du texte (Texte_Effet)
{
	EffectScroll,		// "defile" : de droite a gauche
	EffectReverse,		// "inverse" : de gauche a droite
	EffectUp,		// "monte" : du bas vers le haut, page par page
	EffectDown,		// "descend" : du haut vers le bas
	EffectTyping,		// "lettres" : lettre par lettre
	EffectRain,		// "pluie" : les lettres tombent
	EffectFade,		// "fondu" : apparition en fondu
	EffectCount
};

const char *BannerEffectName (unsigned nEffect);	// "defile"...
boolean IsPagedEffect (unsigned nEffect);		// le texte est coupe en pages qui tiennent

struct TBannerStyle
{
	boolean	   bActive;			// Texte_Active : dans la rotation, toutes les N GIF
	unsigned   Effect;			// Texte_Effet (TBannerEffect)
	unsigned   nEvery;			// Texte_Every (1 a 20)
	unsigned   nPasses;			// Texte_Passages : nombre de passages (1 a 10)
	unsigned   nSpeed;			// Texte_Vitesse (1 a 10)
	char	   Text[BANNER_TEXT_LEN];	// Texte_Message
	char	   From[6];			// Texte_Du : "MM-JJ" ou vide (schedule.h)
	char	   To[6];			// Texte_Au
	TTextStyle Style;			// Texte_Font, Texte_Font_Size, Texte_Font_Pattern, Texte_Color
};

void BannerStyleDefaults (TBannerStyle *pStyle);
boolean SetBannerStyleValue (TBannerStyle *pStyle, const char *pKey, const char *pValue);
boolean IsValidBannerText (const char *pText);	// UTF-8, sans retour a la ligne ni guillemet
unsigned BannerPixelsPerSecond (unsigned nSpeed);

// --- compte a rebours (page Messages) : cles Compte_* ---

#define COUNTDOWN_XMAS		"noel"		// 25 decembre 00:00, chaque annee
#define COUNTDOWN_NEW_YEAR	"nouvel-an"	// 1er janvier 00:00, chaque annee
#define COUNTDOWN_TITLE_LEN	48

struct TCountdownStyle
{
	boolean	   bActive;			// Compte_Active : dans la rotation, toutes les N GIF
	unsigned   nEvery;			// Compte_Every (1 a 20)
	unsigned   nDisplayTime;		// Compte_Display_Time (ms)
	char	   Target[20];			// Compte_Date : "noel", "nouvel-an" ou "AAAA-MM-JJ HH:MM"
	char	   Title[COUNTDOWN_TITLE_LEN];	// Compte_Titre ("NOEL DANS")
	char	   Done[COUNTDOWN_TITLE_LEN];	// Compte_Fin : montre le jour J ("JOYEUX NOEL !")
	TTextStyle Style;			// valeur : Compte_Font, Compte_Font_Size, Compte_Font_Pattern, Compte_Color
	u8	   TitleColor[3];		// Compte_Titre_Color
	char	   From[6];			// Compte_Du : "MM-JJ" ou vide (schedule.h)
	char	   To[6];			// Compte_Au
};

void CountdownStyleDefaults (TCountdownStyle *pStyle);
boolean SetCountdownStyleValue (TCountdownStyle *pStyle, const char *pKey, const char *pValue);
boolean IsValidCountdownTarget (const char *pTarget);

enum TCountdownState
{
	CountdownNone,		// date passee depuis plus d'un jour (date choisie) ou invalide
	CountdownRunning,	// *pValue : "12 JOURS", "3 J 14 H" ou "14:02:33"
	CountdownReached	// jour J (24 h apres l'heure choisie) : montrer Compte_Fin
};

// heure locale -> etat et texte a afficher
TCountdownState FormatCountdown (const char *pTarget, const TDateTime &rNow, char *pValue, unsigned nSize);

// texte a dessiner : copie UTF-8 (sans couper un caractere), retours a la ligne -> espaces ;
// les polices dessinent l'Unicode (police Pixel : latin, accents retires, cyrillique)
void TextForDisplay (const char *pUTF8, char *pOut, unsigned nOutSize);

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
