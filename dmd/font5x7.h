//
// font5x7.h - petite police bitmap 5x7 (chiffres, majuscules, ponctuation)
//
#ifndef _font5x7_h
#define _font5x7_h

#include <circle/types.h>

#define FONT_WIDTH	5
#define FONT_HEIGHT	7
#define FONT_ADVANCE	6	// largeur + 1 colonne d'espace

// renvoie les 7 lignes du caractere (chaque ligne : 5 caracteres '#' ou '.'),
// ou 0 si le caractere n'existe pas (les minuscules sont converties en majuscules)
const char * const *FontGetGlyph (char chChar);

#endif
