//
// hapayload.h - textes echanges avec Home Assistant (MQTT), sans reseau : testables sur PC
//
// Sujets (base = "orbitdmd/<nom>", nom = Network_name en minuscules) :
//   <base>/dispo			"online" / "offline" (derniere volonte)
//   <base>/<objet>/set		commandes de Home Assistant
//   <base>/<objet>/etat		etats publies par le panneau
//   <base>/notification		texte simple, ou JSON {"message": ..., "effet": ..., "couleur": ...}
//   homeassistant/<type>/<nom>/<objet>/config	annonces (decouverte automatique)
//
#ifndef _hapayload_h
#define _hapayload_h

#include <circle/string.h>
#include <circle/types.h>
#include "clockstyle.h"

#define HA_DISCOVERY_PREFIX	"homeassistant"
#define HA_NOTIFY_GIF_LEN	140

struct THAEntity
{
	const char *pComponent;		// light, switch, button, sensor, notify
	const char *pObject;		// panneau, pause...
	const char *pName;		// nom affiche dans Home Assistant
	const char *pExtra;		// champs JSON en plus (avec %s = base des sujets), ou 0
};

// "OrbitDMD" -> "orbitdmd" (lettres, chiffres, _ et - seulement)
void HAMakeNodeId (const char *pNetworkName, char *pOut, unsigned nSize);

unsigned HAGetEntityCount (void);
const THAEntity &HAGetEntity (unsigned nIndex);

// sujet et texte d'annonce de l'element nIndex
void HABuildDiscovery (unsigned nIndex, const char *pNodeId, const char *pDeviceName,
		       const char *pVersion, const char *pIPAddress, CString *pTopic, CString *pPayload);

struct THANotification
{
	TBannerStyle Style;
	char Gif[HA_NOTIFY_GIF_LEN];	// "Categorie/nom.gif" a jouer avant, ou vide
};

// texte simple ou JSON -> notification (style de depart : rDefault, reglable depuis Home
// Assistant) ; FALSE si aucun message ni GIF
boolean HAParseNotification (const char *pPayload, const TBannerStyle &rDefault, THANotification *pNotification);

// listes de choix de Home Assistant (effet et couleur des messages)
unsigned HAGetEffectCount (void);
const char *HAGetEffectLabel (unsigned nEffect);		// "Pluie de lettres"
boolean HASetEffectFromLabel (const char *pLabel, TBannerStyle *pStyle);
const char *HAGetColorLabel (const TTextStyle &rStyle);	// "Rouge", "Arc-en-ciel", ou "Autre"
boolean HASetColorFromLabel (const char *pLabel, TTextStyle *pStyle);

// interrupteur d'un texte ou d'un compte a rebours de la page Messages (objet "texte_3",
// "compte_0") ; pLabel vide : annonce vide, qui retire l'element de Home Assistant
void HABuildItemDiscovery (boolean bText, unsigned nIndex, const char *pLabel, const char *pNodeId,
			   const char *pDeviceName, CString *pTopic, CString *pPayload);

// liste des commandes (attributs du capteur "Commandes", sujet <base>/commandes)
void HABuildCommandList (const char *pNodeId, CString *pJSON);

// valeur d'une cle d'un objet JSON simple (texte, nombre ou booleen) ; FALSE si absente
boolean HAGetJSONValue (const char *pJSON, const char *pKey, char *pValue, unsigned nSize);

#endif
