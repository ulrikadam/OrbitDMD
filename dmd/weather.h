//
// weather.h - meteo du lieu choisi, demandee a Open-Meteo.com (HTTP, sans cle)
//
// Une tache demande la meteo toutes les 15 minutes (2 minutes apres un echec),
// et tout de suite quand le lieu change ou que la page web le demande.
// Donnees : Open-Meteo.com, licence CC BY 4.0.
//
#ifndef _weather_h
#define _weather_h

#include <circle/sched/task.h>
#include <circle/net/netsubsystem.h>
#include <circle/types.h>
#include "config.h"
#include "weatherdata.h"

class CWeather : public CTask
{
public:
	CWeather (CNetSubSystem *pNetSubSystem, CConfig *pConfig);

	void Run (void);

	// donnees de moins de 3 heures pour le lieu actuel ; FALSE sinon
	boolean GetData (TWeatherData *pData) const;
	int GetAge (void) const;			// s depuis la derniere mise a jour, -1 si aucune
	const char *GetError (void) const	{ return m_Error; }	// "" si tout va bien

	void RequestUpdate (void)		{ m_bUpdateRequested = TRUE; }

	static CWeather *Get (void)		{ return s_pThis; }

private:
	boolean Fetch (const char *pLatitude, const char *pLongitude);

private:
	CNetSubSystem *m_pNet;
	CConfig *m_pConfig;

	TWeatherData m_Data;
	boolean m_bHasData;
	unsigned m_nUpdated;			// temps de fonctionnement (s)
	char m_Error[64];
	char m_Latitude[16];			// lieu des donnees
	char m_Longitude[16];
	volatile boolean m_bUpdateRequested;
	u8 *m_pBuffer;

	static CWeather *s_pThis;
};

#endif
