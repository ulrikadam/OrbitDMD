//
// netutil.cpp
//
#include "netutil.h"

boolean ParseIPv4 (const char *pText, u8 *pAddress)
{
	for (unsigned i = 0; i < 4; i++)
	{
		unsigned nValue = 0, nDigits = 0;
		while (*pText >= '0' && *pText <= '9')
		{
			nValue = nValue * 10 + (unsigned) (*pText++ - '0');
			if (++nDigits > 3 || nValue > 255)
			{
				return FALSE;
			}
		}

		if (nDigits == 0)
		{
			return FALSE;
		}
		pAddress[i] = (u8) nValue;

		if (i < 3)
		{
			if (*pText++ != '.')
			{
				return FALSE;
			}
		}
	}

	return *pText == '\0';
}

static u32 ToU32 (const u8 *p)
{
	return ((u32) p[0] << 24) | ((u32) p[1] << 16) | ((u32) p[2] << 8) | p[3];
}

// adresse utilisable par un appareil (ni 0.x, ni 127.x, ni multicast ou au-dela)
static boolean IsUsableHost (u32 nAddress)
{
	u8 uchFirst = (u8) (nAddress >> 24);
	return uchFirst != 0 && uchFirst != 127 && uchFirst < 224;
}

const char *CheckStaticIP (const u8 *pIP, const u8 *pMask, const u8 *pGateway, const u8 *pDNS)
{
	u32 nIP = ToU32 (pIP), nMask = ToU32 (pMask), nGateway = ToU32 (pGateway), nDNS = ToU32 (pDNS);

	// masque : des 1 contigus, de /8 a /30
	u32 nInverted = ~nMask;
	if ((nInverted & (nInverted + 1)) != 0)
	{
		return "masque invalide";
	}
	unsigned nPrefix = 0;
	for (u32 m = nMask; m & 0x80000000U; m <<= 1)
	{
		nPrefix++;
	}
	if (nPrefix < 8 || nPrefix > 30)
	{
		return "masque hors des valeurs habituelles (/8 a /30)";
	}

	u32 nNetwork = nIP & nMask;
	u32 nBroadcast = nNetwork | nInverted;

	if (!IsUsableHost (nIP) || nIP == nNetwork || nIP == nBroadcast)
	{
		return "adresse IP invalide pour ce reseau";
	}

	if (   !IsUsableHost (nGateway)
	    || (nGateway & nMask) != nNetwork
	    || nGateway == nNetwork || nGateway == nBroadcast)
	{
		return "la passerelle doit etre dans le meme reseau que l'adresse IP";
	}

	if (nGateway == nIP)
	{
		return "la passerelle ne peut pas etre l'adresse du panneau";
	}

	if (!IsUsableHost (nDNS) || nDNS == nIP)
	{
		return "serveur DNS invalide";
	}

	return 0;
}
