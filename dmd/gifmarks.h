//
// gifmarks.h - GIF favorites et GIF exclues de la rotation (page GIF)
//
// Fichier SD:/Config/gif-marques.txt, une ligne par GIF marquee :
//   +Categorie/nom.gif	favorite : passe plus souvent (lecture au hasard)
//   -Categorie/nom.gif	exclue : gardee sur la carte, mais plus montree dans la rotation
// Les noms sont compares sans tenir compte des majuscules (comme la carte SD).
//
#ifndef _gifmarks_h
#define _gifmarks_h

#include <circle/string.h>
#include <circle/types.h>
#include "common.h"

#define GIF_MARKS_FILE		DRIVE "/Config/gif-marques.txt"
#define GIF_FAVORITE_WEIGHT	3	// une favorite compte pour 3 dans la lecture au hasard

// dans la sauvegarde des reglages, avant les GIF marquees (chaque ligne commence par #)
#define BACKUP_MARKS_HEADER	"# ===== OrbitDMD : GIF favorites (+) et exclues (-) ====="

enum TGifMark
{
	GifMarkNone	 = 0,
	GifMarkFavorite	 = '+',
	GifMarkExcluded	 = '-'
};

class CGifMarks
{
public:
	static const unsigned MaxMarks = 4000;

	static void Load (void);		// lit le fichier (au demarrage)

	static TGifMark Get (const char *pCategory, const char *pName);
	// change la marque d'une GIF et enregistre le fichier ; FALSE si trop de marques
	static boolean Set (const char *pCategory, const char *pName, TGifMark Mark);
	static void ForgetCategory (const char *pCategory);	// categorie supprimee

	// texte du fichier (sauvegarde des reglages), et remplacement complet (restauration)
	static void Export (CString *pText);
	static unsigned Import (const char *pText, boolean bSave);	// nombre de marques lues

	static unsigned GetCount (TGifMark Mark);

private:
	struct TEntry
	{
		char Key[134];		// "Categorie/nom.gif"
		char Mark;
	};

	static int Find (const char *pKey, boolean *pFound);	// position (triee)
	static boolean MakeKey (const char *pCategory, const char *pName, char *pKey);
	static boolean Save (void);

	static TEntry *s_pEntries;
	static unsigned s_nEntries;
};

#endif
