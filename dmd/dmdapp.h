//
// dmdapp.h - application : coeur 0 = affichage + reseau (planificateur), coeur 3 = rafraichissement
//
#ifndef _dmdapp_h
#define _dmdapp_h

#include <circle/multicore.h>
#include <circle/memory.h>
#include <circle/bcmrandom.h>
#include <circle/string.h>
#include <circle/types.h>
#include "matrix.h"
#include "config.h"
#include "canvas.h"
#include "gifanim.h"
#include "clockrender.h"
#include "localtime.h"
#include "network.h"

#ifndef ARM_ALLOW_MULTI_CORE
	#error "ARM_ALLOW_MULTI_CORE doit etre defini (circle/Config.mk)"
#endif

class CDmdApp : public CMultiCoreSupport
{
public:
	static const unsigned MaxFiles = 20000;
	static const unsigned FileBufferSize = 16 * 1024 * 1024;

	static const unsigned RefreshCore = 3;

public:
	CDmdApp (CMemorySystem *pMemorySystem, CMatrix *pMatrix, CConfig *pConfig,
		 CNetwork *pNetwork);
	~CDmdApp (void);

	// a appeler apres l'initialisation des panneaux, avant Initialize()
	boolean Prepare (boolean bFileSystemOK);

	void Run (unsigned nCore);

private:
	void MainLoop (void);
	void RunTestPattern (void);
	void ShowMessage (const char *pLine1, const char *pLine2, u8 r, u8 g, u8 b);
	void CheckConnectedEvent (void);
	void PlayRequestedGifs (void);

	void BuildPlaylist (void);
	void AddFolder (const char *pPath, boolean bRecurse);
	void Shuffle (void);
	boolean PlayGif (const char *pPath);
	boolean LoadGif (const char *pPath);
	// joue l'animation chargee au moins nMinDuration ms (boucles completes) ; avec
	// bUntilBootReady, continue jusqu'a la fin du demarrage (au plus nMaxDuration ms)
	void PlayAnimation (unsigned nMinDuration, boolean bUntilBootReady, unsigned nMaxDuration);

	// ecrans d'accueil (SD:/gif/Accueil) et attente du Wi-Fi au demarrage
	void RunBootSplash (void);
	boolean IsBootReady (void);
	static unsigned GetSplashSeason (const char *pName);

	void ShowClock (unsigned nDuration);			// l'heure, puis la date si reglee ainsi
	void ShowClockScreen (unsigned nDuration, boolean bDateScreen);
	void ShowWeather (unsigned nDuration);		// rien si pas de meteo recente

	// pause ou affichage eteint depuis la page web : attend sur place (pFrame = image a
	// remettre si l'affichage est rallume pendant la pause) ; rend la duree d'attente en us
	u64 HoldWhileStopped (const u8 *pFrame);
	void UpdateBrightness (void);

	// travail de fond : reseau, heure, demandes de la page web, mesures
	void ProcessBackground (void);
	void HandleRemoteRequest (void);
	boolean InstallFile (const char *pFinal, const char *pTemp, const char *pBackup,
			     const u8 *pData, unsigned nSize);
	void Reboot (void);
	void RollbackWifi (void);

	void Wait (unsigned nMilliseconds);
	void WaitUntil (u64 nClockTicks);

	static void YieldHandler (void);

private:
	CMatrix *m_pMatrix;
	CConfig *m_pConfig;
	CNetwork *m_pNetwork;

	boolean m_bFileSystemOK;
	CCanvas *m_pCanvas;
	CGifAnimation *m_pAnimation;
	CClockRenderer *m_pClock;
	CBcmRandomNumberGenerator *m_pRandom;

	u8 *m_pFileBuffer;
	u8 *m_pBlack;			// image noire (affichage eteint)
	CString **m_ppFiles;
	unsigned m_nFiles;

	boolean m_bInRemoteRequest;
	boolean m_bWifiTrial;		// nouveau Wi-Fi en essai
	unsigned m_nGifsSinceWeather;	// GIF montrees depuis la derniere meteo
	u64 m_nRateTicks;
	unsigned m_nRateCount;
};

#endif
