//
// font5x7.h - petite police bitmap 5x7 (chiffres, majuscules latines et cyrilliques, ponctuation)
//
#ifndef _font5x7_h
#define _font5x7_h

#include <circle/types.h>

#define FONT_WIDTH	5
#define FONT_HEIGHT	7
#define FONT_ADVANCE	6	// largeur + 1 colonne d'espace

// renvoie les 7 lignes du caractere Unicode (chaque ligne : 5 caracteres '#' ou '.'),
// ou 0 si le caractere n'existe pas (minuscules dessinees en majuscules, accents retires)
const char * const *FontGetGlyph (unsigned nCode);

// textes UTF-8 : caractere suivant (un octet isole est pris tel quel, comme du Latin-1),
// nombre de caracteres, et position apres nChars caracteres
unsigned Utf8Next (const char **ppText);
unsigned Utf8Length (const char *pText);
const char *Utf8Skip (const char *pText, unsigned nChars);

#endif
