//
// schedule.h - programme : plages horaires, mois des categories, periodes des messages
//
// Cles de config.txt (page Reglages et page GIF) :
//   Prog_Eteint_Active=1   Prog_Eteint_Debut=1   Prog_Eteint_Fin=7	panneau eteint de 1 h a 7 h
//   Prog_Heure_Active=1    Prog_Heure_Debut=23   Prog_Heure_Fin=7	l'heure seule (sans GIF)
//   Gif_Cat_Mois=XMAS:12;Halloween:10;Plage:6-8			categories montrees certains mois
// Une plage d'heures va du debut (compris) a la fin (non comprise) et peut passer minuit.
//
// Messages (messages.txt) : Texte_Du / Texte_Au, Compte_Du / Compte_Au au format MM-JJ ; vide =
// toute l'annee ; la periode peut passer le 1er janvier (12-20 -> 01-05).
//
#ifndef _schedule_h
#define _schedule_h

#include <circle/types.h>
#include "localtime.h"

#define SCHEDULE_MAX_RULES	32
#define SCHEDULE_RAW_LEN	400
#define SCHEDULE_ALL_MONTHS	0x1FFE		// bits 1 a 12

struct TSchedule
{
	boolean	 bOff;				// Prog_Eteint_*
	unsigned nOffFrom, nOffTo;
	boolean	 bClockOnly;			// Prog_Heure_*
	unsigned nClockFrom, nClockTo;

	char	 MonthsRaw[SCHEDULE_RAW_LEN];	// Gif_Cat_Mois, tel quel (normalise)
	unsigned nRules;
	char	 RuleCategory[SCHEDULE_MAX_RULES][33];
	u16	 RuleMonths[SCHEDULE_MAX_RULES];
};

void ScheduleDefaults (TSchedule *pSchedule);
// applique une cle de config.txt ; FALSE si ce n'est pas une cle du programme
boolean SetScheduleValue (TSchedule *pSchedule, const char *pKey, const char *pValue);

// heure dans la plage [debut, fin[ (plage vide si debut == fin)
boolean InHourWindow (unsigned nFrom, unsigned nTo, unsigned nHour);

// "XMAS:12;Plage:6-8" -> regles ; FALSE si le texte est invalide (pSchedule inchange)
boolean ParseCategoryMonths (const char *pValue, TSchedule *pSchedule);
// mois ou la categorie est montree (bits 1 a 12) ; toute l'annee sans regle
u16 GetCategoryMonths (const TSchedule &rSchedule, const char *pCategory);

// "MM-JJ" valable, ou vide
boolean IsValidMonthDay (const char *pValue);
// date dans la periode du .. au (bornes comprises) ; toujours vrai si les deux sont vides
boolean InDateWindow (const char *pFrom, const char *pTo, const TDateTime &rNow);

#endif
