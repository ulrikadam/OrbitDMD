//
// lang.cpp - textes du panneau en 12 langues : francais, anglais, neerlandais, espagnol, russe,
// allemand, italien, portugais (Bresil), polonais, ukrainien, turc, tcheque
//
// Longueur : la police Pixel fait 6 pixels par lettre, soit 21 lettres sur 128 pixels ;
// les descriptions meteo (a cote de l'icone) restent courtes (16 lettres au plus).
// Chaque tableau suit l'ordre de TLanguage (lang.h).
//
#include "lang.h"
#include <circle/string.h>
#include <circle/util.h>

static unsigned s_nLanguage = LangFR;

static const char *s_Codes[LangCount] = {"fr", "en", "nl", "es", "ru", "de", "it", "pt", "pl", "uk", "tr", "cs"};

void SetLanguage (unsigned nLanguage)
{
	s_nLanguage = nLanguage < LangCount ? nLanguage : LangFR;
}

unsigned GetLanguage (void)
{
	return s_nLanguage;
}

const char *LanguageCode (unsigned nLanguage)
{
	return s_Codes[nLanguage < LangCount ? nLanguage : LangFR];
}

boolean ParseLanguage (const char *pCode, unsigned *pLanguage)
{
	for (unsigned i = 0; i < LangCount; i++)
	{
		if (   (pCode[0] | 0x20) == s_Codes[i][0]
		    && (pCode[1] | 0x20) == s_Codes[i][1]
		    && pCode[2] == '\0')
		{
			*pLanguage = i;
			return TRUE;
		}
	}

	return FALSE;
}

// fr, en, nl, es, ru, de, it, pt, pl, uk, tr, cs
static const char *s_Texts[TxtCount][LangCount] =
{
	/* TxtSDCard */
	{"CARTE SD", "SD CARD", "SD-KAART", "TARJETA SD", "SD-КАРТА", "SD-KARTE", "SCHEDA SD", "CARTÃO SD",
	 "KARTA SD", "SD-КАРТКА", "SD KART", "SD KARTA"},
	/* TxtUnreadable */
	{"ILLISIBLE", "UNREADABLE", "ONLEESBAAR", "ILEGIBLE", "НЕ ЧИТАЕТСЯ", "UNLESBAR", "ILLEGGIBILE", "ILEGÍVEL",
	 "NIECZYTELNA", "НЕ ЧИТАЄТЬСЯ", "OKUNAMIYOR", "NEČITELNÁ"},
	/* TxtNoGif */
	{"AUCUNE GIF", "NO GIF", "GEEN GIF", "SIN GIF", "НЕТ GIF", "KEIN GIF", "NESSUNA GIF", "NENHUM GIF",
	 "BRAK GIF", "НЕМАЄ GIF", "GIF YOK", "ŽÁDNÉ GIF"},
	/* TxtInGifFolder */
	{"DANS /GIF", "IN /GIF", "IN /GIF", "EN /GIF", "В ПАПКЕ /GIF", "IN /GIF", "IN /GIF", "EM /GIF",
	 "W /GIF", "У ТЕЦІ /GIF", "/GIF İÇİNDE", "V /GIF"},
	/* TxtClock */
	{"HORLOGE", "CLOCK", "KLOK", "RELOJ", "ЧАСЫ", "UHR", "OROLOGIO", "RELÓGIO",
	 "ZEGAR", "ГОДИННИК", "SAAT", "HODINY"},
	/* TxtMissing */
	{"ABSENTE", "MISSING", "ONTBREEKT", "AUSENTE", "НЕ НАЙДЕНЫ", "FEHLT", "ASSENTE", "AUSENTE",
	 "BRAK", "ВІДСУТНІЙ", "YOK", "CHYBÍ"},
	/* TxtRefreshRate */
	{"RAFRAICHISSEMENT", "REFRESH RATE", "VERVERSING", "REFRESCO", "ЧАСТОТА", "BILDRATE", "FREQUENZA", "ATUALIZAÇÃO",
	 "ODŚWIEŻANIE", "ЧАСТОТА", "YENİLEME", "OBNOVOVÁNÍ"},
	/* TxtWifiConnected */
	{"WIFI CONNECTE", "WIFI CONNECTED", "WIFI VERBONDEN", "WIFI CONECTADO", "WIFI ПОДКЛЮЧЕН", "WLAN VERBUNDEN",
	 "WIFI CONNESSO", "WIFI CONECTADO", "WIFI POŁĄCZONE", "WIFI ПІДКЛЮЧЕНО", "WIFI BAĞLANDI", "WIFI PŘIPOJENO"},
	/* TxtCableConnected */
	{"CABLE CONNECTE", "CABLE CONNECTED", "KABEL VERBONDEN", "CABLE CONECTADO", "КАБЕЛЬ ПОДКЛЮЧЕН", "KABEL VERBUNDEN",
	 "CAVO CONNESSO", "CABO CONECTADO", "KABEL POŁĄCZONY", "КАБЕЛЬ ПІДКЛЮЧЕНО", "KABLO BAĞLANDI", "KABEL PŘIPOJEN"},
	/* TxtCable */
	{"CABLE RESEAU", "NETWORK CABLE", "NETWERKKABEL", "CABLE DE RED", "СЕТЕВОЙ КАБЕЛЬ", "NETZWERKKABEL",
	 "CAVO DI RETE", "CABO DE REDE", "KABEL SIECIOWY", "МЕРЕЖЕВИЙ КАБЕЛЬ", "AĞ KABLOSU", "SÍŤOVÝ KABEL"},
	/* TxtWifi */
	{"WIFI", "WIFI", "WIFI", "WIFI", "WIFI", "WLAN", "WIFI", "WIFI", "WIFI", "WIFI", "WIFI", "WIFI"},
	/* TxtNotConnected */
	{"NON CONNECTE", "NOT CONNECTED", "NIET VERBONDEN", "NO CONECTADO", "НЕ ПОДКЛЮЧЕН", "NICHT VERBUNDEN",
	 "NON CONNESSO", "NÃO CONECTADO", "NIE POŁĄCZONO", "НЕ ПІДКЛЮЧЕНО", "BAĞLI DEĞİL", "NEPŘIPOJENO"},
	/* TxtStarting */
	{"DEMARRAGE", "STARTING", "OPSTARTEN", "INICIANDO", "ЗАПУСК", "START", "AVVIO", "INICIANDO",
	 "URUCHAMIANIE", "ЗАПУСК", "BAŞLATILIYOR", "SPOUŠTĚNÍ"},
	/* TxtTimeUnknown */
	{"HEURE INCONNUE", "TIME UNKNOWN", "TIJD ONBEKEND", "HORA DESCONOCIDA", "ВРЕМЯ НЕИЗВЕСТНО", "ZEIT UNBEKANNT",
	 "ORA SCONOSCIUTA", "HORA DESCONHECIDA", "CZAS NIEZNANY", "ЧАС НЕВІДОМИЙ", "SAAT BİLİNMİYOR", "ČAS NEZNÁMÝ"},
	/* TxtWeather */
	{"METEO", "WEATHER", "WEER", "TIEMPO", "ПОГОДА", "WETTER", "METEO", "TEMPO",
	 "POGODA", "ПОГОДА", "HAVA", "POČASÍ"},
	/* TxtChoosePlace */
	{"LIEU A CHOISIR", "CHOOSE A PLACE", "KIES EEN PLAATS", "ELIGE UN LUGAR", "ВЫБЕРИТЕ МЕСТО", "ORT WÄHLEN",
	 "SCEGLI UN LUOGO", "ESCOLHA UM LOCAL", "WYBIERZ MIEJSCE", "ОБЕРІТЬ МІСЦЕ", "KONUM SEÇİN", "ZVOLTE MÍSTO"},
	/* TxtNotReceived */
	{"PAS ENCORE RECUE", "NOT RECEIVED YET", "NOG NIET ONTVANGEN", "AUN NO RECIBIDO", "ЕЩЁ НЕ ПОЛУЧЕНА",
	 "NOCH NICHT EMPFANGEN", "NON ANCORA RICEVUTO", "AINDA NÃO RECEBIDO", "JESZCZE NIE ODEBRANO", "ЩЕ НЕ ОТРИМАНО",
	 "HENÜZ ALINMADI", "ZATÍM NEPŘIJATO"},
	/* TxtUpdate */
	{"MISE A JOUR", "UPDATE", "UPDATE", "ACTUALIZACION", "ОБНОВЛЕНИЕ", "UPDATE", "AGGIORNAMENTO", "ATUALIZAÇÃO",
	 "AKTUALIZACJA", "ОНОВЛЕННЯ", "GÜNCELLEME", "AKTUALIZACE"},
	/* TxtFirmware */
	{"PROGRAMME", "FIRMWARE", "FIRMWARE", "FIRMWARE", "ПРОШИВКА", "FIRMWARE", "FIRMWARE", "FIRMWARE",
	 "OPROGRAMOWANIE", "ПРОШИВКА", "YAZILIM", "FIRMWARE"},
	/* TxtSettings */
	{"REGLAGES", "SETTINGS", "INSTELLINGEN", "AJUSTES", "НАСТРОЙКИ", "EINSTELLUNGEN", "IMPOSTAZIONI", "CONFIGURAÇÕES",
	 "USTAWIENIA", "НАЛАШТУВАННЯ", "AYARLAR", "NASTAVENÍ"},
	/* TxtUpdateFailed1 */
	{"ECHEC DE LA", "UPDATE", "UPDATE", "ACTUALIZACION", "ОШИБКА", "UPDATE", "AGGIORNAMENTO", "ATUALIZAÇÃO",
	 "AKTUALIZACJA", "ПОМИЛКА", "GÜNCELLEME", "AKTUALIZACE"},
	/* TxtUpdateFailed2 */
	{"MISE A JOUR", "FAILED", "MISLUKT", "FALLIDA", "ОБНОВЛЕНИЯ", "FEHLGESCHLAGEN", "FALLITO", "FALHOU",
	 "NIEUDANA", "ОНОВЛЕННЯ", "BAŞARISIZ", "SELHALA"},
	/* TxtNetworkRefused */
	{"RESEAU REFUSE", "NETWORK FAILED", "NETWERK MISLUKT", "RED RECHAZADA", "СЕТЬ НЕДОСТУПНА", "NETZWERKFEHLER",
	 "RETE RIFIUTATA", "REDE RECUSADA", "SIEĆ ODRZUCONA", "МЕРЕЖА НЕДОСТУПНА", "AĞ REDDEDİLDİ", "SÍŤ ODMÍTNUTA"},
	/* TxtOldSettings */
	{"ANCIENS REGLAGES", "OLD SETTINGS", "OUDE INSTELLINGEN", "AJUSTES ANTERIORES", "СТАРЫЕ НАСТРОЙКИ",
	 "ALTE EINSTELLUNGEN", "VECCHIE IMPOSTAZIONI", "CONFIGURAÇÕES ANTIGAS", "STARE USTAWIENIA", "СТАРІ НАЛАШТУВАННЯ",
	 "ESKİ AYARLAR", "PŮVODNÍ NASTAVENÍ"},
	/* TxtRestarting */
	{"REDEMARRAGE", "RESTARTING", "HERSTARTEN", "REINICIANDO", "ПЕРЕЗАГРУЗКА", "NEUSTART", "RIAVVIO", "REINICIANDO",
	 "RESTARTOWANIE", "ПЕРЕЗАПУСК", "YENİDEN BAŞLATILIYOR", "RESTARTOVÁNÍ"},
	/* TxtDatePassed */
	{"DATE PASSEE", "DATE PASSED", "DATUM VOORBIJ", "FECHA PASADA", "ДАТА ПРОШЛА", "DATUM VORBEI", "DATA PASSATA",
	 "DATA PASSADA", "DATA MINĘŁA", "ДАТА МИНУЛА", "TARİH GEÇTİ", "DATUM UPLYNUL"},
	/* TxtFeels */
	{"RESSENTI", "FEELS", "VOELT", "SENSACION", "ОЩУЩ.", "GEFÜHLT", "PERCEPITA", "SENSAÇÃO",
	 "ODCZUWALNA", "ВІДЧУВ.", "HİSSEDİLEN", "POCITOVĚ"},
	/* TxtHumidityFormat */
	{"HUMIDITE %d%%", "HUMIDITY %d%%", "VOCHT %d%%", "HUMEDAD %d%%", "ВЛАЖН. %d%%", "FEUCHTE %d%%", "UMIDITÀ %d%%",
	 "UMIDADE %d%%", "WILGOTN. %d%%", "ВОЛОГ. %d%%", "NEM %d%%", "VLHKOST %d%%"},
	/* TxtWindFormat */
	{"VENT %d KM/H", "WIND %d KM/H", "WIND %d KM/U", "VIENTO %d KM/H", "ВЕТЕР %d КМ/Ч", "WIND %d KM/H",
	 "VENTO %d KM/H", "VENTO %d KM/H", "WIATR %d KM/H", "ВІТЕР %d КМ/ГОД", "RÜZGAR %d KM/SA", "VÍTR %d KM/H"},
	/* TxtTomorrow */
	{"DEMAIN", "TOMORROW", "MORGEN", "MAÑANA", "ЗАВТРА", "MORGEN", "DOMANI", "AMANHÃ",
	 "JUTRO", "ЗАВТРА", "YARIN", "ZÍTRA"},
	/* TxtDayHourFormat */
	{"%u J %u H", "%uD %uH", "%u D %u U", "%u D %u H", "%u Д %u Ч", "%u T %u STD", "%u G %u H", "%u D %u H",
	 "%u D %u GODZ", "%u Д %u ГОД", "%u G %u SA", "%u D %u H"},
	/* TxtXmasIn */
	{"NOËL DANS", "CHRISTMAS IN", "KERST OVER", "NAVIDAD EN", "ДО РОЖДЕСТВА", "WEIHNACHTEN IN", "NATALE TRA",
	 "NATAL EM", "DO ŚWIĄT", "ДО РІЗДВА", "NOEL'E KALAN", "VÁNOCE ZA"},
	/* TxtMerryXmas */
	{"JOYEUX NOËL !", "MERRY CHRISTMAS!", "VROLIJK KERSTFEEST!", "¡FELIZ NAVIDAD!", "С РОЖДЕСТВОМ!",
	 "FROHE WEIHNACHTEN!", "BUON NATALE!", "FELIZ NATAL!", "WESOŁYCH ŚWIĄT!", "З РІЗДВОМ!", "MUTLU NOELLER!",
	 "VESELÉ VÁNOCE!"},
	/* TxtNoLinux */
	{"SANS LINUX", "NO LINUX", "ZONDER LINUX", "SIN LINUX", "БЕЗ LINUX", "OHNE LINUX", "SENZA LINUX", "SEM LINUX",
	 "BEZ LINUXA", "БЕЗ LINUX", "LINUX YOK", "BEZ LINUXU"},
	/* TxtRed */
	{"ROUGE", "RED", "ROOD", "ROJO", "КРАСНЫЙ", "ROT", "ROSSO", "VERMELHO", "CZERWONY", "ЧЕРВОНИЙ", "KIRMIZI", "ČERVENÁ"},
	/* TxtGreen */
	{"VERT", "GREEN", "GROEN", "VERDE", "ЗЕЛЁНЫЙ", "GRÜN", "VERDE", "VERDE", "ZIELONY", "ЗЕЛЕНИЙ", "YEŞİL", "ZELENÁ"},
	/* TxtBlue */
	{"BLEU", "BLUE", "BLAUW", "AZUL", "СИНИЙ", "BLAU", "BLU", "AZUL", "NIEBIESKI", "СИНІЙ", "MAVİ", "MODRÁ"},
	/* TxtWhite */
	{"BLANC", "WHITE", "WIT", "BLANCO", "БЕЛЫЙ", "WEISS", "BIANCO", "BRANCO", "BIAŁY", "БІЛИЙ", "BEYAZ", "BÍLÁ"},
};

const char *Tr (TText Text)
{
	return (unsigned) Text < TxtCount ? s_Texts[Text][s_nLanguage] : "";
}

// --- jours et mois (sans accent dans les abreviations francaises et espagnoles) ---

static const char *s_DayShort[LangCount][7] =
{
	{"DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM"},
	{"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"},
	{"ZO", "MA", "DI", "WO", "DO", "VR", "ZA"},
	{"DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"},
	{"ВС", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"},
	{"SO", "MO", "DI", "MI", "DO", "FR", "SA"},
	{"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"},
	{"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SÁB"},
	{"NDZ", "PON", "WT", "ŚR", "CZW", "PT", "SOB"},
	{"НД", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"},
	{"PAZ", "PZT", "SAL", "ÇAR", "PER", "CUM", "CMT"},
	{"NE", "PO", "ÚT", "ST", "ČT", "PÁ", "SO"},
};

static const char *s_DayLong[LangCount][7] =
{
	{"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"},
	{"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"},
	{"ZONDAG", "MAANDAG", "DINSDAG", "WOENSDAG", "DONDERDAG", "VRIJDAG", "ZATERDAG"},
	{"DOMINGO", "LUNES", "MARTES", "MIERCOLES", "JUEVES", "VIERNES", "SABADO"},
	{"ВОСКРЕСЕНЬЕ", "ПОНЕДЕЛЬНИК", "ВТОРНИК", "СРЕДА", "ЧЕТВЕРГ", "ПЯТНИЦА", "СУББОТА"},
	{"SONNTAG", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG"},
	{"DOMENICA", "LUNEDÌ", "MARTEDÌ", "MERCOLEDÌ", "GIOVEDÌ", "VENERDÌ", "SABATO"},
	{"DOMINGO", "SEGUNDA", "TERÇA", "QUARTA", "QUINTA", "SEXTA", "SÁBADO"},
	{"NIEDZIELA", "PONIEDZIAŁEK", "WTOREK", "ŚRODA", "CZWARTEK", "PIĄTEK", "SOBOTA"},
	{"НЕДІЛЯ", "ПОНЕДІЛОК", "ВІВТОРОК", "СЕРЕДА", "ЧЕТВЕР", "П'ЯТНИЦЯ", "СУБОТА"},
	{"PAZAR", "PAZARTESİ", "SALI", "ÇARŞAMBA", "PERŞEMBE", "CUMA", "CUMARTESİ"},
	{"NEDĚLE", "PONDĚLÍ", "ÚTERÝ", "STŘEDA", "ČTVRTEK", "PÁTEK", "SOBOTA"},
};

static const char *s_MonthShort[LangCount][12] =
{
	{"JAN", "FEV", "MARS", "AVR", "MAI", "JUIN", "JUIL", "AOUT", "SEPT", "OCT", "NOV", "DEC"},
	{"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"},
	{"JAN", "FEB", "MRT", "APR", "MEI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEC"},
	{"ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"},
	{"ЯНВ", "ФЕВ", "МАР", "АПР", "МАЙ", "ИЮН", "ИЮЛ", "АВГ", "СЕН", "ОКТ", "НОЯ", "ДЕК"},
	{"JAN", "FEB", "MÄR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"},
	{"GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC"},
	{"JAN", "FEV", "MAR", "ABR", "MAI", "JUN", "JUL", "AGO", "SET", "OUT", "NOV", "DEZ"},
	{"STY", "LUT", "MAR", "KWI", "MAJ", "CZE", "LIP", "SIE", "WRZ", "PAŹ", "LIS", "GRU"},
	{"СІЧ", "ЛЮТ", "БЕР", "КВІ", "ТРА", "ЧЕР", "ЛИП", "СЕР", "ВЕР", "ЖОВ", "ЛИС", "ГРУ"},
	{"OCA", "ŞUB", "MAR", "NİS", "MAY", "HAZ", "TEM", "AĞU", "EYL", "EKİ", "KAS", "ARA"},
	{"LED", "ÚNO", "BŘE", "DUB", "KVĚ", "ČVN", "ČVC", "SRP", "ZÁŘ", "ŘÍJ", "LIS", "PRO"},
};

// en russe, ukrainien, polonais et tcheque : forme de la date ("11 НОЯБРЯ", "11 LISTOPADA")
static const char *s_MonthLong[LangCount][12] =
{
	{"JANVIER", "FEVRIER", "MARS", "AVRIL", "MAI", "JUIN", "JUILLET", "AOUT",
	 "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DECEMBRE"},
	{"JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST",
	 "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER"},
	{"JANUARI", "FEBRUARI", "MAART", "APRIL", "MEI", "JUNI", "JULI", "AUGUSTUS",
	 "SEPTEMBER", "OKTOBER", "NOVEMBER", "DECEMBER"},
	{"ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO", "JULIO", "AGOSTO",
	 "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"},
	{"ЯНВАРЯ", "ФЕВРАЛЯ", "МАРТА", "АПРЕЛЯ", "МАЯ", "ИЮНЯ", "ИЮЛЯ", "АВГУСТА",
	 "СЕНТЯБРЯ", "ОКТЯБРЯ", "НОЯБРЯ", "ДЕКАБРЯ"},
	{"JANUAR", "FEBRUAR", "MÄRZ", "APRIL", "MAI", "JUNI", "JULI", "AUGUST",
	 "SEPTEMBER", "OKTOBER", "NOVEMBER", "DEZEMBER"},
	{"GENNAIO", "FEBBRAIO", "MARZO", "APRILE", "MAGGIO", "GIUGNO", "LUGLIO", "AGOSTO",
	 "SETTEMBRE", "OTTOBRE", "NOVEMBRE", "DICEMBRE"},
	{"JANEIRO", "FEVEREIRO", "MARÇO", "ABRIL", "MAIO", "JUNHO", "JULHO", "AGOSTO",
	 "SETEMBRO", "OUTUBRO", "NOVEMBRO", "DEZEMBRO"},
	{"STYCZNIA", "LUTEGO", "MARCA", "KWIETNIA", "MAJA", "CZERWCA", "LIPCA", "SIERPNIA",
	 "WRZEŚNIA", "PAŹDZIERNIKA", "LISTOPADA", "GRUDNIA"},
	{"СІЧНЯ", "ЛЮТОГО", "БЕРЕЗНЯ", "КВІТНЯ", "ТРАВНЯ", "ЧЕРВНЯ", "ЛИПНЯ", "СЕРПНЯ",
	 "ВЕРЕСНЯ", "ЖОВТНЯ", "ЛИСТОПАДА", "ГРУДНЯ"},
	{"OCAK", "ŞUBAT", "MART", "NİSAN", "MAYIS", "HAZİRAN", "TEMMUZ", "AĞUSTOS",
	 "EYLÜL", "EKİM", "KASIM", "ARALIK"},
	{"LEDNA", "ÚNORA", "BŘEZNA", "DUBNA", "KVĚTNA", "ČERVNA", "ČERVENCE", "SRPNA",
	 "ZÁŘÍ", "ŘÍJNA", "LISTOPADU", "PROSINCE"},
};

const char *DayName (unsigned nWeekDay, boolean bLong)
{
	nWeekDay %= 7;
	return bLong ? s_DayLong[s_nLanguage][nWeekDay] : s_DayShort[s_nLanguage][nWeekDay];
}

const char *MonthName (unsigned nMonth, boolean bLong)
{
	nMonth %= 12;
	return bLong ? s_MonthLong[s_nLanguage][nMonth] : s_MonthShort[s_nLanguage][nMonth];
}

// --- meteo (codes WMO regroupes, voir WeatherDescription) ---
// fr, en, nl, es, ru, de, it, pt, pl, uk, tr, cs

static const char *s_Weather[WEATHER_TEXT_COUNT][LangCount] =
{
	{"ENSOLEILLE", "SUNNY", "ZONNIG", "SOLEADO", "СОЛНЕЧНО", "SONNIG", "SOLEGGIATO", "ENSOLARADO",
	 "SŁONECZNIE", "СОНЯЧНО", "GÜNEŞLİ", "SLUNEČNO"},
	{"NUIT CLAIRE", "CLEAR NIGHT", "HELDERE NACHT", "NOCHE CLARA", "ЯСНАЯ НОЧЬ", "KLARE NACHT", "NOTTE SERENA",
	 "NOITE LIMPA", "POGODNA NOC", "ЯСНА НІЧ", "AÇIK GECE", "JASNÁ NOC"},
	{"PEU NUAGEUX", "PARTLY CLOUDY", "LICHT BEWOLKT", "POCO NUBOSO", "МАЛООБЛАЧНО", "LEICHT BEWÖLKT", "POCO NUVOLOSO",
	 "POUCAS NUVENS", "MAŁE ZACHMURZ.", "МАЛОХМАРНО", "AZ BULUTLU", "SKORO JASNO"},
	{"NUAGEUX", "CLOUDY", "BEWOLKT", "NUBOSO", "ОБЛАЧНО", "BEWÖLKT", "NUVOLOSO", "NUBLADO",
	 "POCHMURNO", "ХМАРНО", "BULUTLU", "OBLAČNO"},
	{"COUVERT", "OVERCAST", "ZWAAR BEWOLKT", "CUBIERTO", "ПАСМУРНО", "BEDECKT", "COPERTO", "ENCOBERTO",
	 "ZACHMURZENIE", "ПОХМУРО", "KAPALI", "ZATAŽENO"},
	{"BROUILLARD", "FOG", "MIST", "NIEBLA", "ТУМАН", "NEBEL", "NEBBIA", "NEVOEIRO",
	 "MGŁA", "ТУМАН", "SİS", "MLHA"},
	{"BRUINE", "DRIZZLE", "MOTREGEN", "LLOVIZNA", "МОРОСЬ", "NIESELREGEN", "PIOVIGGINE", "GAROA",
	 "MŻAWKA", "МРЯКА", "ÇİSENTİ", "MRHOLENÍ"},
	{"FORTE BRUINE", "HEAVY DRIZZLE", "ZWARE MOTREGEN", "LLOVIZNA FUERTE", "СИЛЬНАЯ МОРОСЬ", "STARKER NIESEL",
	 "PIOVIGGINE FORTE", "GAROA FORTE", "SILNA MŻAWKA", "СИЛЬНА МРЯКА", "YOĞUN ÇİSENTİ", "SILNÉ MRHOLENÍ"},
	{"BRUINE GEL.", "FREEZ. DRIZZLE", "IJZELMOTREGEN", "LLOVIZNA HELADA", "ЛЕДЯНАЯ МОРОСЬ", "GEFR. NIESEL",
	 "PIOVIGG. GELATA", "GAROA GELADA", "MARZNĄCA MŻAWKA", "КРИЖАНА МРЯКА", "DONAN ÇİSENTİ", "MRZNOUCÍ MRHOL."},
	{"PLUIE FAIBLE", "LIGHT RAIN", "LICHTE REGEN", "LLUVIA DEBIL", "СЛАБЫЙ ДОЖДЬ", "LEICHTER REGEN", "PIOGGIA DEBOLE",
	 "CHUVA FRACA", "SŁABY DESZCZ", "СЛАБКИЙ ДОЩ", "HAFİF YAĞMUR", "SLABÝ DÉŠŤ"},
	{"PLUIE", "RAIN", "REGEN", "LLUVIA", "ДОЖДЬ", "REGEN", "PIOGGIA", "CHUVA",
	 "DESZCZ", "ДОЩ", "YAĞMUR", "DÉŠŤ"},
	{"FORTE PLUIE", "HEAVY RAIN", "ZWARE REGEN", "LLUVIA FUERTE", "СИЛЬНЫЙ ДОЖДЬ", "STARKER REGEN", "PIOGGIA FORTE",
	 "CHUVA FORTE", "ULEWA", "СИЛЬНИЙ ДОЩ", "KUVVETLİ YAĞMUR", "SILNÝ DÉŠŤ"},
	{"VERGLAS", "FREEZING RAIN", "IJZEL", "LLUVIA HELADA", "ГОЛОЛЁД", "GLATTEIS", "GELICIDIO", "CHUVA GELADA",
	 "GOŁOLEDŹ", "ОЖЕЛЕДЬ", "DONAN YAĞMUR", "NÁLEDÍ"},
	{"NEIGE FAIBLE", "LIGHT SNOW", "LICHTE SNEEUW", "NIEVE DEBIL", "СЛАБЫЙ СНЕГ", "LEICHTER SCHNEE", "NEVE DEBOLE",
	 "NEVE FRACA", "SŁABY ŚNIEG", "СЛАБКИЙ СНІГ", "HAFİF KAR", "SLABÉ SNĚŽENÍ"},
	{"NEIGE", "SNOW", "SNEEUW", "NIEVE", "СНЕГ", "SCHNEE", "NEVE", "NEVE",
	 "ŚNIEG", "СНІГ", "KAR", "SNĚŽENÍ"},
	{"FORTE NEIGE", "HEAVY SNOW", "ZWARE SNEEUW", "NEVADA FUERTE", "СИЛЬНЫЙ СНЕГ", "STARKER SCHNEE", "NEVE FORTE",
	 "NEVE FORTE", "SILNY ŚNIEG", "СИЛЬНИЙ СНІГ", "YOĞUN KAR", "SILNÉ SNĚŽENÍ"},
	{"AVERSES", "SHOWERS", "BUIEN", "CHUBASCOS", "ЛИВЕНЬ", "SCHAUER", "ROVESCI", "PANCADAS",
	 "PRZELOTNY DESZCZ", "ЗЛИВА", "SAĞANAK", "PŘEHÁŇKY"},
	{"FORTES AVERSES", "HEAVY SHOWERS", "ZWARE BUIEN", "AGUACEROS", "СИЛЬНЫЙ ЛИВЕНЬ", "STARKE SCHAUER",
	 "ROVESCI FORTI", "PANCADAS FORTES", "SILNE OPADY", "СИЛЬНА ЗЛИВА", "KUVVETLİ SAĞANAK", "SILNÉ PŘEHÁŇKY"},
	{"AVERSES NEIGE", "SNOW SHOWERS", "SNEEUWBUIEN", "CHUBASCOS NIEVE", "СНЕЖНЫЙ ЗАРЯД", "SCHNEESCHAUER",
	 "ROVESCI NEVE", "PANCADAS NEVE", "PRZELOTNY ŚNIEG", "СНІГОПАД", "KAR SAĞANAĞI", "SNĚH. PŘEHÁŇKY"},
	{"ORAGE", "THUNDERSTORM", "ONWEER", "TORMENTA", "ГРОЗА", "GEWITTER", "TEMPORALE", "TROVOADA",
	 "BURZA", "ГРОЗА", "FIRTINA", "BOUŘKA"},
	{"ORAGE GRELE", "HAIL STORM", "ONWEER MET HAGEL", "TORM. GRANIZO", "ГРОЗА С ГРАДОМ", "HAGELGEWITTER", "GRANDINE",
	 "GRANIZO", "BURZA Z GRADEM", "ГРОЗА З ГРАДОМ", "DOLULU FIRTINA", "BOUŘKA S KRUPAMI"},
};

const char *WeatherText (unsigned nIndex)
{
	return nIndex < WEATHER_TEXT_COUNT ? s_Weather[nIndex][s_nLanguage] : "";
}

// --- nombre de jours du compte a rebours (10 jours ou plus) ---

void FormatDays (unsigned nDays, char *pOut, unsigned nSize)
{
	static const char *s_Days[LangCount] =
		{"JOURS", "DAYS", "DAGEN", "DIAS", 0, "TAGE", "GIORNI", "DIAS", "DNI", 0, "GÜN", "DNÍ"};
	const char *pWord = s_Days[s_nLanguage];
	if (pWord == 0)
	{
		// russe et ukrainien : 1, 21, 31... ДЕНЬ ; 2-4, 22-24... ДНЯ / ДНІ ; sinon ДНЕЙ / ДНІВ (11 a 14 aussi)
		boolean bUK = s_nLanguage == LangUK;
		unsigned n10 = nDays % 10, n100 = nDays % 100;
		pWord = n10 == 1 && n100 != 11 ? "ДЕНЬ"
		      : n10 >= 2 && n10 <= 4 && (n100 < 12 || n100 > 14) ? (bUK ? "ДНІ" : "ДНЯ")
		      : (bUK ? "ДНІВ" : "ДНЕЙ");
	}

	CString Text;
	Text.Format ("%u %s", nDays, pWord);
	if (nSize > 0)
	{
		strncpy (pOut, Text, nSize - 1);
		pOut[nSize - 1] = '\0';
	}
}
