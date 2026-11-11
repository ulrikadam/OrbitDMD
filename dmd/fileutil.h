//
// fileutil.h - ecriture sure d'un fichier sur la carte SD
//
#ifndef _fileutil_h
#define _fileutil_h

#include <circle/types.h>
#include <fatfs/ff.h>

// ecrit pTemp, le relit pour verification, garde l'ancien pFinal en pBackup
// (si pBackup != 0), puis renomme pTemp en pFinal
boolean SafeWriteFile (const char *pFinal, const char *pTemp, const char *pBackup,
		       const void *pData, unsigned nSize);

// lit un fichier entier ; *pLength : taille du tampon en entree, octets lus en sortie
boolean ReadWholeFile (const char *pPath, void *pBuffer, unsigned *pLength);

// lit nSize octets d'un fichier ouvert par morceaux de 64 Ko, en rendant la main entre deux
boolean ReadChunked (FIL *pFile, void *pBuffer, unsigned nSize);

// rend la main aux autres taches (Wi-Fi, page web) si le planificateur tourne
void YieldToNetwork (void);

#endif
