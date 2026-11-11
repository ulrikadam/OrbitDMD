//
// configedit.cpp
//
#include "configedit.h"
#include <circle/util.h>

#define MAX_CHANGES	64

// ajoute nLength octets a la sortie ; FALSE si elle deborde
static boolean Put (char *pOut, unsigned nOutSize, unsigned *pPos, const char *pText, unsigned nLength)
{
	if (*pPos + nLength >= nOutSize)
	{
		return FALSE;
	}

	memcpy (pOut + *pPos, pText, nLength);
	*pPos += nLength;

	return TRUE;
}

static boolean PutString (char *pOut, unsigned nOutSize, unsigned *pPos, const char *pText)
{
	return Put (pOut, nOutSize, pPos, pText, (unsigned) strlen (pText));
}

// la ligne [pLine, pLine + nLength[ definit-elle la cle pKey ?
static boolean LineHasKey (const char *pLine, unsigned nLength, const char *pKey)
{
	unsigned i = 0;
	while (i < nLength && (pLine[i] == ' ' || pLine[i] == '\t'))
	{
		i++;
	}

	size_t nKeyLength = strlen (pKey);
	if (i + nKeyLength > nLength || strncasecmp (pLine + i, pKey, nKeyLength) != 0)
	{
		return FALSE;
	}
	i += (unsigned) nKeyLength;

	while (i < nLength && (pLine[i] == ' ' || pLine[i] == '\t'))
	{
		i++;
	}

	return i < nLength && pLine[i] == '=';
}

int EditConfigText (const char *pIn, const TConfigChange *pChanges, unsigned nChanges,
		    char *pOut, unsigned nOutSize)
{
	if (nChanges > MAX_CHANGES)
	{
		return -1;
	}

	boolean Applied[MAX_CHANGES];
	for (unsigned i = 0; i < nChanges; i++)
	{
		Applied[i] = FALSE;
	}

	const char *pNewline = strstr (pIn, "\r\n") != 0 ? "\r\n" : "\n";
	unsigned nPos = 0;

	const char *pLine = pIn;
	while (*pLine != '\0')
	{
		const char *pEnd = strchr (pLine, '\n');
		const char *pNext = pEnd != 0 ? pEnd + 1 : pLine + strlen (pLine);

		// longueur de la ligne sans la fin de ligne
		unsigned nLength = (unsigned) (pNext - pLine);
		if (nLength > 0 && pLine[nLength-1] == '\n')	nLength--;
		if (nLength > 0 && pLine[nLength-1] == '\r')	nLength--;

		int nChange = -1;
		if (pLine[0] != '#')
		{
			for (unsigned i = 0; i < nChanges; i++)
			{
				if (LineHasKey (pLine, nLength, pChanges[i].pKey))
				{
					nChange = (int) i;
					break;
				}
			}
		}

		if (nChange >= 0)
		{
			// garder l'ecriture d'origine de la cle, remplacer la valeur
			unsigned nEqual = 0;
			while (pLine[nEqual] != '=')		// present : verifie par LineHasKey ()
			{
				nEqual++;
			}

			if (   !Put (pOut, nOutSize, &nPos, pLine, nEqual + 1)
			    || !PutString (pOut, nOutSize, &nPos, pChanges[nChange].pValue))
			{
				return -1;
			}

			// recopier la fin de ligne d'origine
			if (!Put (pOut, nOutSize, &nPos, pLine + nLength, (unsigned) (pNext - pLine) - nLength))
			{
				return -1;
			}

			Applied[nChange] = TRUE;
		}
		else if (!Put (pOut, nOutSize, &nPos, pLine, (unsigned) (pNext - pLine)))
		{
			return -1;
		}

		pLine = pNext;
	}

	// cles absentes : ajoutees a la fin
	for (unsigned i = 0; i < nChanges; i++)
	{
		if (Applied[i])
		{
			continue;
		}

		if (nPos > 0 && pOut[nPos-1] != '\n')
		{
			if (!PutString (pOut, nOutSize, &nPos, pNewline))
			{
				return -1;
			}
		}

		if (   !PutString (pOut, nOutSize, &nPos, pChanges[i].pKey)
		    || !PutString (pOut, nOutSize, &nPos, "=")
		    || !PutString (pOut, nOutSize, &nPos, pChanges[i].pValue)
		    || !PutString (pOut, nOutSize, &nPos, pNewline))
		{
			return -1;
		}
	}

	pOut[nPos] = '\0';

	return (int) nPos;
}
