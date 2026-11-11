//
// settings.h - reglages modifiables depuis la page web (/reglages)
//
// Chaque reglage est une cle de config.txt. Les valeurs sont verifiees ici, puis
// seules les lignes concernees de config.txt sont modifiees (configedit.cpp).
//
#ifndef _settings_h
#define _settings_h

#include <circle/string.h>
#include <circle/types.h>
#include "config.h"

#define WIFI_TRIAL_FILE		"SD:/Config/wifi-essai.txt"	// nouveau Wi-Fi en cours d'essai
#define WIFI_TRIAL_SECONDS	150				// delai avant retour a l'ancien Wi-Fi

class CSettings
{
public:
	// reglages actuels (sans les secrets) au format JSON
	static void BuildJSON (CConfig *pConfig, CString *pJSON);

	// applique des reglages "Cle" = "valeur" ; rien n'est modifie si une valeur est invalide
	static boolean Apply (CConfig *pConfig, const char * const *ppNames, const char * const *ppValues,
			      unsigned nFields, CString *pReply);
};

#endif
