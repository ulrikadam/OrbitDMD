//
// ha.cpp
//
#include "ha.h"
#include "remote.h"
#include "webserver.h"		// GetVersion ()
#include "gifmanager.h"
#include "messages.h"
#include <circle/cputhrottle.h>
#include <circle/logger.h>
#include <circle/timer.h>
#include <circle/util.h>

static const char From[] = "homeassistant";

#define MAX_PACKET_SIZE		4096		// annonces : environ 700 octets ; liste des commandes : 2 Ko
#define MAX_PACKETS_QUEUED	8
#define MAX_TOPIC_SIZE		160
#define RECONNECT_SECONDS	15
#define SLOW_UPDATE_SECONDS	60

// m_Last[] : un emplacement par etat publie
enum { LastPanel, LastBrightness, LastPause, LastAuto, LastGif, LastScreen, LastTemperature, LastIP,
       LastEffect, LastColor, LastPasses, LastSpeed };

THANotification CHomeAssistant::s_Notification;
volatile boolean CHomeAssistant::s_bNotification = FALSE;
const char *CHomeAssistant::s_pStatus = 0;

CHomeAssistant::CHomeAssistant (CNetSubSystem *pNetSubSystem, CConfig *pConfig, const char *pIPAddress)
:	CMQTTClient (pNetSubSystem, MAX_PACKET_SIZE, MAX_PACKETS_QUEUED, MAX_TOPIC_SIZE),
	m_pConfig (pConfig),
	m_IPAddress (pIPAddress),
	m_bAnnounce (FALSE),
	m_nReconnectAt (0),
	m_nLastSlowUpdate (0),
	m_nGeneration (0)
{
	HAMakeNodeId (pConfig->NetworkName, m_NodeId, sizeof m_NodeId);
	m_Base.Format ("orbitdmd/%s", m_NodeId);
	m_Availability.Format ("%s/dispo", (const char *) m_Base);

	StartConnect ();
}

CHomeAssistant::~CHomeAssistant (void)
{
}

void CHomeAssistant::StartConnect (void)
{
	const TMqttConfig &M = m_pConfig->Mqtt;
	s_pStatus = "connexion...";
	m_nReconnectAt = 0;

	CString ClientId;
	ClientId.Format ("orbitdmd-%s", m_NodeId);

	CLogger::Get ()->Write (From, LogNotice, "Connexion au serveur MQTT %s:%u", M.Host, M.nPort);

	static const char Offline[] = "offline";
	Connect (M.Host, (u16) M.nPort, ClientId,
		 M.User[0] != '\0' ? M.User : 0, M.Password[0] != '\0' ? M.Password : 0,
		 60, TRUE, m_Availability, MQTT_QOS1, TRUE, (const u8 *) Offline, sizeof Offline - 1);
}

void CHomeAssistant::OnConnect (boolean bSessionPresent)
{
	CLogger::Get ()->Write (From, LogNotice, "Connecte au serveur MQTT");
	s_pStatus = "connecte";

	CString Topic;
	Topic.Format ("%s/+/set", (const char *) m_Base);
	Subscribe (Topic, MQTT_QOS1);
	Topic.Format ("%s/notification", (const char *) m_Base);
	Subscribe (Topic, MQTT_QOS1);
	Subscribe (HA_DISCOVERY_PREFIX "/status", MQTT_QOS1);	// Home Assistant redemarre

	m_bAnnounce = TRUE;
}

void CHomeAssistant::OnDisconnect (TMQTTDisconnectReason Reason)
{
	static const char *Reasons[] =
	{
		"deconnecte", "version de protocole refusee", "identifiant refuse", "serveur indisponible",
		"utilisateur ou mot de passe incorrect", "non autorise", "adresse du serveur introuvable",
		"serveur injoignable", "deconnecte par le serveur"
	};
	s_pStatus = (unsigned) Reason < sizeof Reasons / sizeof Reasons[0] ? Reasons[Reason] : "connexion perdue";

	CLogger::Get ()->Write (From, LogWarning, "MQTT : %s (nouvel essai dans %u s)", s_pStatus, RECONNECT_SECONDS);
	m_nReconnectAt = CTimer::Get ()->GetUptime () + RECONNECT_SECONDS;
}

void CHomeAssistant::OnMessage (const char *pTopic, const u8 *pPayload, size_t nPayloadLength, boolean bRetain)
{
	char Payload[1024];
	if (nPayloadLength >= sizeof Payload)
	{
		nPayloadLength = sizeof Payload - 1;
	}
	memcpy (Payload, pPayload, nPayloadLength);
	Payload[nPayloadLength] = '\0';

	if (strcmp (pTopic, HA_DISCOVERY_PREFIX "/status") == 0)
	{
		if (strcmp (Payload, "online") == 0)
		{
			m_bAnnounce = TRUE;		// Home Assistant a redemarre : on se reannonce
		}
		return;
	}

	size_t nBase = m_Base.GetLength ();
	if (strncmp (pTopic, m_Base, nBase) != 0 || pTopic[nBase] != '/')
	{
		return;
	}
	const char *pObject = pTopic + nBase + 1;
	CRemoteControl *pRemote = CRemoteControl::Get ();
	boolean bOn = strcasecmp (Payload, "ON") == 0;

	if (strcmp (pObject, "notification") == 0 || strcmp (pObject, "texte/set") == 0)
	{
		THANotification Notification;
		boolean bText = strcmp (pObject, "texte/set") == 0;
		if (bText)
		{
			// champ "Texte a afficher" : le texte tel quel (pas de JSON)
			for (char *q = Payload; *q != '\0'; q++)
			{
				if (*q == '{')	*q = '(';
			}
			CString Topic;
			Topic.Format ("%s/texte/etat", (const char *) m_Base);
			Publish (Topic, (const u8 *) Payload, strlen (Payload), MQTT_QOS1, TRUE);
		}
		if (HAParseNotification (Payload, CMessages::s_Notify, &Notification))
		{
			s_Notification = Notification;
			s_bNotification = TRUE;
			pRemote->RequestPlay (NOTIFY_PLAY_REQUEST);
			CLogger::Get ()->Write (From, LogNotice, "Message de Home Assistant");
		}
		return;
	}

	// textes et comptes a rebours de la page Messages : texte_N/set, compte_N/set
	if (   (strncmp (pObject, "texte_", 6) == 0 || strncmp (pObject, "compte_", 7) == 0)
	    && strstr (pObject, "/set") != 0)
	{
		boolean bItemText = pObject[0] == 't';
		unsigned nIndex = (unsigned) atoi (pObject + (bItemText ? 6 : 7));
		CMessages::SetActive (bItemText, nIndex, bOn);
		return;				// etats publies par OnLoop (generation)
	}

	TBannerStyle &N = CMessages::s_Notify;
	if (strcmp (pObject, "msg_effet/set") == 0)
	{
		HASetEffectFromLabel (Payload, &N);
		CMessages::Save ();
	}
	else if (strcmp (pObject, "msg_couleur/set") == 0)
	{
		HASetColorFromLabel (Payload, &N.Style);
		CMessages::Save ();
	}
	else if (strcmp (pObject, "msg_passages/set") == 0 || strcmp (pObject, "msg_vitesse/set") == 0)
	{
		int nValue = atoi (Payload);
		if (nValue < 1)		nValue = 1;
		if (nValue > 10)	nValue = 10;
		if (pObject[4] == 'p')	N.nPasses = (unsigned) nValue;
		else			N.nSpeed = (unsigned) nValue;
		CMessages::Save ();
	}
	else if (strcmp (pObject, "panneau/set") == 0)	pRemote->SetDisplayOff (!bOn);
	else if (strcmp (pObject, "pause/set") == 0)	pRemote->SetPaused (bOn);
	else if (strcmp (pObject, "luminosite/set") == 0)
	{
		int nValue = atoi (Payload);
		if (nValue < 0)		nValue = 0;
		if (nValue > 100)	nValue = 100;
		pRemote->SetBrightnessOverride (nValue);		// jusqu'a "heure par heure"
	}
	else if (strcmp (pObject, "luminosite_auto/set") == 0)
	{
		pRemote->SetBrightnessOverride (bOn ? -1 : (int) pRemote->Status.nBrightness);
	}
	else if (strcmp (pObject, "heure/set") == 0)	pRemote->RequestPlay (CLOCK_PLAY_REQUEST);
	else if (strcmp (pObject, "meteo/set") == 0)	pRemote->RequestPlay (WEATHER_PLAY_REQUEST);
	else if (strcmp (pObject, "redemarrer/set") == 0)
	{
		CLogger::Get ()->Write (From, LogNotice, "Redemarrage demande par Home Assistant");
		pRemote->Submit (RemoteReboot, 0, 0);
	}
	else
	{
		return;
	}

	PublishState (FALSE);		// etat a jour tout de suite
}

void CHomeAssistant::OnLoop (void)
{
	unsigned nNow = CTimer::Get ()->GetUptime ();

	if (!IsConnected ())
	{
		if (m_nReconnectAt != 0 && nNow >= m_nReconnectAt)
		{
			StartConnect ();
		}
		return;
	}

	if (m_bAnnounce)
	{
		m_bAnnounce = FALSE;
		Announce ();
		return;
	}

	if (m_nGeneration != CMessages::s_nGeneration)
	{
		AnnounceItems ();		// listes de messages changees (page web ou Home Assistant)
		return;
	}

	PublishState (FALSE);
}

void CHomeAssistant::Announce (void)
{
	for (unsigned i = 0; i < HAGetEntityCount (); i++)
	{
		CString Topic, Payload;
		HABuildDiscovery (i, m_NodeId, m_pConfig->NetworkName, GetVersion (), m_IPAddress, &Topic, &Payload);
		Publish (Topic, (const u8 *) (const char *) Payload, Payload.GetLength (), MQTT_QOS1, TRUE);
	}

	static const char Online[] = "online";
	Publish (m_Availability, (const u8 *) Online, sizeof Online - 1, MQTT_QOS1, TRUE);

	CString Topic;
	Topic.Format ("%s/version/etat", (const char *) m_Base);
	Publish (Topic, (const u8 *) GetVersion (), strlen (GetVersion ()), MQTT_QOS1, TRUE);

	// liste des commandes : attributs du capteur "Commandes MQTT" (et lisible dans MQTT Explorer)
	CString List;
	HABuildCommandList (m_NodeId, &List);
	Topic.Format ("%s/commandes", (const char *) m_Base);
	Publish (Topic, (const u8 *) (const char *) List, List.GetLength (), MQTT_QOS1, TRUE);
	static const char Count[] = "15";
	Topic.Format ("%s/commandes/etat", (const char *) m_Base);
	Publish (Topic, (const u8 *) Count, sizeof Count - 1, MQTT_QOS1, TRUE);

	AnnounceItems ();
	PublishState (TRUE);
	CLogger::Get ()->Write (From, LogNotice, "Annonce a Home Assistant : %u elements", HAGetEntityCount ());
}

// un interrupteur par texte et par compte a rebours ; les places vides sont retirees
void CHomeAssistant::AnnounceItems (void)
{
	m_nGeneration = CMessages::s_nGeneration;

	for (unsigned nKind = 0; nKind < 2; nKind++)
	{
		boolean bText = nKind == 0;
		unsigned nMax = bText ? MESSAGES_MAX_TEXTS : MESSAGES_MAX_COUNTDOWNS;
		unsigned nCount = bText ? CMessages::s_nTexts : CMessages::s_nCountdowns;
		for (unsigned i = 0; i < nMax; i++)
		{
			// nom : debut du texte, ou titre du compte a rebours (coupe entre deux caracteres)
			char Label[44] = "";
			boolean bActive = FALSE;
			if (i < nCount)
			{
				const char *pSource = bText ? CMessages::s_Texts[i].Text : CMessages::s_Countdowns[i].Title;
				if (!bText && pSource[0] == '\0')
				{
					pSource = CMessages::s_Countdowns[i].Target;
				}
				strncpy (Label, pSource, sizeof Label - 1);
				Label[sizeof Label - 1] = '\0';
				size_t n = strlen (Label);
				while (n > 0 && (Label[n - 1] & 0xC0) == 0x80)	{ Label[--n] = '\0'; }
				if (n > 0 && (Label[n - 1] & 0x80))		{ Label[--n] = '\0'; }
				if (Label[0] == '\0')
				{
					strcpy (Label, "(vide)");
				}
				bActive = bText ? CMessages::s_Texts[i].bActive : CMessages::s_Countdowns[i].bActive;
			}

			CString Topic, Payload;
			HABuildItemDiscovery (bText, i, Label, m_NodeId, m_pConfig->NetworkName, &Topic, &Payload);
			Publish (Topic, (const u8 *) (const char *) Payload, Payload.GetLength (), MQTT_QOS1, TRUE);

			if (i < nCount)
			{
				Topic.Format ("%s/%s_%u/etat", (const char *) m_Base, bText ? "texte" : "compte", i);
				const char *pState = bActive ? "ON" : "OFF";
				Publish (Topic, (const u8 *) pState, strlen (pState), MQTT_QOS1, TRUE);
			}
		}
	}
}

void CHomeAssistant::PublishValue (const char *pObject, const char *pValue, CString *pLast, boolean bForce)
{
	if (!bForce && strcmp (*pLast, pValue) == 0)
	{
		return;
	}
	*pLast = pValue;

	CString Topic;
	Topic.Format ("%s/%s/etat", (const char *) m_Base, pObject);
	// QoS 1 : avec QoS 0, Circle 51 ajoute quand meme un numero de paquet (2 octets en trop
	// au debut du message)
	Publish (Topic, (const u8 *) pValue, strlen (pValue), MQTT_QOS1, TRUE);
}

void CHomeAssistant::PublishState (boolean bAll)
{
	CRemoteControl *pRemote = CRemoteControl::Get ();
	TDisplayStatus &S = pRemote->Status;

	PublishValue ("panneau", pRemote->IsDisplayOff () ? "OFF" : "ON", &m_Last[LastPanel], bAll);
	CString Value;
	Value.Format ("%u", S.nBrightness);
	PublishValue ("luminosite", Value, &m_Last[LastBrightness], bAll);
	PublishValue ("pause", pRemote->IsPaused () ? "ON" : "OFF", &m_Last[LastPause], bAll);
	PublishValue ("luminosite_auto", pRemote->GetBrightnessOverride () < 0 ? "ON" : "OFF", &m_Last[LastAuto], bAll);

	// nom de la GIF sans dossier ni extension
	const char *pGif = S.CurrentGif;
	for (const char *p = pGif; *p != '\0'; p++)
	{
		if (*p == '/')
		{
			pGif = p + 1;
		}
	}
	char Name[100];
	strncpy (Name, pGif, sizeof Name - 1);
	Name[sizeof Name - 1] = '\0';
	size_t nLength = strlen (Name);
	if (nLength > 4 && strcasecmp (Name + nLength - 4, ".gif") == 0)
	{
		Name[nLength - 4] = '\0';
	}
	PublishValue ("gif", Name, &m_Last[LastGif], bAll);

	// style des messages de Home Assistant
	const TBannerStyle &N = CMessages::s_Notify;
	PublishValue ("msg_effet", HAGetEffectLabel (N.Effect), &m_Last[LastEffect], bAll);
	PublishValue ("msg_couleur", HAGetColorLabel (N.Style), &m_Last[LastColor], bAll);
	Value.Format ("%u", N.nPasses);
	PublishValue ("msg_passages", Value, &m_Last[LastPasses], bAll);
	Value.Format ("%u", N.nSpeed);
	PublishValue ("msg_vitesse", Value, &m_Last[LastSpeed], bAll);
	PublishValue ("ecran", S.pScreen != 0 ? S.pScreen : "", &m_Last[LastScreen], bAll);

	unsigned nNow = CTimer::Get ()->GetUptime ();
	if (bAll || nNow - m_nLastSlowUpdate >= SLOW_UPDATE_SECONDS)
	{
		m_nLastSlowUpdate = nNow;
		CCPUThrottle *pThrottle = CCPUThrottle::Get ();
		Value.Format ("%u", pThrottle != 0 ? pThrottle->GetTemperature () : 0);
		PublishValue ("temperature", Value, &m_Last[LastTemperature], bAll);
		PublishValue ("ip", m_IPAddress, &m_Last[LastIP], bAll);
	}
}

boolean CHomeAssistant::TakeNotification (THANotification *pNotification)
{
	if (!s_bNotification)
	{
		return FALSE;
	}
	*pNotification = s_Notification;
	s_bNotification = FALSE;

	return TRUE;
}

const char *CHomeAssistant::GetStatus (void)
{
	return s_pStatus;
}
