//
// network.cpp
//
#include "network.h"
#include "common.h"
#include "netutil.h"
#include "latency.h"
#include "weather.h"
#include "ha.h"
#include "clock.h"
#include <circle/net/ntpdaemon.h>
#include <circle/net/ntpclient.h>
#include <circle/net/dnsclient.h>
#include <circle/sched/scheduler.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <fatfs/ff.h>

static const char From[] = "reseau";

#define FIRMWARE_PATH		DRIVE "/firmware/"
#define SUPPLICANT_CONFIG	DRIVE "/wpa_supplicant.conf"
#define NTP_SERVER		"pool.ntp.org"
#define ETHERNET_WAIT_MS	4000	// attente du cable reseau au demarrage (Reseau=auto)

CNetwork *CNetwork::s_pThis = 0;

CNetwork::CNetwork (CConfig *pConfig)
:	m_pConfig (pConfig),
	m_pWLAN (0),
	m_pUSB (0),
	m_pDevice (0),
	m_bEthernet (FALSE),
	m_pNet (0),
	m_pSupplicant (0),
	m_pPublisher (0),
	m_pWebServer (0),
	m_bServicesStarted (FALSE),
	m_bConnectedEvent (FALSE),
	m_bAttempted (FALSE),
	m_bStaticIP (FALSE),
	m_bTimeoutReported (FALSE),
	m_nStartTime (0),
	m_bLinkUp (FALSE),
	m_nLastCheck (0)
{
	s_pThis = this;
}

CNetwork::~CNetwork (void)
{
}

boolean CNetwork::Initialize (void)
{
	CLogger *pLogger = CLogger::Get ();

	// cable reseau (Ethernet) : choisi au demarrage, Circle ne gere qu'un reseau a la fois
	m_bEthernet = UseEthernet ();

	if (!m_bEthernet && m_pConfig->WifiSSID[0] == '\0')
	{
		pLogger->Write (From, LogWarning, "Wi-Fi non configure (Wifi_ssid vide) et pas de cable reseau");
		return FALSE;
	}

	m_bAttempted = TRUE;
	m_nStartTime = CTimer::Get ()->GetUptime ();

	if (!m_bEthernet)
	{
		if (!WriteSupplicantConfig ())
		{
			return FALSE;
		}

		m_pWLAN = new CBcm4343Device (FIRMWARE_PATH);
		if (!m_pWLAN->Initialize ())
		{
			pLogger->Write (From, LogError, "Puce Wi-Fi non demarree (firmware dans %s ?)",
					FIRMWARE_PATH);
			return FALSE;
		}
		m_pDevice = m_pWLAN;
	}

	// adresse fixe si elle est complete et coherente, sinon adresse donnee par la box
	u8 IP[4], Mask[4], Gateway[4], DNS[4];
	m_bStaticIP = FALSE;
	if (m_pConfig->IPAddress[0] != '\0')
	{
		const char *pError = "valeur illisible";
		if (   ParseIPv4 (m_pConfig->IPAddress, IP) && ParseIPv4 (m_pConfig->IPMask, Mask)
		    && ParseIPv4 (m_pConfig->IPGateway, Gateway) && ParseIPv4 (m_pConfig->IPDNS, DNS))
		{
			pError = CheckStaticIP (IP, Mask, Gateway, DNS);
		}

		if (pError == 0)
		{
			m_bStaticIP = TRUE;
			pLogger->Write (From, LogNotice, "Adresse fixe %s, passerelle %s",
					m_pConfig->IPAddress, m_pConfig->IPGateway);
		}
		else
		{
			pLogger->Write (From, LogError, "Adresse fixe refusee (%s) : adresse automatique", pError);
		}
	}

	const TNetDeviceType Type = m_bEthernet ? NetDeviceTypeEthernet : NetDeviceTypeWLAN;
	m_pNet = m_bStaticIP
	       ? new CNetSubSystem (IP, Mask, Gateway, DNS, m_pConfig->NetworkName, Type)
	       : new CNetSubSystem (0, 0, 0, 0, m_pConfig->NetworkName, Type);
	if (!m_pNet->Initialize (FALSE))
	{
		pLogger->Write (From, LogError, "Reseau non demarre");
		m_pNet = 0;
		return FALSE;
	}

	if (m_bEthernet)
	{
		pLogger->Write (From, LogNotice, "Connexion par le cable reseau ...");
		return TRUE;
	}

	m_pSupplicant = new CWPASupplicant (SUPPLICANT_CONFIG);
	if (!m_pSupplicant->Initialize ())
	{
		pLogger->Write (From, LogError, "WPA non demarre");
		return FALSE;
	}

	pLogger->Write (From, LogNotice, "Connexion au Wi-Fi %s ...", m_pConfig->WifiSSID);

	return TRUE;
}

void CNetwork::GetCurrentConfig (CString *pIP, CString *pMask, CString *pGateway, CString *pDNS,
				 CString *pMAC) const
{
	if (m_pNet == 0 || !m_bServicesStarted)
	{
		return;
	}

	CNetConfig *pConfig = m_pNet->GetConfig ();
	pConfig->GetIPAddress ()->Format (pIP);
	pConfig->GetDefaultGateway ()->Format (pGateway);
	pConfig->GetDNSServer ()->Format (pDNS);

	const u8 *pMaskBytes = pConfig->GetNetMask ();
	pMask->Format ("%u.%u.%u.%u", pMaskBytes[0], pMaskBytes[1], pMaskBytes[2], pMaskBytes[3]);

	if (m_pDevice != 0)
	{
		m_pDevice->GetMACAddress ()->Format (pMAC);
	}
}

// Ethernet ou Wi-Fi ? Reseau=wifi : Wi-Fi sans demarrer l'USB ; Reseau=ethernet : cable
// (s'il existe) ; Reseau=auto (par defaut) : cable s'il est branche, sinon Wi-Fi
boolean CNetwork::UseEthernet (void)
{
	CLogger *pLogger = CLogger::Get ();
	const char *pMode = m_pConfig->NetMode;

	if (strcasecmp (pMode, "wifi") == 0)
	{
		return FALSE;
	}

	// la carte reseau du Pi 3 est branchee sur son USB interne
	m_pUSB = new CUSBHCIDevice (CInterruptSystem::Get (), CTimer::Get ());
	CNetDevice *pEthernet = 0;
	if (m_pUSB->Initialize ())
	{
		pEthernet = CNetDevice::GetNetDevice (NetDeviceTypeEthernet);
	}
	if (pEthernet == 0)
	{
		pLogger->Write (From, LogWarning, "Pas de prise reseau (Ethernet) : Wi-Fi");
		return FALSE;
	}

	if (strcasecmp (pMode, "ethernet") != 0)
	{
		// le cable met 2 a 3 s a s'annoncer apres la mise sous tension de la puce
		unsigned nStart = CTimer::Get ()->GetClockTicks ();
		while (   !pEthernet->IsLinkUp ()
		       && CTimer::Get ()->GetClockTicks () - nStart < ETHERNET_WAIT_MS * 1000)
		{
			CScheduler::Get ()->MsSleep (100);
		}

		if (!pEthernet->IsLinkUp ())
		{
			pLogger->Write (From, LogNotice, "Pas de cable reseau branche : Wi-Fi");
			return FALSE;
		}
	}

	pLogger->Write (From, LogNotice, "Cable reseau (Ethernet) utilise");
	m_pDevice = pEthernet;
	return TRUE;
}

boolean CNetwork::TakeTimeoutEvent (void)
{
	if (   !m_bAttempted
	    || m_bServicesStarted
	    || m_bTimeoutReported
	    || CTimer::Get ()->GetUptime () - m_nStartTime < 60)
	{
		return FALSE;
	}

	m_bTimeoutReported = TRUE;
	if (m_bEthernet)
	{
		CLogger::Get ()->Write (From, LogWarning, "Cable reseau : pas d'adresse apres 60 s "
					"(cable, box ou adresse fixe ?)");
	}
	else
	{
		CLogger::Get ()->Write (From, LogWarning, "Wi-Fi toujours pas connecte apres 60 s "
					"(verifier Wifi_ssid / Wifi_psk, reseau WPA2)");
	}

	return TRUE;
}

void CNetwork::Process (void)
{
	if (m_pNet != 0 && m_bServicesStarted)
	{
		CheckAddress ();
		return;
	}

	// en adresse fixe, IsRunning () est vrai tout de suite : attendre aussi la liaison Wi-Fi
	if (   m_pNet == 0
	    || !m_pNet->IsRunning ()
	    || !m_pDevice->IsLinkUp ())
	{
		return;
	}

	m_pNet->GetConfig ()->GetIPAddress ()->Format (&m_IPAddress);
	CLogger::Get ()->Write (From, LogNotice, "%s connecte : http://%s.local/ ou http://%s/",
				GetInterfaceName (), m_pConfig->NetworkName, (const char *) m_IPAddress);

	new CNTPDaemon (NTP_SERVER, m_pNet);

	m_pPublisher = new CmDNSPublisher (m_pNet);	// repond a <nom>.local
	m_pPublisher->PublishService (m_pConfig->NetworkName, "_http._tcp", 80);

	m_pWebServer = new CWebServer (m_pNet, m_pConfig);

	new CLatencyProbe;		// mesure la reactivite du coeur 0 (dans /api/etat)
	new CWeather (m_pNet, m_pConfig);	// meteo du lieu choisi (page Meteo)

	if (m_pConfig->Mqtt.bActive && m_pConfig->Mqtt.Host[0] != '\0')
	{
		new CHomeAssistant (m_pNet, m_pConfig, m_IPAddress);	// Home Assistant (MQTT)
	}

	m_bServicesStarted = TRUE;
	m_bLinkUp = TRUE;
	m_bConnectedEvent = TRUE;
}

// apres la premiere connexion : Wi-Fi perdu puis retrouve, ou nouvelle adresse donnee par la
// box -> l'adresse est de nouveau affichee sur le panneau (une verification par seconde)
void CNetwork::CheckAddress (void)
{
	unsigned nNow = CTimer::Get ()->GetUptime ();
	if (nNow == m_nLastCheck)
	{
		return;
	}
	m_nLastCheck = nNow;

	CLogger *pLogger = CLogger::Get ();

	if (!m_pDevice->IsLinkUp () || !m_pNet->IsRunning ())
	{
		if (m_bLinkUp)
		{
			pLogger->Write (From, LogWarning, "%s perdu", GetInterfaceName ());
			m_bLinkUp = FALSE;
		}
		return;
	}

	CString IP;
	m_pNet->GetConfig ()->GetIPAddress ()->Format (&IP);
	if (strcmp (IP, "0.0.0.0") == 0)
	{
		return;
	}

	if (!m_bLinkUp || strcmp (IP, m_IPAddress) != 0)
	{
		pLogger->Write (From, LogNotice, "%s %s : http://%s/", GetInterfaceName (),
				m_bLinkUp ? "nouvelle adresse" : "retrouve", (const char *) IP);
		m_IPAddress = IP;
		m_bLinkUp = TRUE;
		m_bConnectedEvent = TRUE;
	}
}

boolean CNetwork::SyncTime (int *pDiff)
{
	CLogger *pLogger = CLogger::Get ();

	if (!IsRunning ())
	{
		return FALSE;
	}

	CIPAddress ServerIP;
	CDNSClient DNSClient (m_pNet);
	if (!DNSClient.Resolve (NTP_SERVER, &ServerIP))
	{
		pLogger->Write (From, LogWarning, "Mise a l'heure : %s introuvable", NTP_SERVER);
		return FALSE;
	}

	CNTPClient NTPClient (m_pNet);
	unsigned nTime = NTPClient.GetTime (ServerIP);
	if (nTime == 0)
	{
		pLogger->Write (From, LogWarning, "Mise a l'heure : pas de reponse de %s", NTP_SERVER);
		return FALSE;
	}

	CTimer *pTimer = CTimer::Get ();
	unsigned nOld = pTimer->GetUniversalTime ();
	CClock::Get ()->RequestRTCWrite ();	// recopier aussi dans l'horloge a pile
	if (!pTimer->SetTime (nTime, FALSE))
	{
		return FALSE;
	}

	*pDiff = (int) (nTime - nOld);
	pLogger->Write (From, LogNotice, "Mise a l'heure depuis Internet (ecart %d s)", *pDiff);

	return TRUE;
}

boolean CNetwork::TakeConnectedEvent (void)
{
	boolean bEvent = m_bConnectedEvent;
	m_bConnectedEvent = FALSE;

	return bEvent;
}

boolean CNetwork::WriteSupplicantConfig (void)
{
	CLogger *pLogger = CLogger::Get ();

	if (   strchr (m_pConfig->WifiSSID, '"') != 0
	    || strchr (m_pConfig->WifiPSK, '"') != 0)
	{
		pLogger->Write (From, LogError, "Guillemets non geres dans Wifi_ssid / Wifi_psk");
		return FALSE;
	}

	size_t nPSKLength = strlen (m_pConfig->WifiPSK);
	if (nPSKLength < 8 || nPSKLength > 63)
	{
		pLogger->Write (From, LogError, "Wifi_psk doit faire de 8 a 63 caracteres");
		return FALSE;
	}

	CString Content;
	Content.Format ("country=%s\n"
			"\n"
			"network={\n"
			"\tssid=\"%s\"\n"
			"\tpsk=\"%s\"\n"
			"\tproto=WPA2\n"
			"\tkey_mgmt=WPA-PSK\n"
			"}\n",
			m_pConfig->WifiCountry, m_pConfig->WifiSSID, m_pConfig->WifiPSK);

	// n'ecrire sur la carte que si le contenu a change
	FIL File;
	if (f_open (&File, SUPPLICANT_CONFIG, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		char Buffer[512];
		UINT nRead = 0;
		FRESULT Result = f_read (&File, Buffer, sizeof Buffer - 1, &nRead);
		f_close (&File);

		if (   Result == FR_OK
		    && nRead == Content.GetLength ()
		    && memcmp (Buffer, (const char *) Content, nRead) == 0)
		{
			return TRUE;
		}
	}

	if (f_open (&File, SUPPLICANT_CONFIG, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
	{
		pLogger->Write (From, LogError, "Ecriture impossible : %s", SUPPLICANT_CONFIG);
		return FALSE;
	}

	UINT nWritten = 0;
	FRESULT Result = f_write (&File, (const char *) Content, Content.GetLength (), &nWritten);
	f_close (&File);

	if (Result != FR_OK || nWritten != Content.GetLength ())
	{
		pLogger->Write (From, LogError, "Ecriture incomplete : %s", SUPPLICANT_CONFIG);
		return FALSE;
	}

	pLogger->Write (From, LogNotice, "%s mis a jour depuis config.txt", SUPPLICANT_CONFIG);

	return TRUE;
}
