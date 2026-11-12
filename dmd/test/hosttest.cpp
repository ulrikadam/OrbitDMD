//
// hosttest.cpp - banc de test sous Windows pour le code du Pi (sans materiel)
//
//   hosttest gifinfo <fichier.gif>...          -> "OK images duree_ms chemin" par fichier
//   hosttest gifdump <fichier.gif> <sortie>    -> images RGB brutes 128x32 a la suite + delais
//   hosttest pattern <sortie>                  -> ecrans de la mire en RGB brut
//   hosttest tz                                -> controles heure d'ete / d'hiver
//   hosttest config <config.txt>               -> reglages lus
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../gifanim.h"
#include "../canvas.h"
#include "../testpattern.h"
#include "../localtime.h"
#include "../config.h"
#include "../configedit.h"
#include "../netutil.h"
#include "../clockstyle.h"
#include "../clockrender.h"
#include "../weatherdata.h"
#include "../gifmarks.h"
#include "../font5x7.h"
#include "../lang.h"
#include "../messages.h"
#include "../schedule.h"
#include "../hapayload.h"

static const unsigned W = 128, H = 32;
#define CHECK(x) do { boolean b_ = (x); if (!b_) nFailed++; printf ("%s %s\n", b_ ? "ok    " : "ECHEC ", #x); } while (0)

static unsigned char *ReadFile (const char *pPath, unsigned *pSize)
{
	FILE *f = fopen (pPath, "rb");
	if (!f) return 0;
	fseek (f, 0, SEEK_END);
	long n = ftell (f);
	fseek (f, 0, SEEK_SET);
	unsigned char *p = (unsigned char *) malloc (n);
	fread (p, 1, n, f);
	fclose (f);
	*pSize = (unsigned) n;
	return p;
}

int main (int argc, char **argv)
{
	if (argc < 2) return 1;

	if (strcmp (argv[1], "gifinfo") == 0)
	{
		CGifAnimation Anim (W, H);
		for (int i = 2; i < argc; i++)
		{
			unsigned nSize;
			unsigned char *p = ReadFile (argv[i], &nSize);
			if (p && Anim.Decode (p, nSize))
				printf ("OK %u %u %s\n", Anim.GetFrameCount (), Anim.GetTotalDuration (), argv[i]);
			else
				printf ("FAIL 0 0 %s\n", argv[i]);
			Anim.Clear ();
			free (p);
		}
		return 0;
	}

	if (strcmp (argv[1], "gifdump") == 0 && argc == 4)
	{
		CGifAnimation Anim (W, H);
		unsigned nSize;
		unsigned char *p = ReadFile (argv[2], &nSize);
		if (!p || !Anim.Decode (p, nSize)) return 2;
		FILE *f = fopen (argv[3], "wb");
		for (unsigned i = 0; i < Anim.GetFrameCount (); i++)
		{
			fwrite (Anim.GetFrame (i), 1, W * H * 3, f);
			printf ("%u\n", Anim.GetDelay (i));
		}
		fclose (f);
		return 0;
	}

	if (strcmp (argv[1], "pattern") == 0 && argc == 3)
	{
		CCanvas Canvas (W, H);
		FILE *f = fopen (argv[2], "wb");
		for (unsigned i = 0; i < TestPatternGetCount (); i++)
		{
			TestPatternDraw (&Canvas, i, 64, 32, 2, 1);
			fwrite (Canvas.GetData (), 1, W * H * 3, f);
		}
		// ecrans dessines par l'application : horloge et rafraichissement
		Canvas.Clear ();
		Canvas.DrawTextCentered (0, W, 4, "18:42:07", 0, 255, 0, 2);
		Canvas.DrawTextCentered (0, W, H - 9, "10/11/2026", 0, 160, 0);
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		Canvas.Clear ();
		Canvas.DrawTextCentered (0, W, 2, "RAFRAICHISSEMENT", 255, 255, 255);
		Canvas.DrawTextCentered (0, W, 14, "152 HZ", 255, 255, 0, 2);
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		Canvas.Clear ();
		Canvas.DrawTextCentered (0, W, (H - 14) / 2, "23:59:58", 255, 170, 0, 2);
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		fclose (f);
		return 0;
	}

	if (strcmp (argv[1], "tz") == 0)
	{
		static const TDateTime Cases[] =
		{
			{2026,  3, 29,  0, 59, 59},	// juste avant le passage a l'heure d'ete
			{2026,  3, 29,  1,  0,  0},	// juste apres
			{2026, 10, 25,  0, 59, 59},	// juste avant le retour a l'heure d'hiver
			{2026, 10, 25,  1,  0,  0},	// juste apres
			{2026, 12, 31, 23, 30,  0},	// changement d'annee
			{2028,  2, 28, 23, 15,  0},	// annee bissextile
			{2026, 11, 10, 15, 42,  0},
		};
		for (unsigned i = 0; i < sizeof Cases / sizeof Cases[0]; i++)
		{
			TDateTime L;
			UTCToLocal (Cases[i], "Europe/Paris", &L);
			printf ("UTC %04u-%02u-%02u %02u:%02u -> Paris %04u-%02u-%02u %02u:%02u\n",
				Cases[i].Year, Cases[i].Month, Cases[i].Day, Cases[i].Hour, Cases[i].Minute,
				L.Year, L.Month, L.Day, L.Hour, L.Minute);
		}
		TDateTime L;
		UTCToLocal (Cases[6], "Europe/London", &L);
		printf ("London %02u:%02u\n", L.Hour, L.Minute);

		// temps Unix : valeurs connues + aller-retour sur 40 ans
		TDateTime T = {2026, 11, 10, 15, 42, 0};
		printf ("Unix 2026-11-10 15:42:00 = %u (attendu 1794325320)\n", DateTimeToUnix (T));
		unsigned nErrors = 0;
		for (u32 t = 1000000000U; t < 2240000000U; t += 86399U * 7 + 3601U)
		{
			TDateTime D;
			UnixToDateTime (t, &D);
			if (DateTimeToUnix (D) != t) nErrors++;
		}
		printf ("Aller-retour Unix : %u erreur(s)\n", nErrors);
		return 0;
	}

	if (strcmp (argv[1], "configedit") == 0)
	{
		static const struct
		{
			const char *pName;
			const char *pIn;
			const char *pExpected;
		}
		Cases[] =
		{
			{"remplacement simple",
			 "# Panneaux\nPanel_XSize=64\nGPIO_Slowdown=2\n",
			 "# Panneaux\nPanel_XSize=64\nGPIO_Slowdown=3\n"},
			{"casse et espaces, commentaire intact",
			 "#GPIO_Slowdown=9\n  gpio_slowdown = 2\n",
			 "#GPIO_Slowdown=9\n  gpio_slowdown =3\n"},
			{"fins de ligne Windows conservees",
			 "A=1\r\nGPIO_Slowdown=2\r\nB=2\r\n",
			 "A=1\r\nGPIO_Slowdown=3\r\nB=2\r\n"},
			{"cle absente ajoutee a la fin (Windows)",
			 "A=1\r\nB=2",
			 "A=1\r\nB=2\r\nGPIO_Slowdown=3\r\n"},
			{"cle prefixe d'une autre (Panel_Brightness_1 vs _10)",
			 "Panel_Brightness_10=60\nPanel_Brightness_1=60\n",
			 "Panel_Brightness_10=60\nPanel_Brightness_1=5\n"},
			{"doublons tous remplaces",
			 "GPIO_Slowdown=1\nX=0\nGPIO_Slowdown=2\n",
			 "GPIO_Slowdown=3\nX=0\nGPIO_Slowdown=3\n"},
		};
		unsigned nFailed = 0;
		for (unsigned i = 0; i < sizeof Cases / sizeof Cases[0]; i++)
		{
			TConfigChange Change = {"GPIO_Slowdown", "3"};
			if (i == 4)
			{
				Change.pKey = "Panel_Brightness_1";
				Change.pValue = "5";
			}
			char Out[512];
			int n = EditConfigText (Cases[i].pIn, &Change, 1, Out, sizeof Out);
			boolean bOK = n >= 0 && strcmp (Out, Cases[i].pExpected) == 0;
			printf ("%s : %s\n", bOK ? "OK    " : "ECHEC ", Cases[i].pName);
			if (!bOK) { nFailed++; printf ("   obtenu : [%s]\n", n >= 0 ? Out : "(debordement)"); }
		}
		char Small[8];
		TConfigChange Big = {"Cle", "valeur-trop-longue"};
		printf ("%s : sortie trop petite -> -1\n",
			EditConfigText ("A=1\n", &Big, 1, Small, sizeof Small) == -1 ? "OK    " : "ECHEC ");
		return nFailed;
	}

	if (strcmp (argv[1], "ip") == 0)
	{
		static const struct { const char *pText; boolean bValid; } Parse[] =
		{
			{"192.168.0.151", TRUE}, {"0.0.0.0", TRUE}, {"255.255.255.255", TRUE},
			{"192.168.0", FALSE}, {"192.168.0.256", FALSE}, {"192.168.0.1.5", FALSE},
			{"192.168..1", FALSE}, {"1234.1.1.1", FALSE}, {" 192.168.0.1", FALSE},
			{"192.168.0.1 ", FALSE}, {"", FALSE}, {"a.b.c.d", FALSE},
		};
		unsigned nFailed = 0;
		for (unsigned i = 0; i < sizeof Parse / sizeof Parse[0]; i++)
		{
			u8 A[4];
			boolean bOK = ParseIPv4 (Parse[i].pText, A) == Parse[i].bValid;
			if (!bOK) nFailed++;
			printf ("%s : lecture \"%s\"\n", bOK ? "OK    " : "ECHEC ", Parse[i].pText);
		}

		static const struct { const char *pIP, *pMask, *pGateway, *pDNS; boolean bValid; } Check[] =
		{
			{"192.168.0.151", "255.255.255.0", "192.168.0.1", "192.168.0.1", TRUE},
			{"192.168.0.151", "255.255.255.0", "192.168.0.1", "8.8.8.8", TRUE},
			{"10.1.2.3", "255.0.0.0", "10.0.0.1", "10.0.0.1", TRUE},
			{"192.168.0.151", "255.0.255.0", "192.168.0.1", "192.168.0.1", FALSE},	// masque troue
			{"192.168.0.151", "255.255.255.255", "192.168.0.1", "192.168.0.1", FALSE},	// /32
			{"192.168.0.0", "255.255.255.0", "192.168.0.1", "192.168.0.1", FALSE},	// adresse reseau
			{"192.168.0.255", "255.255.255.0", "192.168.0.1", "192.168.0.1", FALSE},	// diffusion
			{"192.168.0.151", "255.255.255.0", "192.168.1.1", "192.168.0.1", FALSE},	// autre reseau
			{"192.168.0.151", "255.255.255.0", "192.168.0.151", "192.168.0.1", FALSE},	// passerelle = IP
			{"127.0.0.1", "255.0.0.0", "127.0.0.2", "127.0.0.2", FALSE},		// boucle locale
			{"192.168.0.151", "255.255.255.0", "192.168.0.1", "0.0.0.0", FALSE},	// DNS nul
		};
		for (unsigned i = 0; i < sizeof Check / sizeof Check[0]; i++)
		{
			u8 IP[4], Mask[4], Gateway[4], DNS[4];
			ParseIPv4 (Check[i].pIP, IP); ParseIPv4 (Check[i].pMask, Mask);
			ParseIPv4 (Check[i].pGateway, Gateway); ParseIPv4 (Check[i].pDNS, DNS);
			const char *pError = CheckStaticIP (IP, Mask, Gateway, DNS);
			boolean bOK = (pError == 0) == Check[i].bValid;
			if (!bOK) nFailed++;
			printf ("%s : %s / %s gw %s dns %s -> %s\n", bOK ? "OK    " : "ECHEC ", Check[i].pIP,
				Check[i].pMask, Check[i].pGateway, Check[i].pDNS, pError ? pError : "valide");
		}
		return nFailed;
	}

	// hosttest clock <sortie.rgb> [date] [Cle=valeur]... : l'heure du mardi 10/11/2026 09:05:07
	// dessinee comme sur le panneau (a lancer depuis carte-sd, pour trouver fonts/)
	if (strcmp (argv[1], "clock") == 0 && argc >= 3)
	{
		TClockStyle Style;
		ClockStyleDefaults (&Style);
		boolean bDate = FALSE;
		for (int i = 3; i < argc; i++)
		{
			if (strcmp (argv[i], "date") == 0)
			{
				bDate = TRUE;
				continue;
			}
			char Key[64];
			const char *pEqual = strchr (argv[i], '=');
			if (pEqual == 0 || pEqual - argv[i] >= (int) sizeof Key) return 2;
			memcpy (Key, argv[i], pEqual - argv[i]);
			Key[pEqual - argv[i]] = '\0';
			unsigned nLanguage;
			if (strcmp (Key, "Langue") == 0 && ParseLanguage (pEqual + 1, &nLanguage))
			{
				SetLanguage (nLanguage);		// noms des jours et des mois
				continue;
			}
			if (!SetClockStyleValue (&Style, Key, pEqual + 1))
			{
				fprintf (stderr, "cle inconnue : %s\n", Key);
				return 2;
			}
		}

		TDateTime Time = {2026, 11, 10, 9, 5, 7};
		CClockRenderer Renderer (W, H);
		CCanvas Canvas (W, H);
		CClockRenderer::PickRandom (&Style, (u32) rand () * 2654435761U ^ (u32) time (0));
		Renderer.Prepare (Style);
		Renderer.Render (Style, Time, 0, TRUE, bDate, &Canvas);

		char Text[64];
		FormatDateTime (Style.Time.Format, Time, Text, sizeof Text);
		printf ("heure \"%s\"", Text);
		FormatDateTime (Style.Date.Format, Time, Text, sizeof Text);
		printf (" date \"%s\"\n", Text);

		FILE *f = fopen (argv[2], "wb");
		if (!f) return 3;
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		fclose (f);
		return 0;
	}

	// hosttest weather <sortie.rgb> [json=<reponse.json>] [code=N] [jour=0] [Weather_X=valeur]...
	// lit une reponse d'Open-Meteo (ou un exemple) et dessine l'ecran meteo
	if (strcmp (argv[1], "weather") == 0 && argc >= 3)
	{
		TWeatherStyle Style;
		WeatherStyleDefaults (&Style);
		TWeatherData Data;
		memset (&Data, 0, sizeof Data);
		Data.bValid = TRUE; Data.nTemp10 = 72; Data.nFeels10 = 41; Data.nHumidity = 85;
		Data.nWind10 = 152; Data.nMin10 = 31; Data.nMax10 = 108; Data.nCode = 2; Data.bDay = TRUE;
		Data.nTomorrowCode = 61; Data.nTomorrowMin10 = 40; Data.nTomorrowMax10 = 120;
		unsigned nMillis = 0;

		for (int i = 3; i < argc; i++)
		{
			const char *pArg = argv[i];
			if (strncmp (pArg, "json=", 5) == 0)
			{
				unsigned nSize;
				unsigned char *p = ReadFile (pArg + 5, &nSize);
				if (!p) return 3;
				char *pText = (char *) realloc (p, nSize + 1);
				pText[nSize] = '\0';
				if (!ParseOpenMeteo (pText, &Data)) { fprintf (stderr, "reponse illisible\n"); return 4; }
				free (pText);
			}
			else if (strncmp (pArg, "code=", 5) == 0)	Data.nCode = atoi (pArg + 5);
			else if (strncmp (pArg, "jour=", 5) == 0)	Data.bDay = atoi (pArg + 5) != 0;
			else if (strncmp (pArg, "temps=", 6) == 0)	nMillis = (unsigned) atoi (pArg + 6);
			else
			{
				char Key[64];
				const char *pEqual = strchr (pArg, '=');
				if (pEqual == 0 || pEqual - pArg >= (int) sizeof Key) return 2;
				memcpy (Key, pArg, pEqual - pArg);
				Key[pEqual - pArg] = '\0';
				unsigned nLanguage;
				if (strcmp (Key, "Langue") == 0 && ParseLanguage (pEqual + 1, &nLanguage)) { SetLanguage (nLanguage); continue; }
				if (!SetWeatherStyleValue (&Style, Key, pEqual + 1)) { fprintf (stderr, "cle inconnue : %s\n", Key); return 2; }
			}
		}

		printf ("temp %d ressenti %d humidite %d vent %d code %d (%s) jour %d min %d max %d demain %d %d/%d\n",
			Data.nTemp10, Data.nFeels10, Data.nHumidity, Data.nWind10, Data.nCode,
			WeatherDescription (Data.nCode, Data.bDay), Data.bDay, Data.nMin10, Data.nMax10,
			Data.nTomorrowCode, Data.nTomorrowMin10, Data.nTomorrowMax10);

		CClockRenderer Renderer (W, H);
		CCanvas Canvas (W, H);
		Renderer.PrepareWeather (Style);
		Renderer.RenderWeather (Style, Data, nMillis, &Canvas);
		FILE *f = fopen (argv[2], "wb");
		if (!f) return 3;
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		fclose (f);
		return 0;
	}

	// hosttest marques : GIF favorites et exclues (gifmarks.cpp)
	if (strcmp (argv[1], "marques") == 0)
	{
		unsigned nFailed = 0;
		unsigned nRead = CGifMarks::Import ("+Arcade/sf2.gif\r\n-XXX_Mature/a.gif\nmauvaise ligne\n+sans_categorie.gif\n"
						    "-Arcade/sf2.gif\n+Noel/Sapin.GIF\n", FALSE);
		CHECK (nRead == 4);
		CHECK (CGifMarks::Get ("Arcade", "sf2.gif") == GifMarkExcluded);	// la derniere ligne gagne
		CHECK (CGifMarks::Get ("arcade", "SF2.GIF") == GifMarkExcluded);	// majuscules ignorees
		CHECK (CGifMarks::Get ("Noel", "sapin.gif") == GifMarkFavorite);
		CHECK (CGifMarks::Get ("Noel", "autre.gif") == GifMarkNone);
		CHECK (CGifMarks::Set ("Arcade", "pacman.gif", GifMarkFavorite));
		CHECK (CGifMarks::Set ("Arcade", "sf2.gif", GifMarkNone));
		CHECK (CGifMarks::Get ("Arcade", "sf2.gif") == GifMarkNone);
		CHECK (!CGifMarks::Set ("Ar/cade", "x.gif", GifMarkFavorite));
		CGifMarks::ForgetCategory ("XXX_Mature");
		CHECK (CGifMarks::GetCount (GifMarkExcluded) == 0);
		CString Text;
		CGifMarks::Export (&Text);
		printf ("%s", (const char *) Text);
		CHECK (strcmp (Text, "+Arcade/pacman.gif\n+Noel/Sapin.GIF\n") == 0);
		printf ("%u echec(s)\n", nFailed);
		return nFailed == 0 ? 0 : 1;
	}

	// hosttest ha : annonces Home Assistant (JSON ecrit dans <dossier>/annonce-N.json) et notifications
	if (strcmp (argv[1], "ha") == 0 && argc == 3)
	{
		unsigned nFailed = 0;
		char Node[32];
		HAMakeNodeId ("Salon-DMD 2", Node, sizeof Node);
		CHECK (strcmp (Node, "salon-dmd2") == 0);
		for (unsigned i = 0; i < HAGetEntityCount (); i++)
		{
			CString Topic, Payload;
			HABuildDiscovery (i, Node, "Salon \"DMD\"", "1.4.0", "192.168.0.151", &Topic, &Payload);
			char Path[300];
			snprintf (Path, sizeof Path, "%s/annonce-%02u.json", argv[2], i);
			FILE *f = fopen (Path, "wb");
			if (!f) return 3;
			fprintf (f, "%s\n%s\n", (const char *) Topic, (const char *) Payload);
			fclose (f);
		}

		THANotification N;
		TBannerStyle Def;
		CMessages::NotifyDefaults (&Def);
		CHECK (HAParseNotification ("Quelqu'un sonne !", Def, &N) && strcmp (N.Style.Text, "Quelqu'un sonne !") == 0
		       && N.Style.Effect == EffectScroll && N.Style.nPasses == 2);
		CHECK (HAParseNotification ("{\"message\": \"Lave-linge termin\\u00e9\", \"effet\": \"pluie\", "
					    "\"couleur\": \"arc-en-ciel\", \"passages\": 3, \"gif\": \"XMAS/sapin.gif\"}", Def, &N));
		CHECK (strcmp (N.Style.Text, "Lave-linge termin\xC3\xA9") == 0 && N.Style.Effect == EffectRain);
		CHECK (strcmp (N.Style.Style.Pattern, CLOCK_RAINBOW) == 0 && N.Style.nPasses == 3);
		CHECK (strcmp (N.Gif, "XMAS/sapin.gif") == 0);
		CHECK (HAParseNotification ("{\"message\":\"Colis\",\"couleur\":\"#FF8000\",\"gif\":\"../../Config/config.txt\"}", Def, &N)
		       && N.Style.Style.Color[0] == 0xFF && N.Style.Style.Color[1] == 0x80 && N.Gif[0] == '\0');
		CHECK (!HAParseNotification ("{\"effet\":\"pluie\"}", Def, &N));		// ni message ni GIF
		// style de depart modifie depuis Home Assistant : utilise par un texte simple
		CHECK (HASetEffectFromLabel ("Pluie de lettres", &Def) && Def.Effect == EffectRain);
		CHECK (HASetColorFromLabel ("Rouge", &Def.Style) && strcmp (HAGetColorLabel (Def.Style), "Rouge") == 0);
		CHECK (HASetColorFromLabel ("Arc-en-ciel", &Def.Style) && strcmp (HAGetColorLabel (Def.Style), "Arc-en-ciel") == 0);
		CHECK (HASetColorFromLabel ("123456", &Def.Style) && strcmp (HAGetColorLabel (Def.Style), "Autre") == 0);
		CHECK (HAParseNotification ("Bonne nuit", Def, &N) && N.Style.Effect == EffectRain && strcmp (N.Style.Text, "Bonne nuit") == 0);
		CHECK (HAParseNotification ("{\"message\":\"x\",\"effet\":\"Fondu\",\"couleur\":\"Vert\"}", Def, &N)
		       && N.Style.Effect == EffectFade && strcmp (HAGetColorLabel (N.Style.Style), "Vert") == 0);

		// langues : noms et listes de l'annonce dans la langue du panneau ; commandes dans toutes
		SetLanguage (LangEN);
		for (unsigned i = 0; i < HAGetEntityCount (); i++)
		{
			CString Topic, Payload;
			HABuildDiscovery (i, Node, "Salon", "1.5.0", "192.168.0.151", &Topic, &Payload);
			if (strcmp (HAGetEntity (i).pObject, "msg_effet") == 0)
			{
				CHECK (strstr (Payload, "\"name\":\"Message effect\"") != 0);
				CHECK (strstr (Payload, "\"options\":[\"Scroll left\",\"Scroll right\",\"Rise\",\"Fall\",\"Letter by letter\",\"Raining letters\",\"Fade\"],") != 0);
			}
			if (strcmp (HAGetEntity (i).pObject, "msg_couleur") == 0)
			{
				CHECK (strstr (Payload, "\"Rainbow\",\"Other\"]") != 0);
			}
			CHECK (strstr (Payload, "panneaux") == 0);		// plus de francais
		}
		CHECK (HASetEffectFromLabel ("Raining letters", &Def) && Def.Effect == EffectRain);
		CHECK (HASetEffectFromLabel ("Fondu", &Def) && Def.Effect == EffectFade);	// francais accepte
		CHECK (HASetColorFromLabel ("Rouge", &Def.Style) && strcmp (HAGetColorLabel (Def.Style), "Red") == 0);
		CHECK (HASetColorFromLabel ("\xD0\xA0\xD0\xB0\xD0\xB4\xD1\x83\xD0\xB3\xD0\xB0", &Def.Style)		// Радуга
		       && strcmp (HAGetColorLabel (Def.Style), "Rainbow") == 0);
		CHECK (strcmp (HAScreenName ("meteo"), "Weather") == 0 && strcmp (HAScreenName ("gif"), "gif") == 0);
		CString Item, ItemTopic;
		HABuildItemDiscovery (FALSE, 2, "NOEL", Node, "Salon", &ItemTopic, &Item);
		CHECK (strstr (Item, "\"name\":\"Countdown: NOEL\"") != 0);
		CString Cmds;
		HABuildCommandList (Node, &Cmds);
		CHECK (strstr (Cmds, "message effect: Scroll left, Scroll right") != 0 && strstr (Cmds, "Rainbow, RRGGBB") != 0);
		SetLanguage (LangRU);
		HABuildCommandList (Node, &Cmds);
		CHECK (strstr (Cmds, "\xD0\xA0\xD0\xB0\xD0\xB4\xD1\x83\xD0\xB3\xD0\xB0, RRGGBB") != 0);
		SetLanguage (LangFR);
		CHECK (strcmp (HAScreenName ("meteo"), "meteo") == 0 && strcmp (HAGetColorLabel (Def.Style), "Arc-en-ciel") == 0);

		// interrupteurs des messages et liste des commandes (fichiers JSON pour verification)
		const char *Items[][2] = {{"1", "Joyeux anniversaire L\xC3\xA9" "a \"2026\""}, {"0", "NO\xC3\x8BL DANS"}, {"1", ""}};
		for (unsigned i = 0; i < 3; i++)
		{
			CString Topic, Payload;
			HABuildItemDiscovery (Items[i][0][0] == '1', i, Items[i][1], Node, "Salon", &Topic, &Payload);
			char Path[300];
			snprintf (Path, sizeof Path, "%s/element-%u.json", argv[2], i);
			FILE *f = fopen (Path, "wb");
			if (!f) return 3;
			fprintf (f, "%s\n%s\n", (const char *) Topic, (const char *) Payload);
			fclose (f);
		}
		CString List;
		HABuildCommandList (Node, &List);
		char Path[300];
		snprintf (Path, sizeof Path, "%s/commandes.json", argv[2]);
		FILE *f = fopen (Path, "wb");
		if (!f) return 3;
		fprintf (f, "%s\n", (const char *) List);
		fclose (f);

		char Value[32];
		CHECK (HAGetJSONValue ("{\"a\": \"x:y\", \"b\" : 12, \"c\":true}", "b", Value, sizeof Value) && strcmp (Value, "12") == 0);
		CHECK (HAGetJSONValue ("{\"msg\": \"b\", \"b\": \"vrai\"}", "b", Value, sizeof Value) && strcmp (Value, "vrai") == 0);
		printf ("%u echec(s)\n", nFailed);
		return nFailed == 0 ? 0 : 1;
	}

	// hosttest programme : plages horaires, mois des categories, periodes (schedule.cpp)
	if (strcmp (argv[1], "programme") == 0)
	{
		unsigned nFailed = 0;
		CHECK (InHourWindow (1, 7, 1) && InHourWindow (1, 7, 6) && !InHourWindow (1, 7, 7) && !InHourWindow (1, 7, 0));
		CHECK (InHourWindow (23, 7, 23) && InHourWindow (23, 7, 0) && InHourWindow (23, 7, 6) && !InHourWindow (23, 7, 12));
		CHECK (!InHourWindow (5, 5, 5));
		TSchedule S;
		ScheduleDefaults (&S);
		CHECK (ParseCategoryMonths ("XMAS:12;Halloween:10;Plage:6-8;Hiver:11-2", &S) && S.nRules == 4);
		CHECK (GetCategoryMonths (S, "xmas") == (1 << 12));
		CHECK (GetCategoryMonths (S, "Plage") == ((1 << 6) | (1 << 7) | (1 << 8)));
		CHECK (GetCategoryMonths (S, "Hiver") == ((1 << 11) | (1 << 12) | (1 << 1) | (1 << 2)));
		CHECK (GetCategoryMonths (S, "Arcade") == SCHEDULE_ALL_MONTHS);
		CHECK (!ParseCategoryMonths ("XMAS:13", &S) && S.nRules == 4);		// invalide : inchange
		CHECK (!ParseCategoryMonths ("Mauvais nom:12", &S));
		CHECK (ParseCategoryMonths ("", &S) && S.nRules == 0);
		TDateTime Dec24 = {2026, 12, 24, 12, 0, 0}, Jan3 = {2027, 1, 3, 12, 0, 0}, Jul14 = {2026, 7, 14, 0, 0, 0};
		CHECK (InDateWindow ("", "", Jul14));
		CHECK (InDateWindow ("12-20", "01-05", Dec24) && InDateWindow ("12-20", "01-05", Jan3) && !InDateWindow ("12-20", "01-05", Jul14));
		CHECK (InDateWindow ("07-14", "07-14", Jul14) && !InDateWindow ("07-15", "", Jul14));
		CHECK (IsValidMonthDay ("02-29") && !IsValidMonthDay ("02-30") && !IsValidMonthDay ("13-01") && !IsValidMonthDay ("1-1"));
		CHECK (SetScheduleValue (&S, "Prog_Eteint_Debut", "2") && S.nOffFrom == 2);
		CHECK (SetScheduleValue (&S, "Prog_Eteint_Fin", "24") && S.nOffTo == 7);	// refuse : garde 7
		printf ("%u echec(s)\n", nFailed);
		return nFailed == 0 ? 0 : 1;
	}

	// hosttest liste : fichier messages.txt (messages.cpp) lu puis reecrit
	if (strcmp (argv[1], "liste") == 0)
	{
		unsigned nFailed = 0;
		unsigned nItems = CMessages::Parse (
			"[texte]\nTexte_Message=Joyeux anniversaire L\xC3\xA9" "a !\nTexte_Effet=pluie\nTexte_Font_Pattern=Arc-en-ciel\n"
			"Texte_Active=0\n[compte]\nCompte_Date=2027-03-14 20:00\nCompte_Titre=ANNIV DANS\n"
			"[texte]\nTexte_Message=Partie \xC3\xA0 20 h\nTexte_Effet=inconnu\nTexte_Vitesse=99\n"
			"[autre]\nTexte_Message=ignore\n");
		CHECK (nItems == 3);
		CHECK (CMessages::s_nTexts == 2 && CMessages::s_nCountdowns == 1);
		CHECK (CMessages::s_Texts[0].Effect == EffectRain && !CMessages::s_Texts[0].bActive);
		CHECK (strcmp (CMessages::s_Texts[0].Style.Pattern, CLOCK_RAINBOW) == 0);
		CHECK (CMessages::s_Texts[1].Effect == EffectScroll && CMessages::s_Texts[1].nSpeed == 4);
		CHECK (CMessages::s_Texts[1].bActive && strcmp (CMessages::s_Texts[1].Text, "Partie \xC3\xA0" " 20 h") == 0);
		CHECK (strcmp (CMessages::s_Countdowns[0].Target, "2027-03-14 20:00") == 0);
		CHECK (CMessages::CountActiveTexts () == 1 && CMessages::CountActiveCountdowns () == 1);
		CString Text, Again;
		CMessages::Export (&Text);
		CMessages::Parse (Text);
		CMessages::Export (&Again);
		CHECK (strcmp (Text, Again) == 0);		// relu a l'identique
		CString JSON;
		CMessages::BuildJSON (&JSON);
		printf ("%s", (const char *) JSON);
		printf ("%u echec(s)\n", nFailed);
		return nFailed == 0 ? 0 : 1;
	}

	// hosttest messages : verifications du compte a rebours et des textes (page Messages)
	if (strcmp (argv[1], "messages") == 0)
	{
		struct { TDateTime Now; const char *pTarget; TCountdownState State; const char *pValue; } Cases[] =
		{
			{{2026, 12, 13, 10, 0, 0},  "noel",             CountdownRunning, "11 JOURS"},
			{{2026, 11, 11, 18, 0, 0},  "noel",             CountdownRunning, "43 JOURS"},
			{{2026, 12, 24, 23, 59, 30}, "noel",            CountdownRunning, "00:00:30"},
			{{2026, 12, 25, 15, 0, 0},  "noel",             CountdownReached, ""},
			{{2026, 12, 26, 0, 0, 1},   "noel",             CountdownRunning, "363 JOURS"},
			{{2026, 12, 31, 21, 45, 27}, "nouvel-an",       CountdownRunning, "02:14:33"},
			{{2027, 1, 1, 0, 30, 0},    "nouvel-an",        CountdownReached, ""},
			{{2026, 11, 11, 18, 0, 0},  "2026-11-20 20:00", CountdownRunning, "9 J 2 H"},
			{{2026, 11, 21, 19, 0, 0},  "2026-11-20 20:00", CountdownReached, ""},
			{{2026, 11, 21, 20, 0, 0},  "2026-11-20 20:00", CountdownNone,    ""},
			{{2026, 11, 11, 18, 0, 0},  "2026-02-31 20:00", CountdownNone,    ""},
			{{2026, 11, 11, 18, 0, 0},  "demain",           CountdownNone,    ""},
		};
		unsigned nFailed = 0;
		for (unsigned i = 0; i < sizeof Cases / sizeof Cases[0]; i++)
		{
			char Value[32];
			TCountdownState State = FormatCountdown (Cases[i].pTarget, Cases[i].Now, Value, sizeof Value);
			boolean bOK = State == Cases[i].State && strcmp (Value, Cases[i].pValue) == 0;
			if (!bOK) nFailed++;
			printf ("%s %-16s -> %d \"%s\"\n", bOK ? "ok    " : "ECHEC ", Cases[i].pTarget, State, Value);
		}

		// textes : UTF-8 garde ; ligne coupee entre deux caracteres ; police Pixel (Unicode)
		char Out[64];
		TextForDisplay ("Partie \xC3\xA0 20 h\nce soir", Out, sizeof Out);
		CHECK (strcmp (Out, "Partie \xC3\xA0 20 h ce soir") == 0);
		TextForDisplay ("\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82", Out, 6);	// "Привет" coupe
		CHECK (strcmp (Out, "\xD0\x9F\xD1\x80") == 0);
		const char *pRu = "\xD0\x9F\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82";	// Привет
		CHECK (Utf8Length (pRu) == 6 && Utf8Skip (pRu, 2) == pRu + 4);
		const char *q = pRu;
		CHECK (Utf8Next (&q) == 0x41F && Utf8Next (&q) == 0x440);
		CHECK (FontGetGlyph (0x440) == FontGetGlyph ('P'));		// р minuscule -> Р -> P latin
		CHECK (FontGetGlyph (0x431) == FontGetGlyph (0x411) && FontGetGlyph (0x411) != 0);	// б -> Б
		CHECK (FontGetGlyph (0xE9) == FontGetGlyph ('E'));		// é -> E
		CHECK (FontGetGlyph (0xF1) == FontGetGlyph (0xD1) && FontGetGlyph (0xD1) != FontGetGlyph ('N'));	// ñ -> Ñ
		CHECK (FontGetGlyph ('\'') != 0 && FontGetGlyph (0x20AC) != 0 && FontGetGlyph (0x4E2D) == 0);
		q = "\xB0";
		CHECK (Utf8Next (&q) == 0xB0);					// ancien texte Latin-1 : garde

		// langues du panneau : textes dessinables par la police Pixel, longueurs, formats
		for (unsigned nLang = 0; nLang < LangCount; nLang++)
		{
			SetLanguage (nLang);
			for (unsigned t = 0; t < TxtCount; t++)
			{
				const char *pText = Tr ((TText) t);
				unsigned nMax = t == TxtDayHourFormat || t == TxtHumidityFormat || t == TxtWindFormat ? 16 : 21;
				boolean bOK = *pText != '\0' && Utf8Length (pText) <= nMax;
				for (const char *r = pText; *r != '\0'; )
				{
					unsigned c = Utf8Next (&r);
					bOK = bOK && (c == '%' || FontGetGlyph (c) != 0);
				}
				if (!bOK) printf ("texte trop long ou lettre absente : %s %u \"%s\"\n", LanguageCode (nLang), t, pText);
				CHECK (bOK);
			}
			for (unsigned w = 0; w < WEATHER_TEXT_COUNT; w++)
			{
				const char *pText = WeatherText (w);
				boolean bOK = *pText != '\0' && Utf8Length (pText) <= 16;
				for (const char *r = pText; *r != '\0'; )
				{
					bOK = bOK && FontGetGlyph (Utf8Next (&r)) != 0;
				}
				if (!bOK) printf ("meteo trop longue ou lettre absente : %s %u \"%s\"\n", LanguageCode (nLang), w, pText);
				CHECK (bOK);
			}
			for (unsigned d = 0; d < 7 + 12; d++)
			{
				const char *pText = d < 7 ? DayName (d, TRUE) : MonthName (d - 7, TRUE);
				for (const char *r = pText; *r != '\0'; )
				{
					CHECK (FontGetGlyph (Utf8Next (&r)) != 0);
				}
			}
		}
		unsigned nLang = 99;
		CHECK (ParseLanguage ("EN", &nLang) && nLang == LangEN && !ParseLanguage ("jp", &nLang) && !ParseLanguage ("e", &nLang));
		CHECK (ParseLanguage ("uk", &nLang) && nLang == LangUK && ParseLanguage ("CS", &nLang) && nLang == LangCS);
		// lettres ajoutees : allemand, ukrainien, polonais, tcheque, turc
		CHECK (FontGetGlyph (0xE4) == FontGetGlyph (0xC4) && FontGetGlyph (0xC4) != FontGetGlyph ('A'));	// ä -> Ä
		CHECK (FontGetGlyph (0xDF) != 0 && FontGetGlyph (0x1E9E) == FontGetGlyph (0xDF));			// ß
		CHECK (FontGetGlyph (0x142) == FontGetGlyph ('L') && FontGetGlyph (0x159) == FontGetGlyph ('R'));	// ł, ř
		CHECK (FontGetGlyph (0x131) == FontGetGlyph ('I') && FontGetGlyph (0x15F) == FontGetGlyph ('S'));	// ı, ş
		CHECK (FontGetGlyph (0x456) == FontGetGlyph ('I') && FontGetGlyph (0x457) == FontGetGlyph (0x407)	// і, ї
		       && FontGetGlyph (0x407) != 0 && FontGetGlyph (0x454) == FontGetGlyph (0x404) && FontGetGlyph (0x491) == FontGetGlyph (0x490));
		SetLanguage (LangUK);
		char Uk[32];
		FormatDays (23, Uk, sizeof Uk);
		CHECK (strcmp (Uk, "23 ДНІ") == 0);
		SetLanguage (LangCS);
		FormatDays (22, Uk, sizeof Uk);
		CHECK (strcmp (Uk, "22 DNÍ") == 0);
		TDateTime Wed = {2026, 11, 11, 18, 0, 0};
		char Date[64];
		SetLanguage (LangEN);
		FormatDateTime ("%a %d %b", Wed, Date, sizeof Date);
		CHECK (strcmp (Date, "WED 11 NOV") == 0);
		CHECK (strcmp (WeatherDescription (0, FALSE), "CLEAR NIGHT") == 0);
		char Left[32];
		CHECK (FormatCountdown ("noel", Wed, Left, sizeof Left) == CountdownRunning && strcmp (Left, "43 DAYS") == 0);
		SetLanguage (LangES);
		FormatDateTime ("%A %e %B", Wed, Date, sizeof Date);
		CHECK (strcmp (Date, "MIERCOLES 11 NOVIEMBRE") == 0);
		SetLanguage (LangNL);
		TDateTime Soon = {2026, 11, 18, 18, 0, 0};
		CHECK (FormatCountdown ("2026-11-20 20:00", Soon, Left, sizeof Left) == CountdownRunning && strcmp (Left, "2 D 2 U") == 0);
		SetLanguage (LangRU);
		FormatDateTime ("%d %B", Wed, Date, sizeof Date);
		CHECK (strcmp (Date, "11 НОЯБРЯ") == 0);
		static const struct { unsigned n; const char *p; } Ru[] =
			{{1, "1 ДЕНЬ"}, {3, "3 ДНЯ"}, {5, "5 ДНЕЙ"}, {11, "11 ДНЕЙ"}, {12, "12 ДНЕЙ"}, {21, "21 ДЕНЬ"},
			 {22, "22 ДНЯ"}, {111, "111 ДНЕЙ"}, {114, "114 ДНЕЙ"}, {124, "124 ДНЯ"}};
		for (unsigned i = 0; i < sizeof Ru / sizeof Ru[0]; i++)
		{
			FormatDays (Ru[i].n, Left, sizeof Left);
			CHECK (strcmp (Left, Ru[i].p) == 0);
		}
		SetLanguage (LangFR);

		boolean bChecks[] =
		{
			IsValidBannerText ("Partie \xC3\xA0 20 h !"),
			!IsValidBannerText ("guillemet \" interdit"),
			!IsValidBannerText ("ligne\nsuivante"),
			IsValidCountdownTarget ("2027-01-01 00:00"),
			!IsValidCountdownTarget ("2027-13-01 00:00"),
			!IsValidCountdownTarget ("2027-1-1 0:0"),
		};
		for (unsigned i = 0; i < sizeof bChecks / sizeof bChecks[0]; i++)
		{
			if (!bChecks[i]) nFailed++;
			printf ("%s verification %u\n", bChecks[i] ? "ok    " : "ECHEC ", i);
		}

		printf ("%u echec(s)\n", nFailed);
		return nFailed == 0 ? 0 : 1;
	}

	// hosttest texte <sortie.rgb> <decalage> [Texte_X=valeur]... : une image du texte defilant
	// hosttest compte <sortie.rgb> <AAAA-MM-JJ HH:MM:SS> [Compte_X=valeur]... : compte a rebours
	if ((strcmp (argv[1], "texte") == 0 || strcmp (argv[1], "compte") == 0) && argc >= 4)
	{
		boolean bBanner = strcmp (argv[1], "texte") == 0;
		TBannerStyle Banner;
		TCountdownStyle Count;
		BannerStyleDefaults (&Banner);
		CountdownStyleDefaults (&Count);
		// options "Cle=valeur", ou "@fichier" : une option par ligne, en UTF-8
		static char Lines[64][256];
		const char *Options[64];
		unsigned nOptions = 0;
		for (int i = 4; i < argc && nOptions < 64; i++)
		{
			if (argv[i][0] != '@')
			{
				Options[nOptions++] = argv[i];
				continue;
			}
			FILE *pFile = fopen (argv[i] + 1, "rb");
			if (!pFile) return 3;
			while (nOptions < 64 && fgets (Lines[nOptions], sizeof Lines[0], pFile))
			{
				Lines[nOptions][strcspn (Lines[nOptions], "\r\n")] = '\0';
				if (Lines[nOptions][0] != '\0')
				{
					Options[nOptions] = Lines[nOptions];
					nOptions++;
				}
			}
			fclose (pFile);
		}
		for (unsigned i = 0; i < nOptions; i++)
		{
			char Key[64];
			const char *pEqual = strchr (Options[i], '=');
			if (pEqual == 0 || pEqual - Options[i] >= (int) sizeof Key) return 2;
			memcpy (Key, Options[i], pEqual - Options[i]);
			Key[pEqual - Options[i]] = '\0';
			if (!SetBannerStyleValue (&Banner, Key, pEqual + 1) && !SetCountdownStyleValue (&Count, Key, pEqual + 1))
			{
				fprintf (stderr, "cle inconnue : %s\n", Key);
				return 2;
			}
		}

		CClockRenderer Renderer (W, H);
		CCanvas Canvas (W, H);
		if (bBanner)
		{
			char Text[BANNER_TEXT_LEN];
			TextForDisplay (Banner.Text, Text, sizeof Text);
			Renderer.PrepareText (Banner.Style);
			int nWidth = Renderer.GetTextWidth (Banner.Style, Text);
			// <decalage> pour le defilement, ou "in:500", "hold:0", "out:300" (phase:avancement)
			const char *pColon = strchr (argv[3], ':');
			if (pColon != 0)
			{
				char Pages[20][BANNER_TEXT_LEN];
				Renderer.FitPages (&Banner.Style, Text);
				unsigned nPages = Renderer.SplitPages (Banner.Style, Text, Pages, 20);
				for (unsigned i = 0; i < nPages; i++) printf ("page %u \"%s\"\n", i, Pages[i]);
				unsigned nPhase =   strncmp (argv[3], "in", 2) == 0 ? CClockRenderer::PageIn
						  : strncmp (argv[3], "out", 3) == 0 ? CClockRenderer::PageOut : CClockRenderer::PageHold;
				Renderer.RenderPage (Banner.Style, nPages > 0 ? Pages[0] : "", Banner.Effect, nPhase,
						     (unsigned) atoi (pColon + 1), 1000, &Canvas);
			}
			else
			{
				int nOffset = atoi (argv[3]);
				int x = Banner.Effect == EffectReverse ? nOffset - nWidth : (int) W - nOffset;
				Renderer.RenderScroll (Banner.Style, Text, x, 1000, &Canvas);
			}
			printf ("largeur %d, %u pixels/s, effet %s\n", nWidth, BannerPixelsPerSecond (Banner.nSpeed),
				BannerEffectName (Banner.Effect));
		}
		else
		{
			TDateTime Now = {2026, 11, 11, 18, 0, 0};
			boolean bDone = strcmp (argv[3], "fin") == 0;	// apercu du jour J
			if (!bDone && sscanf (argv[3], "%u-%u-%u %u:%u:%u", &Now.Year, &Now.Month, &Now.Day,
					      &Now.Hour, &Now.Minute, &Now.Second) != 6) return 2;
			char Value[32], Latin[32], Title[COUNTDOWN_TITLE_LEN], Done[COUNTDOWN_TITLE_LEN];
			TCountdownState State = FormatCountdown (Count.Target, Now, Value, sizeof Value);
			if (bDone) State = CountdownReached;
			if (State == CountdownNone) strcpy (Value, "DATE PASSEE");
			TextForDisplay (Count.Title, Title, sizeof Title);
			TextForDisplay (Count.Done, Done, sizeof Done);
			TextForDisplay (Value, Latin, sizeof Latin);
			Renderer.PrepareText (Count.Style);
			if (State == CountdownReached)	Renderer.RenderCountdown (Count, "", Done, 0, &Canvas);
			else				Renderer.RenderCountdown (Count, Title, Latin, 0, &Canvas);
			printf ("etat %d \"%s\"\n", State, Value);
		}

		FILE *f = fopen (argv[2], "wb");
		if (!f) return 3;
		fwrite (Canvas.GetData (), 1, W * H * 3, f);
		fclose (f);
		return 0;
	}

	if (strcmp (argv[1], "config") == 0 && argc == 3)
	{
		CConfig Config;
		if (!Config.Load (argv[2])) return 2;
		Config.Log ();
		printf ("brightness[0]=%u [12]=%u [23]=%u\n", Config.GetBrightness (0),
			Config.GetBrightness (12), Config.GetBrightness (23));
		return 0;
	}

	return 1;
}
