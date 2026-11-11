//
// clock.cpp
//
#include "clock.h"
#include <circle/timer.h>
#include <circle/logger.h>

static const char From[] = "heure";

#define MIN_VALID_TIME		1600000000U	// septembre 2020
#define RTC_WRITE_INTERVAL	3600		// s

CClock *CClock::s_pThis = 0;

CClock::CClock (CRtcDS1307 *pRTC, CConfig *pConfig)
:	m_pRTC (pRTC),
	m_pConfig (pConfig),
	m_Source (TimeSourceNone),
	m_bNetworkUpdate (FALSE),
	m_bForceRTCWrite (FALSE),
	m_bSynced (FALSE),
	m_nLastSync (0),
	m_bRTCWritten (FALSE),
	m_nLastRTCWrite (0)
{
	s_pThis = this;
}

void CClock::Initialize (void)
{
	CTimer *pTimer = CTimer::Get ();
	pTimer->SetTimeZone (0);		// l'heure locale est calculee par UTCToLocal()

	TDateTime UTC;
	if (m_pRTC->Read (&UTC))
	{
		u32 nTime = DateTimeToUnix (UTC);
		if (nTime >= MIN_VALID_TIME && pTimer->SetTime (nTime, FALSE))
		{
			m_Source = TimeSourceRTC;
			CLogger::Get ()->Write (From, LogNotice,
						"Horloge a pile : %04u-%02u-%02u %02u:%02u:%02u UTC",
						UTC.Year, UTC.Month, UTC.Day,
						UTC.Hour, UTC.Minute, UTC.Second);
		}
	}
	else
	{
		CLogger::Get ()->Write (From, LogWarning, "Horloge a pile illisible ou arretee");
	}

	// appele quand le service NTP met l'heure a jour
	pTimer->RegisterUpdateTimeHandler (TimeUpdateHandler);
}

boolean CClock::TimeUpdateHandler (unsigned nNewTime, unsigned nOldTime)
{
	if (s_pThis != 0 && nNewTime >= MIN_VALID_TIME)
	{
		s_pThis->m_bNetworkUpdate = TRUE;
		s_pThis->m_nLastSync = CTimer::Get ()->GetUptime ();
		s_pThis->m_bSynced = TRUE;
	}

	return TRUE;		// accepter la nouvelle heure
}

void CClock::Process (void)
{
	if (!m_bNetworkUpdate)
	{
		return;
	}
	m_bNetworkUpdate = FALSE;

	if (m_Source != TimeSourceNetwork)
	{
		m_Source = TimeSourceNetwork;
		CLogger::Get ()->Write (From, LogNotice, "Heure Internet recue");
	}

	unsigned nUptime = CTimer::Get ()->GetUptime ();
	if (   !m_bForceRTCWrite
	    && m_bRTCWritten && nUptime - m_nLastRTCWrite < RTC_WRITE_INTERVAL)
	{
		return;
	}
	m_bForceRTCWrite = FALSE;

	TDateTime UTC;
	if (GetUTC (&UTC) && m_pRTC->Write (UTC))
	{
		m_bRTCWritten = TRUE;
		m_nLastRTCWrite = nUptime;
		CLogger::Get ()->Write (From, LogNotice, "Horloge a pile mise a l'heure");
	}
}

boolean CClock::GetUTC (TDateTime *pTime)
{
	u32 nTime = CTimer::Get ()->GetUniversalTime ();
	if (nTime < MIN_VALID_TIME)
	{
		return FALSE;
	}

	UnixToDateTime (nTime, pTime);

	return TRUE;
}

boolean CClock::GetLocalTime (TDateTime *pTime)
{
	TDateTime UTC;
	if (!GetUTC (&UTC))
	{
		return FALSE;
	}

	UTCToLocal (UTC, m_pConfig->TimeZone, pTime);

	return TRUE;
}

int CClock::GetSecondsSinceSync (void) const
{
	if (!m_bSynced)
	{
		return -1;
	}

	return (int) (CTimer::Get ()->GetUptime () - m_nLastSync);
}

const char *CClock::GetSourceName (void) const
{
	switch (m_Source)
	{
	case TimeSourceRTC:	return "horloge a pile";
	case TimeSourceNetwork:	return "Internet";
	default:		return "inconnue";
	}
}
