//
// remote.h - echange entre la page web (taches reseau) et la boucle d'affichage
//
// La page web ne touche jamais la carte SD : elle depose ici les fichiers recus,
// et la boucle d'affichage les installe entre deux images.
//
#ifndef _remote_h
#define _remote_h

#include <circle/string.h>
#include <circle/types.h>

#define CLOCK_PLAY_REQUEST	":horloge"	// RequestPlay : montrer l'heure (page Horloge)
#define CLOCK_DEMO_MS		15000		// pendant 15 s
#define WEATHER_PLAY_REQUEST	":meteo"	// RequestPlay : montrer la meteo (page Meteo)

enum TRemoteRequest
{
	RemoteNone,
	RemoteKernel,		// nouveau programme kernel8.img
	RemoteConfig,		// nouveau Config/config.txt
	RemoteReboot
};

struct TDisplayStatus
{
	CString	 CurrentGif;
	unsigned nGifCount;
	unsigned nRefreshRate;	// images/s envoyees aux panneaux
	unsigned nBrightness;	// %
	const char *pScreen;	// ce qui est affiche : "gif", "heure", "date", "meteo", "message",
				// "demarrage" ou "mire" (apercu du tableau de bord)
};

class CRemoteControl
{
public:
	static const unsigned MaxDataSize = 8 * 1024 * 1024;

public:
	CRemoteControl (void);
	~CRemoteControl (void);

	boolean Initialize (void);

	// depuis la page web : FALSE si une demande est deja en cours ou trop grande
	boolean Submit (TRemoteRequest Request, const u8 *pData, unsigned nSize);

	// depuis la boucle d'affichage
	TRemoteRequest GetPending (void) const	{ return m_Pending; }
	const u8 *GetData (void) const		{ return m_pData; }
	unsigned GetSize (void) const		{ return m_nSize; }
	void Done (void)			{ m_Pending = RemoteNone; }

	// liste de lecture a reconstruire (GIF ajoutee/supprimee, categories changees)
	void RequestReload (void)		{ m_bReload = TRUE; }
	boolean TakeReloadRequest (void);

	// une page ou une donnee a ete demandee depuis le reseau (preuve que le panneau est joignable)
	void NoteWebRequest (void)		{ m_bWebRequestSeen = TRUE; }
	boolean WasWebRequestSeen (void) const	{ return m_bWebRequestSeen; }

	// reglages enregistres qui ne seront appliques qu'apres un redemarrage
	void SetRebootPending (void)		{ m_bRebootPending = TRUE; }
	boolean IsRebootPending (void) const	{ return m_bRebootPending; }

	// commandes rapides de la page web : image figee, ou affichage eteint (le panneau
	// reste joignable) ; une GIF demandee avec "Afficher" annule les deux
	void SetPaused (boolean bPaused)	{ m_bPaused = bPaused; }
	boolean IsPaused (void) const		{ return m_bPaused; }
	void SetDisplayOff (boolean bOff)	{ m_bDisplayOff = bOff; }
	boolean IsDisplayOff (void) const	{ return m_bDisplayOff; }

	// GIF a afficher tout de suite (chemin complet sur la carte),
	// ou CLOCK_PLAY_REQUEST pour montrer l'heure (page Horloge)
	void RequestPlay (const char *pPath);
	boolean IsPlayRequested (void) const	{ return m_bPlay; }
	boolean TakePlayRequest (CString *pPath);

	static CRemoteControl *Get (void)	{ return s_pThis; }

public:
	TDisplayStatus Status;

private:
	volatile TRemoteRequest m_Pending;
	u8 *m_pData;
	unsigned m_nSize;

	volatile boolean m_bReload;
	volatile boolean m_bPlay;
	volatile boolean m_bRebootPending;
	volatile boolean m_bWebRequestSeen;
	volatile boolean m_bPaused;
	volatile boolean m_bDisplayOff;
	CString m_PlayPath;

	static CRemoteControl *s_pThis;
};

#endif
