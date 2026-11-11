//
// messages.h - listes de textes defilants et de comptes a rebours (page Messages)
//
// Fichier SD:/Config/messages.txt : une section par element, avec les memes cles que
// l'ancienne version a un seul message (Texte_*, Compte_*) :
//
//   [texte]
//   Texte_Active=1
//   Texte_Message=Joyeux anniversaire !
//   Texte_Effet=pluie
//   ...
//   [compte]
//   Compte_Active=1
//   Compte_Date=noel
//   ...
//
// Chaque element a son propre style. La frequence (toutes les N GIF) reste dans config.txt
// (Texte_Every, Compte_Every).
//
#ifndef _messages_h
#define _messages_h

#include <circle/string.h>
#include <circle/types.h>
#include "clockstyle.h"
#include "common.h"

#define MESSAGES_FILE		DRIVE "/Config/messages.txt"
#define MESSAGES_MAX_TEXTS	10
#define MESSAGES_MAX_COUNTDOWNS	10
#define MESSAGES_MAX_SIZE	16384

// dans la sauvegarde des reglages, avant le fichier des messages (lignes commencant par #)
#define BACKUP_MESSAGES_HEADER	"# ===== OrbitDMD : messages ====="

class CConfig;

class CMessages
{
public:
	// lit le fichier ; sans fichier, reprend le message et le compte a rebours de config.txt
	static void Load (const CConfig *pConfig);

	// remplace les listes par le contenu d'un texte au format du fichier ; elements lus
	static unsigned Parse (const char *pText);
	static void Export (CString *pText);
	static boolean Save (void);
	static void BuildJSON (CString *pJSON);

	static unsigned CountActiveTexts (void);
	static unsigned CountActiveCountdowns (void);

	static TBannerStyle s_Texts[MESSAGES_MAX_TEXTS];
	static unsigned s_nTexts;
	static TCountdownStyle s_Countdowns[MESSAGES_MAX_COUNTDOWNS];
	static unsigned s_nCountdowns;
};

#endif
