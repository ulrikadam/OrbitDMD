// Remplacement minimal de <circle/util.h> pour les tests sous Windows
#ifndef _circle_util_h
#define _circle_util_h

#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
	#define strcasecmp	_stricmp
	#define strncasecmp	_strnicmp
#endif

#endif
