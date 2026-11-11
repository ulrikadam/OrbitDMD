//
// webserver.cpp
//
#include "webserver.h"
#include "common.h"
#include "clock.h"
#include "fileutil.h"
#include "gifmanager.h"
#include "settings.h"
#include "webfiles.h"		// s_WebFiles, genere depuis web/ par outils/web2h.py
#include "latency.h"
#include "clockrender.h"
#include "weather.h"
#include "matrix.h"
#include "network.h"
#include <circle/net/netsubsystem.h>
#include <circle/sched/scheduler.h>
#include <circle/cputhrottle.h>
#include <circle/memory.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/util.h>
#include <fatfs/ff.h>

static const char From[] = "web";

#define MAX_CONTENT_SIZE	(4 * 1024 * 1024)	// assez pour servir une GIF en apercu
#define MAX_UPLOAD_SIZE		(CRemoteControl::MaxDataSize + 65536)
#define TIMEOUT_SECONDS		20	// sans nouvelles du navigateur : la place est liberee

// Circle n'a pas de champ pour les en-tetes en plus : on les ajoute apres le type
// (le serveur ecrit "Content-Type: <type>\r\n", voir aussi outils/web2h.py)
#define GIF_TYPE_CACHED		"image/gif\r\nCache-Control: private, max-age=86400"

#define FREE_SPACE_SECONDS	30	// la place libre sur la carte est recalculee au plus toutes les 30 s

static unsigned s_nFreeMB = 0;
static unsigned s_nFreeUptime = 0;
static boolean s_bFreeValid = FALSE;

#define MIN_KERNEL_SIZE		(64 * 1024)
#define MAX_CONFIG_SIZE		16384

// copie d'un texte en echappant les caracteres speciaux (HTML ou JSON)
static void AppendEscaped (CString *pString, const char *pText, boolean bJSON)
{
	for (; *pText != '\0'; pText++)
	{
		char c = *pText;
		if (bJSON)
		{
			if (c == '"' || c == '\\')
			{
				pString->Append ('\\');
				pString->Append (c);
			}
			else if ((unsigned char) c >= ' ')
			{
				pString->Append (c);
			}
		}
		else
		{
			switch (c)
			{
			case '<':	pString->Append ("&lt;");	break;
			case '>':	pString->Append ("&gt;");	break;
			case '&':	pString->Append ("&amp;");	break;
			case '"':	pString->Append ("&quot;");	break;
			default:	pString->Append (c);		break;
			}
		}
	}
}

static boolean ContainsZero (const u8 *pData, unsigned nLength)
{
	for (unsigned i = 0; i < nLength; i++)
	{
		if (pData[i] == 0)
		{
			return TRUE;
		}
	}

	return FALSE;
}

// la valeur d'une cle "Cle=" est-elle presente et non vide dans le texte ?
static boolean HasNonEmptyKey (const u8 *pData, unsigned nLength, const char *pKey)
{
	size_t nKeyLength = strlen (pKey);
	unsigned i = 0;
	while (i < nLength)
	{
		// debut de ligne
		if (   i + nKeyLength < nLength
		    && memcmp (pData + i, pKey, nKeyLength) == 0
		    && pData[i + nKeyLength] != '\r'
		    && pData[i + nKeyLength] != '\n')
		{
			return TRUE;
		}

		while (i < nLength && pData[i] != '\n')
		{
			i++;
		}
		i++;
	}

	return FALSE;
}

// copie un champ texte de formulaire ; vide si trop long
static void CopyField (char *pDest, unsigned nSize, const u8 *pData, unsigned nLength)
{
	if (nLength >= nSize)
	{
		nLength = 0;
	}

	memcpy (pDest, pData, nLength);
	pDest[nLength] = '\0';
}

static int HexValue (char c)
{
	if ('0' <= c && c <= '9')	return c - '0';
	if ('a' <= c && c <= 'f')	return c - 'a' + 10;
	if ('A' <= c && c <= 'F')	return c - 'A' + 10;
	return -1;
}

CWebServer::CWebServer (CNetSubSystem *pNetSubSystem, CConfig *pConfig, CSocket *pSocket)
:	CHTTPDaemon (pNetSubSystem, pSocket, MAX_CONTENT_SIZE, 80, MAX_UPLOAD_SIZE, TIMEOUT_SECONDS),
	m_pConfig (pConfig)
{
}

CWebServer::~CWebServer (void)
{
}

CHTTPDaemon *CWebServer::CreateWorker (CNetSubSystem *pNetSubSystem, CSocket *pSocket)
{
	return new CWebServer (pNetSubSystem, m_pConfig, pSocket);
}

void CWebServer::WriteAccessLog (const CIPAddress &rRemoteIP, THTTPRequestMethod RequestMethod,
				 const char *pRequestURI, THTTPStatus Status, unsigned nContentLength)
{
	if (Status == HTTPOK && RequestMethod == HTTPRequestMethodGet)
	{
		return;			// visite normale : le journal reste lisible
	}

	CHTTPDaemon::WriteAccessLog (rRemoteIP, RequestMethod, pRequestURI, Status, nContentLength);
}

THTTPStatus CWebServer::GetContent (const char  *pPath,
				    const char  *pParams,
				    const char  *pFormData,
				    u8	        *pBuffer,
				    unsigned    *pLength,
				    const char **ppContentType)
{
	CString Content;
	*ppContentType = "text/plain; charset=utf-8";

	CRemoteControl::Get ()->NoteWebRequest ();	// le panneau est joignable

	// donnees en lecture (sans mot de passe)
	if (strcmp (pPath, "/journal") == 0)
	{
		// le journal est copie directement (16 Ko maximum)
		int nRead = CLogger::Get ()->Read (pBuffer, *pLength, FALSE);
		*pLength = nRead > 0 ? (unsigned) nRead : 0;
		return HTTPOK;
	}
	else if (strcmp (pPath, "/api/etat") == 0)
	{
		BuildStatusJSON (&Content);
		*ppContentType = "application/json";
	}
	else if (strcmp (pPath, "/api/gif") == 0)
	{
		CGifManager::ListCategories (m_pConfig, &Content);
		*ppContentType = "application/json";
	}
	else if (strcmp (pPath, "/api/gif/liste") == 0)
	{
		char Category[64];
		if (!GetParam (pParams, "c", Category, sizeof Category))
		{
			return HTTPBadRequest;
		}
		CGifManager::ListFiles (Category, &Content);
		*ppContentType = "application/json";
	}
	else if (strcmp (pPath, "/gif/voir") == 0)
	{
		char Category[64], Name[128];
		if (   !GetParam (pParams, "c", Category, sizeof Category)
		    || !GetParam (pParams, "f", Name, sizeof Name)
		    || !CGifManager::ReadGif (Category, Name, pBuffer, pLength))
		{
			return HTTPNotFound;
		}
		*ppContentType = GIF_TYPE_CACHED;	// le navigateur garde les apercus un jour
		return HTTPOK;
	}
	// page Horloge : polices, motifs et fonds de SD:/fonts, apercu dessine par le panneau
	else if (strcmp (pPath, "/api/horloge/fichiers") == 0)
	{
		ListClockAssets (&Content);
		*ppContentType = "application/json";
	}
	else if (strcmp (pPath, "/fonts/voir") == 0)
	{
		char Name[CLOCK_NAME_LEN];
		if (   !GetParam (pParams, "f", Name, sizeof Name)
		    || !IsValidAssetName (Name, "png,gif"))
		{
			return HTTPNotFound;
		}
		CString Path;
		Path.Format ("%s/%s", CLOCK_FONT_DIR, Name);
		if (!ReadWholeFile (Path, pBuffer, pLength))
		{
			return HTTPNotFound;
		}
		*ppContentType = (*pLength >= 4 && memcmp (pBuffer, "GIF8", 4) == 0)
				 ? GIF_TYPE_CACHED : "image/png\r\nCache-Control: private, max-age=86400";
		return HTTPOK;
	}
	else if (strcmp (pPath, "/api/ecran") == 0)
	{
		return GetScreen (pBuffer, pLength, ppContentType);
	}
	else if (strcmp (pPath, "/api/meteo") == 0)
	{
		BuildWeatherJSON (&Content);
		*ppContentType = "application/json";
	}
	else if (strcmp (pPath, "/api/horloge/apercu") == 0)
	{
		if (!RenderClockPreview (pParams, pBuffer, pLength))
		{
			return HTTPInternalServerError;
		}
		*ppContentType = "image/bmp\r\nCache-Control: no-store";
		return HTTPOK;
	}
	// demandes protegees par le mot de passe
	else if (   strcmp (pPath, "/maj") == 0
		 || strcmp (pPath, "/config") == 0
		 || strcmp (pPath, "/redemarrer") == 0
		 || strncmp (pPath, "/api/", 5) == 0)
	{
		TForm *pForm = new TForm;	// trop gros pour la pile d'une tache
		if (pForm == 0)
		{
			return HTTPInternalServerError;
		}
		CollectForm (pForm);

		// GET /api/reglages (sans formulaire) : lecture des reglages, sans mot de passe
		if (   strcmp (pPath, "/api/reglages") == 0
		    && pForm->pPassword == 0 && pForm->nFields == 0)
		{
			delete pForm;
			CSettings::BuildJSON (m_pConfig, &Content);
			*ppContentType = "application/json";
		}
		else if (!CheckPassword (pForm->pPassword, pForm->nPasswordLength))
		{
			delete pForm;
			CLogger::Get ()->Write (From, LogWarning, "Mot de passe refuse (%s)", pPath);
			CScheduler::Get ()->Sleep (2);		// freine les essais en serie
			Content = "ERREUR : mot de passe incorrect (ou Web_Password vide dans config.txt)\n";
		}
		else
		{
			THTTPStatus Status = HandlePost (pPath, *pForm, pBuffer, pLength, &Content);
			delete pForm;
			s_bFreeValid = FALSE;		// une GIF a pu etre ajoutee ou supprimee
			if (Status != HTTPOK || Content.GetLength () == 0)
			{
				return Status;		// reponse deja dans pBuffer, ou erreur
			}
		}
	}
	else
	{
		// pages, style, scripts et images de l'interface (integres au programme)
		for (const TWebFile *pFile = s_WebFiles; pFile->pPath != 0; pFile++)
		{
			if (strcmp (pPath, pFile->pPath) == 0)
			{
				if (pFile->nLength > *pLength)
				{
					return HTTPInternalServerError;
				}

				memcpy (pBuffer, pFile->pData, pFile->nLength);
				*pLength = pFile->nLength;
				*ppContentType = pFile->pType;

				return HTTPOK;
			}
		}

		return HTTPNotFound;
	}

	unsigned nLength = Content.GetLength ();
	if (nLength > *pLength)
	{
		return HTTPInternalServerError;
	}

	memcpy (pBuffer, (const char *) Content, nLength);
	*pLength = nLength;

	return HTTPOK;
}

void CWebServer::CollectForm (TForm *pForm)
{
	memset (pForm, 0, sizeof *pForm);

	const char *pHeader;
	const u8 *pData;
	unsigned nLength;
	while (GetMultipartFormPart (&pHeader, &pData, &nLength))
	{
		if (strstr (pHeader, "; name=\"password\"") != 0)
		{
			pForm->pPassword = pData;
			pForm->nPasswordLength = nLength;
		}
		else if (strstr (pHeader, "; name=\"fichier\"") != 0)
		{
			pForm->pFile = pData;
			pForm->nFileLength = nLength;

			// nom d'origine du fichier : filename="..."
			const char *pName = strstr (pHeader, "filename=\"");
			if (pName != 0)
			{
				pName += 10;
				unsigned i = 0;
				while (   pName[i] != '"' && pName[i] != '\0'
				       && i < sizeof pForm->FileName - 1)
				{
					pForm->FileName[i] = pName[i];
					i++;
				}
				pForm->FileName[i] = '\0';
			}
		}
		else if (strstr (pHeader, "; name=\"categorie\"") != 0)
		{
			CopyField (pForm->Category, sizeof pForm->Category, pData, nLength);
		}
		else if (strstr (pHeader, "; name=\"nom\"") != 0)
		{
			CopyField (pForm->Name, sizeof pForm->Name, pData, nLength);
		}
		else if (strstr (pHeader, "; name=\"actives\"") != 0)
		{
			CopyField (pForm->Active, sizeof pForm->Active, pData, nLength);
		}
		else if (pForm->nFields < MaxFields)
		{
			// autre champ texte (reglages) : nom="..." et valeur
			const char *pName = strstr (pHeader, "; name=\"");
			if (pName == 0)
			{
				continue;
			}
			pName += 8;

			unsigned i = 0;
			char *pDest = pForm->FieldName[pForm->nFields];
			while (pName[i] != '"' && pName[i] != '\0' && i < sizeof pForm->FieldName[0] - 1)
			{
				pDest[i] = pName[i];
				i++;
			}
			pDest[i] = '\0';

			if (pName[i] == '"' && nLength < sizeof pForm->FieldValue[0])
			{
				CopyField (pForm->FieldValue[pForm->nFields], sizeof pForm->FieldValue[0],
					   pData, nLength);
				pForm->nFields++;
			}
		}
	}
}

THTTPStatus CWebServer::HandlePost (const char *pPath, const TForm &rForm,
				    u8 *pBuffer, unsigned *pLength, CString *pReply)
{
	if (strcmp (pPath, "/api/verifier") == 0)
	{
		*pReply = "OK : mot de passe accepte\n";
	}
	else if (strcmp (pPath, "/api/reglages") == 0)
	{
		const char *Names[MaxFields];
		const char *Values[MaxFields];
		for (unsigned i = 0; i < rForm.nFields; i++)
		{
			Names[i] = rForm.FieldName[i];
			Values[i] = rForm.FieldValue[i];
		}
		CSettings::Apply (m_pConfig, Names, Values, rForm.nFields, pReply);
	}
	else if (strcmp (pPath, "/api/config/lire") == 0)
	{
		if (!ReadWholeFile (CONFIG_FILE, pBuffer, pLength))
		{
			*pReply = "ERREUR : lecture de config.txt impossible\n";
			return HTTPOK;
		}
		*pReply = "";
		return HTTPOK;			// contenu deja dans pBuffer
	}
	else if (strcmp (pPath, "/api/gif/ajouter") == 0)
	{
		if (rForm.pFile == 0 || rForm.nFileLength == 0)
		{
			*pReply = "ERREUR : aucun fichier recu\n";
		}
		else
		{
			char Name[CGifManager::MaxNameLength + 1];
			CGifManager::SanitizeFileName (rForm.FileName, Name, sizeof Name);
			CGifManager::AddGif (rForm.Category, Name, rForm.pFile, rForm.nFileLength, pReply);
		}
	}
	else if (strcmp (pPath, "/api/gif/supprimer") == 0)
	{
		CGifManager::DeleteGif (m_pConfig, rForm.Category, rForm.Name, pReply);
	}
	else if (strcmp (pPath, "/api/gif/jouer") == 0)
	{
		CGifManager::PlayGif (rForm.Category, rForm.Name, pReply);
	}
	else if (strcmp (pPath, "/api/gif/categories") == 0)
	{
		CGifManager::SetActiveCategories (m_pConfig, rForm.Active, pReply);
	}
	else if (strcmp (pPath, "/api/gif/categorie/supprimer") == 0)
	{
		CGifManager::DeleteCategory (m_pConfig, rForm.Category, pReply);
	}
	else if (strcmp (pPath, "/api/heure/synchroniser") == 0)
	{
		int nDiff = 0;
		CNetwork *pNetwork = CNetwork::Get ();
		if (pNetwork != 0 && pNetwork->SyncTime (&nDiff))
		{
			pReply->Format ("OK : ecart=%d\n", nDiff);
		}
		else
		{
			*pReply = "ERREUR : serveur d'heure Internet injoignable, reessayez\n";
		}
	}
	else if (strcmp (pPath, "/api/meteo/actualiser") == 0)
	{
		CWeather *pWeather = CWeather::Get ();
		if (pWeather == 0)
		{
			*pReply = "ERREUR : Wi-Fi pas encore connecte\n";
		}
		else
		{
			pWeather->RequestUpdate ();
			*pReply = "OK : meteo demandee\n";
		}
	}
	else if (strcmp (pPath, "/api/meteo/montrer") == 0)
	{
		CRemoteControl::Get ()->RequestPlay (WEATHER_PLAY_REQUEST);
		*pReply = "OK : la meteo s'affiche sur le panneau\n";
	}
	else if (strcmp (pPath, "/api/horloge/montrer") == 0)
	{
		CRemoteControl::Get ()->RequestPlay (CLOCK_PLAY_REQUEST);
		*pReply = "OK : l'heure s'affiche sur le panneau\n";
	}
	else if (strcmp (pPath, "/api/horloge/ajouter") == 0)
	{
		const char *pType = "";
		for (unsigned i = 0; i < rForm.nFields; i++)
		{
			if (strcmp (rForm.FieldName[i], "type") == 0)
			{
				pType = rForm.FieldValue[i];
			}
		}
		AddClockAsset (pType, rForm.FileName, rForm.pFile, rForm.nFileLength, pReply);
	}
	else if (strcmp (pPath, "/api/commande") == 0)
	{
		const char *pAction = "";
		for (unsigned i = 0; i < rForm.nFields; i++)
		{
			if (strcmp (rForm.FieldName[i], "action") == 0)
			{
				pAction = rForm.FieldValue[i];
			}
		}
		HandleCommand (pAction, pReply);
	}
	else
	{
		// demandes traitees par la boucle d'affichage, suivies d'un redemarrage
		TRemoteRequest Request;
		const u8 *pFile = rForm.pFile;
		unsigned nFileLength = rForm.nFileLength;

		if (strcmp (pPath, "/maj") == 0)
		{
			if (pFile == 0 || nFileLength < MIN_KERNEL_SIZE
			    || nFileLength > CRemoteControl::MaxDataSize)
			{
				*pReply = "ERREUR : fichier kernel8.img absent ou de taille anormale\n";
				return HTTPOK;
			}
			Request = RemoteKernel;
		}
		else if (strcmp (pPath, "/config") == 0)
		{
			if (pFile == 0 || nFileLength == 0 || nFileLength > MAX_CONFIG_SIZE
			    || ContainsZero (pFile, nFileLength))
			{
				*pReply = "ERREUR : fichier config.txt absent ou invalide\n";
				return HTTPOK;
			}
			// sans ces deux reglages, le panneau ne serait plus joignable a distance
			if (   !HasNonEmptyKey (pFile, nFileLength, "Web_Password=")
			    || !HasNonEmptyKey (pFile, nFileLength, "Wifi_ssid="))
			{
				*pReply = "ERREUR : config.txt doit contenir Wifi_ssid et Web_Password\n";
				return HTTPOK;
			}
			Request = RemoteConfig;
		}
		else if (strcmp (pPath, "/redemarrer") == 0)
		{
			pFile = 0;
			nFileLength = 0;
			Request = RemoteReboot;
		}
		else
		{
			return HTTPNotFound;
		}

		if (!CRemoteControl::Get ()->Submit (Request, pFile, nFileLength))
		{
			*pReply = "ERREUR : une autre demande est deja en cours\n";
			return HTTPOK;
		}

		CLogger::Get ()->Write (From, LogNotice, "Demande recue (%u octets)", nFileLength);
		*pReply = "OK : demande recue, le panneau va redemarrer\n";
	}

	return HTTPOK;
}

// --- page Meteo ---

static void AppendTenths (CString *pJSON, const char *pName, int nTenths)
{
	CString Part;
	Part.Format (",\"%s\":%s%d.%d", pName, nTenths < 0 ? "-" : "",
		     (nTenths < 0 ? -nTenths : nTenths) / 10, (nTenths < 0 ? -nTenths : nTenths) % 10);
	pJSON->Append (Part);
}

void CWebServer::BuildWeatherJSON (CString *pJSON)
{
	const TWeatherStyle &Style = m_pConfig->WeatherStyle;
	CWeather *pWeather = CWeather::Get ();
	TWeatherData Data;
	boolean bValid = pWeather != 0 && pWeather->GetData (&Data);

	CString Part;
	Part.Format ("{\"lieu_choisi\":%s,\"valide\":%s,\"maj_s\":%d,\"erreur\":\"",
		     Style.Latitude[0] != '\0' ? "true" : "false", bValid ? "true" : "false",
		     pWeather != 0 ? pWeather->GetAge () : -1);
	pJSON->Append (Part);
	AppendEscaped (pJSON, pWeather != 0 ? pWeather->GetError () : "", TRUE);
	pJSON->Append ("\"");

	if (bValid)
	{
		AppendTenths (pJSON, "temp", Data.nTemp10);
		AppendTenths (pJSON, "ressenti", Data.nFeels10);
		AppendTenths (pJSON, "vent", Data.nWind10);
		AppendTenths (pJSON, "min", Data.nMin10);
		AppendTenths (pJSON, "max", Data.nMax10);
		Part.Format (",\"humidite\":%d,\"code\":%d,\"jour\":%s,\"description\":\"%s\"",
			     Data.nHumidity, Data.nCode, Data.bDay ? "true" : "false",
			     WeatherDescription (Data.nCode, Data.bDay));
		pJSON->Append (Part);
		if (Data.nTomorrowCode >= 0)
		{
			Part.Format (",\"demain_code\":%d,\"demain_description\":\"%s\"", Data.nTomorrowCode,
				     WeatherDescription (Data.nTomorrowCode, TRUE));
			pJSON->Append (Part);
			AppendTenths (pJSON, "demain_min", Data.nTomorrowMin10);
			AppendTenths (pJSON, "demain_max", Data.nTomorrowMax10);
		}
	}

	pJSON->Append ("}\n");
}

// --- page Horloge ---

// fichiers de SD:/fonts (et d'un niveau de sous-dossiers) : polices TTF/OTF, motifs et
// fonds PNG/GIF (un fond est une image dont le nom commence par "Background")
void CWebServer::ListClockAssets (CString *pJSON)
{
	CString Lists[3];		// polices, motifs, fonds
	unsigned nCount[3] = {0, 0, 0};

	for (unsigned nPass = 0; nPass < 2; nPass++)
	{
		DIR Top;
		FILINFO TopInfo;
		if (f_opendir (&Top, CLOCK_FONT_DIR) != FR_OK)
		{
			break;
		}

		while (f_readdir (&Top, &TopInfo) == FR_OK && TopInfo.fname[0] != '\0')
		{
			YieldToNetwork ();
			if (TopInfo.fattrib & (AM_HID | AM_SYS))
			{
				continue;
			}

			// passe 0 : fichiers du dossier ; passe 1 : fichiers des sous-dossiers
			CString Folder;
			if (nPass == 0)
			{
				if (TopInfo.fattrib & AM_DIR)
				{
					continue;
				}
			}
			else
			{
				if (!(TopInfo.fattrib & AM_DIR))
				{
					continue;
				}
				Folder = TopInfo.fname;
			}

			DIR Sub;
			FILINFO Info;
			boolean bSub = nPass == 1;
			if (bSub)
			{
				CString Path;
				Path.Format ("%s/%s", CLOCK_FONT_DIR, TopInfo.fname);
				if (f_opendir (&Sub, Path) != FR_OK)
				{
					continue;
				}
			}

			while (TRUE)
			{
				const char *pFile;
				if (bSub)
				{
					if (f_readdir (&Sub, &Info) != FR_OK || Info.fname[0] == '\0')
					{
						break;
					}
					if (Info.fattrib & (AM_DIR | AM_HID | AM_SYS))
					{
						continue;
					}
					pFile = Info.fname;
				}
				else
				{
					pFile = TopInfo.fname;
				}

				CString Name;
				if (bSub)
				{
					Name.Format ("%s/%s", (const char *) Folder, pFile);
				}
				else
				{
					Name = pFile;
				}

				int nList = -1;
				if (IsValidAssetName (Name, "ttf,otf"))
				{
					nList = 0;
				}
				else if (IsValidAssetName (Name, "png,gif"))
				{
					nList = strncasecmp (pFile, "background", 10) == 0 ? 2 : 1;
				}

				if (nList >= 0)
				{
					Lists[nList].Append (nCount[nList]++ > 0 ? ",\"" : "\"");
					AppendEscaped (&Lists[nList], Name, TRUE);
					Lists[nList].Append ("\"");
				}

				if (!bSub)
				{
					break;
				}
			}

			if (bSub)
			{
				f_closedir (&Sub);
			}
		}

		f_closedir (&Top);
	}

	pJSON->Append ("{\"polices\":[");
	pJSON->Append (Lists[0]);
	pJSON->Append ("],\"motifs\":[");
	pJSON->Append (Lists[1]);
	pJSON->Append ("],\"fonds\":[");
	pJSON->Append (Lists[2]);
	pJSON->Append ("]}\n");
}

// apercu de l'heure avec les reglages de la page (pas encore enregistres), en image BMP
boolean CWebServer::RenderClockPreview (const char *pParams, u8 *pBuffer, unsigned *pLength)
{
	static const char *Keys[] =
	{
		"Clock_Format", "Clock_Font", "Clock_Font_Size", "Clock_Color", "Clock_Font_Pattern",
		"Clock_Background", "Clock_Blink", "Clock_Smooth", "Date_Active", "Date_Format",
		"Date_Font", "Date_Font_Size", "Date_Color", "Date_Font_Pattern"
	};
	static const char *WeatherKeys[] =
	{
		"Weather_Font", "Weather_Font_Size", "Weather_Font_Pattern", "Weather_Text_color",
		"Weather_Info", "Weather_Info_Color", "Weather_Icon", "Weather_Animate"
	};
	static CClockRenderer *s_pRenderer = 0;		// un seul, garde (polices en memoire)
	static CCanvas *s_pCanvas = 0;
	static boolean s_bBusy = FALSE;

	unsigned W = m_pConfig->PanelCols * m_pConfig->Chain;
	unsigned H = m_pConfig->PanelRows * m_pConfig->Parallel;
	unsigned nRowSize = (W * 3 + 3) & ~3;
	if (W == 0 || H == 0 || W > 512 || H > 256 || 54 + nRowSize * H > *pLength)
	{
		return FALSE;
	}

	TClockStyle Style = m_pConfig->ClockStyle;
	for (unsigned i = 0; i < sizeof Keys / sizeof Keys[0]; i++)
	{
		char Value[CLOCK_NAME_LEN];
		if (GetParam (pParams, Keys[i], Value, sizeof Value))
		{
			SetClockStyleValue (&Style, Keys[i], Value);
		}
	}
	char Screen[8];
	boolean bScreen = GetParam (pParams, "ecran", Screen, sizeof Screen);
	boolean bDateScreen = bScreen && strcmp (Screen, "date") == 0;
	boolean bWeatherScreen = bScreen && strcmp (Screen, "meteo") == 0;

	// meteo : reglages de la page, et meteo actuelle (ou exemple : &code=61&jour=0)
	TWeatherStyle WeatherStyle = m_pConfig->WeatherStyle;
	TWeatherData Weather;
	if (bWeatherScreen)
	{
		for (unsigned i = 0; i < sizeof WeatherKeys / sizeof WeatherKeys[0]; i++)
		{
			char Value[CLOCK_NAME_LEN];
			if (GetParam (pParams, WeatherKeys[i], Value, sizeof Value))
			{
				SetWeatherStyleValue (&WeatherStyle, WeatherKeys[i], Value);
			}
		}

		char Code[8] = "";
		CWeather *pWeather = CWeather::Get ();
		if (   GetParam (pParams, "code", Code, sizeof Code)
		    || pWeather == 0 || !pWeather->GetData (&Weather))
		{
			memset (&Weather, 0, sizeof Weather);
			Weather.bValid = TRUE;
			Weather.nCode = Code[0] != '\0' ? atoi (Code) : 2;	// pas de meteo : exemple
			Weather.nTemp10 = 70; Weather.nFeels10 = 40; Weather.nHumidity = 85;
			Weather.nWind10 = 150; Weather.nMin10 = 30; Weather.nMax10 = 110;
			Weather.nTomorrowCode = 2; Weather.nTomorrowMin10 = 40; Weather.nTomorrowMax10 = 120;
			char Day[4];
			Weather.bDay = !(GetParam (pParams, "jour", Day, sizeof Day) && Day[0] == '0');
		}
	}

	TDateTime Time;
	if (!CClock::Get ()->GetLocalTime (&Time))
	{
		Time.Year = 2026; Time.Month = 12; Time.Day = 24;	// heure inconnue : exemple
		Time.Hour = 12; Time.Minute = 34; Time.Second = 56;
	}

	// une seule image a la fois (les chargements rendent la main aux autres requetes)
	while (s_bBusy)
	{
		CScheduler::Get ()->Yield ();
	}
	s_bBusy = TRUE;

	if (s_pRenderer == 0)
	{
		s_pRenderer = new CClockRenderer (W, H);
		s_pCanvas = new CCanvas (W, H);
	}
	unsigned nMillis = (unsigned) (CTimer::GetClockTicks64 () / 1000);
	if (bWeatherScreen)
	{
		s_pRenderer->PrepareWeather (WeatherStyle);
		s_pRenderer->RenderWeather (WeatherStyle, Weather, nMillis, s_pCanvas);
	}
	else
	{
		s_pRenderer->Prepare (Style);
		s_pRenderer->Render (Style, Time, nMillis, TRUE, bDateScreen, s_pCanvas);
	}

	EncodeBMP (s_pCanvas->GetData (), W, H, pBuffer, pLength);

	s_bBusy = FALSE;

	return TRUE;
}

// image RGB888 -> fichier BMP 24 bits (lignes de bas en haut, en BGR) ; FALSE si trop grande
boolean CWebServer::EncodeBMP (const u8 *pRGB, unsigned W, unsigned H, u8 *pBuffer, unsigned *pLength)
{
	unsigned nRowSize = (W * 3 + 3) & ~3;
	unsigned nFileSize = 54 + nRowSize * H;
	if (pRGB == 0 || W == 0 || H == 0 || nFileSize > *pLength)
	{
		return FALSE;
	}

	u8 *p = pBuffer;
	memset (p, 0, 54);
	p[0] = 'B'; p[1] = 'M';
	p[2] = nFileSize & 0xFF; p[3] = (nFileSize >> 8) & 0xFF; p[4] = (nFileSize >> 16) & 0xFF;
	p[10] = 54; p[14] = 40;
	p[18] = W & 0xFF; p[19] = (W >> 8) & 0xFF;
	p[22] = H & 0xFF; p[23] = (H >> 8) & 0xFF;
	p[26] = 1; p[28] = 24;
	for (unsigned y = 0; y < H; y++)
	{
		u8 *pRow = p + 54 + (H - 1 - y) * nRowSize;
		memset (pRow, 0, nRowSize);
		for (unsigned x = 0; x < W; x++)
		{
			const u8 *pPixel = pRGB + (y * W + x) * 3;
			pRow[x * 3 + 0] = pPixel[2];
			pRow[x * 3 + 1] = pPixel[1];
			pRow[x * 3 + 2] = pPixel[0];
		}
	}
	*pLength = nFileSize;

	return TRUE;
}

// ce que montre le panneau : le nom de la GIF en cours (le navigateur l'anime lui-meme),
// ou une copie de l'image affichee (heure, meteo, message, pause...) ; type dans "X-Ecran"
THTTPStatus CWebServer::GetScreen (u8 *pBuffer, unsigned *pLength, const char **ppContentType)
{
	static const char *Types[][2] =
	{
		{"heure",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: heure"},
		{"date",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: date"},
		{"meteo",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: meteo"},
		{"message",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: message"},
		{"demarrage",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: demarrage"},
		{"mire",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: mire"},
		{"pause",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: pause"},
		{"eteint",	"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: eteint"},
		{"gif",		"image/bmp\r\nCache-Control: no-store\r\nX-Ecran: autre"}
	};

	CRemoteControl *pRemote = CRemoteControl::Get ();
	const char *pScreen = pRemote->IsDisplayOff () ? "eteint"
			    : pRemote->IsPaused () ? "pause" : pRemote->Status.pScreen;

	if (strcmp (pScreen, "gif") == 0)
	{
		const char *pGif = pRemote->Status.CurrentGif;
		unsigned nLength = strlen (pGif);
		if (nLength > *pLength)
		{
			return HTTPInternalServerError;
		}
		memcpy (pBuffer, pGif, nLength);
		*pLength = nLength;
		*ppContentType = "text/plain; charset=utf-8\r\nCache-Control: no-store\r\nX-Ecran: gif";
		return HTTPOK;
	}

	CMatrix *pMatrix = CMatrix::Get ();
	if (   pMatrix == 0
	    || !EncodeBMP (pMatrix->GetLastFrame (), pMatrix->GetWidth (), pMatrix->GetHeight (),
			   pBuffer, pLength))
	{
		return HTTPInternalServerError;
	}

	*ppContentType = Types[sizeof Types / sizeof Types[0] - 1][1];
	for (unsigned i = 0; i < sizeof Types / sizeof Types[0]; i++)
	{
		if (strcmp (pScreen, Types[i][0]) == 0)
		{
			*ppContentType = Types[i][1];
			break;
		}
	}

	return HTTPOK;
}

// nouvelle police (TTF/OTF), nouveau motif ou nouveau fond (PNG/GIF) dans SD:/fonts
void CWebServer::AddClockAsset (const char *pType, const char *pFileName, const u8 *pData,
				unsigned nLength, CString *pReply)
{
	boolean bFont = strcmp (pType, "police") == 0;
	boolean bBackground = strcmp (pType, "fond") == 0;
	if (!bFont && !bBackground && strcmp (pType, "motif") != 0)
	{
		*pReply = "ERREUR : type de fichier inconnu\n";
		return;
	}

	if (pData == 0 || nLength < 16)
	{
		*pReply = "ERREUR : aucun fichier recu\n";
		return;
	}

	// verification du contenu
	boolean bTTF = memcmp (pData, "\x00\x01\x00\x00", 4) == 0 || memcmp (pData, "OTTO", 4) == 0
		       || memcmp (pData, "true", 4) == 0;
	boolean bPNG = memcmp (pData, "\x89PNG", 4) == 0;
	boolean bGIF = memcmp (pData, "GIF8", 4) == 0;
	if (bFont ? !bTTF : !(bPNG || bGIF))
	{
		pReply->Format ("ERREUR : %s n'est pas %s\n", pFileName,
				bFont ? "une police TTF ou OTF" : "une image PNG ou GIF");
		return;
	}

	// nom sur : lettres, chiffres, - et _ ; extension d'apres le contenu
	char Base[48];
	unsigned n = 0;
	for (const char *p = pFileName; *p != '\0' && n < sizeof Base - 1; p++)
	{
		char c = *p;
		if (c == '.' && strchr (p + 1, '.') == 0)
		{
			break;			// extension d'origine : remplacee
		}
		boolean bOK = ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')
			      || c == '-' || c == '_';
		Base[n++] = bOK ? c : '_';
	}
	Base[n] = '\0';
	if (n == 0)
	{
		strcpy (Base, bFont ? "police" : "image");
	}

	const char *pExtension = bFont ? (memcmp (pData, "OTTO", 4) == 0 ? "otf" : "ttf") : (bPNG ? "png" : "gif");
	const char *pPrefix = "";
	if (bBackground && strncasecmp (Base, "background", 10) != 0)
	{
		pPrefix = "Background_";		// les fonds se reconnaissent a leur nom
	}
	else if (!bFont && !bBackground && strncasecmp (Base, "background", 10) == 0)
	{
		pPrefix = "Motif_";
	}

	CString Name;
	Name.Format ("%s%s.%s", pPrefix, Base, pExtension);
	CString Path;
	Path.Format ("%s/%s", CLOCK_FONT_DIR, (const char *) Name);

	f_mkdir (CLOCK_FONT_DIR);
	FIL File;
	FRESULT Result = f_open (&File, Path, FA_WRITE | FA_CREATE_NEW);
	if (Result == FR_EXIST)
	{
		pReply->Format ("ERREUR : %s existe deja\n", (const char *) Name);
		return;
	}
	if (Result != FR_OK)
	{
		pReply->Format ("ERREUR : ecriture de %s impossible\n", (const char *) Name);
		return;
	}

	UINT nWritten = 0;
	Result = f_write (&File, pData, nLength, &nWritten);
	if (f_close (&File) != FR_OK || Result != FR_OK || nWritten != nLength)
	{
		f_unlink (Path);
		pReply->Format ("ERREUR : ecriture de %s incomplete (carte pleine ?)\n", (const char *) Name);
		return;
	}

	CLogger::Get ()->Write (From, LogNotice, "Ajout : %s (%u octets)", (const char *) Path, nLength);
	pReply->Format ("OK : %s ajoute\n", (const char *) Name);
}

// commandes rapides : image figee, affichage eteint (le redemarrage passe par /redemarrer)
void CWebServer::HandleCommand (const char *pAction, CString *pReply)
{
	CRemoteControl *pRemote = CRemoteControl::Get ();

	if (strcmp (pAction, "pause") == 0)
	{
		pRemote->SetPaused (TRUE);
		*pReply = "OK : image figee\n";
	}
	else if (strcmp (pAction, "reprendre") == 0)
	{
		pRemote->SetPaused (FALSE);
		*pReply = "OK : lecture reprise\n";
	}
	else if (strcmp (pAction, "eteindre") == 0)
	{
		pRemote->SetDisplayOff (TRUE);
		*pReply = "OK : affichage eteint, le panneau reste joignable\n";
	}
	else if (strcmp (pAction, "allumer") == 0)
	{
		pRemote->SetDisplayOff (FALSE);
		*pReply = "OK : affichage rallume\n";
	}
	else
	{
		*pReply = "ERREUR : commande inconnue\n";
		return;
	}

	CLogger::Get ()->Write (From, LogNotice, "Commande : %s", pAction);
}

boolean CWebServer::GetParam (const char *pParams, const char *pName, char *pValue, unsigned nSize)
{
	size_t nNameLength = strlen (pName);
	const char *p = pParams;

	while (p != 0 && *p != '\0')
	{
		if (strncmp (p, pName, nNameLength) == 0 && p[nNameLength] == '=')
		{
			// valeur trouvee : decodage %XX et +
			p += nNameLength + 1;
			unsigned n = 0;
			while (*p != '\0' && *p != '&')
			{
				char c = *p++;
				if (c == '+')
				{
					c = ' ';
				}
				else if (c == '%' && HexValue (p[0]) >= 0 && HexValue (p[1]) >= 0)
				{
					c = (char) (HexValue (p[0]) * 16 + HexValue (p[1]));
					p += 2;
				}

				if (n >= nSize - 1 || c == '\0')
				{
					return FALSE;
				}
				pValue[n++] = c;
			}
			pValue[n] = '\0';

			return n > 0;
		}

		p = strchr (p, '&');
		if (p != 0)
		{
			p++;
		}
	}

	return FALSE;
}

boolean CWebServer::CheckPassword (const u8 *pData, unsigned nLength) const
{
	size_t nExpected = strlen (m_pConfig->WebPassword);

	return    nExpected > 0
	       && pData != 0
	       && nLength == nExpected
	       && memcmp (pData, m_pConfig->WebPassword, nLength) == 0;
}

void CWebServer::BuildStatusJSON (CString *pJSON)
{
	CRemoteControl *pRemote = CRemoteControl::Get ();
	CClock *pClock = CClock::Get ();

	CString IP;
	CNetSubSystem::Get ()->GetConfig ()->GetIPAddress ()->Format (&IP);

	CString Time ("");
	TDateTime Local;
	if (pClock->GetLocalTime (&Local))
	{
		Time.Format ("%04u-%02u-%02u %02u:%02u:%02u", Local.Year, Local.Month, Local.Day,
			     Local.Hour, Local.Minute, Local.Second);
	}

	// place libre sur la carte SD (pas a chaque interrogation : la page la demande toutes les 3 s)
	unsigned nUptime = CTimer::Get ()->GetUptime ();
	if (!s_bFreeValid || nUptime - s_nFreeUptime >= FREE_SPACE_SECONDS)
	{
		DWORD nFreeClusters;
		FATFS *pFileSystem;
		if (f_getfree (DRIVE, &nFreeClusters, &pFileSystem) == FR_OK)
		{
			s_nFreeMB = (unsigned) ((u64) nFreeClusters * pFileSystem->csize * 512 / (1024 * 1024));
			s_nFreeUptime = nUptime;
			s_bFreeValid = TRUE;
		}
	}
	unsigned nFreeMB = s_nFreeMB;

	CCPUThrottle *pThrottle = CCPUThrottle::Get ();
	unsigned nTemperature = pThrottle != 0 ? pThrottle->GetTemperature () : 0;

	CString Part;
	Part.Format ("{\"version\":\"%s\",\"programme\":\"%s\",\"uptime_s\":%u,\"ip\":\"%s\",\"heure\":\"%s\","
		     "\"source_heure\":\"%s\",\"rafraichissement_hz\":%u,\"luminosite\":%u,"
		     "\"nb_gif\":%u,\"memoire_libre_mo\":%u,\"carte_libre_mo\":%u,\"temperature_c\":%u,"
		     "\"latence_ms\":%u,\"latence_max_ms\":%u,\"synchro_s\":%d,",
		     GetVersion (), GetBuildDate (), nUptime, (const char *) IP,
		     (const char *) Time, pClock->GetSourceName (),
		     pRemote->Status.nRefreshRate, pRemote->Status.nBrightness,
		     pRemote->Status.nGifCount,
		     (unsigned) (CMemorySystem::Get ()->GetHeapFreeSpace (HEAP_ANY) >> 20),
		     nFreeMB, nTemperature,
		     CLatencyProbe::GetRecentMs (), CLatencyProbe::GetMaxMs (),
		     pClock->GetSecondsSinceSync ());
	pJSON->Append (Part);

	// meteo (tableau de bord)
	CWeather *pWeather = CWeather::Get ();
	TWeatherData Weather;
	if (m_pConfig->WeatherStyle.bActive && pWeather != 0 && pWeather->GetData (&Weather))
	{
		Part.Format ("\"meteo_temp\":%d,\"meteo_code\":%d,\"meteo_jour\":%s,\"meteo_texte\":\"%s\",",
			     Weather.nTemp10, Weather.nCode, Weather.bDay ? "true" : "false",
			     WeatherDescription (Weather.nCode, Weather.bDay));
		pJSON->Append (Part);
	}

	pJSON->Append ("\"nom\":\"");
	AppendEscaped (pJSON, m_pConfig->NetworkName, TRUE);
	pJSON->Append ("\",\"wifi\":\"");
	AppendEscaped (pJSON, m_pConfig->WifiSSID, TRUE);
	pJSON->Append ("\",\"gif\":\"");
	AppendEscaped (pJSON, pRemote->Status.CurrentGif, TRUE);
	pJSON->Append (pRemote->IsPaused () ? "\",\"pause\":true" : "\",\"pause\":false");
	pJSON->Append (pRemote->IsDisplayOff () ? ",\"ecran_eteint\":true}\n" : ",\"ecran_eteint\":false}\n");
}
