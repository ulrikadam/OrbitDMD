//
// gifmanager.h - gestion des GIF sur la carte SD (appele par la page web)
//
// Les acces a la carte depuis la page web sont surs : le pilote SD ne rend jamais
// la main au planificateur en cours d'operation, donc les acces ne se chevauchent pas.
//
#ifndef _gifmanager_h
#define _gifmanager_h

#include <circle/string.h>
#include <circle/types.h>
#include "config.h"

class CGifManager
{
public:
	static const unsigned MaxCategoryLength = 32;
	static const unsigned MaxNameLength = 100;

public:
	// noms de categorie : lettres, chiffres, _ et - (32 max)
	static boolean IsValidCategory (const char *pName);
	// noms de fichier existants : pas de / \ : ni .. , se termine par .gif
	static boolean IsValidFileName (const char *pName);
	// rend un nom de fichier envoye utilisable (caracteres surs, extension .gif)
	static void SanitizeFileName (const char *pIn, char *pOut, unsigned nOutSize);

	static void ListCategories (CConfig *pConfig, CString *pJSON);
	static void ListFiles (const char *pCategory, CString *pJSON);
	static boolean ReadGif (const char *pCategory, const char *pName, u8 *pBuffer, unsigned *pLength);

	static boolean AddGif (const char *pCategory, const char *pName,
			       const u8 *pData, unsigned nLength, CString *pReply);
	static boolean DeleteGif (CConfig *pConfig, const char *pCategory, const char *pName,
				  CString *pReply);
	// supprime un dossier de categorie et tout ce qu'il contient (sauf Accueil, et pas la
	// derniere categorie affichee) ; la categorie est aussi retiree des categories actives
	static boolean DeleteCategory (CConfig *pConfig, const char *pCategory, CString *pReply);
	static boolean PlayGif (const char *pCategory, const char *pName, CString *pReply);
	static boolean SetActiveCategories (CConfig *pConfig, const char *pList, CString *pReply);

private:
	static void MakePath (CString *pPath, const char *pCategory, const char *pName = 0);
	static boolean IsActive (CConfig *pConfig, const char *pCategory);
	static boolean DeleteTree (const char *pPath, unsigned nDepth, unsigned *pFiles);
};

#endif
