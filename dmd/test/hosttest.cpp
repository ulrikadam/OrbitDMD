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

static const unsigned W = 128, H = 32;

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
			if (!SetClockStyleValue (&Style, Key, pEqual + 1))
			{
				fprintf (stderr, "cle inconnue : %s\n", Key);
				return 2;
			}
		}

		TDateTime Time = {2026, 11, 10, 9, 5, 7};
		CClockRenderer Renderer (W, H);
		CCanvas Canvas (W, H);
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
