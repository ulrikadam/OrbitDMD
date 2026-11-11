//
// fileutil.cpp
//
#include "fileutil.h"
#include <fatfs/ff.h>
#include <circle/sched/scheduler.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "fichier";

#define CHUNK_SIZE	65536

boolean SafeWriteFile (const char *pFinal, const char *pTemp, const char *pBackup,
		       const void *pData, unsigned nSize)
{
	CLogger *pLogger = CLogger::Get ();
	const u8 *pBytes = (const u8 *) pData;

	// 1. ecrire le fichier temporaire
	FIL File;
	if (f_open (&File, pTemp, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
	{
		pLogger->Write (From, LogError, "Creation impossible : %s", pTemp);
		return FALSE;
	}

	boolean bOK = TRUE;
	for (unsigned nOffset = 0; bOK && nOffset < nSize; nOffset += CHUNK_SIZE)
	{
		UINT nChunk = nSize - nOffset < CHUNK_SIZE ? nSize - nOffset : CHUNK_SIZE;
		UINT nWritten = 0;
		bOK =    f_write (&File, pBytes + nOffset, nChunk, &nWritten) == FR_OK
		      && nWritten == nChunk;
	}

	if (f_close (&File) != FR_OK || !bOK)
	{
		pLogger->Write (From, LogError, "Ecriture incomplete : %s", pTemp);
		f_unlink (pTemp);
		return FALSE;
	}

	// 2. relire et comparer
	u8 *pChunk = new u8[CHUNK_SIZE];
	if (   pChunk == 0
	    || f_open (&File, pTemp, FA_READ | FA_OPEN_EXISTING) != FR_OK)
	{
		delete [] pChunk;
		pLogger->Write (From, LogError, "Relecture impossible : %s", pTemp);
		f_unlink (pTemp);
		return FALSE;
	}

	bOK = f_size (&File) == nSize;
	for (unsigned nOffset = 0; bOK && nOffset < nSize; nOffset += CHUNK_SIZE)
	{
		UINT nChunk = nSize - nOffset < CHUNK_SIZE ? nSize - nOffset : CHUNK_SIZE;
		UINT nRead = 0;
		bOK =    f_read (&File, pChunk, nChunk, &nRead) == FR_OK
		      && nRead == nChunk
		      && memcmp (pChunk, pBytes + nOffset, nChunk) == 0;
	}
	f_close (&File);
	delete [] pChunk;

	if (!bOK)
	{
		pLogger->Write (From, LogError, "Verification echouee : %s", pTemp);
		f_unlink (pTemp);
		return FALSE;
	}

	// 3. garder l'ancienne version, puis mettre la nouvelle en place
	FRESULT Result;
	if (pBackup != 0)
	{
		f_unlink (pBackup);
		Result = f_rename (pFinal, pBackup);
	}
	else
	{
		Result = f_unlink (pFinal);
	}

	if (Result != FR_OK && Result != FR_NO_FILE)
	{
		pLogger->Write (From, LogError, "Remplacement impossible : %s", pFinal);
		f_unlink (pTemp);
		return FALSE;
	}

	if (f_rename (pTemp, pFinal) != FR_OK)
	{
		pLogger->Write (From, LogError, "Remplacement impossible : %s", pFinal);
		if (pBackup != 0)
		{
			f_rename (pBackup, pFinal);
		}
		return FALSE;
	}

	return TRUE;
}

boolean ReadWholeFile (const char *pPath, void *pBuffer, unsigned *pLength)
{
	FIL File;
	if (f_open (&File, pPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
	{
		return FALSE;
	}

	FSIZE_t nSize = f_size (&File);
	if (nSize > *pLength)
	{
		f_close (&File);
		return FALSE;
	}

	boolean bOK = ReadChunked (&File, pBuffer, (unsigned) nSize);
	f_close (&File);

	if (!bOK)
	{
		return FALSE;
	}

	*pLength = (unsigned) nSize;

	return TRUE;
}

boolean ReadChunked (FIL *pFile, void *pBuffer, unsigned nSize)
{
	u8 *pBytes = (u8 *) pBuffer;

	for (unsigned nOffset = 0; nOffset < nSize; nOffset += CHUNK_SIZE)
	{
		UINT nChunk = nSize - nOffset < CHUNK_SIZE ? nSize - nOffset : CHUNK_SIZE;
		UINT nRead = 0;
		if (f_read (pFile, pBytes + nOffset, nChunk, &nRead) != FR_OK || nRead != nChunk)
		{
			return FALSE;
		}

		// le pilote SD ne rend pas la main : on la rend ici, pour le Wi-Fi et la page web
		YieldToNetwork ();
	}

	return TRUE;
}

void YieldToNetwork (void)
{
	if (CScheduler::IsActive ())
	{
		CScheduler::Get ()->Yield ();
	}
}
