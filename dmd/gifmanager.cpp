//
// gifmanager.cpp
//
#include "gifmanager.h"
#include "common.h"
#include "fileutil.h"
#include "remote.h"
#include <fatfs/ff.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "gif";

#define MAX_CONFIG_SIZE		16384

static boolean EndsWithGif (const char *pName)
{
	size_t nLength = strlen (pName);
	return nLength >= 5 && strcasecmp (pName + nLength - 4, ".gif") == 0;
}

static void AppendJSONString (CString *pJSON, const char *pText)
{
	pJSON->Append ('"');
	for (; *pText != '\0'; pText++)
	{
		if (*pText == '"' || *pText == '\\')
		{
			pJSON->Append ('\\');
			pJSON->Append (*pText);
		}
		else if ((unsigned char) *pText >= ' ')
		{
			pJSON->Append (*pText);
		}
	}
	pJSON->Append ('"');
}

boolean CGifManager::IsValidCategory (const char *pName)
{
	size_t nLength = strlen (pName);
	if (nLength == 0 || nLength > MaxCategoryLength)
	{
		return FALSE;
	}

	for (; *pName != '\0'; pName++)
	{
		char c = *pName;
		if (!(   ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')
		      || c == '_' || c == '-'))
		{
			return FALSE;
		}
	}

	return TRUE;
}

boolean CGifManager::IsValidFileName (const char *pName)
{
	size_t nLength = strlen (pName);
	if (   nLength == 0 || nLength > MaxNameLength
	    || pName[0] == '.'
	    || strstr (pName, "..") != 0
	    || !EndsWithGif (pName))
	{
		return FALSE;
	}

	for (; *pName != '\0'; pName++)
	{
		char c = *pName;
		if (   (unsigned char) c < ' '
		    || c == '/' || c == '\\' || c == ':' || c == '*' || c == '?'
		    || c == '"' || c == '<' || c == '>' || c == '|')
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CGifManager::SanitizeFileName (const char *pIn, char *pOut, unsigned nOutSize)
{
	// garder seulement le nom (certains navigateurs envoient un chemin complet)
	const char *pBase = pIn;
	for (const char *p = pIn; *p != '\0'; p++)
	{
		if (*p == '/' || *p == '\\')
		{
			pBase = p + 1;
		}
	}

	// enlever l'extension, elle est remise en minuscules a la fin
	size_t nLength = strlen (pBase);
	if (nLength >= 4 && strcasecmp (pBase + nLength - 4, ".gif") == 0)
	{
		nLength -= 4;
	}

	unsigned nMax = nOutSize - 5;		// place pour ".gif" et le zero final
	if (nMax > MaxNameLength - 4)
	{
		nMax = MaxNameLength - 4;
	}

	unsigned n = 0;
	for (size_t i = 0; i < nLength && n < nMax; i++)
	{
		char c = pBase[i];
		boolean bSafe =    ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')
				|| ('0' <= c && c <= '9') || c == '_' || c == '-'
				|| (c == '.' && n > 0);
		pOut[n++] = bSafe ? c : '_';
	}

	if (n == 0)
	{
		pOut[n++] = 'g';
		pOut[n++] = 'i';
		pOut[n++] = 'f';
	}

	strcpy (pOut + n, ".gif");
}

void CGifManager::MakePath (CString *pPath, const char *pCategory, const char *pName)
{
	if (pName != 0)
	{
		pPath->Format ("%s/%s/%s", GIF_DIR, pCategory, pName);
	}
	else
	{
		pPath->Format ("%s/%s", GIF_DIR, pCategory);
	}
}

boolean CGifManager::IsActive (CConfig *pConfig, const char *pCategory)
{
	for (unsigned i = 0; i < pConfig->nCategories; i++)
	{
		if (strcasecmp (pConfig->Categories[i], pCategory) == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CGifManager::ListCategories (CConfig *pConfig, CString *pJSON)
{
	// place libre sur la carte
	unsigned nFreeMB = 0;
	DWORD nFreeClusters;
	FATFS *pFileSystem;
	if (f_getfree (DRIVE, &nFreeClusters, &pFileSystem) == FR_OK)
	{
		nFreeMB = (unsigned) ((u64) nFreeClusters * pFileSystem->csize * 512 / (1024 * 1024));
	}

	CString Part;
	Part.Format ("{\"libre_mo\":%u,\"categories\":[", nFreeMB);
	pJSON->Append (Part);

	boolean bFirst = TRUE;
	DIR Directory;
	FILINFO Info;
	if (f_opendir (&Directory, GIF_DIR) == FR_OK)
	{
		while (   f_readdir (&Directory, &Info) == FR_OK
		       && Info.fname[0] != '\0')
		{
			if (   !(Info.fattrib & AM_DIR)
			    || (Info.fattrib & (AM_HID | AM_SYS))
			    || !IsValidCategory (Info.fname))
			{
				continue;
			}

			// nombre de GIF dans la categorie
			unsigned nCount = 0;
			CString Path;
			MakePath (&Path, Info.fname);
			DIR SubDirectory;
			FILINFO SubInfo;
			if (f_opendir (&SubDirectory, Path) == FR_OK)
			{
				while (   f_readdir (&SubDirectory, &SubInfo) == FR_OK
				       && SubInfo.fname[0] != '\0')
				{
					YieldToNetwork ();	// laisser tourner le Wi-Fi

					if (   !(SubInfo.fattrib & (AM_DIR | AM_HID | AM_SYS))
					    && EndsWithGif (SubInfo.fname))
					{
						nCount++;
					}
				}
				f_closedir (&SubDirectory);
			}

			pJSON->Append (bFirst ? "{\"nom\":" : ",{\"nom\":");
			AppendJSONString (pJSON, Info.fname);
			Part.Format (",\"nb\":%u,\"active\":%s}", nCount,
				     IsActive (pConfig, Info.fname) ? "true" : "false");
			pJSON->Append (Part);
			bFirst = FALSE;
		}

		f_closedir (&Directory);
	}

	pJSON->Append ("]}\n");
}

void CGifManager::ListFiles (const char *pCategory, CString *pJSON)
{
	pJSON->Append ("{\"categorie\":");
	AppendJSONString (pJSON, pCategory);
	pJSON->Append (",\"fichiers\":[");

	if (IsValidCategory (pCategory))
	{
		CString Path;
		MakePath (&Path, pCategory);

		boolean bFirst = TRUE;
		DIR Directory;
		FILINFO Info;
		if (f_opendir (&Directory, Path) == FR_OK)
		{
			while (   f_readdir (&Directory, &Info) == FR_OK
			       && Info.fname[0] != '\0')
			{
				YieldToNetwork ();		// laisser tourner le Wi-Fi

				if (   (Info.fattrib & (AM_DIR | AM_HID | AM_SYS))
				    || !EndsWithGif (Info.fname))
				{
					continue;
				}

				pJSON->Append (bFirst ? "{\"nom\":" : ",{\"nom\":");
				AppendJSONString (pJSON, Info.fname);
				CString Size;
				Size.Format (",\"taille\":%u}", (unsigned) Info.fsize);
				pJSON->Append (Size);
				bFirst = FALSE;
			}

			f_closedir (&Directory);
		}
	}

	pJSON->Append ("]}\n");
}

boolean CGifManager::ReadGif (const char *pCategory, const char *pName, u8 *pBuffer, unsigned *pLength)
{
	if (!IsValidCategory (pCategory) || !IsValidFileName (pName))
	{
		return FALSE;
	}

	CString Path;
	MakePath (&Path, pCategory, pName);

	return ReadWholeFile (Path, pBuffer, pLength);
}

boolean CGifManager::AddGif (const char *pCategory, const char *pName,
			     const u8 *pData, unsigned nLength, CString *pReply)
{
	if (!IsValidCategory (pCategory))
	{
		*pReply = "ERREUR : nom de categorie invalide (lettres, chiffres, _ ou -)\n";
		return FALSE;
	}

	if (   nLength < 13
	    || (memcmp (pData, "GIF87a", 6) != 0 && memcmp (pData, "GIF89a", 6) != 0))
	{
		pReply->Format ("ERREUR : %s n'est pas un fichier GIF\n", pName);
		return FALSE;
	}

	unsigned nWidth  = pData[6] | (pData[7] << 8);
	unsigned nHeight = pData[8] | (pData[9] << 8);

	CString Folder;
	MakePath (&Folder, pCategory);
	FRESULT Result = f_mkdir (Folder);
	if (Result != FR_OK && Result != FR_EXIST)
	{
		pReply->Format ("ERREUR : creation du dossier %s impossible\n", pCategory);
		return FALSE;
	}

	CString Path;
	MakePath (&Path, pCategory, pName);

	FIL File;
	Result = f_open (&File, Path, FA_WRITE | FA_CREATE_NEW);
	if (Result == FR_EXIST)
	{
		pReply->Format ("ERREUR : %s existe deja dans %s\n", pName, pCategory);
		return FALSE;
	}
	if (Result != FR_OK)
	{
		pReply->Format ("ERREUR : ecriture de %s impossible\n", pName);
		return FALSE;
	}

	UINT nWritten = 0;
	Result = f_write (&File, pData, nLength, &nWritten);
	if (f_close (&File) != FR_OK || Result != FR_OK || nWritten != nLength)
	{
		f_unlink (Path);
		pReply->Format ("ERREUR : ecriture de %s incomplete (carte pleine ?)\n", pName);
		return FALSE;
	}

	CLogger::Get ()->Write (From, LogNotice, "Ajout : %s (%u octets, %ux%u)",
				(const char *) Path, nLength, nWidth, nHeight);

	if (nWidth == 128 && nHeight == 32)
	{
		pReply->Format ("OK : %s ajoutee dans %s\n", pName, pCategory);
	}
	else
	{
		pReply->Format ("OK : %s ajoutee dans %s (taille %ux%u : elle sera centree)\n",
				pName, pCategory, nWidth, nHeight);
	}

	CRemoteControl::Get ()->RequestReload ();

	return TRUE;
}

boolean CGifManager::DeleteGif (CConfig *pConfig, const char *pCategory, const char *pName,
				CString *pReply)
{
	if (!IsValidCategory (pCategory) || !IsValidFileName (pName))
	{
		*pReply = "ERREUR : nom invalide\n";
		return FALSE;
	}

	CString Path;
	MakePath (&Path, pCategory, pName);
	if (f_unlink (Path) != FR_OK)
	{
		pReply->Format ("ERREUR : %s introuvable\n", pName);
		return FALSE;
	}

	CLogger::Get ()->Write (From, LogNotice, "Suppression : %s", (const char *) Path);

	// dossier devenu vide et categorie inactive : on le retire aussi
	if (!IsActive (pConfig, pCategory))
	{
		CString Folder;
		MakePath (&Folder, pCategory);
		f_unlink (Folder);		// echoue sans rien faire si le dossier n'est pas vide
	}

	pReply->Format ("OK : %s supprimee\n", pName);
	CRemoteControl::Get ()->RequestReload ();

	return TRUE;
}

boolean CGifManager::DeleteCategory (CConfig *pConfig, const char *pCategory, CString *pReply)
{
	if (!IsValidCategory (pCategory))
	{
		*pReply = "ERREUR : nom de categorie invalide\n";
		return FALSE;
	}

	// les ecrans de demarrage du panneau sont dans Accueil (SPLASH_DIR = GIF_DIR "/Accueil")
	if (strcasecmp (pCategory, SPLASH_DIR + strlen (GIF_DIR "/")) == 0)
	{
		pReply->Format ("ERREUR : %s contient les ecrans de demarrage du panneau, "
				"supprimez ses GIF une par une si besoin\n", pCategory);
		return FALSE;
	}

	CString Folder;
	MakePath (&Folder, pCategory);
	FILINFO Info;
	if (f_stat (Folder, &Info) != FR_OK || !(Info.fattrib & AM_DIR))
	{
		pReply->Format ("ERREUR : categorie %s introuvable\n", pCategory);
		return FALSE;
	}

	// d'abord la retirer des categories actives (si l'ecriture echoue, rien n'est supprime)
	if (IsActive (pConfig, pCategory))
	{
		CString List;
		for (unsigned i = 0; i < pConfig->nCategories; i++)
		{
			CString Other;
			MakePath (&Other, pConfig->Categories[i]);
			if (   strcasecmp (pConfig->Categories[i], pCategory) == 0
			    || f_stat (Other, &Info) != FR_OK)
			{
				continue;
			}
			if (List.GetLength () > 0)
			{
				List.Append (",");
			}
			List.Append (pConfig->Categories[i]);
		}

		if (List.GetLength () == 0)
		{
			pReply->Format ("ERREUR : %s est la seule categorie affichee, "
					"activez-en une autre d'abord\n", pCategory);
			return FALSE;
		}

		CString Reply;
		if (!SetActiveCategories (pConfig, List, &Reply))
		{
			*pReply = Reply;
			return FALSE;
		}
	}

	unsigned nFiles = 0;
	boolean bOK = DeleteTree (Folder, 0, &nFiles);

	CLogger::Get ()->Write (From, bOK ? LogNotice : LogError, "Categorie %s supprimee : %u fichier(s)%s",
				pCategory, nFiles, bOK ? "" : ", suppression incomplete");
	CRemoteControl::Get ()->RequestReload ();

	if (!bOK)
	{
		pReply->Format ("ERREUR : %s n'a ete supprimee qu'en partie (%u fichier(s) supprime(s))\n",
				pCategory, nFiles);
		return FALSE;
	}

	pReply->Format ("OK : categorie %s supprimee (%u fichier(s))\n", pCategory, nFiles);

	return TRUE;
}

// supprime pPath et son contenu ; les noms sont lus par paquets, dossier ferme,
// avant d'etre supprimes (on ne modifie pas un dossier en cours de lecture)
boolean CGifManager::DeleteTree (const char *pPath, unsigned nDepth, unsigned *pFiles)
{
	static const unsigned BatchSize = 16;
	static const unsigned MaxDepth = 3;

	if (nDepth > MaxDepth)
	{
		return FALSE;
	}

	struct TEntry
	{
		char Name[FF_MAX_LFN + 1];
		BYTE Attributes;
	};
	TEntry *pBatch = new TEntry[BatchSize];		// trop gros pour la pile d'une tache
	if (pBatch == 0)
	{
		return FALSE;
	}

	boolean bOK = TRUE;
	while (bOK)
	{
		unsigned nCount = 0;
		DIR Directory;
		FILINFO Info;
		if (f_opendir (&Directory, pPath) != FR_OK)
		{
			bOK = FALSE;
			break;
		}
		while (   nCount < BatchSize
		       && f_readdir (&Directory, &Info) == FR_OK
		       && Info.fname[0] != '\0')
		{
			strcpy (pBatch[nCount].Name, Info.fname);
			pBatch[nCount].Attributes = Info.fattrib;
			nCount++;
		}
		f_closedir (&Directory);

		if (nCount == 0)
		{
			break;			// dossier vide
		}

		for (unsigned i = 0; i < nCount && bOK; i++)
		{
			CString Path;
			Path.Format ("%s/%s", pPath, pBatch[i].Name);

			if (pBatch[i].Attributes & AM_RDO)
			{
				f_chmod (Path, 0, AM_RDO);
			}

			if (pBatch[i].Attributes & AM_DIR)
			{
				bOK = DeleteTree (Path, nDepth + 1, pFiles);
			}
			else if (f_unlink (Path) == FR_OK)
			{
				(*pFiles)++;
			}
			else
			{
				bOK = FALSE;
			}

			YieldToNetwork ();	// laisser tourner le Wi-Fi entre deux fichiers
		}
	}

	delete [] pBatch;

	return bOK && f_unlink (pPath) == FR_OK;
}

boolean CGifManager::PlayGif (const char *pCategory, const char *pName, CString *pReply)
{
	if (!IsValidCategory (pCategory) || !IsValidFileName (pName))
	{
		*pReply = "ERREUR : nom invalide\n";
		return FALSE;
	}

	CString Path;
	MakePath (&Path, pCategory, pName);

	FILINFO Info;
	if (f_stat (Path, &Info) != FR_OK)
	{
		pReply->Format ("ERREUR : %s introuvable\n", pName);
		return FALSE;
	}

	CRemoteControl::Get ()->RequestPlay (Path);
	pReply->Format ("OK : %s va s'afficher\n", pName);

	return TRUE;
}

boolean CGifManager::SetActiveCategories (CConfig *pConfig, const char *pList, CString *pReply)
{
	// liste "Arcade,Consoles,..." -> lignes Gif_Cat_N
	CString Lines;
	unsigned nCount = 0;
	char Name[MaxCategoryLength + 1];
	unsigned nNameLength = 0;
	boolean bCRLF = FALSE;

	for (const char *p = pList; ; p++)
	{
		if (*p == ',' || *p == '\0')
		{
			Name[nNameLength] = '\0';
			if (nNameLength > 0)
			{
				CString Folder;
				MakePath (&Folder, Name);
				FILINFO Info;
				if (   !IsValidCategory (Name)
				    || f_stat (Folder, &Info) != FR_OK
				    || !(Info.fattrib & AM_DIR))
				{
					pReply->Format ("ERREUR : categorie inconnue : %s\n", Name);
					return FALSE;
				}

				if (++nCount > CONFIG_MAX_CATEGORIES)
				{
					*pReply = "ERREUR : trop de categories actives\n";
					return FALSE;
				}

				CString Line;
				Line.Format ("Gif_Cat_%u=%s\n", nCount, Name);
				Lines.Append (Line);
			}
			nNameLength = 0;

			if (*p == '\0')
			{
				break;
			}
		}
		else if (nNameLength < MaxCategoryLength)
		{
			Name[nNameLength++] = *p;
		}
		else
		{
			*pReply = "ERREUR : nom de categorie trop long\n";
			return FALSE;
		}
	}

	// sans Gif_Cat_N, le panneau afficherait TOUTES les categories : on l'interdit ici
	if (nCount == 0)
	{
		*pReply = "ERREUR : gardez au moins une categorie active\n";
		return FALSE;
	}

	// relire config.txt
	char *pOld = new char[MAX_CONFIG_SIZE + 1];
	unsigned nOldLength = MAX_CONFIG_SIZE;
	if (!ReadWholeFile (CONFIG_FILE, pOld, &nOldLength))
	{
		delete [] pOld;
		*pReply = "ERREUR : lecture de config.txt impossible\n";
		return FALSE;
	}
	pOld[nOldLength] = '\0';
	bCRLF = strstr (pOld, "\r\n") != 0;

	// recopier toutes les lignes sauf Gif_Cat_*, et inserer les nouvelles a la place
	CString New;
	boolean bInserted = FALSE;
	char *pLine = pOld;
	while (*pLine != '\0')
	{
		char *pEnd = strchr (pLine, '\n');
		char *pNext = pEnd != 0 ? pEnd + 1 : pLine + strlen (pLine);
		char chSaved = *pNext;
		*pNext = '\0';

		if (strncasecmp (pLine, "Gif_Cat_", 8) == 0)
		{
			if (!bInserted)
			{
				New.Append (Lines);
				bInserted = TRUE;
			}
		}
		else
		{
			New.Append (pLine);
		}

		*pNext = chSaved;
		pLine = pNext;
	}
	delete [] pOld;

	if (!bInserted)
	{
		New.Append ("\n");
		New.Append (Lines);
	}

	// fins de ligne Windows si le fichier en avait
	if (bCRLF)
	{
		CString Converted;
		for (const char *p = New; *p != '\0'; p++)
		{
			if (*p == '\n' && (p == (const char *) New || p[-1] != '\r'))
			{
				Converted.Append ('\r');
			}
			Converted.Append (*p);
		}
		New = Converted;
	}

	if (!SafeWriteFile (CONFIG_FILE, DRIVE "/Config/config.new", DRIVE "/Config/config.bak",
			    (const char *) New, New.GetLength ()))
	{
		*pReply = "ERREUR : ecriture de config.txt impossible\n";
		return FALSE;
	}

	pConfig->Load (CONFIG_FILE);
	CRemoteControl::Get ()->RequestReload ();

	CLogger::Get ()->Write (From, LogNotice, "Categories actives : %s", pList);
	pReply->Format ("OK : %u categorie(s) active(s)\n", nCount);

	return TRUE;
}
