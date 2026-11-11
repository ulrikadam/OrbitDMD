//
// netutil.h - adresses IPv4 : lecture et controle d'une configuration fixe
//
// Code sans dependance a Circle (teste sur le PC par dmd/test/hosttest).
//
#ifndef _netutil_h
#define _netutil_h

#include <circle/types.h>

// "192.168.0.151" -> 4 octets ; FALSE si le texte n'est pas une adresse IPv4
boolean ParseIPv4 (const char *pText, u8 *pAddress);

// controle une adresse fixe ; renvoie 0 si tout va bien, sinon la raison (en francais)
const char *CheckStaticIP (const u8 *pIP, const u8 *pMask, const u8 *pGateway, const u8 *pDNS);

#endif
