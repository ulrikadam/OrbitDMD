//
// webserver.h - page web du panneau
//
//   GET  /, /gif, /systeme    pages de l'interface (web/, integrees via webfiles.h)
//   GET  /journal             journal (texte)
//   GET  /api/etat            etat (JSON)
//   GET  /api/gif             categories (JSON)
//   GET  /api/gif/liste?c=X   GIF d'une categorie (JSON)
//   GET  /gif/voir?c=X&f=Y    fichier GIF (apercu)
//
//   POST (champ password obligatoire, reponses "OK ..." ou "ERREUR ...") :
//   /maj                     nouveau kernel8.img     (fichier)
//   /config                  nouveau config.txt      (fichier)
//   /redemarrer              redemarrage
//   /api/verifier            verifie le mot de passe
//   /api/config/lire         renvoie config.txt
//   /api/gif/ajouter         ajoute une GIF          (categorie, fichier)
//   /api/gif/supprimer       supprime une GIF        (categorie, nom)
//   /api/gif/jouer           affiche une GIF         (categorie, nom)
//   /api/gif/categories      categories actives      (actives = "A,B,C")
//   /api/reglages            modifie des reglages    (champs = cles de config.txt)
//
//   GET  /api/reglages        reglages actuels, sans les secrets (JSON)
//
#ifndef _webserver_h
#define _webserver_h

#include <circle/net/httpdaemon.h>
#include <circle/string.h>
#include "config.h"
#include "remote.h"

class CWebServer : public CHTTPDaemon
{
public:
	CWebServer (CNetSubSystem *pNetSubSystem, CConfig *pConfig, CSocket *pSocket = 0);
	~CWebServer (void);

	CHTTPDaemon *CreateWorker (CNetSubSystem *pNetSubSystem, CSocket *pSocket);

	THTTPStatus GetContent (const char  *pPath,
				const char  *pParams,
				const char  *pFormData,
				u8	    *pBuffer,
				unsigned    *pLength,
				const char **ppContentType);

	// journal : seulement les erreurs et les modifications (pas chaque visite)
	void WriteAccessLog (const CIPAddress	&rRemoteIP,
			     THTTPRequestMethod	 RequestMethod,
			     const char		*pRequestURI,
			     THTTPStatus	 Status,
			     unsigned		 nContentLength);

private:
	static const unsigned MaxFields = 48;

	struct TForm
	{
		const u8 *pPassword;
		unsigned  nPasswordLength;
		const u8 *pFile;
		unsigned  nFileLength;
		char	  FileName[128];
		char	  Category[64];
		char	  Name[128];
		char	  Active[1024];

		// tous les champs texte (pour /api/reglages)
		unsigned  nFields;
		char	  FieldName[MaxFields][40];
		char	  FieldValue[MaxFields][256];	// texte defilant : 200 octets
	};

	void CollectForm (TForm *pForm);
	THTTPStatus HandlePost (const char *pPath, const TForm &rForm,
				u8 *pBuffer, unsigned *pLength, CString *pReply);

	void BuildStatusJSON (CString *pJSON);
	boolean CheckPassword (const u8 *pData, unsigned nLength) const;

	static void HandleCommand (const char *pAction, CString *pReply);

	// page Meteo
	void BuildWeatherJSON (CString *pJSON);

	// apercu du tableau de bord
	static THTTPStatus GetScreen (u8 *pBuffer, unsigned *pLength, const char **ppContentType);
	static boolean EncodeBMP (const u8 *pRGB, unsigned W, unsigned H, u8 *pBuffer, unsigned *pLength);

	// page Horloge
	static void ListClockAssets (CString *pJSON);
	boolean RenderClockPreview (const char *pParams, u8 *pBuffer, unsigned *pLength);
	static void AddClockAsset (const char *pType, const char *pFileName, const u8 *pData,
				   unsigned nLength, CString *pReply);
	static boolean GetParam (const char *pParams, const char *pName, char *pValue, unsigned nSize);

private:
	CConfig *m_pConfig;
};

const char *GetBuildDate (void);	// version.cpp
const char *GetVersion (void);		// version.cpp (fichier VERSION)

#endif
