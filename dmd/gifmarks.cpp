//
// gifmarks.cpp
//
#include "gifmarks.h"
#include "common.h"
#include "fileutil.h"
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "gif";

#define GIF_MARKS_TEMP		DRIVE "/Config/gif-marques.new"
#define MAX_FILE_SIZE		(CGifMarks::MaxMarks * 140)

CGifMarks::TEntry *CGifMarks::s_pEntries = 0;
unsigned CGifMarks::s_nEntries = 0;

boolean CGifMarks::MakeKey (const char *pCategory, const char *pName, char *pKey)
{
	size_t nCategory = strlen (pCategory), nName = strlen (pName);
	if (   nCategory == 0 || nName == 0 || nCategory + 1 + nName >= sizeof s_pEntries->Key
	    || strchr (pCategory, '/') != 0 || strchr (pName, '/') != 0)
	{
		return FALSE;
	}

	memcpy (pKey, pCategory, nCategory);
	pKey[nCategory] = '/';
	memcpy (pKey + nCategory + 1, pName, nName + 1);

	return TRUE;
}

// recherche dichotomique ; si absent, position ou l'inserer
int CGifMarks::Find (const char *pKey, boolean *pFound)
{
	int nLow = 0, nHigh = (int) s_nEntries - 1;
	while (nLow <= nHigh)
	{
		int nMiddle = (nLow + nHigh) / 2;
		int nCompare = strcasecmp (pKey, s_pEntries[nMiddle].Key);
		if (nCompare == 0)
		{
			*pFound = TRUE;
			return nMiddle;
		}
		if (nCompare < 0)	nHigh = nMiddle - 1;
		else			nLow = nMiddle + 1;
	}

	*pFound = FALSE;
	return nLow;
}

TGifMark CGifMarks::Get (const char *pCategory, const char *pName)
{
	char Key[sizeof s_pEntries->Key];
	boolean bFound;
	if (s_nEntries == 0 || !MakeKey (pCategory, pName, Key))
	{
		return GifMarkNone;
	}

	int nPos = Find (Key, &bFound);

	return bFound ? (TGifMark) s_pEntries[nPos].Mark : GifMarkNone;
}

boolean CGifMarks::Set (const char *pCategory, const char *pName, TGifMark Mark)
{
	if (s_pEntries == 0)
	{
		s_pEntries = new TEntry[MaxMarks];
	}

	char Key[sizeof s_pEntries->Key];
	if (!MakeKey (pCategory, pName, Key))
	{
		return FALSE;
	}

	boolean bFound;
	int nPos = Find (Key, &bFound);
	if (bFound)
	{
		if (Mark == GifMarkNone)
		{
			memmove (&s_pEntries[nPos], &s_pEntries[nPos + 1], (s_nEntries - nPos - 1) * sizeof (TEntry));
			s_nEntries--;
		}
		else
		{
			s_pEntries[nPos].Mark = (char) Mark;
		}
	}
	else if (Mark != GifMarkNone)
	{
		if (s_nEntries >= MaxMarks)
		{
			return FALSE;
		}
		memmove (&s_pEntries[nPos + 1], &s_pEntries[nPos], (s_nEntries - nPos) * sizeof (TEntry));
		strcpy (s_pEntries[nPos].Key, Key);
		s_pEntries[nPos].Mark = (char) Mark;
		s_nEntries++;
	}
	else
	{
		return TRUE;			// rien a changer
	}

	return Save ();
}

void CGifMarks::ForgetCategory (const char *pCategory)
{
	size_t nLength = strlen (pCategory);
	unsigned nKept = 0;
	for (unsigned i = 0; i < s_nEntries; i++)
	{
		if (   strncasecmp (s_pEntries[i].Key, pCategory, nLength) == 0
		    && s_pEntries[i].Key[nLength] == '/')
		{
			continue;
		}
		s_pEntries[nKept++] = s_pEntries[i];
	}

	if (nKept != s_nEntries)
	{
		s_nEntries = nKept;
		Save ();
	}
}

void CGifMarks::Export (CString *pText)
{
	for (unsigned i = 0; i < s_nEntries; i++)
	{
		pText->Append (s_pEntries[i].Mark == GifMarkFavorite ? "+" : "-");
		pText->Append (s_pEntries[i].Key);
		pText->Append ("\n");
	}
}

unsigned CGifMarks::Import (const char *pText, boolean bSave)
{
	if (s_pEntries == 0)
	{
		s_pEntries = new TEntry[MaxMarks];
	}
	s_nEntries = 0;

	unsigned nRead = 0;
	const char *p = pText;
	while (*p != '\0')
	{
		const char *pEnd = p;
		while (*pEnd != '\0' && *pEnd != '\n' && *pEnd != '\r')
		{
			pEnd++;
		}

		// "+Categorie/nom.gif" ou "-Categorie/nom.gif"
		size_t nLength = pEnd - p;
		if (   nLength >= 3 && nLength - 1 < sizeof s_pEntries->Key && (*p == '+' || *p == '-')
		    && s_nEntries < MaxMarks)
		{
			char Key[sizeof s_pEntries->Key];
			memcpy (Key, p + 1, nLength - 1);
			Key[nLength - 1] = '\0';

			char *pSlash = strchr (Key, '/');
			if (pSlash != 0 && pSlash != Key && pSlash[1] != '\0' && strchr (pSlash + 1, '/') == 0)
			{
				boolean bFound;
				int nPos = Find (Key, &bFound);
				if (!bFound)
				{
					memmove (&s_pEntries[nPos + 1], &s_pEntries[nPos],
						 (s_nEntries - nPos) * sizeof (TEntry));
					strcpy (s_pEntries[nPos].Key, Key);
					s_nEntries++;
				}
				s_pEntries[nPos].Mark = *p;
				nRead++;
			}
		}

		p = pEnd;
		while (*p == '\n' || *p == '\r')
		{
			p++;
		}
	}

	if (bSave)
	{
		Save ();
	}

	return nRead;
}

unsigned CGifMarks::GetCount (TGifMark Mark)
{
	unsigned n = 0;
	for (unsigned i = 0; i < s_nEntries; i++)
	{
		if (s_pEntries[i].Mark == (char) Mark)
		{
			n++;
		}
	}

	return n;
}

#ifndef HOST_TEST

void CGifMarks::Load (void)
{
	char *pBuffer = new char[MAX_FILE_SIZE + 1];
	unsigned nLength = MAX_FILE_SIZE;
	if (!ReadWholeFile (GIF_MARKS_FILE, pBuffer, &nLength))
	{
		nLength = 0;			// pas encore de fichier : aucune marque
	}
	pBuffer[nLength] = '\0';

	Import (pBuffer, FALSE);
	delete [] pBuffer;

	if (s_nEntries > 0)
	{
		CLogger::Get ()->Write (From, LogNotice, "%u GIF favorite(s), %u exclue(s)",
					GetCount (GifMarkFavorite), GetCount (GifMarkExcluded));
	}
}

boolean CGifMarks::Save (void)
{
	CString Text;
	Export (&Text);

	if (!SafeWriteFile (GIF_MARKS_FILE, GIF_MARKS_TEMP, 0, (const char *) Text, Text.GetLength ()))
	{
		CLogger::Get ()->Write (From, LogError, "Ecriture de %s impossible", GIF_MARKS_FILE);
		return FALSE;
	}

	return TRUE;
}

#else

void CGifMarks::Load (void) {}
boolean CGifMarks::Save (void) { return TRUE; }

#endif
