//
// weatherdata.h - donnees meteo (Open-Meteo.com, sans cle) et leur lecture
//
#ifndef _weatherdata_h
#define _weatherdata_h

#include <circle/types.h>

struct TWeatherData
{
	boolean	bValid;
	int	nTemp10;		// temperature en dixiemes de degre
	int	nFeels10;		// ressentie
	int	nHumidity;		// %
	int	nWind10;		// vent en dixiemes de km/h
	int	nCode;			// code meteo OMM (0 ciel clair ... 99 orage)
	boolean	bDay;			// jour (sinon nuit)
	int	nMin10, nMax10;		// aujourd'hui
	int	nTomorrowCode;
	int	nTomorrowMin10, nTomorrowMax10;
};

// lit la reponse de /v1/forecast (current=temperature_2m,apparent_temperature,
// relative_humidity_2m,weather_code,wind_speed_10m,is_day & daily=temperature_2m_max,
// temperature_2m_min,weather_code & forecast_days=2)
boolean ParseOpenMeteo (const char *pJSON, TWeatherData *pData);

// description courte en majuscules, sans accent (ecran LED)
const char *WeatherDescription (int nCode, boolean bDay);

// famille d'icone
enum TWeatherIcon
{
	IconClear, IconPartly, IconCloudy, IconFog, IconDrizzle, IconRain, IconSnow, IconStorm
};
TWeatherIcon WeatherIconFor (int nCode);

#endif
