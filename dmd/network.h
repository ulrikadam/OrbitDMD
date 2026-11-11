//
// network.h - Wi-Fi (WPA2), adresse automatique, heure Internet, nom <nom>.local, page web
//
#ifndef _network_h
#define _network_h

#include <wlan/bcm4343.h>
#include <wlan/hostap/wpa_supplicant/wpasupplicant.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/mdnspublisher.h>
#include <circle/netdevice.h>
#include <circle/usb/usbhcidevice.h>
#include <circle/string.h>
#include <circle/types.h>
#include "config.h"
#include "webserver.h"

class CNetwork
{
public:
	CNetwork (CConfig *pConfig);
	~CNetwork (void);

	// demarre le Wi-Fi (a appeler depuis la tache principale, apres lecture de config.txt)
	boolean Initialize (void);

	// a appeler regulierement : demarre les services une fois connecte
	void Process (void);

	boolean IsConfigured (void) const	{ return m_pNet != 0; }
	boolean IsRunning (void) const		{ return m_bServicesStarted; }

	// reseau choisi au demarrage : cable (Ethernet) ou Wi-Fi
	boolean IsEthernet (void) const		{ return m_bEthernet; }
	const char *GetInterfaceName (void) const { return m_bEthernet ? "Cable reseau" : "Wi-Fi"; }
	const char *GetIPAddress (void) const	{ return m_IPAddress; }

	// TRUE juste apres la connexion, puis a chaque reconnexion ou changement d'adresse
	boolean TakeConnectedEvent (void);

	// TRUE une seule fois si le Wi-Fi n'est toujours pas connecte 60 s apres le demarrage
	boolean TakeTimeoutEvent (void);

	// configuration reseau en service (vide tant que le Wi-Fi n'est pas connecte)
	boolean IsStaticIP (void) const		{ return m_bStaticIP; }
	void GetCurrentConfig (CString *pIP, CString *pMask, CString *pGateway, CString *pDNS,
			       CString *pMAC) const;

	// mise a l'heure immediate depuis Internet (page web) ; *pDiff : correction en secondes
	// (positive si le panneau retardait). Bloque quelques secondes au plus.
	boolean SyncTime (int *pDiff);

	static CNetwork *Get (void)		{ return s_pThis; }

private:
	boolean WriteSupplicantConfig (void);
	boolean UseEthernet (void);
	void CheckAddress (void);

private:
	CConfig *m_pConfig;

	CBcm4343Device *m_pWLAN;
	CUSBHCIDevice *m_pUSB;		// USB interne du Pi 3 (sa prise reseau y est branchee)
	CNetDevice *m_pDevice;		// Wi-Fi ou Ethernet
	boolean m_bEthernet;
	CNetSubSystem *m_pNet;
	CWPASupplicant *m_pSupplicant;
	CmDNSPublisher *m_pPublisher;
	CWebServer *m_pWebServer;

	boolean m_bServicesStarted;
	boolean m_bConnectedEvent;
	boolean m_bAttempted;		// Wi-Fi configure, demarrage tente
	boolean m_bStaticIP;		// adresse fixe en service

	static CNetwork *s_pThis;
	boolean m_bTimeoutReported;
	unsigned m_nStartTime;		// temps de fonctionnement au demarrage du Wi-Fi (s)
	CString m_IPAddress;
	boolean m_bLinkUp;		// Wi-Fi connecte (apres la premiere connexion)
	unsigned m_nLastCheck;		// derniere verification de l'adresse (s)
};

#endif
