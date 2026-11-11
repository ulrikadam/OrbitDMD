//
// configedit.h - modification de lignes "Cle=valeur" dans le texte de config.txt
//
// Code sans dependance a Circle (teste sur le PC par dmd/test/hosttest).
//
#ifndef _configedit_h
#define _configedit_h

#include <circle/types.h>

struct TConfigChange
{
	const char *pKey;	// ex. "Panel_Brightness_7"
	const char *pValue;	// nouvelle valeur
};

// Remplace la valeur des cles presentes (toutes leurs occurrences, comparaison sans
// tenir compte de la casse), ajoute les cles absentes a la fin, garde le reste du
// texte intact (commentaires, ordre, fins de ligne Windows ou Unix).
// Renvoie la longueur du texte produit, ou -1 si pOut est trop petit.
int EditConfigText (const char *pIn, const TConfigChange *pChanges, unsigned nChanges,
		    char *pOut, unsigned nOutSize);

#endif
