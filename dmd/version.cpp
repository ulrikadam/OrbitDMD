//
// version.cpp - recompile a chaque construction (voir Makefile) pour dater le programme
//
// Le numero de version vient du fichier VERSION a la racine du projet (ORBITDMD_VERSION).
//
#ifndef ORBITDMD_VERSION
	#define ORBITDMD_VERSION	"0.0.0"
#endif

const char *GetVersion (void)
{
	return ORBITDMD_VERSION;
}

const char *GetBuildDate (void)
{
	return __DATE__ " " __TIME__;
}
