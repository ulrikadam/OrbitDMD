//
// stbimpl.cpp - bibliotheques stb (domaine public) compilees pour Circle
//
// stb_truetype : dessin des polices TTF de SD:/fonts (horloge)
// stb_image    : lecture des motifs PNG de SD:/fonts
// Les fichiers d'origine sont dans stb/ (github.com/nothings/stb), sans modification.
//
#include <circle/alloc.h>
#include <circle/util.h>

#ifndef NULL
	#define NULL	0
#endif

// pas de bibliotheque C complete sous Circle : on fournit ce que stb utilise
extern "C" double pow (double, double);
extern "C" double fmod (double, double);
extern "C" double cos (double);
extern "C" double acos (double);

// --- stb_truetype ---
#define STBTT_ifloor(x)		((int) __builtin_floor (x))
#define STBTT_iceil(x)		((int) __builtin_ceil (x))
#define STBTT_sqrt(x)		__builtin_sqrt (x)
#define STBTT_pow(x, y)		pow (x, y)
#define STBTT_fmod(x, y)	fmod (x, y)
#define STBTT_cos(x)		cos (x)
#define STBTT_acos(x)		acos (x)
#define STBTT_fabs(x)		__builtin_fabs (x)
#define STBTT_malloc(x, u)	((void) (u), malloc (x))
#define STBTT_free(x, u)	((void) (u), free (x))
#define STBTT_assert(x)		((void) 0)
#define STBTT_strlen(x)		strlen (x)
#define STBTT_memcpy		memcpy
#define STBTT_memset		memset

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

// --- stb_image (PNG seulement, depuis la memoire) ---
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCALS
#define STBI_ASSERT(x)		((void) 0)
#define STBI_MALLOC(n)		malloc (n)
#define STBI_REALLOC(p, n)	realloc (p, n)
#define STBI_FREE(p)		free (p)
#define STBI_MAX_DIMENSIONS	4096		// motifs et fonds : petites images

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
