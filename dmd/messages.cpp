//
// messages.cpp
//
#include "messages.h"
#include "config.h"
#include "fileutil.h"
#include <circle/logger.h>
#include <circle/util.h>

static const char From[] = "messages";

#define MESSAGES_TEMP		DRIVE "/Config/messages.new"

TBannerStyle CMessages::s_Texts[MESSAGES_MAX_TEXTS];
unsigned CMessages::s_nTexts = 0;
TCountdownStyle CMessages::s_Countdowns[MESSAGES_MAX_COUNTDOWNS];
unsigned CMessages::s_nCountdowns = 0;

unsigned CMessages::Parse (const char *pText)
{
	s_nTexts = 0;
	s_nCountdowns = 0;

	TBannerStyle *pText_ = 0;
	TCountdownStyle *pCount = 0;
	unsigned nItems = 0;

	const char *p = pText;
	while (*p != '\0')
	{
		const char *pEnd = p;
		while (*pEnd != '\0' && *pEnd != '\n' && *pEnd != '\r')
		{
			pEnd++;
		}

		char Line[300];
		size_t nLength = pEnd - p;
		if (nLength < sizeof Line)
		{
			memcpy (Line, p, nLength);
			Line[nLength] = '\0';

			if (strcasecmp (Line, "[texte]") == 0)
			{
				pText_ = 0;
				pCount = 0;
				if (s_nTexts < MESSAGES_MAX_TEXTS)
				{
					pText_ = &s_Texts[s_nTexts++];
					BannerStyleDefaults (pText_);
					pText_->bActive = TRUE;
					nItems++;
				}
			}
			else if (strcasecmp (Line, "[compte]") == 0)
			{
				pText_ = 0;
				pCount = 0;
				if (s_nCountdowns < MESSAGES_MAX_COUNTDOWNS)
				{
					pCount = &s_Countdowns[s_nCountdowns++];
					CountdownStyleDefaults (pCount);
					pCount->bActive = TRUE;
					nItems++;
				}
			}
			else if (Line[0] == '[')
			{
				pText_ = 0;		// section inconnue : ignoree
				pCount = 0;
			}
			else
			{
				char *pEqual = strchr (Line, '=');
				if (pEqual != 0 && Line[0] != '#')
				{
					*pEqual = '\0';
					if (pText_ != 0)
					{
						SetBannerStyleValue (pText_, Line, pEqual + 1);
					}
					else if (pCount != 0)
					{
						SetCountdownStyleValue (pCount, Line, pEqual + 1);
					}
				}
			}
		}

		p = pEnd;
		while (*p == '\n' || *p == '\r')
		{
			p++;
		}
	}

	return nItems;
}

static void AppendLine (CString *pText, const char *pKey, const char *pValue)
{
	pText->Append (pKey);
	pText->Append ("=");
	pText->Append (pValue);
	pText->Append ("\n");
}

static void AppendNumber (CString *pText, const char *pKey, unsigned nValue)
{
	CString Value;
	Value.Format ("%u", nValue);
	AppendLine (pText, pKey, Value);
}

static void AppendColor (CString *pText, const char *pKey, const u8 *pColor)
{
	CString Value;
	Value.Format ("%02X%02X%02X", pColor[0], pColor[1], pColor[2]);
	AppendLine (pText, pKey, Value);
}

void CMessages::Export (CString *pText)
{
	for (unsigned i = 0; i < s_nTexts; i++)
	{
		const TBannerStyle &T = s_Texts[i];
		pText->Append ("[texte]\n");
		AppendNumber (pText, "Texte_Active", T.bActive ? 1 : 0);
		AppendLine (pText, "Texte_Message", T.Text);
		AppendLine (pText, "Texte_Effet", BannerEffectName (T.Effect));
		AppendNumber (pText, "Texte_Vitesse", T.nSpeed);
		AppendNumber (pText, "Texte_Passages", T.nPasses);
		AppendLine (pText, "Texte_Font", T.Style.Font);
		AppendNumber (pText, "Texte_Font_Size", T.Style.nSize);
		AppendColor (pText, "Texte_Color", T.Style.Color);
		AppendLine (pText, "Texte_Font_Pattern", T.Style.Pattern);
	}

	for (unsigned i = 0; i < s_nCountdowns; i++)
	{
		const TCountdownStyle &C = s_Countdowns[i];
		pText->Append ("[compte]\n");
		AppendNumber (pText, "Compte_Active", C.bActive ? 1 : 0);
		AppendLine (pText, "Compte_Date", C.Target);
		AppendLine (pText, "Compte_Titre", C.Title);
		AppendLine (pText, "Compte_Fin", C.Done);
		AppendNumber (pText, "Compte_Display_Time", C.nDisplayTime);
		AppendLine (pText, "Compte_Font", C.Style.Font);
		AppendNumber (pText, "Compte_Font_Size", C.Style.nSize);
		AppendColor (pText, "Compte_Color", C.Style.Color);
		AppendColor (pText, "Compte_Titre_Color", C.TitleColor);
		AppendLine (pText, "Compte_Font_Pattern", C.Style.Pattern);
	}
}

// texte JSON (guillemets et barres obliques inverses echappes ; les textes n'en contiennent pas)
static void AppendJSON (CString *pJSON, const char *pKey, const char *pValue, boolean bFirst)
{
	pJSON->Append (bFirst ? "\"" : ",\"");
	pJSON->Append (pKey);
	pJSON->Append ("\":\"");
	for (const char *p = pValue; *p != '\0'; p++)
	{
		if (*p == '"' || *p == '\\')
		{
			pJSON->Append ('\\');
		}
		if ((unsigned char) *p >= ' ')
		{
			pJSON->Append (*p);
		}
	}
	pJSON->Append ("\"");
}

static void AppendJSONNumber (CString *pJSON, const char *pKey, unsigned nValue)
{
	CString Part;
	Part.Format (",\"%s\":%u", pKey, nValue);
	pJSON->Append (Part);
}

static void AppendJSONColor (CString *pJSON, const char *pKey, const u8 *pColor)
{
	CString Part;
	Part.Format (",\"%s\":\"%02X%02X%02X\"", pKey, pColor[0], pColor[1], pColor[2]);
	pJSON->Append (Part);
}

void CMessages::BuildJSON (CString *pJSON)
{
	pJSON->Append ("{\"textes\":[");
	for (unsigned i = 0; i < s_nTexts; i++)
	{
		const TBannerStyle &T = s_Texts[i];
		pJSON->Append (i > 0 ? ",{" : "{");
		AppendJSON (pJSON, "Texte_Message", T.Text, TRUE);
		AppendJSONNumber (pJSON, "Texte_Active", T.bActive ? 1 : 0);
		AppendJSON (pJSON, "Texte_Effet", BannerEffectName (T.Effect), FALSE);
		AppendJSONNumber (pJSON, "Texte_Vitesse", T.nSpeed);
		AppendJSONNumber (pJSON, "Texte_Passages", T.nPasses);
		AppendJSON (pJSON, "Texte_Font", T.Style.Font, FALSE);
		AppendJSONNumber (pJSON, "Texte_Font_Size", T.Style.nSize);
		AppendJSONColor (pJSON, "Texte_Color", T.Style.Color);
		AppendJSON (pJSON, "Texte_Font_Pattern", T.Style.Pattern, FALSE);
		pJSON->Append ("}");
	}

	pJSON->Append ("],\"comptes\":[");
	for (unsigned i = 0; i < s_nCountdowns; i++)
	{
		const TCountdownStyle &C = s_Countdowns[i];
		pJSON->Append (i > 0 ? ",{" : "{");
		AppendJSON (pJSON, "Compte_Date", C.Target, TRUE);
		AppendJSONNumber (pJSON, "Compte_Active", C.bActive ? 1 : 0);
		AppendJSON (pJSON, "Compte_Titre", C.Title, FALSE);
		AppendJSON (pJSON, "Compte_Fin", C.Done, FALSE);
		AppendJSONNumber (pJSON, "Compte_Display_Time", C.nDisplayTime);
		AppendJSON (pJSON, "Compte_Font", C.Style.Font, FALSE);
		AppendJSONNumber (pJSON, "Compte_Font_Size", C.Style.nSize);
		AppendJSONColor (pJSON, "Compte_Color", C.Style.Color);
		AppendJSONColor (pJSON, "Compte_Titre_Color", C.TitleColor);
		AppendJSON (pJSON, "Compte_Font_Pattern", C.Style.Pattern, FALSE);
		pJSON->Append ("}");
	}

	CString End;
	End.Format ("],\"max_textes\":%u,\"max_comptes\":%u}\n", MESSAGES_MAX_TEXTS, MESSAGES_MAX_COUNTDOWNS);
	pJSON->Append (End);
}

unsigned CMessages::CountActiveTexts (void)
{
	unsigned n = 0;
	for (unsigned i = 0; i < s_nTexts; i++)
	{
		if (s_Texts[i].bActive && s_Texts[i].Text[0] != '\0')
		{
			n++;
		}
	}

	return n;
}

unsigned CMessages::CountActiveCountdowns (void)
{
	unsigned n = 0;
	for (unsigned i = 0; i < s_nCountdowns; i++)
	{
		if (s_Countdowns[i].bActive)
		{
			n++;
		}
	}

	return n;
}

#ifndef HOST_TEST

void CMessages::Load (const CConfig *pConfig)
{
	char *pBuffer = new char[MESSAGES_MAX_SIZE + 1];
	unsigned nLength = MESSAGES_MAX_SIZE;
	if (ReadWholeFile (MESSAGES_FILE, pBuffer, &nLength))
	{
		pBuffer[nLength] = '\0';
		Parse (pBuffer);
	}
	else
	{
		// premiere fois : le message et le compte a rebours de config.txt (version 1.2.0)
		s_nTexts = 0;
		if (pConfig->Banner.Text[0] != '\0')
		{
			s_Texts[s_nTexts++] = pConfig->Banner;
		}
		s_nCountdowns = 0;
		s_Countdowns[s_nCountdowns++] = pConfig->Countdown;
	}
	delete [] pBuffer;

	CLogger::Get ()->Write (From, LogNotice, "%u texte(s), %u compte(s) a rebours (%u et %u actifs)",
				s_nTexts, s_nCountdowns, CountActiveTexts (), CountActiveCountdowns ());
}

boolean CMessages::Save (void)
{
	CString Text;
	Export (&Text);

	if (!SafeWriteFile (MESSAGES_FILE, MESSAGES_TEMP, 0, (const char *) Text, Text.GetLength ()))
	{
		CLogger::Get ()->Write (From, LogError, "Ecriture de %s impossible", MESSAGES_FILE);
		return FALSE;
	}

	return TRUE;
}

#else

void CMessages::Load (const CConfig *pConfig) {}
boolean CMessages::Save (void) { return TRUE; }

#endif
