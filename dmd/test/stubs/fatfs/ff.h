// Remplacement minimal de FatFs (lecture de fichiers via stdio) pour les tests
#ifndef _ff_stub_h
#define _ff_stub_h

#include <stdio.h>

typedef unsigned int UINT;
typedef enum { FR_OK = 0, FR_NO_FILE = 4 } FRESULT;
typedef struct { FILE *pFile; } FIL;

#define FA_READ		0x01
#define FA_OPEN_EXISTING 0x00

static inline FRESULT f_open (FIL *fp, const char *path, int mode)
{
	(void) mode;
	// "SD:/..." -> chemin relatif au dossier courant
	if (path[0] == 'S' && path[1] == 'D' && path[2] == ':')
	{
		path += 4;
	}
	fp->pFile = fopen (path, "rb");
	return fp->pFile ? FR_OK : FR_NO_FILE;
}

static inline FRESULT f_read (FIL *fp, void *buff, UINT btr, UINT *br)
{
	*br = (UINT) fread (buff, 1, btr, fp->pFile);
	return FR_OK;
}

static inline FRESULT f_close (FIL *fp)
{
	fclose (fp->pFile);
	return FR_OK;
}

// informations sur un fichier (taille seulement)
typedef unsigned long long FSIZE_t;
typedef unsigned char BYTE;
typedef struct { FSIZE_t fsize; BYTE fattrib; char fname[256]; } FILINFO;
#define AM_RDO	0x01
#define AM_HID	0x02
#define AM_SYS	0x04
#define AM_DIR	0x10

static inline FRESULT f_stat (const char *path, FILINFO *fno)
{
	FIL File;
	if (f_open (&File, path, FA_READ) != FR_OK)
	{
		return FR_NO_FILE;
	}
	fseek (File.pFile, 0, SEEK_END);
	fno->fsize = (FSIZE_t) ftell (File.pFile);
	fno->fattrib = 0;
	fno->fname[0] = '\0';
	f_close (&File);
	return FR_OK;
}

#endif
