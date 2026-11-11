//
// localtime.h - date/heure et conversion UTC -> heure locale (fuseaux europeens)
//
#ifndef _localtime_h
#define _localtime_h

#include <circle/types.h>

struct TDateTime
{
	unsigned Year;		// 2000..2099
	unsigned Month;		// 1..12
	unsigned Day;		// 1..31
	unsigned Hour;		// 0..23
	unsigned Minute;
	unsigned Second;
};

// convertit une heure UTC en heure locale pour un fuseau du type "Europe/Paris"
// (heure d'ete europeenne : dernier dimanche de mars -> dernier dimanche d'octobre, 1 h UTC)
void UTCToLocal (const TDateTime &rUTC, const char *pTimeZone, TDateTime *pLocal);

// conversions avec le temps Unix (secondes depuis le 1er janvier 1970, UTC)
u32 DateTimeToUnix (const TDateTime &rTime);
void UnixToDateTime (u32 nTime, TDateTime *pTime);

#endif
