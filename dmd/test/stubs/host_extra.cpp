// Fonctions du panneau remplacees pour les tests sous Windows :
// lecture de fichier (fileutil) et bibliotheques stb avec leurs reglages par defaut
#include <circle/types.h>
#include <fatfs/ff.h>

boolean ReadWholeFile (const char *pPath, void *pBuffer, unsigned *pLength)
{
	FIL File;
	if (f_open (&File, pPath, FA_READ) != FR_OK)
	{
		return FALSE;
	}
	UINT nRead = 0;
	f_read (&File, pBuffer, *pLength, &nRead);
	f_close (&File);
	*pLength = nRead;
	return TRUE;
}

#define STB_TRUETYPE_IMPLEMENTATION
#include "../../stb/stb_truetype.h"

#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STB_IMAGE_IMPLEMENTATION
#include "../../stb/stb_image.h"
