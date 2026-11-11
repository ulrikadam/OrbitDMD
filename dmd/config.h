//
// config.h - lecture de Config/config.txt
//
// Les noms de reglages sont ceux de l'ancien RPI2DMD, pour pouvoir reprendre
// le fichier existant tel quel. Les reglages nouveaux sont signales "(nouveau)".
//
#ifndef _config_h
#define _config_h

#include <circle/types.h>
#include "clockstyle.h"
#include "schedule.h"

struct TMqttConfig
{
	boolean	 bActive;
	char	 Host[64];		// adresse IP (ou nom connu du DNS de la box)
	unsigned nPort;
	char	 User[64];
	char	 Password[64];		// jamais ecrit dans le journal ni renvoye a la page web
};

#define CONFIG_MAX_CATEGORIES	20
#define CONFIG_NAME_LEN		64

class CConfig
{
public:
	CConfig (void);

	boolean Load (const char *pFileName);
	void Log (void) const;

	unsigned GetBrightness (unsigned nHour) const;	// en %, nHour 0..23

public:
	// Panneaux
	unsigned PanelCols;		// Panel_XSize
	unsigned PanelRows;		// Panel_YSize
	unsigned Chain;			// Panel_XNumber (panneaux a la suite)
	unsigned Parallel;		// Panel_YNumber (chaines en parallele)
	unsigned Slowdown;		// GPIO_Slowdown
	char	 RGBOrder[4];		// RGB_Order
	unsigned PWMBits;		// PWM_Bits
	unsigned PWMLSBNanos;		// PWM_LSB_Nanoseconds (nouveau)
	unsigned Brightness[24];	// Panel_Brightness_0..23

	// GIF
	boolean	 GifActive;		// Gif_Active
	unsigned nCategories;		// categories actives (Gif_Cat_N sauf "disabled")
	char	 Categories[CONFIG_MAX_CATEGORIES][CONFIG_NAME_LEN];
	unsigned GifMinDuration;	// Gif_Min_Duration en ms (nouveau)
	unsigned IPScreenTime;		// IP_Ecran en ms : adresse IP sur le panneau (0 = jamais)

	// Horloge
	boolean	 ClockActive;		// Clock_Active
	unsigned ClockDisplayTime;	// Clock_Display_Time en ms
	char	 TimeZone[CONFIG_NAME_LEN];	// Clock_TZ
	TClockStyle ClockStyle;			// police, couleurs, formats, date (clockstyle.h)

	// Meteo (Weather_*)
	TWeatherStyle WeatherStyle;

	// Messages (Texte_*, Compte_*) et ordre de lecture des GIF
	TBannerStyle Banner;
	TCountdownStyle Countdown;
	boolean	 GifInOrder;		// Gif_Ordre : "ordre" (alphabetique) ou "hasard"
	TSchedule Schedule;		// programme : Prog_*, Gif_Cat_Mois (schedule.h)

	// Demarrage
	boolean	 TestPattern;		// Test_Pattern (nouveau)
	boolean	 LogToScreen;		// Journal_Ecran (nouveau) : journal aussi sur l'ecran HDMI

	// Reseau
	char	 NetMode[16];			// Reseau : auto (cable si branche), wifi ou ethernet
	char	 WifiSSID[CONFIG_NAME_LEN];	// Wifi_ssid
	char	 WifiPSK[CONFIG_NAME_LEN];	// Wifi_psk (jamais ecrit dans le journal)
	char	 WifiCountry[8];		// Wifi_country
	char	 NetworkName[32];		// Network_name (nom sur le reseau : <nom>.local)
	char	 WebPassword[CONFIG_NAME_LEN];	// Web_Password (nouveau, jamais ecrit dans le journal)

	// Home Assistant (MQTT) : HA_Active, HA_Serveur, HA_Port, HA_Utilisateur, HA_MotDePasse
	TMqttConfig Mqtt;

	// Adresse IP fixe (nouveau) : IP_Fixe vide = adresse donnee par la box (DHCP)
	char	 IPAddress[16];			// IP_Fixe
	char	 IPMask[16];			// IP_Masque
	char	 IPGateway[16];			// IP_Passerelle
	char	 IPDNS[16];			// IP_DNS

private:
	void SetValue (const char *pKey, const char *pValue);

	char m_RawCategory[CONFIG_MAX_CATEGORIES][CONFIG_NAME_LEN];
};

#endif
