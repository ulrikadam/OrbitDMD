//
// hapayload.cpp
//
#include "hapayload.h"
#include <circle/util.h>

// %s : base des sujets ("orbitdmd/<nom>")
static const THAEntity s_Entities[] =
{
	{"light", "panneau", "Panneau",
	 "\"command_topic\":\"%s/panneau/set\",\"state_topic\":\"%s/panneau/etat\","
	 "\"brightness_command_topic\":\"%s/luminosite/set\",\"brightness_state_topic\":\"%s/luminosite/etat\","
	 "\"brightness_scale\":100,\"icon\":\"mdi:led-strip-variant\""},
	{"switch", "pause", "Pause",
	 "\"command_topic\":\"%s/pause/set\",\"state_topic\":\"%s/pause/etat\",\"icon\":\"mdi:pause\""},
	{"switch", "luminosite_auto", "Luminosité heure par heure",
	 "\"command_topic\":\"%s/luminosite_auto/set\",\"state_topic\":\"%s/luminosite_auto/etat\","
	 "\"entity_category\":\"config\",\"icon\":\"mdi:brightness-auto\""},
	{"button", "heure", "Afficher l'heure",
	 "\"command_topic\":\"%s/heure/set\",\"icon\":\"mdi:clock-outline\""},
	{"button", "meteo", "Afficher la météo",
	 "\"command_topic\":\"%s/meteo/set\",\"icon\":\"mdi:weather-partly-cloudy\""},
	{"button", "redemarrer", "Redémarrer",
	 "\"command_topic\":\"%s/redemarrer/set\",\"device_class\":\"restart\",\"entity_category\":\"config\""},
	{"notify", "notification", "Message",
	 "\"command_topic\":\"%s/notification\",\"icon\":\"mdi:message-text\""},
	{"text", "texte", "Texte à afficher",
	 "\"command_topic\":\"%s/texte/set\",\"state_topic\":\"%s/texte/etat\",\"max\":200,\"mode\":\"text\","
	 "\"icon\":\"mdi:message-draw\""},
	{"select", "msg_effet", "Effet des messages",
	 "\"command_topic\":\"%s/msg_effet/set\",\"state_topic\":\"%s/msg_effet/etat\","
	 "\"options\":[\"Défile à gauche\",\"Défile à droite\",\"Monte\",\"Descend\",\"Lettre par lettre\","
	 "\"Pluie de lettres\",\"Fondu\"],\"entity_category\":\"config\",\"icon\":\"mdi:animation-play\""},
	{"select", "msg_couleur", "Couleur des messages",
	 "\"command_topic\":\"%s/msg_couleur/set\",\"state_topic\":\"%s/msg_couleur/etat\","
	 "\"options\":[\"Blanc\",\"Rouge\",\"Orange\",\"Ambre\",\"Jaune\",\"Vert\",\"Turquoise\",\"Bleu\","
	 "\"Violet\",\"Rose\",\"Arc-en-ciel\",\"Autre\"],\"entity_category\":\"config\",\"icon\":\"mdi:palette\""},
	{"number", "msg_passages", "Passages des messages",
	 "\"command_topic\":\"%s/msg_passages/set\",\"state_topic\":\"%s/msg_passages/etat\","
	 "\"min\":1,\"max\":10,\"step\":1,\"mode\":\"box\",\"entity_category\":\"config\",\"icon\":\"mdi:repeat\""},
	{"number", "msg_vitesse", "Vitesse des messages",
	 "\"command_topic\":\"%s/msg_vitesse/set\",\"state_topic\":\"%s/msg_vitesse/etat\","
	 "\"min\":1,\"max\":10,\"step\":1,\"mode\":\"slider\",\"entity_category\":\"config\",\"icon\":\"mdi:speedometer\""},
	{"sensor", "commandes", "Commandes MQTT",
	 "\"state_topic\":\"%s/commandes/etat\",\"json_attributes_topic\":\"%s/commandes\","
	 "\"entity_category\":\"diagnostic\",\"icon\":\"mdi:format-list-bulleted\""},
	{"sensor", "gif", "GIF affichée",
	 "\"state_topic\":\"%s/gif/etat\",\"icon\":\"mdi:file-gif-box\""},
	{"sensor", "ecran", "Écran",
	 "\"state_topic\":\"%s/ecran/etat\",\"icon\":\"mdi:monitor\""},
	{"sensor", "temperature", "Température du processeur",
	 "\"state_topic\":\"%s/temperature/etat\",\"device_class\":\"temperature\","
	 "\"unit_of_measurement\":\"°C\",\"state_class\":\"measurement\",\"entity_category\":\"diagnostic\""},
	{"sensor", "ip", "Adresse IP",
	 "\"state_topic\":\"%s/ip/etat\",\"entity_category\":\"diagnostic\",\"icon\":\"mdi:ip-network\""},
	{"sensor", "version", "Version",
	 "\"state_topic\":\"%s/version/etat\",\"entity_category\":\"diagnostic\",\"icon\":\"mdi:information-outline\""},
};

unsigned HAGetEntityCount (void)
{
	return sizeof s_Entities / sizeof s_Entities[0];
}

const THAEntity &HAGetEntity (unsigned nIndex)
{
	return s_Entities[nIndex];
}

void HAMakeNodeId (const char *pNetworkName, char *pOut, unsigned nSize)
{
	unsigned n = 0;
	for (const char *p = pNetworkName; *p != '\0' && n < nSize - 1; p++)
	{
		char c = *p;
		if ('A' <= c && c <= 'Z')
		{
			c = c - 'A' + 'a';
		}
		if (('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '_' || c == '-')
		{
			pOut[n++] = c;
		}
	}
	if (n == 0)
	{
		strcpy (pOut, "orbitdmd");
		return;
	}
	pOut[n] = '\0';
}

// texte JSON echappe (guillemets, barres obliques inverses, caracteres de controle)
static void AppendJSONText (CString *pOut, const char *pText)
{
	for (const char *p = pText; *p != '\0'; p++)
	{
		if (*p == '"' || *p == '\\')
		{
			pOut->Append ('\\');
		}
		if ((unsigned char) *p >= ' ')
		{
			pOut->Append (*p);
		}
	}
}

void HABuildDiscovery (unsigned nIndex, const char *pNodeId, const char *pDeviceName,
		       const char *pVersion, const char *pIPAddress, CString *pTopic, CString *pPayload)
{
	const THAEntity &E = s_Entities[nIndex];
	CString Base;
	Base.Format ("orbitdmd/%s", pNodeId);

	pTopic->Format ("%s/%s/%s/%s/config", HA_DISCOVERY_PREFIX, E.pComponent, pNodeId, E.pObject);

	*pPayload = "{\"name\":\"";
	AppendJSONText (pPayload, E.pName);
	pPayload->Append ("\",\"unique_id\":\"orbitdmd_");
	pPayload->Append (pNodeId);
	pPayload->Append ("_");
	pPayload->Append (E.pObject);
	pPayload->Append ("\",\"availability_topic\":\"");
	pPayload->Append (Base);
	pPayload->Append ("/dispo\",");

	// champs propres a l'element : chaque %s devient la base des sujets
	for (const char *p = E.pExtra; *p != '\0'; p++)
	{
		if (p[0] == '%' && p[1] == 's')
		{
			pPayload->Append (Base);
			p++;
		}
		else
		{
			pPayload->Append (*p);
		}
	}

	pPayload->Append (",\"device\":{\"identifiers\":[\"orbitdmd_");
	pPayload->Append (pNodeId);
	pPayload->Append ("\"],\"name\":\"");
	AppendJSONText (pPayload, pDeviceName);
	pPayload->Append ("\",\"manufacturer\":\"OrbitDMD\",\"model\":\"Raspberry Pi 3 + panneaux HUB75\",\"sw_version\":\"");
	AppendJSONText (pPayload, pVersion);
	pPayload->Append ("\",\"configuration_url\":\"http://");
	AppendJSONText (pPayload, pIPAddress);
	pPayload->Append ("/\"}}");
}

// --- interrupteurs des textes et comptes a rebours ---

void HABuildItemDiscovery (boolean bText, unsigned nIndex, const char *pLabel, const char *pNodeId,
			   const char *pDeviceName, CString *pTopic, CString *pPayload)
{
	CString Object, Base;
	Object.Format ("%s_%u", bText ? "texte" : "compte", nIndex);
	Base.Format ("orbitdmd/%s", pNodeId);
	pTopic->Format ("%s/switch/%s/%s/config", HA_DISCOVERY_PREFIX, pNodeId, (const char *) Object);

	if (pLabel[0] == '\0')
	{
		*pPayload = "";			// annonce vide : Home Assistant retire l'element
		return;
	}

	*pPayload = "{\"name\":\"";
	pPayload->Append (bText ? "Texte : " : "Compte à rebours : ");
	AppendJSONText (pPayload, pLabel);
	pPayload->Append ("\",\"unique_id\":\"orbitdmd_");
	pPayload->Append (pNodeId);
	pPayload->Append ("_");
	pPayload->Append (Object);
	pPayload->Append ("\",\"availability_topic\":\"");
	pPayload->Append (Base);
	pPayload->Append ("/dispo\",\"command_topic\":\"");
	pPayload->Append (Base);
	pPayload->Append ("/");
	pPayload->Append (Object);
	pPayload->Append ("/set\",\"state_topic\":\"");
	pPayload->Append (Base);
	pPayload->Append ("/");
	pPayload->Append (Object);
	pPayload->Append ("/etat\",\"icon\":\"");
	pPayload->Append (bText ? "mdi:message-text-outline" : "mdi:timer-sand");
	pPayload->Append ("\",\"device\":{\"identifiers\":[\"orbitdmd_");
	pPayload->Append (pNodeId);
	pPayload->Append ("\"],\"name\":\"");
	AppendJSONText (pPayload, pDeviceName);
	pPayload->Append ("\"}}");
}

// --- effet et couleur des messages ---

static const char *s_EffectLabels[] =
	{"Défile à gauche", "Défile à droite", "Monte", "Descend", "Lettre par lettre", "Pluie de lettres", "Fondu"};

static const struct { const char *pLabel; u8 RGB[3]; } s_Colors[] =
{
	{"Blanc", {0xFF, 0xFF, 0xFF}}, {"Rouge", {0xFF, 0x20, 0x20}}, {"Orange", {0xFF, 0x7A, 0x00}},
	{"Ambre", {0xFF, 0xAA, 0x00}}, {"Jaune", {0xFF, 0xE0, 0x00}}, {"Vert", {0x20, 0xE0, 0x40}},
	{"Turquoise", {0x01, 0xA7, 0xC5}}, {"Bleu", {0x20, 0x60, 0xFF}}, {"Violet", {0xA0, 0x40, 0xFF}},
	{"Rose", {0xFF, 0x40, 0xA0}}
};

unsigned HAGetEffectCount (void)
{
	return sizeof s_EffectLabels / sizeof s_EffectLabels[0];
}

const char *HAGetEffectLabel (unsigned nEffect)
{
	return nEffect < HAGetEffectCount () ? s_EffectLabels[nEffect] : s_EffectLabels[0];
}

boolean HASetEffectFromLabel (const char *pLabel, TBannerStyle *pStyle)
{
	for (unsigned i = 0; i < HAGetEffectCount (); i++)
	{
		if (strcmp (pLabel, s_EffectLabels[i]) == 0 || strcasecmp (pLabel, BannerEffectName (i)) == 0)
		{
			pStyle->Effect = i;
			return TRUE;
		}
	}

	return FALSE;
}

const char *HAGetColorLabel (const TTextStyle &rStyle)
{
	if (strcmp (rStyle.Pattern, CLOCK_RAINBOW) == 0)
	{
		return "Arc-en-ciel";
	}
	for (unsigned i = 0; i < sizeof s_Colors / sizeof s_Colors[0]; i++)
	{
		if (memcmp (rStyle.Color, s_Colors[i].RGB, 3) == 0)
		{
			return s_Colors[i].pLabel;
		}
	}

	return "Autre";
}

boolean HASetColorFromLabel (const char *pLabel, TTextStyle *pStyle)
{
	if (strcasecmp (pLabel, "Arc-en-ciel") == 0 || strcasecmp (pLabel, "rainbow") == 0)
	{
		strcpy (pStyle->Pattern, CLOCK_RAINBOW);
		return TRUE;
	}
	for (unsigned i = 0; i < sizeof s_Colors / sizeof s_Colors[0]; i++)
	{
		if (strcasecmp (pLabel, s_Colors[i].pLabel) == 0)
		{
			strcpy (pStyle->Pattern, CLOCK_NONE);
			memcpy (pStyle->Color, s_Colors[i].RGB, 3);
			return TRUE;
		}
	}
	if (ParseAnyColor (pLabel, pStyle->Color))	// "FF8000"
	{
		strcpy (pStyle->Pattern, CLOCK_NONE);
		return TRUE;
	}

	return FALSE;
}

// --- liste des commandes ---

void HABuildCommandList (const char *pNodeId, CString *pJSON)
{
	static const char *Commands[][2] =
	{
		{"panneau/set",		"ON (allume) ou OFF (eteint, la page web reste joignable)"},
		{"luminosite/set",	"0 a 100 (%)"},
		{"luminosite_auto/set",	"ON : luminosite heure par heure des reglages"},
		{"pause/set",		"ON (image figee) ou OFF"},
		{"heure/set",		"PRESS : afficher l'heure 15 s"},
		{"meteo/set",		"PRESS : afficher la meteo 15 s"},
		{"redemarrer/set",	"PRESS : redemarrer le panneau"},
		{"texte/set",		"texte a afficher tout de suite (style des messages)"},
		{"notification",	"texte, ou JSON {message, effet, couleur, police, taille, vitesse, passages, gif}"},
		{"msg_effet/set",	"effet des messages : Défile à gauche, Défile à droite, Monte, Descend, Lettre par lettre, Pluie de lettres, Fondu"},
		{"msg_couleur/set",	"couleur des messages : Blanc, Rouge, Orange, Ambre, Jaune, Vert, Turquoise, Bleu, Violet, Rose, Arc-en-ciel ou RRGGBB"},
		{"msg_passages/set",	"1 a 10"},
		{"msg_vitesse/set",	"1 a 10"},
		{"texte_N/set",		"ON / OFF : activer le texte N de la page Messages (0 = premier)"},
		{"compte_N/set",	"ON / OFF : activer le compte a rebours N de la page Messages"},
	};

	pJSON->Append ("{");
	for (unsigned i = 0; i < sizeof Commands / sizeof Commands[0]; i++)
	{
		pJSON->Append (i > 0 ? ",\"orbitdmd/" : "\"orbitdmd/");
		pJSON->Append (pNodeId);
		pJSON->Append ("/");
		pJSON->Append (Commands[i][0]);
		pJSON->Append ("\":\"");
		AppendJSONText (pJSON, Commands[i][1]);
		pJSON->Append ("\"");
	}
	pJSON->Append (",\"aide\":\"page web du panneau : /aide\"}");
}

// --- lecture d'un JSON simple ---

static void AppendUTF8 (char *pOut, unsigned *pN, unsigned nSize, unsigned nCode)
{
	unsigned char Bytes[3];
	unsigned nBytes;
	if (nCode < 0x80)		{ Bytes[0] = (unsigned char) nCode; nBytes = 1; }
	else if (nCode < 0x800)		{ Bytes[0] = 0xC0 | nCode >> 6; Bytes[1] = 0x80 | (nCode & 0x3F); nBytes = 2; }
	else				{ Bytes[0] = 0xE0 | nCode >> 12; Bytes[1] = 0x80 | (nCode >> 6 & 0x3F);
					  Bytes[2] = 0x80 | (nCode & 0x3F); nBytes = 3; }
	if (*pN + nBytes < nSize)
	{
		memcpy (pOut + *pN, Bytes, nBytes);
		*pN += nBytes;
	}
}

static int HexDigit (char c)
{
	if ('0' <= c && c <= '9')	return c - '0';
	if ('a' <= c && c <= 'f')	return c - 'a' + 10;
	if ('A' <= c && c <= 'F')	return c - 'A' + 10;
	return -1;
}

boolean HAGetJSONValue (const char *pJSON, const char *pKey, char *pValue, unsigned nSize)
{
	size_t nKey = strlen (pKey);
	const char *p = pJSON;
	while ((p = strchr (p, '"')) != 0)
	{
		// une cle : "nom" suivi de :
		const char *pName = p + 1;
		const char *pEnd = pName;
		while (*pEnd != '\0' && *pEnd != '"')
		{
			pEnd += *pEnd == '\\' && pEnd[1] != '\0' ? 2 : 1;
		}
		if (*pEnd == '\0')
		{
			return FALSE;
		}
		const char *q = pEnd + 1;
		while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n')
		{
			q++;
		}
		if (*q != ':')
		{
			p = pEnd + 1;			// c'etait une valeur, pas une cle
			continue;
		}
		q++;
		while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n')
		{
			q++;
		}

		boolean bMatch = (size_t) (pEnd - pName) == nKey && strncmp (pName, pKey, nKey) == 0;
		unsigned n = 0;
		if (*q == '"')
		{
			for (q++; *q != '\0' && *q != '"'; q++)
			{
				char c = *q;
				if (c == '\\' && q[1] != '\0')
				{
					c = *++q;
					if (c == 'n' || c == 'r' || c == 't')
					{
						c = ' ';
					}
					else if (c == 'u' && HexDigit (q[1]) >= 0 && HexDigit (q[2]) >= 0
						 && HexDigit (q[3]) >= 0 && HexDigit (q[4]) >= 0)
					{
						unsigned nCode = HexDigit (q[1]) << 12 | HexDigit (q[2]) << 8
							       | HexDigit (q[3]) << 4 | HexDigit (q[4]);
						q += 4;
						if (bMatch)
						{
							AppendUTF8 (pValue, &n, nSize, nCode);
						}
						continue;
					}
				}
				if (bMatch && n < nSize - 1)
				{
					pValue[n++] = c;
				}
			}
			if (*q == '"')
			{
				q++;
			}
		}
		else
		{
			while (*q != '\0' && *q != ',' && *q != '}' && *q != ' ' && *q != '\r' && *q != '\n')
			{
				if (bMatch && n < nSize - 1)
				{
					pValue[n++] = *q;
				}
				q++;
			}
		}

		if (bMatch)
		{
			pValue[n] = '\0';
			return TRUE;
		}
		p = q;
	}

	return FALSE;
}

boolean HAParseNotification (const char *pPayload, const TBannerStyle &rDefault, THANotification *pNotification)
{
	TBannerStyle &S = pNotification->Style;
	S = rDefault;
	S.bActive = TRUE;
	S.Text[0] = '\0';
	pNotification->Gif[0] = '\0';

	const char *p = pPayload;
	while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	{
		p++;
	}

	char Value[BANNER_TEXT_LEN];
	if (*p != '{')
	{
		// texte simple : le message lui-meme (une ligne)
		unsigned n = 0;
		for (; *p != '\0' && n < sizeof Value - 1; p++)
		{
			Value[n++] = (*p == '\r' || *p == '\n' || *p == '"' || *p == '\\') ? ' ' : *p;
		}
		Value[n] = '\0';
		SetBannerStyleValue (&S, "Texte_Message", Value);
		return S.Text[0] != '\0';
	}

	if (HAGetJSONValue (p, "message", Value, sizeof Value))
	{
		for (char *q = Value; *q != '\0'; q++)
		{
			if (*q == '"' || *q == '\\')
			{
				*q = '\'';		// refuses dans les messages
			}
		}
		SetBannerStyleValue (&S, "Texte_Message", Value);
	}
	if (HAGetJSONValue (p, "effet", Value, sizeof Value)
	    && !HASetEffectFromLabel (Value, &S))				SetBannerStyleValue (&S, "Texte_Effet", Value);
	if (HAGetJSONValue (p, "police", Value, sizeof Value))		SetBannerStyleValue (&S, "Texte_Font", Value);
	if (HAGetJSONValue (p, "taille", Value, sizeof Value))		SetBannerStyleValue (&S, "Texte_Font_Size", Value);
	if (HAGetJSONValue (p, "vitesse", Value, sizeof Value))		SetBannerStyleValue (&S, "Texte_Vitesse", Value);
	if (HAGetJSONValue (p, "passages", Value, sizeof Value))	SetBannerStyleValue (&S, "Texte_Passages", Value);
	if (HAGetJSONValue (p, "couleur", Value, sizeof Value))
	{
		HASetColorFromLabel (Value, &S.Style);		// "Rouge", "arc-en-ciel", "FF8000"
	}
	if (HAGetJSONValue (p, "gif", Value, sizeof Value))
	{
		size_t nLength = strlen (Value);
		const char *pSlash = strchr (Value, '/');
		if (   nLength < sizeof pNotification->Gif && pSlash != 0 && strchr (pSlash + 1, '/') == 0
		    && strstr (Value, "..") == 0 && strchr (Value, '\\') == 0 && strchr (Value, ':') == 0)
		{
			strcpy (pNotification->Gif, Value);	// verifie encore a l'affichage (gifmanager)
		}
	}

	return S.Text[0] != '\0' || pNotification->Gif[0] != '\0';
}
