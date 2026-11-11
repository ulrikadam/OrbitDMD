//
// dmdapp.cpp
//
#include "dmdapp.h"
#include "gifmarks.h"
#include "messages.h"
#include "common.h"
#include "testpattern.h"
#include "clock.h"
#include "remote.h"
#include "fileutil.h"
#include "settings.h"
#include "weather.h"
#include <fatfs/ff.h>
#include <circle/sched/scheduler.h>
#include <circle/startup.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "dmd";

CDmdApp::CDmdApp (CMemorySystem *pMemorySystem, CMatrix *pMatrix, CConfig *pConfig,
		  CNetwork *pNetwork)
:	CMultiCoreSupport (pMemorySystem),
	m_pMatrix (pMatrix),
	m_pConfig (pConfig),
	m_pNetwork (pNetwork),
	m_bFileSystemOK (FALSE),
	m_pCanvas (0),
	m_pAnimation (0),
	m_pClock (0),
	m_pRandom (0),
	m_pFileBuffer (0),
	m_pBlack (0),
	m_ppFiles (0),
	m_nFiles (0),
	m_bInRemoteRequest (FALSE),
	m_bWifiTrial (FALSE),
	m_nGifsSinceWeather (0),
	m_nExcluded (0),
	m_nGifsSinceBanner (0),
	m_nGifsSinceCountdown (0),
	m_nNextText (0),
	m_nNextCountdown (0),
	m_nRateTicks (0),
	m_nRateCount (0)
{
}

CDmdApp::~CDmdApp (void)
{
}

boolean CDmdApp::Prepare (boolean bFileSystemOK)
{
	m_bFileSystemOK = bFileSystemOK;

	unsigned nWidth = m_pMatrix->GetWidth ();
	unsigned nHeight = m_pMatrix->GetHeight ();

	m_pCanvas = new CCanvas (nWidth, nHeight);
	m_pAnimation = new CGifAnimation (nWidth, nHeight);
	m_pClock = new CClockRenderer (nWidth, nHeight);
	m_pRandom = new CBcmRandomNumberGenerator;
	m_pFileBuffer = new u8[FileBufferSize];
	m_pBlack = new u8[nWidth * nHeight * 3];
	m_ppFiles = new CString *[MaxFiles];

	CGifAnimation::SetYieldHandler (YieldHandler);

	if (m_bFileSystemOK)
	{
		CGifMarks::Load ();		// GIF favorites et exclues
		CMessages::Load (m_pConfig);	// textes et comptes a rebours
	}

	if (m_pBlack != 0)
	{
		memset (m_pBlack, 0, nWidth * nHeight * 3);
	}

	return    m_pCanvas != 0 && m_pAnimation != 0 && m_pClock != 0 && m_pRandom != 0
	       && m_pFileBuffer != 0 && m_pBlack != 0 && m_ppFiles != 0;
}

void CDmdApp::Run (unsigned nCore)
{
	switch (nCore)
	{
	case 0:
		MainLoop ();
		break;

	case RefreshCore:
		m_pMatrix->RefreshLoop ();	// ne revient jamais
		break;

	default:
		break;				// coeurs 1 et 2 libres pour l'instant
	}
}

void CDmdApp::MainLoop (void)
{
	UpdateBrightness ();

	// nouveau Wi-Fi en essai (change depuis la page Reglages) ?
	FILINFO TrialInfo;
	m_bWifiTrial = m_bFileSystemOK && f_stat (WIFI_TRIAL_FILE, &TrialInfo) == FR_OK;
	if (m_bWifiTrial)
	{
		CLogger::Get ()->Write (From, LogNotice, "Essai des nouveaux reglages reseau (%u s maximum)",
					WIFI_TRIAL_SECONDS);
	}

	RunBootSplash ();		// demarre aussi le Wi-Fi

	if (m_pConfig->TestPattern)
	{
		RunTestPattern ();
	}

	if (!m_bFileSystemOK)
	{
		ShowMessage ("CARTE SD", "ILLISIBLE", 255, 0, 0);
		while (TRUE)
		{
			Wait (1000);
		}
	}

	if (m_pConfig->GifActive)
	{
		BuildPlaylist ();
		if (m_nFiles == 0)
		{
			ShowMessage ("AUCUNE GIF", "DANS /GIF", 255, 128, 0);
			Wait (5000);
		}
	}

	CRemoteControl *pRemote = CRemoteControl::Get ();

	while (TRUE)
	{
		if (m_pConfig->GifActive && m_nFiles > 0)
		{
			if (m_pConfig->GifInOrder)
			{
				SortPlaylist ();
			}
			else
			{
				Shuffle ();
			}

			for (unsigned i = 0; i < m_nFiles; i++)
			{
				PlayRequestedGifs ();

				// GIF ajoutee, supprimee ou categories changees depuis la page web
				if (pRemote->TakeReloadRequest ())
				{
					BuildPlaylist ();
					break;		// la liste a change : on la remelange
				}

				UpdateBrightness ();
				PlayGif (*m_ppFiles[i]);
				CheckConnectedEvent ();

				if (m_pConfig->ClockActive && !pRemote->IsPlayRequested ())
				{
					ShowClock (m_pConfig->ClockDisplayTime);
				}

				// meteo toutes les N GIF
				if (   ++m_nGifsSinceWeather >= m_pConfig->WeatherStyle.nEvery
				    && !pRemote->IsPlayRequested ())
				{
					m_nGifsSinceWeather = 0;
					ShowWeather (m_pConfig->WeatherStyle.nDisplayTime);
				}

				ShowMessages (FALSE);
			}
		}
		else
		{
			PlayRequestedGifs ();
			if (pRemote->TakeReloadRequest () && m_pConfig->GifActive)
			{
				BuildPlaylist ();
				continue;
			}

			UpdateBrightness ();
			CheckConnectedEvent ();
			ShowClock (m_pConfig->GifActive ? 5000 : 60000);
			if (!pRemote->IsPlayRequested ())
			{
				ShowWeather (m_pConfig->WeatherStyle.nDisplayTime);
			}
			ShowMessages (TRUE);
		}
	}
}

void CDmdApp::PlayRequestedGifs (void)
{
	CString Path;
	while (CRemoteControl::Get ()->TakePlayRequest (&Path))
	{
		UpdateBrightness ();
		if (strcmp (Path, CLOCK_PLAY_REQUEST) == 0)
		{
			ShowClock (CLOCK_DEMO_MS);	// "Voir sur le panneau" de la page Horloge
		}
		else if (strcmp (Path, WEATHER_PLAY_REQUEST) == 0)
		{
			ShowWeather (CLOCK_DEMO_MS);	// "Voir sur le panneau" de la page Meteo
		}
		else if (   strncmp (Path, BANNER_PLAY_REQUEST ":", strlen (BANNER_PLAY_REQUEST) + 1) == 0
			 || strncmp (Path, COUNTDOWN_PLAY_REQUEST ":", strlen (COUNTDOWN_PLAY_REQUEST) + 1) == 0)
		{
			ShowMessageNow (Path);		// "Afficher maintenant" de la page Messages
		}
		else
		{
			PlayGif (Path);
		}
	}
}

void CDmdApp::RunTestPattern (void)
{
	CRemoteControl::Get ()->Status.pScreen = "mire";
	CCanvas &C = *m_pCanvas;
	const int W = (int) C.GetWidth ();
	const int H = (int) C.GetHeight ();
	CLogger *pLogger = CLogger::Get ();

	// accueil, couleurs pleines, degrades, geometrie (voir testpattern.cpp)
	for (unsigned i = 0; i < TestPatternGetCount (); i++)
	{
		TestPatternDraw (&C, i, m_pConfig->PanelCols, m_pConfig->PanelRows,
				 m_pConfig->Chain, m_pConfig->Parallel);
		m_pMatrix->Show (C.GetData ());
		pLogger->Write (From, LogNotice, "Mire : %s", TestPatternGetInfo (i)->pName);
		Wait (TestPatternGetInfo (i)->nDuration);
	}

	// heure (horloge a pile, ou Internet si deja recue)
	TDateTime Time;
	C.Clear ();
	if (CClock::Get ()->GetLocalTime (&Time))
	{
		CString Line1, Line2;
		Line1.Format ("%02u:%02u:%02u", Time.Hour, Time.Minute, Time.Second);
		Line2.Format ("%02u/%02u/%04u", Time.Day, Time.Month, Time.Year);
		C.DrawTextCentered (0, W, 4, Line1, 0, 255, 0, 2);
		C.DrawTextCentered (0, W, H - 9, Line2, 0, 160, 0);
		pLogger->Write (From, LogNotice, "Heure : %s %s (%s)", (const char *) Line1,
				(const char *) Line2, CClock::Get ()->GetSourceName ());
	}
	else
	{
		C.DrawTextCentered (0, W, 4, "HORLOGE", 255, 0, 0);
		C.DrawTextCentered (0, W, H - 11, "ABSENTE", 255, 0, 0);
	}
	m_pMatrix->Show (C.GetData ());
	Wait (3000);

	// frequence de rafraichissement mesuree
	unsigned nCount = m_pMatrix->GetRefreshCount ();
	Wait (1000);
	nCount = m_pMatrix->GetRefreshCount () - nCount;

	CString Rate;
	Rate.Format ("%u HZ", nCount);
	C.Clear ();
	C.DrawTextCentered (0, W, 2, "RAFRAICHISSEMENT", 255, 255, 255);
	C.DrawTextCentered (0, W, 14, Rate, 255, 255, 0, 2);
	m_pMatrix->Show (C.GetData ());
	pLogger->Write (From, LogNotice, "Rafraichissement : %u images/s", nCount);
	Wait (3000);

	CheckConnectedEvent ();
}

void CDmdApp::ShowMessage (const char *pLine1, const char *pLine2, u8 r, u8 g, u8 b)
{
	CRemoteControl::Get ()->Status.pScreen = "message";
	CCanvas &C = *m_pCanvas;
	const int W = (int) C.GetWidth ();
	const int H = (int) C.GetHeight ();

	C.Clear ();
	C.DrawTextCentered (0, W, H / 2 - 9, pLine1, r, g, b);
	C.DrawTextCentered (0, W, H / 2 + 2, pLine2, r, g, b);
	m_pMatrix->Show (C.GetData ());

	CLogger::Get ()->Write (From, LogNotice, "Panneau : %s %s", pLine1, pLine2);
}

// adresse de la page web : "WIFI CONNECTE" (ou "CABLE CONNECTE"), <nom>.local et l'adresse IP (reglage IP_Ecran)
void CDmdApp::ShowAddress (void)
{
	const char *pIP = m_pNetwork->GetIPAddress ();
	const char *pTitle = m_pNetwork->IsEthernet () ? "CABLE CONNECTE" : "WIFI CONNECTE";
	CString Name;
	Name.Format ("%s.local", m_pConfig->NetworkName);

	CLogger::Get ()->Write (From, LogNotice, "Panneau : adresse %s / %s",
				(const char *) Name, pIP);

	unsigned nTime = m_pConfig->IPScreenTime;
	if (nTime == 0)
	{
		return;
	}
	if (nTime < 2000)
	{
		nTime = 2000;
	}

	CRemoteControl::Get ()->Status.pScreen = "message";
	CCanvas &C = *m_pCanvas;
	const int W = (int) C.GetWidth ();
	const int H = (int) C.GetHeight ();

	C.Clear ();
	if (H >= 26 && CCanvas::GetTextWidth (Name) <= W)
	{
		C.DrawTextCentered (0, W, H / 2 - 13, pTitle, 0, 255, 0);
		C.DrawTextCentered (0, W, H / 2 - 4, Name, 255, 255, 255);
		C.DrawTextCentered (0, W, H / 2 + 5, pIP, 0, 220, 255);
	}
	else
	{
		C.DrawTextCentered (0, W, H / 2 - 9, pTitle, 0, 255, 0);
		C.DrawTextCentered (0, W, H / 2 + 2, pIP, 0, 220, 255);
	}
	m_pMatrix->Show (C.GetData ());

	Wait (nTime);
}

void CDmdApp::CheckConnectedEvent (void)
{
	if (m_pNetwork->TakeConnectedEvent ())
	{
		ShowAddress ();
	}
	else if (m_pNetwork->TakeTimeoutEvent ())
	{
		ShowMessage (m_pNetwork->IsEthernet () ? "CABLE RESEAU" : "WIFI", "NON CONNECTE", 255, 0, 0);
		Wait (5000);
	}
}

void CDmdApp::BuildPlaylist (void)
{
	for (unsigned i = 0; i < m_nFiles; i++)
	{
		delete m_ppFiles[i];
	}
	m_nFiles = 0;
	m_nExcluded = 0;

	if (m_pConfig->nCategories > 0)
	{
		for (unsigned i = 0; i < m_pConfig->nCategories; i++)
		{
			CString Path;
			Path.Format ("%s/%s", GIF_DIR, m_pConfig->Categories[i]);
			AddFolder (Path, FALSE);
		}
	}
	else
	{
		AddFolder (GIF_DIR, TRUE);	// toutes les categories
	}

	CRemoteControl::Get ()->Status.nGifCount = m_nFiles;
	CLogger::Get ()->Write (From, LogNotice, "%u GIF dans la liste de lecture (%s)%s", m_nFiles,
				m_pConfig->GifInOrder ? "dans l'ordre" : "au hasard",
				m_nExcluded > 0 ? ", GIF exclues retirees" : "");
}

void CDmdApp::AddFolder (const char *pPath, boolean bRecurse)
{
	const char *pCategory = pPath;			// "SD:/gif/Arcade" -> "Arcade"
	for (const char *p = pPath; *p != '\0'; p++)
	{
		if (*p == '/')
		{
			pCategory = p + 1;
		}
	}

	DIR Directory;
	if (f_opendir (&Directory, pPath) != FR_OK)
	{
		CLogger::Get ()->Write (From, LogWarning, "Dossier introuvable : %s", pPath);
		return;
	}

	FILINFO Info;
	while (   f_readdir (&Directory, &Info) == FR_OK
	       && Info.fname[0] != '\0')
	{
		YieldToNetwork ();		// laisser tourner le Wi-Fi

		if (Info.fattrib & (AM_HID | AM_SYS))
		{
			continue;
		}

		CString Path;
		Path.Format ("%s/%s", pPath, Info.fname);

		if (Info.fattrib & AM_DIR)
		{
			if (bRecurse)
			{
				AddFolder (Path, FALSE);
			}
			continue;
		}

		size_t nLength = strlen (Info.fname);
		if (   nLength < 5
		    || strcasecmp (Info.fname + nLength - 4, ".gif") != 0
		    || m_nFiles >= MaxFiles)
		{
			continue;
		}

		// GIF exclue : pas dans la rotation ; favorite : plusieurs fois (lecture au hasard)
		TGifMark Mark = CGifMarks::Get (pCategory, Info.fname);
		if (Mark == GifMarkExcluded)
		{
			m_nExcluded++;
			continue;
		}
		unsigned nCopies = Mark == GifMarkFavorite && !m_pConfig->GifInOrder ? GIF_FAVORITE_WEIGHT : 1;
		for (unsigned i = 0; i < nCopies && m_nFiles < MaxFiles; i++)
		{
			m_ppFiles[m_nFiles++] = new CString ((const char *) Path);
		}
	}

	f_closedir (&Directory);
}

// lecture dans l'ordre : par categorie puis par nom (sans tenir compte des majuscules) ;
// tri de Shell (Circle n'a pas de qsort), rapide pour 20000 noms
void CDmdApp::SortPlaylist (void)
{
	static const unsigned Gaps[] = {13776, 4592, 1968, 861, 336, 112, 48, 21, 7, 3, 1};

	for (unsigned g = 0; g < sizeof Gaps / sizeof Gaps[0]; g++)
	{
		unsigned nGap = Gaps[g];
		for (unsigned i = nGap; i < m_nFiles; i++)
		{
			CString *pItem = m_ppFiles[i];
			unsigned j = i;
			while (j >= nGap && strcasecmp (*m_ppFiles[j - nGap], *pItem) > 0)
			{
				m_ppFiles[j] = m_ppFiles[j - nGap];
				j -= nGap;
			}
			m_ppFiles[j] = pItem;
		}
		YieldToNetwork ();
	}
}

void CDmdApp::Shuffle (void)
{
	for (unsigned i = m_nFiles; i > 1; i--)
	{
		unsigned j = m_pRandom->GetNumber () % i;
		CString *pTemp = m_ppFiles[i-1];
		m_ppFiles[i-1] = m_ppFiles[j];
		m_ppFiles[j] = pTemp;
	}
}

boolean CDmdApp::PlayGif (const char *pPath)
{
	if (!LoadGif (pPath))
	{
		return FALSE;
	}

	CRemoteControl::Get ()->Status.CurrentGif = pPath;
	CRemoteControl::Get ()->Status.pScreen = "gif";
	PlayAnimation (m_pConfig->GifMinDuration, FALSE, 0);

	// la derniere image reste affichee pendant le chargement du GIF suivant
	m_pAnimation->Clear ();

	return TRUE;
}

boolean CDmdApp::LoadGif (const char *pPath)
{
	CLogger *pLogger = CLogger::Get ();

	FIL File;
	if (f_open (&File, pPath, FA_READ | FA_OPEN_EXISTING) != FR_OK)
	{
		pLogger->Write (From, LogWarning, "Ouverture impossible : %s", pPath);
		return FALSE;
	}

	FSIZE_t nSize = f_size (&File);
	if (nSize == 0 || nSize > FileBufferSize)
	{
		f_close (&File);
		pLogger->Write (From, LogWarning, "Taille non geree : %s", pPath);
		return FALSE;
	}

	boolean bRead = ReadChunked (&File, m_pFileBuffer, (unsigned) nSize);	// rend la main au Wi-Fi
	f_close (&File);
	if (!bRead)
	{
		pLogger->Write (From, LogWarning, "Lecture impossible : %s", pPath);
		return FALSE;
	}

	if (!m_pAnimation->Decode (m_pFileBuffer, (unsigned) nSize))
	{
		pLogger->Write (From, LogWarning, "GIF invalide : %s", pPath);
		return FALSE;
	}

	pLogger->Write (From, LogNotice, "%s : %u images, %u ms", pPath,
			m_pAnimation->GetFrameCount (), m_pAnimation->GetTotalDuration ());

	return TRUE;
}

void CDmdApp::PlayAnimation (unsigned nMinDuration, boolean bUntilBootReady, unsigned nMaxDuration)
{
	unsigned nFrames = m_pAnimation->GetFrameCount ();
	unsigned nTotal = m_pAnimation->GetTotalDuration ();
	if (nFrames == 0)
	{
		return;
	}

	CRemoteControl *pRemote = CRemoteControl::Get ();
	boolean bWasRequested = pRemote->IsPlayRequested ();	// normalement FALSE ici

	u64 nStart = CTimer::GetClockTicks64 ();
	u64 nNext = nStart;
	while (TRUE)
	{
		// une boucle complete de l'animation
		for (unsigned i = 0; i < nFrames; i++)
		{
			// pause ou affichage eteint : le temps d'attente ne compte pas
			u64 nHeld = HoldWhileStopped (m_pAnimation->GetFrame (i));
			nNext += nHeld;
			nStart += nHeld;

			// une GIF demandee depuis la page web interrompt celle-ci
			if (!bWasRequested && pRemote->IsPlayRequested ())
			{
				return;
			}

			m_pMatrix->Show (m_pAnimation->GetFrame (i));

			nNext += (u64) m_pAnimation->GetDelay (i) * 1000;
			WaitUntil (nNext);
		}

		unsigned nElapsed = (unsigned) ((CTimer::GetClockTicks64 () - nStart) / 1000);
		if (bUntilBootReady)
		{
			if (   (IsBootReady () && nElapsed >= nMinDuration)
			    || nElapsed >= nMaxDuration)
			{
				return;
			}
		}
		else if (nElapsed >= nMinDuration || nTotal == 0)
		{
			return;
		}
	}
}

boolean CDmdApp::IsBootReady (void)
{
	// Wi-Fi connecte et heure Internet recue (ou pas de Wi-Fi du tout)
	return    !m_pNetwork->IsConfigured ()
	       || (   m_pNetwork->IsRunning ()
		   && CClock::Get ()->GetSource () == TimeSourceNetwork);
}

unsigned CDmdApp::GetSplashSeason (const char *pName)
{
	// mois pendant lequel un ecran d'accueil saisonnier est utilise (0 = toute l'annee)
	static const struct
	{
		const char *pWord;
		unsigned    nMonth;
	}
	Seasons[] =
	{
		{"noel",	12},
		{"xmas",	12},
		{"halloween",	10}
	};

	for (unsigned i = 0; i < sizeof Seasons / sizeof Seasons[0]; i++)
	{
		const char *pWord = Seasons[i].pWord;
		size_t nWordLength = strlen (pWord);
		for (const char *p = pName; *p != '\0'; p++)
		{
			if (strncasecmp (p, pWord, nWordLength) == 0)
			{
				return Seasons[i].nMonth;
			}
		}
	}

	return 0;
}

void CDmdApp::RunBootSplash (void)
{
	CRemoteControl::Get ()->Status.pScreen = "demarrage";

	// ecrans d'accueil : SD:/gif/Accueil (categorie normalement non cochee)
	static const unsigned MaxSplash = 32;
	CString Names[MaxSplash];
	unsigned nNames = 0;
	CString BootName;

	if (m_bFileSystemOK)
	{
		DIR Directory;
		FILINFO Info;
		if (f_opendir (&Directory, SPLASH_DIR) == FR_OK)
		{
			while (   f_readdir (&Directory, &Info) == FR_OK
			       && Info.fname[0] != '\0')
			{
				size_t nLength = strlen (Info.fname);
				if (   (Info.fattrib & (AM_DIR | AM_HID | AM_SYS))
				    || nLength < 5
				    || strcasecmp (Info.fname + nLength - 4, ".gif") != 0)
				{
					continue;
				}

				CString Path;
				Path.Format ("%s/%s", SPLASH_DIR, Info.fname);

				boolean bBoot = FALSE;
				for (const char *p = Info.fname; *p != '\0'; p++)
				{
					if (strncasecmp (p, "demarrage", 9) == 0)
					{
						bBoot = TRUE;
					}
				}

				if (bBoot && BootName.GetLength () == 0)
				{
					BootName = Path;
				}
				else if (!bBoot && nNames < MaxSplash)
				{
					Names[nNames++] = Path;
				}
			}

			f_closedir (&Directory);
		}
	}

	// 1. animation de demarrage pendant la connexion Wi-Fi (20 s au plus)
	boolean bBootLoaded = BootName.GetLength () > 0 && LoadGif (BootName);
	if (bBootLoaded)
	{
		m_pMatrix->Show (m_pAnimation->GetFrame (0));
	}
	else
	{
		ShowMessage ("ORBITDMD", "DEMARRAGE", 0, 220, 255);
	}

	if (m_bFileSystemOK)
	{
		m_pNetwork->Initialize ();	// le Wi-Fi se connecte ensuite en arriere-plan
	}

	if (bBootLoaded)
	{
		PlayAnimation (2000, TRUE, 20000);
		m_pAnimation->Clear ();
	}
	else
	{
		u64 nEnd = CTimer::GetClockTicks64 () + 20000000;
		while (!IsBootReady () && CTimer::GetClockTicks64 () < nEnd)
		{
			Wait (100);
		}
	}

	// 2. ecran d'accueil : celui de la saison s'il existe, sinon un au hasard
	TDateTime Time;
	unsigned nMonth = CClock::Get ()->GetLocalTime (&Time) ? Time.Month : 0;

	unsigned Candidates[MaxSplash];
	unsigned nCandidates = 0;
	for (unsigned i = 0; i < nNames; i++)
	{
		if (nMonth != 0 && GetSplashSeason (Names[i]) == nMonth)
		{
			Candidates[nCandidates++] = i;
		}
	}
	if (nCandidates == 0)
	{
		for (unsigned i = 0; i < nNames; i++)
		{
			if (GetSplashSeason (Names[i]) == 0)
			{
				Candidates[nCandidates++] = i;
			}
		}
	}

	if (nCandidates > 0)
	{
		unsigned nChoice = Candidates[m_pRandom->GetNumber () % nCandidates];
		if (LoadGif (Names[nChoice]))
		{
			PlayAnimation (4000, FALSE, 0);
			m_pAnimation->Clear ();
		}
	}

	CheckConnectedEvent ();
}

void CDmdApp::ShowClock (unsigned nDuration)
{
	ShowClockScreen (nDuration, FALSE);

	// date seule apres l'heure (comme l'ancien RPI2DMD)
	if (   m_pConfig->ClockStyle.DateMode == DateScreen
	    && !CRemoteControl::Get ()->IsPlayRequested ())
	{
		ShowClockScreen (m_pConfig->ClockStyle.nDateDisplayTime, TRUE);
	}
}

void CDmdApp::ShowClockScreen (unsigned nDuration, boolean bDateScreen)
{
	CClock *pClock = CClock::Get ();

	TDateTime Time;
	if (!pClock->GetLocalTime (&Time))
	{
		if (!bDateScreen && (!m_pConfig->GifActive || m_nFiles == 0))
		{
			ShowMessage ("ORBITDMD", "HEURE INCONNUE", 255, 128, 0);
			Wait (nDuration);
		}
		return;
	}

	u64 nStart = CTimer::GetClockTicks64 ();
	u64 nEnd = nStart + (u64) nDuration * 1000;
	u64 nSecondStart = nStart;		// debut de la seconde en cours (deux-points clignotants)
	CRemoteControl::Get ()->Status.pScreen = bDateScreen ? "date" : "heure";
	TClockStyle Picked = m_pConfig->ClockStyle;		// motif et fond "au hasard" : un tirage
	CClockRenderer::PickRandom (&Picked, m_pRandom->GetNumber ());
	unsigned nLastSecond = Time.Second;
	u32 nLastKey = 0;
	boolean bRedraw = TRUE;

	while (   CTimer::GetClockTicks64 () < nEnd
	       && !CRemoteControl::Get ()->IsPlayRequested ())
	{
		u64 nHeld = HoldWhileStopped (m_pCanvas->GetData ());
		if (nHeld != 0)
		{
			nEnd += nHeld;
			bRedraw = TRUE;
		}

		if (!pClock->GetLocalTime (&Time))
		{
			break;
		}

		u64 nNow = CTimer::GetClockTicks64 ();
		if (Time.Second != nLastSecond)
		{
			nLastSecond = Time.Second;
			nSecondStart = nNow;
		}

		// copie : les reglages peuvent changer depuis la page web pendant un chargement ;
		// choix "au hasard" : ceux tires au debut de cet ecran
		TClockStyle Style = m_pConfig->ClockStyle;
		if (strcmp (Style.Time.Pattern, CLOCK_RANDOM) == 0)
		{
			strcpy (Style.Time.Pattern, Picked.Time.Pattern);
			memcpy (Style.Time.Color, Picked.Time.Color, 3);
		}
		if (strcmp (Style.Date.Pattern, CLOCK_RANDOM) == 0)
		{
			strcpy (Style.Date.Pattern, Picked.Date.Pattern);
			memcpy (Style.Date.Color, Picked.Date.Color, 3);
		}
		if (strcmp (Style.Background, CLOCK_RANDOM) == 0)
		{
			strcpy (Style.Background, Picked.Background);
		}
		m_pClock->Prepare (Style);

		unsigned nMillis = (unsigned) ((nNow - nStart) / 1000);
		boolean bColonOn = !Style.bBlink || nNow - nSecondStart < 500000;
		u32 nKey = m_pClock->GetFrameKey (Style, Time, nMillis, bColonOn, bDateScreen);
		if (bRedraw || nKey != nLastKey)
		{
			m_pClock->Render (Style, Time, nMillis, bColonOn, bDateScreen, m_pCanvas);
			m_pMatrix->Show (m_pCanvas->GetData ());
			nLastKey = nKey;
			bRedraw = FALSE;
		}

		Wait (m_pClock->IsAnimated (Style, bDateScreen) ? 10 : 40);
	}
}

void CDmdApp::ShowWeather (unsigned nDuration)
{
	CWeather *pWeather = CWeather::Get ();
	TWeatherData Data;
	if (   !m_pConfig->WeatherStyle.bActive
	    || pWeather == 0 || !pWeather->GetData (&Data))
	{
		return;			// pas de lieu choisi, ou pas de meteo recente
	}

	CRemoteControl::Get ()->Status.pScreen = "meteo";
	u64 nStart = CTimer::GetClockTicks64 ();
	u64 nEnd = nStart + (u64) nDuration * 1000;
	u32 nLastKey = 0;
	boolean bRedraw = TRUE;

	while (   CTimer::GetClockTicks64 () < nEnd
	       && !CRemoteControl::Get ()->IsPlayRequested ())
	{
		u64 nHeld = HoldWhileStopped (m_pCanvas->GetData ());
		if (nHeld != 0)
		{
			nEnd += nHeld;
			bRedraw = TRUE;
		}

		TWeatherStyle Style = m_pConfig->WeatherStyle;	// copie (voir ShowClockScreen)
		m_pClock->PrepareWeather (Style);

		unsigned nMillis = (unsigned) ((CTimer::GetClockTicks64 () - nStart) / 1000);
		boolean bFast = Style.bAnimate || strcmp (Style.Text.Pattern, CLOCK_RAINBOW) == 0;
		u32 nKey = bFast ? nMillis / 40 : nMillis / 1000;
		if (bRedraw || nKey != nLastKey)
		{
			pWeather->GetData (&Data);		// mise a jour arrivee entre-temps ?
			m_pClock->RenderWeather (Style, Data, nMillis, m_pCanvas);
			m_pMatrix->Show (m_pCanvas->GetData ());
			nLastKey = nKey;
			bRedraw = FALSE;
		}

		Wait (Style.bAnimate ? 10 : 40);
	}
}

// texte defilant et compte a rebours de la rotation (toutes les N GIF, ou a chaque tour sans GIF) :
// l'element actif suivant de chaque liste (messages.h)
void CDmdApp::ShowMessages (boolean bEveryTime)
{
	CRemoteControl *pRemote = CRemoteControl::Get ();

	if (   CMessages::CountActiveTexts () > 0
	    && (bEveryTime || ++m_nGifsSinceBanner >= m_pConfig->Banner.nEvery)
	    && !pRemote->IsPlayRequested ())
	{
		m_nGifsSinceBanner = 0;
		for (unsigned n = 0; n < CMessages::s_nTexts; n++)
		{
			unsigned i = m_nNextText++ % CMessages::s_nTexts;
			const TBannerStyle &T = CMessages::s_Texts[i];
			if (T.bActive && T.Text[0] != '\0')
			{
				ShowBanner (T);
				break;
			}
		}
	}

	if (   CMessages::CountActiveCountdowns () > 0
	    && (bEveryTime || ++m_nGifsSinceCountdown >= m_pConfig->Countdown.nEvery)
	    && !pRemote->IsPlayRequested ())
	{
		m_nGifsSinceCountdown = 0;
		for (unsigned n = 0; n < CMessages::s_nCountdowns; n++)
		{
			unsigned i = m_nNextCountdown++ % CMessages::s_nCountdowns;
			const TCountdownStyle &C = CMessages::s_Countdowns[i];
			if (C.bActive && ShowCountdown (C, C.nDisplayTime))
			{
				break;			// sinon (date passee) : le suivant
			}
		}
	}
}

// "Afficher maintenant" de la page Messages : ":texte:N" ou ":compte:N"
void CDmdApp::ShowMessageNow (const char *pRequest)
{
	boolean bText = strncmp (pRequest, BANNER_PLAY_REQUEST ":", strlen (BANNER_PLAY_REQUEST) + 1) == 0;
	const char *pNumber = pRequest + strlen (bText ? BANNER_PLAY_REQUEST : COUNTDOWN_PLAY_REQUEST) + 1;
	unsigned nIndex = 0;
	while ('0' <= *pNumber && *pNumber <= '9')
	{
		nIndex = nIndex * 10 + (*pNumber++ - '0');
	}

	if (bText && nIndex < CMessages::s_nTexts)
	{
		TBannerStyle T = CMessages::s_Texts[nIndex];
		ShowBanner (T);
	}
	else if (!bText && nIndex < CMessages::s_nCountdowns)
	{
		TCountdownStyle C = CMessages::s_Countdowns[nIndex];
		ShowCountdown (C, CLOCK_DEMO_MS);
	}
}

// une phase d'un effet de page ; FALSE si interrompue (GIF demandee depuis la page web)
boolean CDmdApp::RunPagePhase (const TBannerStyle &rItem, const char *pPage, unsigned nPhase,
			       unsigned nDuration, u64 nStart)
{
	u64 nPhaseStart = CTimer::GetClockTicks64 ();
	unsigned nLastKey = 0xFFFFFFFF;
	boolean bRainbow = strcmp (rItem.Style.Pattern, CLOCK_RAINBOW) == 0;

	for (;;)
	{
		if (CRemoteControl::Get ()->IsPlayRequested ())
		{
			return FALSE;
		}

		u64 nHeld = HoldWhileStopped (m_pCanvas->GetData ());
		nPhaseStart += nHeld;

		u64 nNow = CTimer::GetClockTicks64 ();
		unsigned nElapsed = (unsigned) ((nNow - nPhaseStart) / 1000);
		unsigned nProgress = nDuration > 0 ? nElapsed * 1000 / nDuration : 1000;
		unsigned nMillis = (unsigned) ((nNow - nStart) / 1000);

		// redessiner quand l'image change : avancement, ou arc-en-ciel / motif anime
		unsigned nKey = nPhase == CClockRenderer::PageHold ? 0 : nProgress;
		if (bRainbow || strcmp (rItem.Style.Pattern, CLOCK_NONE) != 0)
		{
			nKey = nKey * 4099 + nMillis / 30;
		}
		if (nKey != nLastKey || nHeld != 0)
		{
			m_pClock->RenderPage (rItem.Style, pPage, rItem.Effect, nPhase,
					      nProgress > 1000 ? 1000 : nProgress, nMillis, m_pCanvas);
			m_pMatrix->Show (m_pCanvas->GetData ());
			nLastKey = nKey;
		}

		if (nElapsed >= nDuration)
		{
			return TRUE;
		}

		Wait (8);
	}
}

void CDmdApp::ShowBanner (const TBannerStyle &rItem)
{
	TBannerStyle Banner = rItem;	// copie : la liste peut changer depuis la page web
	if (Banner.Text[0] == '\0')
	{
		return;
	}

	boolean bPixel = strcasecmp (Banner.Style.Font, CLOCK_FONT_PIXEL) == 0;
	char Text[BANNER_TEXT_LEN];
	TextToLatin1 (Banner.Text, Text, sizeof Text, bPixel);

	m_pClock->PrepareText (Banner.Style);
	CRemoteControl::Get ()->Status.pScreen = "texte";
	const int W = (int) m_pCanvas->GetWidth ();
	u64 nStart = CTimer::GetClockTicks64 ();

	// effets de page : le texte est coupe en pages qui tiennent sur le panneau
	if (IsPagedEffect (Banner.Effect))
	{
		static char Pages[20][BANNER_TEXT_LEN];
		m_pClock->FitPages (&Banner.Style, Text);
		unsigned nPages = m_pClock->SplitPages (Banner.Style, Text, Pages, 20);
		unsigned s = Banner.nSpeed;		// 1 (lent) a 10 (rapide)

		for (unsigned nPass = 0; nPass < Banner.nPasses; nPass++)
		{
			for (unsigned i = 0; i < nPages; i++)
			{
				unsigned nLength = strlen (Pages[i]);
				unsigned nIn = 1300 - s * 90;				// 1,2 s a 0,4 s
				if (Banner.Effect == EffectTyping)	nIn = nLength * (200 - s * 15);
				if (Banner.Effect == EffectRain)	nIn = 600 + nLength * (130 - s * 9);
				unsigned nHold = 1200 + nLength * 60;
				if (nHold > 5000)	nHold = 5000;
				unsigned nOut = Banner.Effect == EffectTyping ? nIn / 3 : nIn;

				if (   !RunPagePhase (Banner, Pages[i], CClockRenderer::PageIn, nIn, nStart)
				    || !RunPagePhase (Banner, Pages[i], CClockRenderer::PageHold, nHold, nStart)
				    || !RunPagePhase (Banner, Pages[i], CClockRenderer::PageOut, nOut, nStart))
				{
					return;
				}
			}
		}

		return;
	}

	// defilement : de droite a gauche, ou de gauche a droite (inverse)
	int nTextWidth = m_pClock->GetTextWidth (Banner.Style, Text);
	int nDistance = W + nTextWidth;		// un passage complet
	unsigned nSpeed = BannerPixelsPerSecond (Banner.nSpeed);
	boolean bReverse = Banner.Effect == EffectReverse;
	boolean bAnimated = strcmp (Banner.Style.Pattern, CLOCK_NONE) != 0;
	int nLastOffset = -1;
	unsigned nLastMillis = 0;

	while (!CRemoteControl::Get ()->IsPlayRequested ())
	{
		u64 nHeld = HoldWhileStopped (m_pCanvas->GetData ());
		nStart += nHeld;
		if (nHeld != 0)
		{
			nLastOffset = -1;
		}

		unsigned nMillis = (unsigned) ((CTimer::GetClockTicks64 () - nStart) / 1000);
		u64 nTravel = (u64) nMillis * nSpeed / 1000;
		if (nTravel >= (u64) nDistance * Banner.nPasses)
		{
			break;
		}

		int nOffset = (int) (nTravel % nDistance);
		if (nOffset != nLastOffset || (bAnimated && nMillis - nLastMillis >= 30))
		{
			int x = bReverse ? nOffset - nTextWidth : W - nOffset;
			m_pClock->RenderScroll (Banner.Style, Text, x, nMillis, m_pCanvas);
			m_pMatrix->Show (m_pCanvas->GetData ());
			nLastOffset = nOffset;
			nLastMillis = nMillis;
		}

		Wait (5);
	}
}

// FALSE si rien n'a ete montre (date passee, heure inconnue)
boolean CDmdApp::ShowCountdown (const TCountdownStyle &rItem, unsigned nDuration)
{
	TDateTime Time;
	if (!CClock::Get ()->GetLocalTime (&Time))
	{
		return FALSE;			// heure pas encore connue
	}

	TCountdownStyle Style = rItem;
	char Value[32];
	TCountdownState State = FormatCountdown (Style.Target, Time, Value, sizeof Value);
	if (State == CountdownNone)
	{
		return FALSE;			// date passee
	}

	boolean bPixel = strcasecmp (Style.Style.Font, CLOCK_FONT_PIXEL) == 0;
	char Title[COUNTDOWN_TITLE_LEN], Done[COUNTDOWN_TITLE_LEN];
	TextToLatin1 (Style.Title, Title, sizeof Title, TRUE);		// titre en police Pixel
	TextToLatin1 (Style.Done, Done, sizeof Done, bPixel);

	m_pClock->PrepareText (Style.Style);
	CRemoteControl::Get ()->Status.pScreen = "compte";
	u64 nStart = CTimer::GetClockTicks64 ();
	u64 nEnd = nStart + (u64) nDuration * 1000;
	unsigned nLastSecond = 99;

	while (   CTimer::GetClockTicks64 () < nEnd
	       && !CRemoteControl::Get ()->IsPlayRequested ())
	{
		u64 nHeld = HoldWhileStopped (m_pCanvas->GetData ());
		nEnd += nHeld;
		if (nHeld != 0)
		{
			nLastSecond = 99;
		}

		if (!CClock::Get ()->GetLocalTime (&Time))
		{
			break;
		}

		unsigned nMillis = (unsigned) ((CTimer::GetClockTicks64 () - nStart) / 1000);
		boolean bAnimated = strcasecmp (Style.Style.Pattern, CLOCK_NONE) != 0;
		if (Time.Second != nLastSecond || bAnimated)
		{
			State = FormatCountdown (Style.Target, Time, Value, sizeof Value);
			if (State == CountdownReached)
			{
				m_pClock->RenderCountdown (Style, "", Done, nMillis, m_pCanvas);
			}
			else
			{
				char Latin[32];
				TextToLatin1 (Value, Latin, sizeof Latin, bPixel);
				m_pClock->RenderCountdown (Style, Title, Latin, nMillis, m_pCanvas);
			}
			m_pMatrix->Show (m_pCanvas->GetData ());
			nLastSecond = Time.Second;
		}

		Wait (bAnimated ? 10 : 40);
	}

	return TRUE;
}

u64 CDmdApp::HoldWhileStopped (const u8 *pFrame)
{
	CRemoteControl *pRemote = CRemoteControl::Get ();
	if (!pRemote->IsPaused () && !pRemote->IsDisplayOff ())
	{
		return 0;
	}

	u64 nStart = CTimer::GetClockTicks64 ();
	boolean bBlack = FALSE;

	// la page web, le Wi-Fi et les mises a jour continuent pendant l'attente (Wait)
	while (pRemote->IsPaused () || pRemote->IsDisplayOff ())
	{
		if (pRemote->IsDisplayOff () && !bBlack)
		{
			m_pMatrix->Show (m_pBlack);
			bBlack = TRUE;
		}
		else if (!pRemote->IsDisplayOff () && bBlack)
		{
			m_pMatrix->Show (pFrame);	// rallume pendant la pause : image figee
			bBlack = FALSE;
		}

		UpdateBrightness ();
		Wait (20);
	}

	return CTimer::GetClockTicks64 () - nStart;
}

void CDmdApp::UpdateBrightness (void)
{
	TDateTime Time;
	unsigned nHour = CClock::Get ()->GetLocalTime (&Time) ? Time.Hour : 12;
	unsigned nBrightness = m_pConfig->GetBrightness (nHour);

	m_pMatrix->SetBrightness (nBrightness);
	CRemoteControl::Get ()->Status.nBrightness = nBrightness;
}

void CDmdApp::ProcessBackground (void)
{
	m_pNetwork->Process ();
	CClock::Get ()->Process ();

	if (CRemoteControl::Get ()->GetPending () != RemoteNone)
	{
		HandleRemoteRequest ();
	}

	// essai de nouveaux reglages reseau : valides quand le panneau est connecte ET qu'il a
	// recu l'heure Internet ou une visite de sa page web ; sinon retour aux anciens
	if (m_bWifiTrial && !m_bInRemoteRequest)
	{
		if (   m_pNetwork->IsRunning ()
		    && (   CClock::Get ()->GetSource () == TimeSourceNetwork
			|| CRemoteControl::Get ()->WasWebRequestSeen ()))
		{
			f_unlink (WIFI_TRIAL_FILE);
			m_bWifiTrial = FALSE;
			CLogger::Get ()->Write (From, LogNotice, "Nouveaux reglages reseau valides");
		}
		else if (CTimer::Get ()->GetUptime () >= WIFI_TRIAL_SECONDS)
		{
			RollbackWifi ();
		}
	}

	// frequence de rafraichissement, mesuree toutes les 2 s
	u64 nNow = CTimer::GetClockTicks64 ();
	if (nNow - m_nRateTicks >= 2000000)
	{
		unsigned nCount = m_pMatrix->GetRefreshCount ();
		if (m_nRateTicks != 0)
		{
			CRemoteControl::Get ()->Status.nRefreshRate =
				(unsigned) ((u64) (nCount - m_nRateCount) * 1000000 / (nNow - m_nRateTicks));
		}
		m_nRateCount = nCount;
		m_nRateTicks = nNow;
	}
}

void CDmdApp::HandleRemoteRequest (void)
{
	if (m_bInRemoteRequest)
	{
		return;
	}
	m_bInRemoteRequest = TRUE;

	CRemoteControl *pRemote = CRemoteControl::Get ();
	boolean bOK = TRUE;

	switch (pRemote->GetPending ())
	{
	case RemoteKernel:
		ShowMessage ("MISE A JOUR", "PROGRAMME", 0, 170, 255);
		bOK = InstallFile (DRIVE "/kernel8.img", DRIVE "/kernel8.new", DRIVE "/kernel8.bak",
				   pRemote->GetData (), pRemote->GetSize ());
		break;

	case RemoteConfig:
		ShowMessage ("MISE A JOUR", "REGLAGES", 0, 170, 255);
		bOK = InstallFile (CONFIG_FILE, DRIVE "/Config/config.new", DRIVE "/Config/config.bak",
				   pRemote->GetData (), pRemote->GetSize ());
		break;

	default:
		break;
	}

	pRemote->Done ();

	if (!bOK)
	{
		ShowMessage ("ECHEC DE LA", "MISE A JOUR", 255, 0, 0);
		CScheduler::Get ()->Sleep (3);
		m_bInRemoteRequest = FALSE;
		return;
	}

	Reboot ();
}

boolean CDmdApp::InstallFile (const char *pFinal, const char *pTemp, const char *pBackup,
			      const u8 *pData, unsigned nSize)
{
	if (!SafeWriteFile (pFinal, pTemp, pBackup, pData, nSize))
	{
		return FALSE;
	}

	CLogger::Get ()->Write (From, LogNotice, "%s installe (%u octets), ancienne version : %s",
				pFinal, nSize, pBackup);

	return TRUE;
}

void CDmdApp::RollbackWifi (void)
{
	CLogger *pLogger = CLogger::Get ();
	m_bWifiTrial = FALSE;
	m_bInRemoteRequest = TRUE;

	pLogger->Write (From, LogWarning, "Nouveaux reglages reseau non valides apres %u s : retour aux anciens",
			WIFI_TRIAL_SECONDS);
	ShowMessage ("RESEAU REFUSE", "ANCIENS REGLAGES", 255, 128, 0);

	// config.bak = config.txt d'avant le changement ; la version refusee est gardee
	unsigned nLength = FileBufferSize;
	if (   ReadWholeFile (DRIVE "/Config/config.bak", m_pFileBuffer, &nLength)
	    && SafeWriteFile (CONFIG_FILE, DRIVE "/Config/config.new",
			      DRIVE "/Config/config-wifi-refuse.txt", m_pFileBuffer, nLength))
	{
		pLogger->Write (From, LogNotice, "Ancien config.txt remis (version refusee : config-wifi-refuse.txt)");
	}
	else
	{
		pLogger->Write (From, LogError, "Ancien config.txt introuvable : Wi-Fi inchange");
	}

	f_unlink (WIFI_TRIAL_FILE);
	CScheduler::Get ()->Sleep (3);
	Reboot ();
}

void CDmdApp::Reboot (void)
{
	ShowMessage ("REDEMARRAGE", "", 255, 170, 0);

	CScheduler::Get ()->Sleep (2);		// laisser partir la reponse de la page web

	m_pCanvas->Clear ();
	m_pMatrix->Show (m_pCanvas->GetData ());
	CScheduler::Get ()->MsSleep (50);

	f_mount (0, DRIVE, 0);			// tout ecrire sur la carte

	reboot ();
}

void CDmdApp::Wait (unsigned nMilliseconds)
{
	WaitUntil (CTimer::GetClockTicks64 () + (u64) nMilliseconds * 1000);
}

void CDmdApp::WaitUntil (u64 nClockTicks)
{
	while (TRUE)
	{
		ProcessBackground ();

		u64 nNow = CTimer::GetClockTicks64 ();
		if (nNow >= nClockTicks)
		{
			break;
		}

		// laisser tourner les taches reseau pendant l'attente
		u64 nRemaining = nClockTicks - nNow;
		CScheduler::Get ()->usSleep (nRemaining > 20000 ? 20000 : (unsigned) nRemaining);
	}
}

void CDmdApp::YieldHandler (void)
{
	CScheduler::Get ()->Yield ();
}
