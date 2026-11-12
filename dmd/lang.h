//
// lang.h - langue des textes affiches sur le panneau (reglage Langue=fr|en|nl|es|ru|de|it|pt|pl|uk|tr|cs)
//
// Les textes sont en UTF-8 et en majuscules (la police Pixel dessine le latin, les accents
// et le cyrillique). La langue s'applique tout de suite (jours, mois, meteo, messages).
//
#ifndef _lang_h
#define _lang_h

#include <circle/types.h>

enum TLanguage
{
	LangFR,
	LangEN,
	LangNL,
	LangES,
	LangRU,
	LangDE,
	LangIT,
	LangPT,
	LangPL,
	LangUK,
	LangTR,
	LangCS,
	LangCount
};

#define LANGUAGE_CHOICES	"fr,en,nl,es,ru,de,it,pt,pl,uk,tr,cs"

void SetLanguage (unsigned nLanguage);
unsigned GetLanguage (void);
const char *LanguageCode (unsigned nLanguage);			// "fr"...
boolean ParseLanguage (const char *pCode, unsigned *pLanguage);	// "fr", "EN"... ; FALSE si inconnu

enum TText
{
	TxtSDCard, TxtUnreadable,
	TxtNoGif, TxtInGifFolder,
	TxtClock, TxtMissing,
	TxtRefreshRate,
	TxtWifiConnected, TxtCableConnected,
	TxtCable, TxtWifi, TxtNotConnected,
	TxtStarting, TxtTimeUnknown,
	TxtWeather, TxtChoosePlace, TxtNotReceived,
	TxtUpdate, TxtFirmware, TxtSettings,
	TxtUpdateFailed1, TxtUpdateFailed2,
	TxtNetworkRefused, TxtOldSettings,
	TxtRestarting,
	TxtDatePassed,
	TxtFeels, TxtHumidityFormat, TxtWindFormat, TxtTomorrow,	// meteo, deuxieme ligne
	TxtDayHourFormat,						// "%u J %u H"
	TxtXmasIn, TxtMerryXmas,					// compte a rebours par defaut
	TxtNoLinux, TxtRed, TxtGreen, TxtBlue, TxtWhite,		// mire de test
	TxtCount
};

const char *Tr (TText Text);					// texte dans la langue choisie

const char *DayName (unsigned nWeekDay, boolean bLong);		// 0 = dimanche
const char *MonthName (unsigned nMonth, boolean bLong);		// 0 = janvier
const char *WeatherText (unsigned nIndex);			// voir WeatherDescription
void FormatDays (unsigned nDays, char *pOut, unsigned nSize);	// "12 JOURS" (pluriel russe)

#define WEATHER_TEXT_COUNT	21

#endif
