//
// remote.cpp
//
#include "remote.h"
#include <circle/util.h>

CRemoteControl *CRemoteControl::s_pThis = 0;

CRemoteControl::CRemoteControl (void)
:	m_Pending (RemoteNone),
	m_pData (0),
	m_nSize (0),
	m_bReload (FALSE),
	m_bPlay (FALSE),
	m_bRebootPending (FALSE),
	m_bWebRequestSeen (FALSE),
	m_bPaused (FALSE),
	m_bDisplayOff (FALSE)
{
	Status.nGifCount = 0;
	Status.nRefreshRate = 0;
	Status.nBrightness = 0;
	Status.pScreen = "demarrage";

	s_pThis = this;
}

CRemoteControl::~CRemoteControl (void)
{
	delete [] m_pData;
	s_pThis = 0;
}

boolean CRemoteControl::Initialize (void)
{
	m_pData = new u8[MaxDataSize];

	return m_pData != 0;
}

boolean CRemoteControl::Submit (TRemoteRequest Request, const u8 *pData, unsigned nSize)
{
	if (   m_Pending != RemoteNone
	    || nSize > MaxDataSize
	    || m_pData == 0)
	{
		return FALSE;
	}

	if (nSize > 0)
	{
		memcpy (m_pData, pData, nSize);
	}
	m_nSize = nSize;

	m_Pending = Request;		// en dernier : la demande est complete

	return TRUE;
}

boolean CRemoteControl::TakeReloadRequest (void)
{
	boolean bReload = m_bReload;
	m_bReload = FALSE;

	return bReload;
}

void CRemoteControl::RequestPlay (const char *pPath)
{
	m_PlayPath = pPath;
	m_bPlay = TRUE;

	m_bPaused = FALSE;		// on veut la voir : fin de la pause, affichage rallume
	m_bDisplayOff = FALSE;
}

boolean CRemoteControl::TakePlayRequest (CString *pPath)
{
	if (!m_bPlay)
	{
		return FALSE;
	}

	*pPath = m_PlayPath;
	m_bPlay = FALSE;

	return TRUE;
}
