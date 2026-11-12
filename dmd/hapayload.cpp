//
// hapayload.cpp
//
#include "hapayload.h"
#include <circle/util.h>

static void AppendOptions (CString *pOut, boolean bEffect);

// %s : base des sujets ("orbitdmd/<nom>")
static const THAEntity s_Entities[] =
{
	{"light", "panneau", {"Panneau", "Panel", "Paneel", "Panel", "Панель", "Panel", "Pannello", "Painel", "Panel", "Панель", "Panel", "Panel"},
	 "\"command_topic\":\"%s/panneau/set\",\"state_topic\":\"%s/panneau/etat\","
	 "\"brightness_command_topic\":\"%s/luminosite/set\",\"brightness_state_topic\":\"%s/luminosite/etat\","
	 "\"brightness_scale\":100,\"icon\":\"mdi:led-strip-variant\""},
	{"switch", "pause", {"Pause", "Pause", "Pauze", "Pausa", "Пауза", "Pause", "Pausa", "Pausa", "Pauza", "Пауза", "Duraklat", "Pauza"},
	 "\"command_topic\":\"%s/pause/set\",\"state_topic\":\"%s/pause/etat\",\"icon\":\"mdi:pause\""},
	{"switch", "luminosite_auto", {"Luminosité heure par heure", "Hourly brightness", "Helderheid per uur", "Brillo por horas", "Яркость по часам", "Helligkeit nach Uhrzeit", "Luminosità oraria", "Brilho por hora", "Jasność godzinowa", "Яскравість по годинах", "Saatlik parlaklık", "Jas podle hodin"},
	 "\"command_topic\":\"%s/luminosite_auto/set\",\"state_topic\":\"%s/luminosite_auto/etat\","
	 "\"entity_category\":\"config\",\"icon\":\"mdi:brightness-auto\""},
	{"button", "heure", {"Afficher l'heure", "Show time", "Tijd tonen", "Mostrar la hora", "Показать время", "Uhrzeit anzeigen", "Mostra l'ora", "Mostrar a hora", "Pokaż godzinę", "Показати час", "Saati göster", "Zobrazit čas"},
	 "\"command_topic\":\"%s/heure/set\",\"icon\":\"mdi:clock-outline\""},
	{"button", "meteo", {"Afficher la météo", "Show weather", "Weer tonen", "Mostrar el tiempo", "Показать погоду", "Wetter anzeigen", "Mostra il meteo", "Mostrar o tempo", "Pokaż pogodę", "Показати погоду", "Havayı göster", "Zobrazit počasí"},
	 "\"command_topic\":\"%s/meteo/set\",\"icon\":\"mdi:weather-partly-cloudy\""},
	{"button", "redemarrer", {"Redémarrer", "Restart", "Herstarten", "Reiniciar", "Перезапуск", "Neustart", "Riavvia", "Reiniciar", "Uruchom ponownie", "Перезапуск", "Yeniden başlat", "Restartovat"},
	 "\"command_topic\":\"%s/redemarrer/set\",\"device_class\":\"restart\",\"entity_category\":\"config\""},
	{"notify", "notification", {"Message", "Message", "Bericht", "Mensaje", "Сообщение", "Nachricht", "Messaggio", "Mensagem", "Wiadomość", "Повідомлення", "Mesaj", "Zpráva"},
	 "\"command_topic\":\"%s/notification\",\"icon\":\"mdi:message-text\""},
	{"text", "texte", {"Texte à afficher", "Text to show", "Te tonen tekst", "Texto a mostrar", "Текст для показа", "Anzuzeigender Text", "Testo da mostrare", "Texto a mostrar", "Tekst do wyświetlenia", "Текст для показу", "Gösterilecek metin", "Text k zobrazení"},
	 "\"command_topic\":\"%s/texte/set\",\"state_topic\":\"%s/texte/etat\",\"max\":200,\"mode\":\"text\","
	 "\"icon\":\"mdi:message-draw\""},
	{"select", "msg_effet", {"Effet des messages", "Message effect", "Effect van berichten", "Efecto de los mensajes", "Эффект сообщений", "Nachrichteneffekt", "Effetto dei messaggi", "Efeito das mensagens", "Efekt wiadomości", "Ефект повідомлень", "Mesaj efekti", "Efekt zpráv"},
	 "\"command_topic\":\"%s/msg_effet/set\",\"state_topic\":\"%s/msg_effet/etat\","
	 "\"entity_category\":\"config\",\"icon\":\"mdi:animation-play\""},
	{"select", "msg_couleur", {"Couleur des messages", "Message colour", "Kleur van berichten", "Color de los mensajes", "Цвет сообщений", "Nachrichtenfarbe", "Colore dei messaggi", "Cor das mensagens", "Kolor wiadomości", "Колір повідомлень", "Mesaj rengi", "Barva zpráv"},
	 "\"command_topic\":\"%s/msg_couleur/set\",\"state_topic\":\"%s/msg_couleur/etat\","
	 "\"entity_category\":\"config\",\"icon\":\"mdi:palette\""},
	{"number", "msg_passages", {"Passages des messages", "Message passes", "Aantal keer berichten", "Pasadas de los mensajes", "Проходы сообщений", "Nachrichtendurchläufe", "Passaggi dei messaggi", "Passagens das mensagens", "Przebiegi wiadomości", "Проходи повідомлень", "Mesaj geçişleri", "Průchody zpráv"},
	 "\"command_topic\":\"%s/msg_passages/set\",\"state_topic\":\"%s/msg_passages/etat\","
	 "\"min\":1,\"max\":10,\"step\":1,\"mode\":\"box\",\"entity_category\":\"config\",\"icon\":\"mdi:repeat\""},
	{"number", "msg_vitesse", {"Vitesse des messages", "Message speed", "Snelheid van berichten", "Velocidad de los mensajes", "Скорость сообщений", "Nachrichtentempo", "Velocità dei messaggi", "Velocidade das mensagens", "Prędkość wiadomości", "Швидкість повідомлень", "Mesaj hızı", "Rychlost zpráv"},
	 "\"command_topic\":\"%s/msg_vitesse/set\",\"state_topic\":\"%s/msg_vitesse/etat\","
	 "\"min\":1,\"max\":10,\"step\":1,\"mode\":\"slider\",\"entity_category\":\"config\",\"icon\":\"mdi:speedometer\""},
	{"sensor", "commandes", {"Commandes MQTT", "MQTT commands", "MQTT-opdrachten", "Comandos MQTT", "Команды MQTT", "MQTT-Befehle", "Comandi MQTT", "Comandos MQTT", "Polecenia MQTT", "Команди MQTT", "MQTT komutları", "Příkazy MQTT"},
	 "\"state_topic\":\"%s/commandes/etat\",\"json_attributes_topic\":\"%s/commandes\","
	 "\"entity_category\":\"diagnostic\",\"icon\":\"mdi:format-list-bulleted\""},
	{"sensor", "gif", {"GIF affichée", "Current GIF", "Getoonde GIF", "GIF mostrado", "Текущий GIF", "Aktuelles GIF", "GIF mostrata", "GIF exibido", "Wyświetlany GIF", "Поточний GIF", "Gösterilen GIF", "Zobrazený GIF"},
	 "\"state_topic\":\"%s/gif/etat\",\"icon\":\"mdi:file-gif-box\""},
	{"sensor", "ecran", {"Écran", "Screen", "Scherm", "Pantalla", "Экран", "Bildschirm", "Schermo", "Tela", "Ekran", "Екран", "Ekran", "Obrazovka"},
	 "\"state_topic\":\"%s/ecran/etat\",\"icon\":\"mdi:monitor\""},
	{"sensor", "temperature", {"Température du processeur", "Processor temperature", "Processortemperatuur", "Temperatura del procesador", "Температура процессора", "Prozessortemperatur", "Temperatura del processore", "Temperatura do processador", "Temperatura procesora", "Температура процесора", "İşlemci sıcaklığı", "Teplota procesoru"},
	 "\"state_topic\":\"%s/temperature/etat\",\"device_class\":\"temperature\","
	 "\"unit_of_measurement\":\"°C\",\"state_class\":\"measurement\",\"entity_category\":\"diagnostic\""},
	{"sensor", "ip", {"Adresse IP", "IP address", "IP-adres", "Dirección IP", "IP-адрес", "IP-Adresse", "Indirizzo IP", "Endereço IP", "Adres IP", "IP-адреса", "IP adresi", "IP adresa"},
	 "\"state_topic\":\"%s/ip/etat\",\"entity_category\":\"diagnostic\",\"icon\":\"mdi:ip-network\""},
	{"sensor", "version", {"Version", "Version", "Versie", "Versión", "Версия", "Version", "Versione", "Versão", "Wersja", "Версія", "Sürüm", "Verze"},
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
	AppendJSONText (pPayload, E.pName[GetLanguage ()]);
	pPayload->Append ("\",\"unique_id\":\"orbitdmd_");
	pPayload->Append (pNodeId);
	pPayload->Append ("_");
	pPayload->Append (E.pObject);
	pPayload->Append ("\",\"availability_topic\":\"");
	pPayload->Append (Base);
	pPayload->Append ("/dispo\",");

	// listes de choix (dans la langue du panneau)
	if (strcmp (E.pObject, "msg_effet") == 0 || strcmp (E.pObject, "msg_couleur") == 0)
	{
		AppendOptions (pPayload, strcmp (E.pObject, "msg_effet") == 0);
	}

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
	static const char *s_Model[LangCount] = {"Raspberry Pi 3 + panneaux HUB75", "Raspberry Pi 3 + HUB75 panels",
		"Raspberry Pi 3 + HUB75-panelen", "Raspberry Pi 3 + paneles HUB75", "Raspberry Pi 3 + панели HUB75",
		"Raspberry Pi 3 + HUB75-Panels", "Raspberry Pi 3 + pannelli HUB75", "Raspberry Pi 3 + painéis HUB75",
		"Raspberry Pi 3 + panele HUB75", "Raspberry Pi 3 + панелі HUB75", "Raspberry Pi 3 + HUB75 paneller",
		"Raspberry Pi 3 + panely HUB75"};
	pPayload->Append ("\",\"manufacturer\":\"OrbitDMD\",\"model\":\"");
	pPayload->Append (s_Model[GetLanguage ()]);
	pPayload->Append ("\",\"sw_version\":\"");
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
	static const char *s_Prefix[2][LangCount] =
	{
		{"Compte à rebours : ", "Countdown: ", "Afteller: ", "Cuenta atrás: ", "Отсчёт: ", "Countdown: ",
		 "Conto alla rovescia: ", "Contagem regressiva: ", "Odliczanie: ", "Відлік: ", "Geri sayım: ", "Odpočet: "},
		{"Texte : ", "Text: ", "Tekst: ", "Texto: ", "Текст: ", "Text: ", "Testo: ", "Texto: ", "Tekst: ",
		 "Текст: ", "Metin: ", "Text: "}
	};
	pPayload->Append (s_Prefix[bText ? 1 : 0][GetLanguage ()]);
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

#define HA_EFFECTS	7
#define HA_COLORS	10

static const char *s_EffectLabels[LangCount][HA_EFFECTS] =
{
	{"Défile à gauche", "Défile à droite", "Monte", "Descend", "Lettre par lettre", "Pluie de lettres", "Fondu"},
	{"Scroll left", "Scroll right", "Rise", "Fall", "Letter by letter", "Raining letters", "Fade"},
	{"Naar links", "Naar rechts", "Omhoog", "Omlaag", "Letter voor letter", "Regen van letters", "Vervagen"},
	{"Hacia la izquierda", "Hacia la derecha", "Sube", "Baja", "Letra a letra", "Lluvia de letras", "Fundido"},
	{"Влево", "Вправо", "Вверх", "Вниз", "По буквам", "Дождь из букв", "Плавно"},
	{"Nach links", "Nach rechts", "Nach oben", "Nach unten", "Buchstabe für Buchstabe", "Buchstabenregen", "Einblenden"},
	{"A sinistra", "A destra", "Sale", "Scende", "Lettera per lettera", "Pioggia di lettere", "Dissolvenza"},
	{"Para a esquerda", "Para a direita", "Sobe", "Desce", "Letra por letra", "Chuva de letras", "Esmaecer"},
	{"W lewo", "W prawo", "W górę", "W dół", "Litera po literze", "Deszcz liter", "Przenikanie"},
	{"Ліворуч", "Праворуч", "Вгору", "Вниз", "По літерах", "Дощ із літер", "Плавно"},
	{"Sola", "Sağa", "Yukarı", "Aşağı", "Harf harf", "Harf yağmuru", "Solma"},
	{"Doleva", "Doprava", "Nahoru", "Dolů", "Písmeno po písmenu", "Déšť písmen", "Prolínání"},
};

static const u8 s_ColorRGB[HA_COLORS][3] =
{
	{0xFF, 0xFF, 0xFF}, {0xFF, 0x20, 0x20}, {0xFF, 0x7A, 0x00}, {0xFF, 0xAA, 0x00}, {0xFF, 0xE0, 0x00},
	{0x20, 0xE0, 0x40}, {0x01, 0xA7, 0xC5}, {0x20, 0x60, 0xFF}, {0xA0, 0x40, 0xFF}, {0xFF, 0x40, 0xA0}
};

// couleurs, puis "arc-en-ciel" et "autre"
static const char *s_ColorLabels[LangCount][HA_COLORS + 2] =
{
	{"Blanc", "Rouge", "Orange", "Ambre", "Jaune", "Vert", "Turquoise", "Bleu", "Violet", "Rose", "Arc-en-ciel", "Autre"},
	{"White", "Red", "Orange", "Amber", "Yellow", "Green", "Turquoise", "Blue", "Purple", "Pink", "Rainbow", "Other"},
	{"Wit", "Rood", "Oranje", "Amber", "Geel", "Groen", "Turkoois", "Blauw", "Paars", "Roze", "Regenboog", "Anders"},
	{"Blanco", "Rojo", "Naranja", "Ámbar", "Amarillo", "Verde", "Turquesa", "Azul", "Violeta", "Rosa", "Arcoíris", "Otro"},
	{"Белый", "Красный", "Оранжевый", "Янтарный", "Жёлтый", "Зелёный", "Бирюзовый", "Синий", "Фиолетовый", "Розовый", "Радуга", "Другой"},
	{"Weiß", "Rot", "Orange", "Bernstein", "Gelb", "Grün", "Türkis", "Blau", "Violett", "Rosa", "Regenbogen", "Andere"},
	{"Bianco", "Rosso", "Arancione", "Ambra", "Giallo", "Verde", "Turchese", "Blu", "Viola", "Rosa", "Arcobaleno", "Altro"},
	{"Branco", "Vermelho", "Laranja", "Âmbar", "Amarelo", "Verde", "Turquesa", "Azul", "Roxo", "Rosa", "Arco-íris", "Outra"},
	{"Biały", "Czerwony", "Pomarańczowy", "Bursztynowy", "Żółty", "Zielony", "Turkusowy", "Niebieski", "Fioletowy", "Różowy", "Tęcza", "Inny"},
	{"Білий", "Червоний", "Помаранчевий", "Бурштиновий", "Жовтий", "Зелений", "Бірюзовий", "Синій", "Фіолетовий", "Рожевий", "Веселка", "Інший"},
	{"Beyaz", "Kırmızı", "Turuncu", "Kehribar", "Sarı", "Yeşil", "Turkuaz", "Mavi", "Mor", "Pembe", "Gökkuşağı", "Diğer"},
	{"Bílá", "Červená", "Oranžová", "Jantarová", "Žlutá", "Zelená", "Tyrkysová", "Modrá", "Fialová", "Růžová", "Duha", "Jiná"},
};

// "options":["...",...] de la liste des effets ou des couleurs
static void AppendOptions (CString *pOut, boolean bEffect)
{
	unsigned nLang = GetLanguage ();
	unsigned nCount = bEffect ? HA_EFFECTS : HA_COLORS + 2;
	pOut->Append ("\"options\":[");
	for (unsigned i = 0; i < nCount; i++)
	{
		pOut->Append (i > 0 ? ",\"" : "\"");
		AppendJSONText (pOut, bEffect ? s_EffectLabels[nLang][i] : s_ColorLabels[nLang][i]);
		pOut->Append ("\"");
	}
	pOut->Append ("],");
}

unsigned HAGetEffectCount (void)
{
	return HA_EFFECTS;
}

const char *HAGetEffectLabel (unsigned nEffect)
{
	return s_EffectLabels[GetLanguage ()][nEffect < HA_EFFECTS ? nEffect : 0];
}

boolean HASetEffectFromLabel (const char *pLabel, TBannerStyle *pStyle)
{
	for (unsigned i = 0; i < HA_EFFECTS; i++)
	{
		boolean bMatch = strcasecmp (pLabel, BannerEffectName (i)) == 0;	// "pluie"
		for (unsigned l = 0; l < LangCount && !bMatch; l++)
		{
			bMatch = strcasecmp (pLabel, s_EffectLabels[l][i]) == 0;	// toutes les langues
		}
		if (bMatch)
		{
			pStyle->Effect = i;
			return TRUE;
		}
	}

	return FALSE;
}

const char *HAGetColorLabel (const TTextStyle &rStyle)
{
	unsigned nLang = GetLanguage ();
	if (strcmp (rStyle.Pattern, CLOCK_RAINBOW) == 0)
	{
		return s_ColorLabels[nLang][HA_COLORS];
	}
	for (unsigned i = 0; i < HA_COLORS; i++)
	{
		if (memcmp (rStyle.Color, s_ColorRGB[i], 3) == 0)
		{
			return s_ColorLabels[nLang][i];
		}
	}

	return s_ColorLabels[nLang][HA_COLORS + 1];
}

boolean HASetColorFromLabel (const char *pLabel, TTextStyle *pStyle)
{
	for (unsigned l = 0; l < LangCount; l++)
	{
		if (strcasecmp (pLabel, s_ColorLabels[l][HA_COLORS]) == 0)
		{
			strcpy (pStyle->Pattern, CLOCK_RAINBOW);
			return TRUE;
		}
		for (unsigned i = 0; i < HA_COLORS; i++)
		{
			if (strcasecmp (pLabel, s_ColorLabels[l][i]) == 0)
			{
				strcpy (pStyle->Pattern, CLOCK_NONE);
				memcpy (pStyle->Color, s_ColorRGB[i], 3);
				return TRUE;
			}
		}
	}
	if (ParseAnyColor (pLabel, pStyle->Color))	// "FF8000"
	{
		strcpy (pStyle->Pattern, CLOCK_NONE);
		return TRUE;
	}

	return FALSE;
}

// --- etat du capteur "Ecran" ---

const char *HAScreenName (const char *pScreen)
{
	static const char *s_Screens[][LangCount + 1] =
	{
		{"heure", "heure", "Time", "Tijd", "Hora", "Время", "Uhrzeit", "Ora", "Hora", "Godzina", "Час", "Saat", "Čas"},
		{"date", "date", "Date", "Datum", "Fecha", "Дата", "Datum", "Data", "Data", "Data", "Дата", "Tarih", "Datum"},
		{"meteo", "meteo", "Weather", "Weer", "Tiempo", "Погода", "Wetter", "Meteo", "Tempo", "Pogoda", "Погода", "Hava", "Počasí"},
		{"message", "message", "Message", "Bericht", "Mensaje", "Сообщение", "Nachricht", "Messaggio", "Mensagem", "Wiadomość", "Повідомлення", "Mesaj", "Zpráva"},
		{"texte", "texte", "Text", "Tekst", "Texto", "Текст", "Text", "Testo", "Texto", "Tekst", "Текст", "Metin", "Text"},
		{"compte", "compte", "Countdown", "Afteller", "Cuenta atrás", "Отсчёт", "Countdown", "Conto alla rovescia", "Contagem", "Odliczanie", "Відлік", "Geri sayım", "Odpočet"},
		{"demarrage", "demarrage", "Start-up", "Opstarten", "Arranque", "Запуск", "Start", "Avvio", "Inicialização", "Uruchamianie", "Запуск", "Başlatma", "Spouštění"},
		{"mire", "mire", "Test pattern", "Testbeeld", "Patrón de prueba", "Тестовая таблица", "Testbild", "Schermata di test", "Padrão de teste", "Obraz testowy", "Тестова таблиця", "Test deseni", "Testovací obrazec"},
	};
	for (unsigned i = 0; i < sizeof s_Screens / sizeof s_Screens[0]; i++)
	{
		if (strcmp (pScreen, s_Screens[i][0]) == 0)
		{
			return s_Screens[i][1 + GetLanguage ()];
		}
	}

	return pScreen;		// "gif"
}

// --- liste des commandes ---

void HABuildCommandList (const char *pNodeId, CString *pJSON)
{
	static const char *Topics[] =
	{
		"panneau/set", "luminosite/set", "luminosite_auto/set", "pause/set", "heure/set", "meteo/set",
		"redemarrer/set", "texte/set", "notification", "msg_effet/set", "msg_couleur/set", "msg_passages/set",
		"msg_vitesse/set", "texte_N/set", "compte_N/set"
	};
	static const char *Texts[LangCount][16] =
	{
		{"ON (allume) ou OFF (eteint, la page web reste joignable)", "0 a 100 (%)",
		 "ON : luminosite heure par heure des reglages", "ON (image figee) ou OFF", "PRESS : afficher l'heure 15 s",
		 "PRESS : afficher la meteo 15 s", "PRESS : redemarrer le panneau", "texte a afficher tout de suite (style des messages)",
		 "texte, ou JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "effet des messages : ",
		 "couleur des messages : ", "1 a 10", "1 a 10", "ON / OFF : activer le texte N de la page Messages (0 = premier)",
		 "ON / OFF : activer le compte a rebours N de la page Messages", "page web du panneau : /aide"},
		{"ON (on) or OFF (off, the web page stays reachable)", "0 to 100 (%)",
		 "ON: hour-by-hour brightness from the settings", "ON (frozen image) or OFF", "PRESS: show the time for 15 s",
		 "PRESS: show the weather for 15 s", "PRESS: restart the panel", "text to show right away (message style)",
		 "text, or JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "message effect: ",
		 "message colour: ", "1 to 10", "1 to 10", "ON / OFF: enable text N of the Messages page (0 = first)",
		 "ON / OFF: enable countdown N of the Messages page", "panel web page: /aide"},
		{"ON (aan) of OFF (uit, de webpagina blijft bereikbaar)", "0 tot 100 (%)",
		 "ON: helderheid per uur uit de instellingen", "ON (bevroren beeld) of OFF", "PRESS: tijd 15 s tonen",
		 "PRESS: weer 15 s tonen", "PRESS: paneel herstarten", "tekst om meteen te tonen (stijl van berichten)",
		 "tekst, of JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "effect van berichten: ",
		 "kleur van berichten: ", "1 tot 10", "1 tot 10", "ON / OFF: tekst N van de pagina Berichten inschakelen (0 = eerste)",
		 "ON / OFF: afteller N van de pagina Berichten inschakelen", "webpagina van het paneel: /aide"},
		{"ON (encendido) u OFF (apagado, la página web sigue accesible)", "0 a 100 (%)",
		 "ON: brillo hora por hora de los ajustes", "ON (imagen congelada) u OFF", "PRESS: mostrar la hora 15 s",
		 "PRESS: mostrar el tiempo 15 s", "PRESS: reiniciar el panel", "texto para mostrar enseguida (estilo de los mensajes)",
		 "texto, o JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "efecto de los mensajes: ",
		 "color de los mensajes: ", "1 a 10", "1 a 10", "ON / OFF: activar el texto N de la página Mensajes (0 = el primero)",
		 "ON / OFF: activar la cuenta atrás N de la página Mensajes", "página web del panel: /aide"},
		{"ON (вкл.) или OFF (выкл., веб-страница доступна)", "от 0 до 100 (%)",
		 "ON: яркость по часам из настроек", "ON (заморозка) или OFF", "PRESS: показать время на 15 с",
		 "PRESS: показать погоду на 15 с", "PRESS: перезапустить панель", "текст для немедленного показа (стиль сообщений)",
		 "текст или JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "эффект сообщений: ",
		 "цвет сообщений: ", "от 1 до 10", "от 1 до 10", "ON / OFF: включить текст N со страницы «Сообщения» (0 — первый)",
		 "ON / OFF: включить отсчёт N со страницы «Сообщения»", "веб-страница панели: /aide"},
		{"ON (an) oder OFF (aus, die Webseite bleibt erreichbar)", "0 bis 100 (%)", "ON: Helligkeit nach Uhrzeit aus den Einstellungen", "ON (Standbild) oder OFF", "PRESS: Uhrzeit 15 s anzeigen", "PRESS: Wetter 15 s anzeigen", "PRESS: Panel neu starten", "Text, der sofort angezeigt wird (Nachrichtenstil)", "Text oder JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "Nachrichteneffekt: ", "Nachrichtenfarbe: ", "1 bis 10", "1 bis 10", "ON / OFF: Text N der Seite Nachrichten aktivieren (0 = erster)", "ON / OFF: Countdown N der Seite Nachrichten aktivieren", "Webseite des Panels: /aide"},
		{"ON (acceso) o OFF (spento, la pagina web resta raggiungibile)", "da 0 a 100 (%)", "ON: luminosità oraria delle impostazioni", "ON (immagine ferma) o OFF", "PRESS: mostra l'ora per 15 s", "PRESS: mostra il meteo per 15 s", "PRESS: riavvia il pannello", "testo da mostrare subito (stile dei messaggi)", "testo, o JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "effetto dei messaggi: ", "colore dei messaggi: ", "da 1 a 10", "da 1 a 10", "ON / OFF: attiva il testo N della pagina Messaggi (0 = il primo)", "ON / OFF: attiva il conto alla rovescia N della pagina Messaggi", "pagina web del pannello: /aide"},
		{"ON (ligado) ou OFF (desligado, a página web continua acessível)", "0 a 100 (%)", "ON: brilho por hora das configurações", "ON (imagem congelada) ou OFF", "PRESS: mostrar a hora por 15 s", "PRESS: mostrar o tempo por 15 s", "PRESS: reiniciar o painel", "texto para mostrar já (estilo das mensagens)", "texto, ou JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "efeito das mensagens: ", "cor das mensagens: ", "1 a 10", "1 a 10", "ON / OFF: ativar o texto N da página Mensagens (0 = o primeiro)", "ON / OFF: ativar a contagem regressiva N da página Mensagens", "página web do painel: /aide"},
		{"ON (włączony) lub OFF (wyłączony, strona WWW nadal dostępna)", "od 0 do 100 (%)", "ON: jasność godzinowa z ustawień", "ON (zatrzymany obraz) lub OFF", "PRESS: pokaż godzinę przez 15 s", "PRESS: pokaż pogodę przez 15 s", "PRESS: uruchom panel ponownie", "tekst do natychmiastowego wyświetlenia (styl wiadomości)", "tekst lub JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "efekt wiadomości: ", "kolor wiadomości: ", "od 1 do 10", "od 1 do 10", "ON / OFF: włącz tekst N ze strony Wiadomości (0 = pierwszy)", "ON / OFF: włącz odliczanie N ze strony Wiadomości", "strona WWW panelu: /aide"},
		{"ON (увімк.) або OFF (вимк., вебсторінка доступна)", "від 0 до 100 (%)", "ON: яскравість по годинах із налаштувань", "ON (заморожене зображення) або OFF", "PRESS: показати час на 15 с", "PRESS: показати погоду на 15 с", "PRESS: перезапустити панель", "текст для негайного показу (стиль повідомлень)", "текст або JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "ефект повідомлень: ", "колір повідомлень: ", "від 1 до 10", "від 1 до 10", "ON / OFF: увімкнути текст N зі сторінки «Повідомлення» (0 — перший)", "ON / OFF: увімкнути відлік N зі сторінки «Повідомлення»", "вебсторінка панелі: /aide"},
		{"ON (açık) veya OFF (kapalı, web sayfası erişilebilir kalır)", "0 ile 100 arası (%)", "ON: ayarlardaki saatlik parlaklık", "ON (dondurulmuş görüntü) veya OFF", "PRESS: saati 15 sn göster", "PRESS: havayı 15 sn göster", "PRESS: paneli yeniden başlat", "hemen gösterilecek metin (mesaj stili)", "metin veya JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "mesaj efekti: ", "mesaj rengi: ", "1 ile 10 arası", "1 ile 10 arası", "ON / OFF: Mesajlar sayfasındaki N. metni etkinleştir (0 = ilki)", "ON / OFF: Mesajlar sayfasındaki N. geri sayımı etkinleştir", "panelin web sayfası: /aide"},
		{"ON (zapnuto) nebo OFF (vypnuto, webová stránka zůstává dostupná)", "0 až 100 (%)", "ON: jas podle hodin z nastavení", "ON (zastavený obraz) nebo OFF", "PRESS: zobrazit čas na 15 s", "PRESS: zobrazit počasí na 15 s", "PRESS: restartovat panel", "text k okamžitému zobrazení (styl zpráv)", "text nebo JSON {message, effet, couleur, police, taille, vitesse, passages, gif}", "efekt zpráv: ", "barva zpráv: ", "1 až 10", "1 až 10", "ON / OFF: zapnout text N ze stránky Zprávy (0 = první)", "ON / OFF: zapnout odpočet N ze stránky Zprávy", "webová stránka panelu: /aide"},
	};
	unsigned nLang = GetLanguage ();

	pJSON->Append ("{");
	for (unsigned i = 0; i < sizeof Topics / sizeof Topics[0]; i++)
	{
		pJSON->Append (i > 0 ? ",\"orbitdmd/" : "\"orbitdmd/");
		pJSON->Append (pNodeId);
		pJSON->Append ("/");
		pJSON->Append (Topics[i]);
		pJSON->Append ("\":\"");
		AppendJSONText (pJSON, Texts[nLang][i]);
		if (i == 9 || i == 10)		// effets et couleurs : la liste des choix
		{
			for (unsigned j = 0; j < (i == 9 ? HA_EFFECTS : HA_COLORS + 1); j++)
			{
				if (j > 0) pJSON->Append (", ");
				AppendJSONText (pJSON, i == 9 ? s_EffectLabels[nLang][j] : s_ColorLabels[nLang][j]);
			}
			if (i == 10) pJSON->Append (", RRGGBB");
		}
		pJSON->Append ("\"");
	}
	pJSON->Append (",\"aide\":\"");
	AppendJSONText (pJSON, Texts[nLang][15]);
	pJSON->Append ("\"}");
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
