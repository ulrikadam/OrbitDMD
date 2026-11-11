//
// ha.h - Home Assistant par MQTT (decouverte automatique), voir hapayload.h pour les sujets
//
// Le panneau se connecte au serveur MQTT (module Mosquitto de Home Assistant), s'y annonce,
// recoit les commandes (allumer, luminosite, pause, boutons, messages) et publie son etat.
// Reconnexion automatique toutes les 15 s si le serveur est injoignable.
//
#ifndef _ha_h
#define _ha_h

#include <circle/net/mqttclient.h>
#include <circle/net/netsubsystem.h>
#include <circle/string.h>
#include <circle/types.h>
#include "config.h"
#include "hapayload.h"

#define NOTIFY_PLAY_REQUEST	":notif"	// RequestPlay : notification de Home Assistant

class CHomeAssistant : public CMQTTClient
{
public:
	CHomeAssistant (CNetSubSystem *pNetSubSystem, CConfig *pConfig, const char *pIPAddress);
	~CHomeAssistant (void);

	// notification en attente (boucle d'affichage) ; FALSE s'il n'y en a pas
	static boolean TakeNotification (THANotification *pNotification);

	// "connecte", "connexion..." ou "erreur : ..." (page web) ; 0 si Home Assistant est inactif
	static const char *GetStatus (void);

private:
	void OnConnect (boolean bSessionPresent);
	void OnDisconnect (TMQTTDisconnectReason Reason);
	void OnMessage (const char *pTopic, const u8 *pPayload, size_t nPayloadLength, boolean bRetain);
	void OnLoop (void);

	void StartConnect (void);
	void Announce (void);			// annonces + etat complet
	void AnnounceItems (void);		// textes et comptes a rebours de la page Messages
	void PublishState (boolean bAll);
	void PublishValue (const char *pObject, const char *pValue, CString *pLast, boolean bForce);

private:
	CConfig *m_pConfig;
	CString m_IPAddress;
	char m_NodeId[32];
	CString m_Base;				// orbitdmd/<nom>
	CString m_Availability;

	boolean m_bAnnounce;			// annonces a (re)publier
	unsigned m_nReconnectAt;		// temps de fonctionnement (s), 0 : pas de reconnexion prevue
	unsigned m_nLastSlowUpdate;		// temperature, IP : toutes les 60 s

	CString m_Last[16];			// dernieres valeurs publiees (publier seulement les changements)
	unsigned m_nGeneration;			// version des listes de messages deja annoncee

	static THANotification s_Notification;
	static volatile boolean s_bNotification;
	static const char *s_pStatus;
};

#endif
