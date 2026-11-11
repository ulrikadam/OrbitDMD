//
// clock.h - heure du systeme : horloge a pile au demarrage, puis heure Internet (NTP)
//
// L'horloge interne de Circle (CTimer) est tenue en UTC. Au demarrage elle est
// reglee depuis l'horloge a pile ; ensuite le service NTP la corrige, et chaque
// correction est recopiee dans l'horloge a pile (au plus une fois par heure).
//
#ifndef _clock_h
#define _clock_h

#include <circle/types.h>
#include "rtc.h"
#include "config.h"
#include "localtime.h"

enum TTimeSource
{
	TimeSourceNone,
	TimeSourceRTC,
	TimeSourceNetwork
};

class CClock
{
public:
	CClock (CRtcDS1307 *pRTC, CConfig *pConfig);

	void Initialize (void);		// apres CTimer et la lecture de config.txt
	void Process (void);		// a appeler regulierement depuis la tache principale

	boolean GetUTC (TDateTime *pTime);
	boolean GetLocalTime (TDateTime *pTime);
	TTimeSource GetSource (void) const	{ return m_Source; }
	const char *GetSourceName (void) const;

	// secondes depuis la derniere heure Internet recue, -1 si jamais
	int GetSecondsSinceSync (void) const;

	// la prochaine heure Internet sera recopiee dans l'horloge a pile sans attendre
	void RequestRTCWrite (void)		{ m_bForceRTCWrite = TRUE; }

	static CClock *Get (void)		{ return s_pThis; }

private:
	static boolean TimeUpdateHandler (unsigned nNewTime, unsigned nOldTime);

private:
	CRtcDS1307 *m_pRTC;
	CConfig *m_pConfig;

	TTimeSource m_Source;
	volatile boolean m_bNetworkUpdate;
	volatile boolean m_bForceRTCWrite;
	volatile boolean m_bSynced;
	volatile unsigned m_nLastSync;	// temps de fonctionnement (s) a la derniere heure Internet
	boolean m_bRTCWritten;
	unsigned m_nLastRTCWrite;	// temps de fonctionnement (s)

	static CClock *s_pThis;
};

#endif
