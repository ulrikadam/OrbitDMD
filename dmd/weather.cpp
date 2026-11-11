//
// weather.cpp
//
#include "weather.h"
#include "network.h"
#include <circle/net/dnsclient.h>
#include <circle/net/httpclient.h>
#include <circle/sched/scheduler.h>
#include <circle/string.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "meteo";

#define WEATHER_SERVER		"api.open-meteo.com"
#define UPDATE_SECONDS		900		// 15 min (Open-Meteo actualise toutes les 15 min)
#define RETRY_SECONDS		120
#define QUICK_RETRY_SECONDS	20		// 3 premiers echecs (la box met parfois du temps a trouver le serveur)
#define MAX_AGE_SECONDS		(3 * 3600)	// au-dela, les donnees ne sont plus affichees
#define BUFFER_SIZE		16384

CWeather *CWeather::s_pThis = 0;

CWeather::CWeather (CNetSubSystem *pNetSubSystem, CConfig *pConfig)
:	m_pNet (pNetSubSystem),
	m_pConfig (pConfig),
	m_bHasData (FALSE),
	m_nUpdated (0),
	m_bUpdateRequested (FALSE),
	m_pBuffer (new u8[BUFFER_SIZE + 1])
{
	m_Error[0] = '\0';
	m_Latitude[0] = '\0';
	m_Longitude[0] = '\0';
	memset (&m_Data, 0, sizeof m_Data);

	SetName ("meteo");
	s_pThis = this;
}

void CWeather::Run (void)
{
	unsigned nNextAttempt = 0;
	unsigned nFailures = 0;

	while (TRUE)
	{
		const TWeatherStyle &Style = m_pConfig->WeatherStyle;
		CNetwork *pNetwork = CNetwork::Get ();
		unsigned nNow = CTimer::Get ()->GetUptime ();

		boolean bPlace = Style.Latitude[0] != '\0' && Style.Longitude[0] != '\0';
		boolean bMoved =    strcmp (Style.Latitude, m_Latitude) != 0
				 || strcmp (Style.Longitude, m_Longitude) != 0;

		if (   Style.bActive && bPlace && pNetwork != 0 && pNetwork->IsRunning ()
		    && (bMoved || m_bUpdateRequested || nNow >= nNextAttempt))
		{
			m_bUpdateRequested = FALSE;

			// copie : config.txt peut etre relu pendant la requete
			char Latitude[16], Longitude[16];
			strcpy (Latitude, Style.Latitude);
			strcpy (Longitude, Style.Longitude);

			if (bMoved)
			{
				m_bHasData = FALSE;		// meteo d'un autre lieu
				strcpy (m_Latitude, Latitude);
				strcpy (m_Longitude, Longitude);
			}

			if (Fetch (Latitude, Longitude))
			{
				nFailures = 0;
				nNextAttempt = nNow + UPDATE_SECONDS;
			}
			else
			{
				nNextAttempt = nNow + (++nFailures <= 3 ? QUICK_RETRY_SECONDS : RETRY_SECONDS);
			}
		}

		CScheduler::Get ()->Sleep (1);
	}
}

boolean CWeather::Fetch (const char *pLatitude, const char *pLongitude)
{
	CLogger *pLogger = CLogger::Get ();

	CIPAddress ServerIP;
	CDNSClient DNSClient (m_pNet);
	if (!DNSClient.Resolve (WEATHER_SERVER, &ServerIP))
	{
		strcpy (m_Error, "serveur meteo introuvable");
		pLogger->Write (From, LogWarning, "%s introuvable", WEATHER_SERVER);
		return FALSE;
	}

	CString Path;
	Path.Format ("/v1/forecast?latitude=%s&longitude=%s"
		     "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,"
		     "wind_speed_10m,is_day"
		     "&daily=temperature_2m_max,temperature_2m_min,weather_code"
		     "&forecast_days=2&timezone=auto", pLatitude, pLongitude);

	unsigned nLength = BUFFER_SIZE;
	CHTTPClient Client (m_pNet, ServerIP, 80, WEATHER_SERVER);
	THTTPStatus Status = Client.Get (Path, m_pBuffer, &nLength);
	if (Status != HTTPOK)
	{
		strcpy (m_Error, "pas de reponse du serveur meteo");
		pLogger->Write (From, LogWarning, "Reponse %u de %s", Status, WEATHER_SERVER);
		return FALSE;
	}
	m_pBuffer[nLength] = '\0';

	TWeatherData Data;
	if (!ParseOpenMeteo ((const char *) m_pBuffer, &Data))
	{
		strcpy (m_Error, "reponse meteo illisible");
		pLogger->Write (From, LogWarning, "Reponse illisible (%u octets)", nLength);
		return FALSE;
	}

	// lieu change pendant la requete : donnees ignorees
	if (strcmp (pLatitude, m_Latitude) != 0 || strcmp (pLongitude, m_Longitude) != 0)
	{
		return TRUE;
	}

	if (!m_bHasData)
	{
		pLogger->Write (From, LogNotice, "Meteo recue : %s, %d.%u C",
				WeatherDescription (Data.nCode, Data.bDay),
				Data.nTemp10 / 10, (unsigned) (Data.nTemp10 < 0 ? -Data.nTemp10 : Data.nTemp10) % 10);
	}

	m_Data = Data;
	m_nUpdated = CTimer::Get ()->GetUptime ();
	m_bHasData = TRUE;
	m_Error[0] = '\0';

	return TRUE;
}

boolean CWeather::GetData (TWeatherData *pData) const
{
	int nAge = GetAge ();
	if (nAge < 0 || nAge > MAX_AGE_SECONDS)
	{
		return FALSE;
	}

	*pData = m_Data;

	return TRUE;
}

int CWeather::GetAge (void) const
{
	if (!m_bHasData)
	{
		return -1;
	}

	return (int) (CTimer::Get ()->GetUptime () - m_nUpdated);
}
