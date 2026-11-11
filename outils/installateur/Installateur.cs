//
// Installateur.cs - assistant de creation d'une carte micro-SD OrbitDMD (Windows)
//
// L'assistant pose les questions (nom, panneaux, Wi-Fi, mot de passe, fuseau, meteo...),
// prepare dans un dossier temporaire le contenu de la carte (fichiers integres a l'exe,
// config.txt cree a partir des reponses, GIF choisis), puis lance preparer.ps1 en
// administrateur pour effacer la carte choisie, la formater en FAT32 et y copier ce contenu.
// Le dossier temporaire (qui contient le mot de passe Wi-Fi) est efface a la fin.
//
// Seuls les disques amovibles (USB, SD) sont proposes, jamais le disque de Windows.
//
// Essais sans carte :
//   OrbitDMD-Installateur.exe /dossier:C:\essai          (assistant normal, ecrit dans le dossier)
//   OrbitDMD-Installateur.exe /auto:reponses.txt /dossier:C:\essai   (sans fenetre)
//   OrbitDMD-Installateur.exe /captures:C:\images        (images de chaque etape, puis quitte)
//
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Net.NetworkInformation;
using System.Net.Sockets;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using System.Windows.Forms;

[assembly: AssemblyTitle ("Installateur OrbitDMD")]
[assembly: AssemblyProduct ("OrbitDMD")]
[assembly: AssemblyCompany ("Ulrik Adam - www.nexadam.be")]
[assembly: AssemblyCopyright ("OrbitDMD - Ulrik Adam")]

namespace OrbitDMD
{
	// --- reponses de l'assistant ---

	class Reponses
	{
		public string Nom = "OrbitDMD";
		public int Largeur = 64, Hauteur = 32, Chaine = 2, Parallele = 1, Ralentissement = 2;
		public string OrdreCouleurs = "RGB";
		public int LumJour = 60, LumNuit = 20, NuitDebut = 23, NuitFin = 7;
		public string Connexion = "auto";		// auto (cable s'il est branche, sinon Wi-Fi), wifi ou ethernet
		public string Ssid = "", Psk = "", Pays = "FR";
		public bool IpFixe = false;
		public string Ip = "", Masque = "", Passerelle = "", Dns = "";
		public string MotDePasse = "";
		public string Fuseau = "Europe/Paris";
		public string Lieu = "", Latitude = "", Longitude = "";
		public string DossierGif = "";
		public string CategoriesInactives = "";		// copiees mais pas affichees (separees par des virgules)
		public string GifFournies = "*";		// categories fournies a installer ("*" : choix par defaut)

		// fichier "cle=valeur" (mode /auto)
		public static Reponses Lire (string fichier)
		{
			var r = new Reponses ();
			foreach (string ligne in File.ReadAllLines (fichier, Encoding.UTF8))
			{
				int i = ligne.IndexOf ('=');
				if (i <= 0 || ligne.TrimStart ().StartsWith ("#")) continue;
				string cle = ligne.Substring (0, i).Trim (), val = ligne.Substring (i + 1).Trim ();
				FieldInfo champ = typeof (Reponses).GetField (cle);
				if (champ == null) throw new Exception ("Cle inconnue : " + cle);
				champ.SetValue (r, Convert.ChangeType (val, champ.FieldType, CultureInfo.InvariantCulture));
			}
			return r;
		}
	}

	// --- verifications (memes regles que le panneau) ---

	static class Regles
	{
		public static string Nom (string s) =>
			Regex.IsMatch (s, "^[A-Za-z0-9](?:[A-Za-z0-9-]{0,29}[A-Za-z0-9])?$") ? null
			: "Nom : 1 à 31 lettres, chiffres ou tirets (pas d'espace, pas d'accent).";

		static bool Imprimable (string s) => s.All (c => c >= ' ' && c <= '~' && c != '"');

		public static string Ssid (string s) =>
			s.Length >= 1 && s.Length <= 32 && Imprimable (s) && s.Trim () == s ? null
			: "Nom du réseau Wi-Fi : 1 à 32 caractères, sans accent ni guillemet.";

		public static string Wifi (string connexion, string ssid, string psk)
		{
			if (connexion != "auto" && connexion != "wifi" && connexion != "ethernet") return "Connexion : auto, wifi ou ethernet.";
			if (connexion == "ethernet" && psk.Length == 0) return null;	// pas de Wi-Fi de secours
			return Ssid (ssid) ?? Psk (psk);
		}

		public static string Psk (string s) =>
			s.Length >= 8 && s.Length <= 63 && Imprimable (s) && s.Trim () == s ? null
			: "Mot de passe Wi-Fi : 8 à 63 caractères, sans accent ni guillemet.";

		public static string MotDePasse (string s) =>
			s.Length >= 8 && s.Length <= 63 && Imprimable (s) && !s.Contains (" ") ? null
			: "Mot de passe de la page web : 8 caractères minimum, sans espace ni accent.";

		public static bool Ipv4 (string s, out byte[] octets)
		{
			octets = null;
			IPAddress a;
			if (!Regex.IsMatch (s, @"^\d{1,3}(\.\d{1,3}){3}$") || !IPAddress.TryParse (s, out a)) return false;
			octets = a.GetAddressBytes ();
			return true;
		}

		public static string AdresseFixe (string ip, string masque, string passerelle, string dns)
		{
			byte[] i, m, p, d;
			if (!Ipv4 (ip, out i)) return "Adresse IP invalide.";
			if (!Ipv4 (masque, out m)) return "Masque invalide.";
			if (!Ipv4 (passerelle, out p)) return "Passerelle (box) invalide.";
			if (!Ipv4 (dns, out d)) return "Serveur DNS invalide.";
			uint ui = Mot (i), um = Mot (m), up = Mot (p);
			int bits = 0;
			while (bits < 32 && (um & (0x80000000u >> bits)) != 0) bits++;
			if (bits < 8 || bits > 30 || um != (bits == 0 ? 0 : 0xFFFFFFFFu << (32 - bits))) return "Masque invalide (ex. 255.255.255.0).";
			if ((ui & ~um) == 0 || (ui & ~um) == ~um) return "Adresse IP invalide pour ce réseau.";
			if ((ui & um) != (up & um) || ui == up) return "La passerelle doit être sur le même réseau, à une autre adresse.";
			return null;
		}

		static uint Mot (byte[] b) => (uint) (b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3]);
	}

	// --- contenu de la carte ---

	static class Carte
	{
		public static readonly string[] Fuseaux =
		{
			"Europe/Paris", "Europe/Brussels", "Europe/Luxembourg", "Europe/Zurich", "Europe/Monaco",
			"Europe/Berlin", "Europe/Amsterdam", "Europe/Madrid", "Europe/Rome", "Europe/Vienna",
			"Europe/London", "Europe/Dublin", "Europe/Lisbon",
			"Europe/Helsinki", "Europe/Athens", "Europe/Bucharest", "Europe/Sofia", "Europe/Kyiv",
			"Europe/Istanbul", "Europe/Moscow", "UTC"
		};

		public static readonly string[][] Pays =
		{
			new[] { "FR", "France" }, new[] { "BE", "Belgique" }, new[] { "CH", "Suisse" },
			new[] { "LU", "Luxembourg" }, new[] { "CA", "Canada" }, new[] { "DE", "Allemagne" },
			new[] { "NL", "Pays-Bas" }, new[] { "ES", "Espagne" }, new[] { "IT", "Italie" },
			new[] { "PT", "Portugal" }, new[] { "GB", "Royaume-Uni" }, new[] { "US", "États-Unis" },
			new[] { "MA", "Maroc" }, new[] { "TN", "Tunisie" }, new[] { "DZ", "Algérie" }
		};

		public static string Config (Reponses r, IList<string> categories)
		{
			var s = new StringBuilder ();
			Action<string> L = t => s.Append (t).Append ("\r\n");

			L ("# OrbitDMD " + Construction.Numero + " - reglages du panneau");
			L ("# Cree par l'installateur le " + DateTime.Now.ToString ("dd/MM/yyyy HH:mm") + ".");
			L ("# Ensuite, tout se regle depuis la page web du panneau : http://" + r.Nom.ToLowerInvariant () + ".local/");
			L ("");
			L ("# Panneaux");
			L ("Panel_XSize=" + r.Largeur);
			L ("Panel_YSize=" + r.Hauteur);
			L ("Panel_XNumber=" + r.Chaine);
			L ("Panel_YNumber=" + r.Parallele);
			L ("GPIO_Slowdown=" + r.Ralentissement);
			L ("RGB_Order=" + r.OrdreCouleurs);
			L ("PWM_Bits=11");
			L ("Test_Pattern=0");
			L ("Journal_Ecran=0");
			L ("");
			L ("# Reseau (le mot de passe Wi-Fi n'est enregistre que sur cette carte)");
			L ("Network_name=" + r.Nom);
			L ("Reseau=" + r.Connexion);
			L ("Wifi_ssid=" + r.Ssid);
			L ("Wifi_psk=" + r.Psk);
			L ("Wifi_country=" + r.Pays);
			L ("IP_Fixe=" + (r.IpFixe ? r.Ip : ""));
			L ("IP_Masque=" + (r.IpFixe ? r.Masque : ""));
			L ("IP_Passerelle=" + (r.IpFixe ? r.Passerelle : ""));
			L ("IP_DNS=" + (r.IpFixe ? r.Dns : ""));
			L ("");
			L ("# Page web (mot de passe des modifications)");
			L ("Web_Password=" + r.MotDePasse);
			L ("");
			L ("# GIF (une categorie = un dossier de gif/)");
			L ("Gif_Active=1");
			L ("Gif_Min_Duration=4000");
			L ("IP_Ecran=10000");
			for (int i = 0; i < categories.Count && i < 20; i++) L ("Gif_Cat_" + (i + 1) + "=" + categories[i]);
			L ("");
			L ("# Horloge");
			L ("Clock_Active=1");
			L ("Clock_Display_Time=3000");
			L ("Clock_TZ=" + r.Fuseau);
			L ("Clock_Format=%H:%M:%S");
			L ("Clock_Font=Pixel");
			L ("Clock_Font_Size=0");
			L ("Clock_Color=FFAA00");
			L ("Clock_Font_Pattern=None");
			L ("Clock_Background=None");
			L ("Date_Active=0");
			L ("");
			L ("# Meteo (Open-Meteo.com, sans cle)");
			L ("Weather_Active=1");
			L ("Weather_Every=2");
			L ("Weather_Display_Time=5000");
			L ("Weather_Place=" + r.Lieu);
			L ("Weather_Latitude=" + r.Latitude);
			L ("Weather_Longitude=" + r.Longitude);
			L ("");
			L ("# Luminosite de chaque heure (en %)");
			for (int h = 0; h < 24; h++)
			{
				bool nuit = r.NuitDebut > r.NuitFin ? (h >= r.NuitDebut || h < r.NuitFin) : (h >= r.NuitDebut && h < r.NuitFin);
				L ("Panel_Brightness_" + h + "=" + (nuit ? r.LumNuit : r.LumJour));
			}
			return s.ToString ();
		}

		// tout le contenu de la carte dans un dossier ; rend le nombre de GIF ajoutees
		public static int Preparer (Reponses r, string dossier, Action<string> journal)
		{
			Assembly asm = Assembly.GetExecutingAssembly ();
			foreach (string nom in asm.GetManifestResourceNames ().Where (n => n.StartsWith ("carte/")))
			{
				string chemin = Path.Combine (dossier, nom.Substring (6).Replace ('/', '\\'));
				Directory.CreateDirectory (Path.GetDirectoryName (chemin));
				using (Stream src = asm.GetManifestResourceStream (nom))
				using (FileStream dst = File.Create (chemin))
				{
					src.CopyTo (dst);
				}
			}

			var categories = new List<string> ();
			int nbGif = 0;

			// GIF fournies (projet RPI2DMD et creations OrbitDMD) : les categories choisies
			var fournies = CategoriesFournies ();
			List<string> choisies = r.GifFournies == "*"
				? fournies.Select (c => c.Key).Where (ParDefaut).ToList ()
				: r.GifFournies.Split (',').Select (x => x.Trim ()).Where (x => fournies.Any (c => c.Key == x)).ToList ();
			if (choisies.Count > 0)
			{
				foreach (string nom in asm.GetManifestResourceNames ().Where (n => n.StartsWith ("gifs/")))
				{
					string[] p = nom.Split ('/');		// gifs/<categorie>/<fichier>
					if (p.Length != 3 || !choisies.Contains (p[1])) continue;
					string chemin = Path.Combine (dossier, "gif", p[1], p[2]);
					Directory.CreateDirectory (Path.GetDirectoryName (chemin));
					using (Stream src = asm.GetManifestResourceStream (nom))
					using (FileStream dst = File.Create (chemin))
					{
						src.CopyTo (dst);
					}
					nbGif++;
				}
				categories.AddRange (choisies.Take (20));
				journal ("GIF fournies : " + nbGif + " dans " + choisies.Count + " catégorie(s)");
			}

			if (r.DossierGif != "" && Directory.Exists (r.DossierGif))
			{
				var inactives = new HashSet<string> (r.CategoriesInactives.Split (',').Select (x => x.Trim ()).Where (x => x != ""),
								      StringComparer.OrdinalIgnoreCase);
				nbGif += ImporterGif (r.DossierGif, Path.Combine (dossier, "gif"), inactives, categories, journal);
			}
			if (categories.Count == 0)
			{
				categories.Add ("Accueil");	// rien d'autre a montrer : les animations OrbitDMD
			}

			Directory.CreateDirectory (Path.Combine (dossier, "Config"));
			File.WriteAllText (Path.Combine (dossier, "Config", "config.txt"), Config (r, categories), new UTF8Encoding (false));
			return nbGif;
		}

		public static bool EstAccueil (string categorie) => string.Equals (categorie, "Accueil", StringComparison.OrdinalIgnoreCase);

		// categories de GIF integrees a l'installateur, avec leur nombre de GIF
		public static List<KeyValuePair<string, int>> CategoriesFournies ()
		{
			return Assembly.GetExecutingAssembly ().GetManifestResourceNames ()
				.Where (n => n.StartsWith ("gifs/")).Select (n => n.Split ('/')).Where (p => p.Length == 3)
				.GroupBy (p => p[1]).OrderBy (g => g.Key, StringComparer.OrdinalIgnoreCase)
				.Select (g => new KeyValuePair<string, int> (g.Key, g.Count ())).ToList ();
		}

		// cochee au depart (les categories pour adultes ne le sont pas)
		public static bool ParDefaut (string categorie) => !Regex.IsMatch (categorie, "xxx|mature|adult", RegexOptions.IgnoreCase);

		// categories du dossier de l'utilisateur : ses GIF -> "Mes_GIF", chaque sous-dossier -> une
		// categorie a son nom (noms rendus surs et uniques) ; "Accueil" = ecrans de demarrage
		public static List<KeyValuePair<string, string[]>> Groupes (string source)
		{
			var groupes = new List<KeyValuePair<string, string[]>> ();
			var vus = new HashSet<string> (StringComparer.OrdinalIgnoreCase);
			Action<string, string[]> ajouter = (nom, fichiers) =>
			{
				string cat = nom;
				for (int k = 2; vus.Contains (cat); k++) cat = (nom.Length > 28 ? nom.Substring (0, 28) : nom) + "_" + k;
				vus.Add (cat);
				groupes.Add (new KeyValuePair<string, string[]> (cat, fichiers));
			};

			Func<string[], string[]> tri = l => l.Where (f => !Regex.IsMatch (Path.GetFileName (f), "rpi2dmd|rpiteam", RegexOptions.IgnoreCase)).ToArray ();
			string[] dessus = tri (Directory.GetFiles (source, "*.gif"));
			if (dessus.Length > 0) ajouter ("Mes_GIF", dessus);
			foreach (string sous in Directory.GetDirectories (source).OrderBy (d => d, StringComparer.OrdinalIgnoreCase))
			{
				string[] f = tri (Directory.GetFiles (sous, "*.gif", SearchOption.AllDirectories));
				if (f.Length > 0) ajouter (NomCategorie (Path.GetFileName (sous)), f);
			}
			return groupes;
		}

		// copie les GIF ; les categories affichees (20 au plus) sont ajoutees a "categories"
		public static int ImporterGif (string source, string destGif, HashSet<string> inactives,
					       List<string> categories, Action<string> journal)
		{
			int total = 0;
			foreach (var g in Groupes (source))
			{
				// un dossier "Accueil" rejoint les ecrans de demarrage deja fournis (sans doublon)
				bool accueil = EstAccueil (g.Key);
				string dossierCat = Path.Combine (destGif, accueil ? "Accueil" : g.Key);
				int n = 0, deja = 0;
				foreach (string f in g.Value)
				{
					var info = new FileInfo (f);
					if (info.Length < 13 || info.Length > 16 * 1024 * 1024) continue;	// le panneau lit 16 Mo au plus
					byte[] tete = new byte[6];
					using (FileStream fs = File.OpenRead (f)) fs.Read (tete, 0, 6);
					if (Encoding.ASCII.GetString (tete, 0, 4) != "GIF8") continue;
					if (Regex.IsMatch (Path.GetFileName (f), "rpi2dmd|rpiteam", RegexOptions.IgnoreCase)) continue;	// logos de l'ancien projet : retires

					string nom = NomFichier (Path.GetFileNameWithoutExtension (f));
					if (File.Exists (Path.Combine (dossierCat, nom + ".gif")))
					{
						deja++;			// deja fournie par l'installateur (meme nom)
						continue;
					}
					string unique = nom;
					for (int k = 2; File.Exists (Path.Combine (dossierCat, unique + ".gif")); k++) unique = nom + "_" + k;
					Directory.CreateDirectory (dossierCat);
					File.Copy (f, Path.Combine (dossierCat, unique + ".gif"));
					n++;
				}
				total += n;

				if (accueil)
				{
					journal ("Écrans d'accueil : " + n + " GIF ajoutée(s)" + (deja > 0 ? ", " + deja + " déjà présente(s)" : ""));
					continue;
				}
				if (n == 0)
				{
					if (deja > 0) journal ("Catégorie " + g.Key + " : déjà fournie (" + deja + " GIF)");
					continue;
				}

				bool deja_affichee = categories.Contains (g.Key, StringComparer.OrdinalIgnoreCase);
				bool affichee = deja_affichee || !inactives.Contains (g.Key);
				if (affichee && !deja_affichee)
				{
					if (categories.Count < 20) categories.Add (g.Key);
					else affichee = false;		// le panneau en affiche 20 au plus
				}
				journal ("Catégorie " + g.Key + " : " + n + " GIF" + (deja > 0 ? " (+" + deja + " déjà fournies)" : "") +
					 (affichee ? "" : " (copiée, pas affichée)"));
			}
			return total;
		}

		public static string NomCategorie (string s)
		{
			string n = new string (Sans_accents (s).Select (c => char.IsLetterOrDigit (c) && c < 128 || c == '-' || c == '_' ? c : '_').ToArray ()).Trim ('_');
			if (n.Length > 32) n = n.Substring (0, 32);
			return n == "" ? "Categorie" : n;
		}

		static string NomFichier (string s)
		{
			string n = new string (Sans_accents (s).Select (c => char.IsLetterOrDigit (c) && c < 128 || c == '-' || c == '_' ? c : '_').ToArray ());
			if (n.Length > 90) n = n.Substring (0, 90);
			return n == "" ? "animation" : n;
		}

		static string Sans_accents (string s)
		{
			var sb = new StringBuilder ();
			foreach (char c in s.Normalize (NormalizationForm.FormD))
			{
				if (CharUnicodeInfo.GetUnicodeCategory (c) != UnicodeCategory.NonSpacingMark) sb.Append (c);
			}
			return sb.ToString ().Normalize (NormalizationForm.FormC);
		}
	}

	// --- Windows : disques, Wi-Fi, reseau, PowerShell ---

	class ChoixCategorie
	{
		public string Nom;
		public int Nombre;
		public override string ToString () => Nom + "  (" + Nombre + " GIF)";
	}

	class Disque
	{
		public int Numero;
		public string Modele, Bus, Volumes;
		public long Taille;

		public override string ToString ()
		{
			string go = (Taille / 1e9).ToString ("0.#", CultureInfo.GetCultureInfo ("fr-FR"));
			return Modele + " — " + go + " Go" + (Volumes != "" ? " — " + Volumes : "") + "  (disque " + Numero + ")";
		}
	}

	static class Systeme
	{
		public static string PowerShell (string script)
		{
			string commande = "[Console]::OutputEncoding = [Text.Encoding]::UTF8; " + script;
			var psi = new ProcessStartInfo ("powershell.exe",
				"-NoProfile -NonInteractive -ExecutionPolicy Bypass -EncodedCommand " +
				Convert.ToBase64String (Encoding.Unicode.GetBytes (commande)))
			{
				UseShellExecute = false, CreateNoWindow = true,
				RedirectStandardOutput = true, RedirectStandardError = true,
				StandardOutputEncoding = Encoding.UTF8
			};
			using (Process p = Process.Start (psi))
			{
				string sortie = p.StandardOutput.ReadToEnd ();
				p.WaitForExit ();
				return sortie.Trim ();
			}
		}

		// cartes SD et disques USB, jamais le disque de Windows
		public static List<Disque> Disques ()
		{
			string json = PowerShell (
				"$r = foreach ($d in Get-Disk) { " +
				"  if (@('USB','SD','MMC') -contains [string]$d.BusType -and -not $d.IsSystem -and -not $d.IsBoot -and $d.Size -gt 0) { " +
				"    $v = @(Get-Partition -DiskNumber $d.Number -ErrorAction SilentlyContinue | Get-Volume -ErrorAction SilentlyContinue | " +
				"      ForEach-Object { if ($_.DriveLetter) { \"$($_.DriveLetter): $($_.FileSystemLabel)\".Trim() } elseif ($_.FileSystemLabel) { $_.FileSystemLabel } }) -join ', '; " +
				"    [pscustomobject]@{ Numero = $d.Number; Modele = [string]$d.FriendlyName; Taille = [int64]$d.Size; Bus = [string]$d.BusType; Volumes = $v } } }; " +
				"ConvertTo-Json -InputObject @($r) -Compress");
			var liste = new List<Disque> ();
			if (json == "") return liste;
			var lus = new JavaScriptSerializer ().Deserialize<object[]> (json);
			foreach (Dictionary<string, object> d in lus)
			{
				liste.Add (new Disque
				{
					Numero = Convert.ToInt32 (d["Numero"]), Modele = (string) d["Modele"],
					Taille = Convert.ToInt64 (d["Taille"]), Bus = (string) d["Bus"], Volumes = (string) d["Volumes"] ?? ""
				});
			}
			return liste;
		}

		static string Netsh (string arguments)
		{
			try
			{
				var psi = new ProcessStartInfo ("netsh.exe", arguments)
				{
					UseShellExecute = false, CreateNoWindow = true, RedirectStandardOutput = true,
					StandardOutputEncoding = Encoding.GetEncoding (CultureInfo.CurrentCulture.TextInfo.OEMCodePage)
				};
				using (Process p = Process.Start (psi))
				{
					string s = p.StandardOutput.ReadToEnd ();
					p.WaitForExit ();
					return s;
				}
			}
			catch { return ""; }
		}

		// reseaux Wi-Fi visibles depuis ce PC (vide si le PC n'a pas de Wi-Fi), celui du PC en premier
		public static List<string> ReseauxWifi ()
		{
			var liste = new List<string> ();
			Match actuel = Regex.Match (Netsh ("wlan show interfaces"), @"^\s*SSID\s*:\s*(.+?)\s*$", RegexOptions.Multiline);
			if (actuel.Success) liste.Add (actuel.Groups[1].Value);
			foreach (Match m in Regex.Matches (Netsh ("wlan show networks"), @"^SSID \d+\s*:\s*(.+?)\s*$", RegexOptions.Multiline))
			{
				if (m.Groups[1].Value != "" && !liste.Contains (m.Groups[1].Value)) liste.Add (m.Groups[1].Value);
			}
			return liste;
		}

		// masque, passerelle et DNS du reseau de ce PC (pour preremplir l'adresse fixe)
		public static string[] ReseauDuPC ()
		{
			foreach (NetworkInterface ni in NetworkInterface.GetAllNetworkInterfaces ())
			{
				if (ni.OperationalStatus != OperationalStatus.Up || ni.NetworkInterfaceType == NetworkInterfaceType.Loopback) continue;
				IPInterfaceProperties p = ni.GetIPProperties ();
				var gw = p.GatewayAddresses.Select (g => g.Address).FirstOrDefault (a => a.AddressFamily == AddressFamily.InterNetwork && !a.Equals (IPAddress.Any));
				var ip = p.UnicastAddresses.FirstOrDefault (u => u.Address.AddressFamily == AddressFamily.InterNetwork);
				if (gw == null || ip == null) continue;
				var dns = p.DnsAddresses.FirstOrDefault (a => a.AddressFamily == AddressFamily.InterNetwork) ?? gw;
				return new[] { ip.Address.ToString (), ip.IPv4Mask.ToString (), gw.ToString (), dns.ToString () };
			}
			return null;
		}

		// recherche d'une ville (service de geocodage d'Open-Meteo)
		public static List<Dictionary<string, object>> ChercherVille (string nom)
		{
			ServicePointManager.SecurityProtocol |= SecurityProtocolType.Tls12;
			using (var wc = new WebClient ())
			{
				wc.Encoding = Encoding.UTF8;
				wc.Headers[HttpRequestHeader.UserAgent] = "OrbitDMD-Installateur";
				string json = wc.DownloadString ("https://geocoding-api.open-meteo.com/v1/search?count=8&language=fr&format=json&name=" + Uri.EscapeDataString (nom));
				var res = new JavaScriptSerializer ().Deserialize<Dictionary<string, object>> (json);
				var liste = new List<Dictionary<string, object>> ();
				object r;
				if (res.TryGetValue ("results", out r) && r is object[])
				{
					foreach (object o in (object[]) r) liste.Add ((Dictionary<string, object>) o);
				}
				return liste;
			}
		}
	}

	// --- fenetre ---

	class Assistant : Form
	{
		readonly string m_DossierEssai;		// /dossier: : ecrire dans ce dossier au lieu d'une carte
		readonly Reponses R = new Reponses ();
		readonly List<Panel> m_Pages = new List<Panel> ();
		readonly string[] m_Titres = { "Bienvenue", "Les panneaux", "La connexion", "Accès, heure et météo", "La carte SD", "Création de la carte" };
		int m_Page;

		Label m_Etape, m_TitrePage;
		Button m_Precedent, m_Suivant, m_Annuler;
		readonly ToolTip m_Aide = new ToolTip ();

		// controles lus a la validation
		TextBox tNom, tPsk, tMdp1, tMdp2, tIp, tMasque, tPasserelle, tDns, tVille, tDossierGif;
		ComboBox cPreset, cLargeur, cHauteur, cOrdre, cSsid, cPays, cFuseau, cDisque;
		NumericUpDown nChaine, nParallele, nRalenti, nLumJour, nLumNuit, nNuitDebut, nNuitFin;
		RadioButton rAuto, rFixe, rConnAuto, rConnWifi, rConnCable;
		ListBox lVilles;
		Label lLieu, lGif, lDisqueInfo, lAdresse;
		CheckBox kEffacer;
		CheckedListBox kCategories, kFournies;
		Action<string> m_ChoisirGif;
		public string GifEssai;		// /gif: (captures) : dossier de GIF a montrer a l'etape de la carte
		Panel pAvance, pFixe;
		ProgressBar m_Progression;
		TextBox m_Journal;
		Button bEjecter, bOuvrir;
		string m_LettreCarte;

		static readonly Color Bleu = Color.FromArgb (1, 86, 209), Fond = Color.FromArgb (10, 18, 32), Gris = Color.FromArgb (90, 106, 130);

		// echelle de l'ecran (100 %, 125 %, 150 %...) : les tailles en pixels sont donnees pour 100 %
		static float s_Echelle = 1f;
		static string Version => Construction.Numero + " (" + Construction.Date + ")";	// build/version.cs, ecrit par construire.sh
		static int P (int pixels) => (int) Math.Round (pixels * s_Echelle);
		static Size Z (int largeur, int hauteur) => new Size (P (largeur), P (hauteur));

		public Assistant (string dossierEssai)
		{
			m_DossierEssai = dossierEssai;
			using (Graphics g = CreateGraphics ()) s_Echelle = g.DpiX / 96f;

			Text = "Installateur OrbitDMD " + Version;
			Font = new Font ("Segoe UI", 9.75f);
			AutoScaleMode = AutoScaleMode.None;	// les polices suivent deja l'echelle ; le reste passe par P ()
			ClientSize = Z (820, 740);
			MinimumSize = Z (720, 600);
			StartPosition = FormStartPosition.CenterScreen;
			BackColor = Color.White;
			try { Icon = Icon.ExtractAssociatedIcon (Application.ExecutablePath); } catch { }

			// en-tete
			var tete = new Panel { Dock = DockStyle.Top, Height = P (92), BackColor = Fond };
			var logo = new PictureBox { SizeMode = PictureBoxSizeMode.Zoom, Location = new Point (P (22), P (14)), Size = Z (190, 63) };
			using (Stream s = Assembly.GetExecutingAssembly ().GetManifestResourceStream ("logo.png")) if (s != null) logo.Image = Image.FromStream (s);
			m_Etape = new Label { ForeColor = Color.FromArgb (151, 166, 189), AutoSize = true, Location = new Point (P (240), P (24)), Font = new Font ("Segoe UI", 9f) };
			m_TitrePage = new Label { ForeColor = Color.White, AutoSize = true, Location = new Point (P (238), P (44)), Font = new Font ("Segoe UI Semibold", 15f) };
			tete.Controls.AddRange (new Control[] { logo, m_Etape, m_TitrePage });

			// pied
			var pied = new Panel { Dock = DockStyle.Bottom, Height = P (58), BackColor = Color.FromArgb (243, 246, 251) };
			m_Annuler = Bouton ("Quitter", false);
			m_Precedent = Bouton ("< Précédent", false);
			m_Suivant = Bouton ("Suivant >", true);
			m_Annuler.Click += (s, e) => Close ();
			m_Precedent.Click += (s, e) => Afficher (m_Page - 1);
			m_Suivant.Click += (s, e) => Suivant ();
			var credit = new Label { Text = "Développé par Ulrik Adam · www.nexadam.be", ForeColor = Gris, AutoSize = true, Font = new Font ("Segoe UI", 8.5f) };
			pied.Controls.AddRange (new Control[] { m_Annuler, m_Precedent, m_Suivant, credit });
			pied.Resize += (s, e) =>
			{
				m_Suivant.Location = new Point (pied.Width - m_Suivant.Width - P (18), P (12));
				m_Precedent.Location = new Point (m_Suivant.Left - m_Precedent.Width - P (8), P (12));
				m_Annuler.Location = new Point (P (18), P (12));
				credit.Location = new Point (m_Annuler.Right + P (14), P (21));
			};

			var corps = new Panel { Dock = DockStyle.Fill, Padding = new Padding (P (28), P (18), P (28), P (8)) };
			Controls.Add (corps);
			Controls.Add (pied);
			Controls.Add (tete);

			m_Pages.Add (PageBienvenue ());
			m_Pages.Add (PagePanneaux ());
			m_Pages.Add (PageWifi ());
			m_Pages.Add (PageAcces ());
			m_Pages.Add (PageCarte ());
			m_Pages.Add (PageCreation ());
			foreach (Panel p in m_Pages) { p.Dock = DockStyle.Fill; p.Visible = false; corps.Controls.Add (p); }
			Afficher (0);
		}

		Button Bouton (string texte, bool principal)
		{
			var b = new Button { Text = texte, AutoSize = true, MinimumSize = Z (110, 34), FlatStyle = FlatStyle.Flat, Cursor = Cursors.Hand };
			b.FlatAppearance.BorderColor = principal ? Bleu : Color.FromArgb (211, 220, 233);
			if (principal) { b.BackColor = Bleu; b.ForeColor = Color.White; }
			else b.BackColor = Color.White;
			return b;
		}

		// --- construction des pages ---

		static TableLayoutPanel Grille ()
		{
			var t = new TableLayoutPanel { ColumnCount = 2, AutoScroll = true, Padding = new Padding (P (0), P (4), P (12), P (0)) };
			t.ColumnStyles.Add (new ColumnStyle (SizeType.Absolute, P (230)));
			t.ColumnStyles.Add (new ColumnStyle (SizeType.Percent, 100));
			return t;
		}

		// grille dans une grille (reglages avances) : sans defilement, a la taille de son contenu
		static TableLayoutPanel Imbriquee ()
		{
			TableLayoutPanel t = Grille ();
			t.AutoScroll = false;
			t.AutoSize = true;
			t.AutoSizeMode = AutoSizeMode.GrowAndShrink;
			t.Padding = new Padding (0);
			t.Margin = new Padding (0);
			t.Visible = false;
			return t;
		}

		static Panel Enveloppe (Control c) { var p = new Panel (); c.Dock = DockStyle.Fill; p.Controls.Add (c); return p; }

		static Label Texte (string t, int largeur = 640, bool gris = false) =>
			new Label { Text = t, AutoSize = true, MaximumSize = Z (largeur, 0), ForeColor = gris ? Gris : Color.FromArgb (14, 26, 43), Margin = new Padding (P (0), P (4), P (0), P (8)) };

		void Ligne (TableLayoutPanel t, string libelle, Control c, string aide = null)
		{
			var l = new Label { Text = libelle, AutoSize = true, Margin = new Padding (P (0), P (9), P (8), P (4)), Font = new Font ("Segoe UI Semibold", 9.75f) };
			c.Margin = new Padding (P (0), P (5), P (0), P (4));
			int ligne = t.RowCount++;
			t.Controls.Add (l, 0, ligne);
			if (aide == null)
			{
				t.Controls.Add (c, 1, ligne);
				return;
			}
			var f = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = new Padding (0) };
			f.Controls.Add (c);
			f.Controls.Add (new Label { Text = aide, AutoSize = true, MaximumSize = Z (400, 0), ForeColor = Gris, Font = new Font ("Segoe UI", 8.5f), Margin = new Padding (P (0), P (0), P (0), P (4)) });
			t.Controls.Add (f, 1, ligne);
		}

		static void Large (TableLayoutPanel t, Control c)
		{
			int ligne = t.RowCount++;
			t.Controls.Add (c, 0, ligne);
			t.SetColumnSpan (c, 2);
		}

		static ComboBox Liste (int largeur, bool modifiable = false) =>
			new ComboBox { Width = P (largeur), DropDownStyle = modifiable ? ComboBoxStyle.DropDown : ComboBoxStyle.DropDownList };

		static NumericUpDown Nombre (int min, int max, int val, int largeur = 70) =>
			new NumericUpDown { Minimum = min, Maximum = max, Value = val, Width = P (largeur) };

		Panel PageBienvenue ()
		{
			var t = Grille ();
			Large (t, new Label { Text = "Bienvenue !", AutoSize = true, Font = new Font ("Segoe UI Semibold", 13f), Margin = new Padding (P (0), P (0), P (0), P (6)) });
			Large (t, Texte ("Cet assistant prépare une carte micro-SD pour un Raspberry Pi 3 relié à des panneaux LED HUB75. " +
				"OrbitDMD affiche vos GIF, l'heure et la météo, et se règle depuis une page web, sans Linux."));
			Large (t, Texte ("Il vous faut :\n" +
				"  •  une carte micro-SD (32 Go ou moins conseillé) et un lecteur de carte ;\n" +
				"  •  le nom et le mot de passe de votre Wi-Fi, ou un câble réseau ;\n" +
				"  •  un Raspberry Pi 3 avec une carte adaptatrice HUB75 (brochage « regular »)."));
			Large (t, Texte ("Attention : tout le contenu de la carte choisie sera effacé.", gris: false));
			if (m_DossierEssai != null)
				Large (t, Texte ("Mode essai : rien ne sera formaté, la carte sera écrite dans " + m_DossierEssai, gris: true));
			return Enveloppe (t);
		}

		static readonly int[][] Presets =
		{
			new[] { 64, 32, 2, 1 }, new[] { 64, 32, 1, 1 }, new[] { 64, 32, 4, 1 },
			new[] { 64, 64, 1, 1 }, new[] { 64, 64, 2, 1 }, new[] { 32, 16, 1, 1 }
		};
		static readonly string[] NomsPresets =
		{
			"2 panneaux 64×32 côte à côte (128×32)", "1 panneau 64×32", "4 panneaux 64×32 côte à côte (256×32)",
			"1 panneau 64×64", "2 panneaux 64×64 côte à côte (128×64)", "1 panneau 32×16", "Autre (réglages avancés)"
		};

		Panel PagePanneaux ()
		{
			var t = Grille ();
			tNom = new TextBox { Text = R.Nom, Width = P (200), MaxLength = 31 };
			lAdresse = new Label { AutoSize = true, ForeColor = Gris, Font = new Font ("Segoe UI", 8.5f) };
			tNom.TextChanged += (s, e) => lAdresse.Text = "Adresse de sa page : http://" + tNom.Text.ToLowerInvariant () + ".local/";
			var fNom = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = new Padding (0) };
			fNom.Controls.Add (tNom);
			fNom.Controls.Add (lAdresse);
			Ligne (t, "Nom du panneau", fNom);
			tNom.Text = R.Nom;

			cPreset = Liste (330);
			cPreset.Items.AddRange (NomsPresets);
			cPreset.SelectedIndex = 0;
			Ligne (t, "Panneaux", cPreset);

			var a = Imbriquee ();
			cLargeur = Liste (90); cLargeur.Items.AddRange (new object[] { "16", "32", "64", "128" }); cLargeur.SelectedItem = "64";
			cHauteur = Liste (90); cHauteur.Items.AddRange (new object[] { "16", "32", "64" }); cHauteur.SelectedItem = "32";
			nChaine = Nombre (1, 8, 2);
			nParallele = Nombre (1, 3, 1);
			Ligne (a, "Largeur d'un panneau", cLargeur);
			Ligne (a, "Hauteur d'un panneau", cHauteur);
			Ligne (a, "Panneaux à la suite", nChaine, "Reliés les uns aux autres (sortie vers entrée).");
			Ligne (a, "Sorties en parallèle", nParallele, "1 si tout est branché sur la même sortie de l'adaptateur.");
			pAvance = a;
			Large (t, pAvance);
			cPreset.SelectedIndexChanged += (s, e) =>
			{
				bool autre = cPreset.SelectedIndex == Presets.Length;
				pAvance.Visible = autre;
				if (!autre)
				{
					int[] p = Presets[cPreset.SelectedIndex];
					cLargeur.SelectedItem = p[0].ToString (); cHauteur.SelectedItem = p[1].ToString ();
					nChaine.Value = p[2]; nParallele.Value = p[3];
				}
			};

			nRalenti = Nombre (0, 4, 2);
			Ligne (t, "Ralentissement des sorties", nRalenti, "2 convient à la plupart des panneaux. Augmentez si l'image scintille ou se décale.");
			cOrdre = Liste (90); cOrdre.Items.AddRange (new object[] { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR" }); cOrdre.SelectedItem = "RGB";
			Ligne (t, "Ordre des couleurs", cOrdre, "À changer si le rouge s'affiche en vert ou en bleu.");

			nLumJour = Nombre (5, 100, 60);
			nLumNuit = Nombre (0, 100, 20);
			nNuitDebut = Nombre (0, 23, 23, 60);
			nNuitFin = Nombre (0, 23, 7, 60);
			var fNuit = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fNuit.Controls.AddRange (new Control[] { nLumNuit, new Label { Text = "%   de", AutoSize = true, Margin = new Padding (P (4), P (8), P (4), P (0)) },
				nNuitDebut, new Label { Text = "h à", AutoSize = true, Margin = new Padding (P (4), P (8), P (4), P (0)) }, nNuitFin,
				new Label { Text = "h", AutoSize = true, Margin = new Padding (P (4), P (8), P (0), P (0)) } });
			var fJour = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fJour.Controls.AddRange (new Control[] { nLumJour, new Label { Text = "%", AutoSize = true, Margin = new Padding (P (4), P (8), P (0), P (0)) } });
			Ligne (t, "Luminosité le jour", fJour);
			Ligne (t, "Luminosité la nuit", fNuit, "Réglable ensuite heure par heure depuis la page web.");
			return Enveloppe (t);
		}

		Panel PageWifi ()
		{
			var t = Grille ();
			Large (t, Texte ("Le panneau se connecte par câble réseau ou par Wi-Fi (2,4 GHz : le Raspberry Pi 3 ne voit pas le 5 GHz). " +
				"Le mot de passe Wi-Fi n'est enregistré que sur la carte.", gris: true));

			rConnAuto = new RadioButton { Text = "Automatique : câble s'il est branché au démarrage, sinon Wi-Fi — conseillé", AutoSize = true, Checked = true };
			rConnWifi = new RadioButton { Text = "Wi-Fi seulement (démarrage un peu plus rapide)", AutoSize = true };
			rConnCable = new RadioButton { Text = "Câble réseau seulement (Wi-Fi facultatif)", AutoSize = true };
			var fConn = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, Margin = new Padding (0), WrapContents = false };
			fConn.Controls.AddRange (new Control[] { rConnAuto, rConnWifi, rConnCable });
			Ligne (t, "Connexion", fConn);

			cSsid = Liste (260, true);
			cSsid.Items.AddRange (Systeme.ReseauxWifi ().Cast<object> ().ToArray ());
			if (cSsid.Items.Count > 0) cSsid.SelectedIndex = 0;
			Ligne (t, "Réseau Wi-Fi", cSsid, cSsid.Items.Count > 0 ? "Choisissez dans la liste ou tapez le nom." : "Tapez le nom exact du réseau.");

			tPsk = new TextBox { Width = P (260), UseSystemPasswordChar = true, MaxLength = 63 };
			var voir = new CheckBox { Text = "afficher", AutoSize = true, Margin = new Padding (P (8), P (7), P (0), P (0)) };
			voir.CheckedChanged += (s, e) => tPsk.UseSystemPasswordChar = !voir.Checked;
			var fPsk = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fPsk.Controls.AddRange (new Control[] { tPsk, voir });
			Ligne (t, "Mot de passe Wi-Fi", fPsk);

			cPays = Liste (200);
			foreach (string[] p in Carte.Pays) cPays.Items.Add (p[1] + " (" + p[0] + ")");
			string region = RegionInfo.CurrentRegion.TwoLetterISORegionName;
			int idx = Array.FindIndex (Carte.Pays, p => p[0] == region);
			cPays.SelectedIndex = idx >= 0 ? idx : 0;
			Ligne (t, "Pays", cPays, "Pour les canaux Wi-Fi autorisés.");

			rAuto = new RadioButton { Text = "Automatique (donnée par la box) — conseillé", AutoSize = true, Checked = true };
			rFixe = new RadioButton { Text = "Fixe", AutoSize = true };
			var fMode = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, Margin = new Padding (0), WrapContents = false };
			fMode.Controls.AddRange (new Control[] { rAuto, rFixe });
			Ligne (t, "Adresse IP", fMode);

			var g = Imbriquee ();
			string[] pc = Systeme.ReseauDuPC ();
			tIp = new TextBox { Width = P (150) };
			tMasque = new TextBox { Width = P (150), Text = pc != null ? pc[1] : "255.255.255.0" };
			tPasserelle = new TextBox { Width = P (150), Text = pc != null ? pc[2] : "" };
			tDns = new TextBox { Width = P (150), Text = pc != null ? pc[3] : "" };
			string exemple = pc != null ? Regex.Replace (pc[0], @"\d+$", "150") : "192.168.1.150";
			Ligne (g, "Adresse du panneau", tIp, "Par exemple " + exemple + " (libre sur votre réseau).");
			Ligne (g, "Masque", tMasque);
			Ligne (g, "Passerelle (box)", tPasserelle);
			Ligne (g, "Serveur DNS", tDns);
			pFixe = g;
			Large (t, pFixe);
			rFixe.CheckedChanged += (s, e) => pFixe.Visible = rFixe.Checked;
			return Enveloppe (t);
		}

		Panel PageAcces ()
		{
			var t = Grille ();
			Large (t, Texte ("Ce mot de passe protège les modifications faites depuis la page web du panneau.", gris: true));
			tMdp1 = new TextBox { Width = P (220), UseSystemPasswordChar = true, MaxLength = 63 };
			tMdp2 = new TextBox { Width = P (220), UseSystemPasswordChar = true, MaxLength = 63 };
			Ligne (t, "Mot de passe de la page", tMdp1, "8 caractères minimum, sans espace.");
			Ligne (t, "Confirmation", tMdp2);

			cFuseau = Liste (220);
			cFuseau.Items.AddRange (Carte.Fuseaux);
			string region = RegionInfo.CurrentRegion.TwoLetterISORegionName;
			cFuseau.SelectedItem = region == "BE" ? "Europe/Brussels" : region == "CH" ? "Europe/Zurich" : region == "LU" ? "Europe/Luxembourg"
				: region == "GB" ? "Europe/London" : "Europe/Paris";
			Ligne (t, "Fuseau horaire", cFuseau, "Heure d'été automatique.");

			tVille = new TextBox { Width = P (220) };
			var bChercher = Bouton ("Chercher", false);
			bChercher.MinimumSize = Z (90, 28);
			var fVille = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fVille.Controls.AddRange (new Control[] { tVille, bChercher });
			Ligne (t, "Ville pour la météo", fVille, "Facultatif : se choisit aussi plus tard sur la page Météo.");
			lVilles = new ListBox { Width = P (420), Height = P (100), Visible = false };
			lLieu = new Label { AutoSize = true, ForeColor = Bleu, Margin = new Padding (P (0), P (4), P (0), P (4)) };
			Ligne (t, "", lVilles);
			Ligne (t, "", lLieu);

			List<Dictionary<string, object>> resultats = null;
			EventHandler chercher = async (s, e) =>
			{
				string q = tVille.Text.Trim ();
				if (q.Length < 2) return;
				bChercher.Enabled = false;
				lLieu.Text = "Recherche…";
				try
				{
					resultats = await Task.Run (() => Systeme.ChercherVille (q));
					lVilles.Items.Clear ();
					foreach (var v in resultats)
					{
						object a1, pays;
						v.TryGetValue ("admin1", out a1);
						v.TryGetValue ("country", out pays);
						lVilles.Items.Add (v["name"] + " — " + string.Join (", ", new[] { a1 as string, pays as string }.Where (x => !string.IsNullOrEmpty (x))));
					}
					lVilles.Visible = resultats.Count > 0;
					lLieu.Text = resultats.Count > 0 ? "Choisissez votre ville dans la liste." : "Aucune ville trouvée.";
				}
				catch
				{
					lLieu.Text = "Recherche impossible (pas d'Internet ?). La ville se choisira sur la page Météo.";
				}
				bChercher.Enabled = true;
			};
			bChercher.Click += chercher;
			tVille.KeyDown += (s, e) => { if (e.KeyCode == Keys.Enter) { e.SuppressKeyPress = true; chercher (s, e); } };
			lVilles.SelectedIndexChanged += (s, e) =>
			{
				if (resultats == null || lVilles.SelectedIndex < 0) return;
				var v = resultats[lVilles.SelectedIndex];
				object cc;
				v.TryGetValue ("country_code", out cc);
				string lieu = v["name"] + (cc != null ? " (" + cc + ")" : "");
				while (Encoding.UTF8.GetByteCount (lieu) > 47) lieu = lieu.Substring (0, lieu.Length - 1);
				R.Lieu = lieu.Replace ("\"", "").Replace ("\\", "");
				R.Latitude = Convert.ToDouble (v["latitude"]).ToString ("0.####", CultureInfo.InvariantCulture);
				R.Longitude = Convert.ToDouble (v["longitude"]).ToString ("0.####", CultureInfo.InvariantCulture);
				lLieu.Text = "Météo de : " + R.Lieu;
			};
			return Enveloppe (t);
		}

		Panel PageCarte ()
		{
			var t = Grille ();
			Large (t, Texte ("Insérez la carte micro-SD dans le lecteur, puis choisissez-la. Seules les cartes et clés amovibles sont proposées.", gris: true));
			cDisque = Liste (380);
			var bActualiser = Bouton ("Actualiser", false);
			bActualiser.MinimumSize = Z (100, 28);
			bActualiser.Click += (s, e) => ListerDisques ();
			var fDisque = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fDisque.Controls.AddRange (new Control[] { cDisque, bActualiser });
			Ligne (t, "Carte", fDisque);
			lDisqueInfo = new Label { AutoSize = true, MaximumSize = Z (440, 0), ForeColor = Gris };
			Ligne (t, "", lDisqueInfo);
			cDisque.SelectedIndexChanged += (s, e) => InfoDisque ();

			// GIF integrees a l'installateur (projet RPI2DMD + creations OrbitDMD)
			kFournies = new CheckedListBox { Width = P (380), Height = P (130), CheckOnClick = true, IntegralHeight = false };
			foreach (var c in Carte.CategoriesFournies ())
			{
				kFournies.Items.Add (new ChoixCategorie { Nom = c.Key, Nombre = c.Value }, Carte.ParDefaut (c.Key));
			}
			var fFournies = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = new Padding (0) };
			fFournies.Controls.Add (kFournies);
			fFournies.Controls.Add (new Label { Text = "Pack de GIF animées gratuit de base de la RpiTeaM (projet RPI2DMD), intégré avec l'accord d'eLLuiGi, " +
				"et créations OrbitDMD. Les catégories cochées sont copiées et affichées. " +
				"Pack de 11 000 animations : rpiteam.carrd.co ou le forum Neo-Arcadia.",
				AutoSize = true, MaximumSize = Z (380, 0), ForeColor = Gris, Font = new Font ("Segoe UI", 8.5f) });
			Ligne (t, "GIF fournies", fFournies);

			tDossierGif = new TextBox { Width = P (330), ReadOnly = true };
			var bGif = Bouton ("Choisir…", false);
			bGif.MinimumSize = Z (90, 28);
			var bSansGif = new LinkLabel { Text = "aucun", AutoSize = true, Margin = new Padding (P (8), P (8), P (0), P (0)) };
			var fGif = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (0), WrapContents = false };
			fGif.Controls.AddRange (new Control[] { tDossierGif, bGif, bSansGif });
			Ligne (t, "Vos GIF (facultatif)", fGif, "Les GIF du dossier vont dans « Mes_GIF » ; chaque sous-dossier devient une catégorie. On peut aussi les ajouter plus tard depuis la page web.");
			lGif = new Label { AutoSize = true, ForeColor = Bleu };
			Ligne (t, "", lGif);

			// categories trouvees : cochees = affichees sur le panneau (les autres sont copiees quand meme)
			kCategories = new CheckedListBox { Width = P (380), Height = P (120), CheckOnClick = true, IntegralHeight = false };
			var fCat = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.TopDown, WrapContents = false, Margin = new Padding (0), Visible = false };
			fCat.Controls.Add (kCategories);
			fCat.Controls.Add (new Label { Text = "Décochez celles à ne pas montrer pour l'instant : elles sont copiées quand même, et s'activent depuis la page GIF.",
				AutoSize = true, MaximumSize = Z (380, 0), ForeColor = Gris, Font = new Font ("Segoe UI", 8.5f) });
			Ligne (t, "Catégories affichées", fCat);
			var lCat = t.GetControlFromPosition (0, t.RowCount - 1);	// son titre, cache avec elle
			lCat.Visible = false;
			m_ChoisirGif = chemin =>
			{
				List<KeyValuePair<string, string[]>> groupes;
				try { groupes = Carte.Groupes (chemin); }
				catch (Exception ex) { MessageBox.Show (this, "Dossier illisible : " + ex.Message, "OrbitDMD"); return; }

				tDossierGif.Text = chemin;
				kCategories.Items.Clear ();
				int total = 0, accueil = 0;
				foreach (var g in groupes)
				{
					total += g.Value.Length;
					if (Carte.EstAccueil (g.Key)) { accueil = g.Value.Length; continue; }
					bool coche = !Regex.IsMatch (g.Key, "xxx|mature|adult", RegexOptions.IgnoreCase);
					kCategories.Items.Add (new ChoixCategorie { Nom = g.Key, Nombre = g.Value.Length }, coche);
				}
				lGif.Text = total + " GIF trouvée(s) dans " + groupes.Count + " dossier(s)" +
					(accueil > 0 ? " ; « Accueil » rejoint les écrans de démarrage." : ".");
				fCat.Visible = lCat.Visible = kCategories.Items.Count > 0;
			};
			bGif.Click += (s, e) =>
			{
				using (var d = new FolderBrowserDialog { Description = "Dossier contenant vos GIF (128×32 de préférence)", ShowNewFolderButton = false })
				{
					if (d.ShowDialog (this) == DialogResult.OK) m_ChoisirGif (d.SelectedPath);
				}
			};
			bSansGif.LinkClicked += (s, e) => { tDossierGif.Text = ""; lGif.Text = ""; kCategories.Items.Clear (); fCat.Visible = lCat.Visible = false; };

			kEffacer = new CheckBox { Text = "J'ai compris que tout le contenu de cette carte sera effacé.", AutoSize = true, Margin = new Padding (P (0), P (14), P (0), P (0)) };
			Large (t, kEffacer);
			return Enveloppe (t);
		}

		Panel PageCreation ()
		{
			var t = Grille ();
			m_Progression = new ProgressBar { Width = P (640), Height = P (18), Maximum = 100 };
			Large (t, m_Progression);
			m_Journal = new TextBox { Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical, Width = P (640), Height = P (230), BackColor = Color.FromArgb (246, 248, 252), BorderStyle = BorderStyle.FixedSingle };
			Large (t, m_Journal);
			bEjecter = Bouton ("Éjecter la carte", false);
			bOuvrir = Bouton ("Ouvrir la page du panneau", false);
			bEjecter.Visible = bOuvrir.Visible = false;
			bEjecter.Click += (s, e) => Ejecter ();
			bOuvrir.Click += (s, e) => { try { Process.Start ("http://" + R.Nom.ToLowerInvariant () + ".local/"); } catch { } };
			var f = new FlowLayoutPanel { AutoSize = true, Margin = new Padding (P (0), P (8), P (0), P (0)) };
			f.Controls.AddRange (new Control[] { bEjecter, bOuvrir });
			Large (t, f);
			return Enveloppe (t);
		}

		// --- navigation ---

		void Afficher (int page)
		{
			if (page < 0 || page >= m_Pages.Count) return;
			m_Page = page;
			for (int i = 0; i < m_Pages.Count; i++) m_Pages[i].Visible = i == page;
			m_Etape.Text = page < m_Pages.Count - 1 ? "Étape " + (page + 1) + " sur " + (m_Pages.Count - 1) : "Dernière étape";
			m_TitrePage.Text = m_Titres[page];
			m_Precedent.Visible = page > 0 && page < m_Pages.Count - 1;
			m_Suivant.Text = page == m_Pages.Count - 2 ? (m_DossierEssai != null ? "Écrire l'essai" : "Créer la carte") : "Suivant >";
			m_Suivant.Visible = page < m_Pages.Count - 1;
			if (page == 4) ListerDisques ();
		}

		void Suivant ()
		{
			string erreur = Valider (m_Page);
			if (erreur != null)
			{
				MessageBox.Show (this, erreur, "OrbitDMD", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				return;
			}
			if (m_Page == m_Pages.Count - 2)
			{
				Creer ();
				return;
			}
			Afficher (m_Page + 1);
		}

		string Valider (int page)
		{
			switch (page)
			{
			case 1:
				string e = Regles.Nom (tNom.Text.Trim ());
				if (e != null) return e;
				R.Nom = tNom.Text.Trim ();
				R.Largeur = int.Parse ((string) cLargeur.SelectedItem);
				R.Hauteur = int.Parse ((string) cHauteur.SelectedItem);
				R.Chaine = (int) nChaine.Value;
				R.Parallele = (int) nParallele.Value;
				R.Ralentissement = (int) nRalenti.Value;
				R.OrdreCouleurs = (string) cOrdre.SelectedItem;
				R.LumJour = (int) nLumJour.Value;
				R.LumNuit = (int) nLumNuit.Value;
				R.NuitDebut = (int) nNuitDebut.Value;
				R.NuitFin = (int) nNuitFin.Value;
				return null;

			case 2:
				string ssid = cSsid.Text, psk = tPsk.Text;
				R.Connexion = rConnCable.Checked ? "ethernet" : rConnWifi.Checked ? "wifi" : "auto";
				if ((e = Regles.Wifi (R.Connexion, ssid, psk)) != null) return e;
				if (R.Connexion == "ethernet" && psk.Length == 0) ssid = "";	// cable seul, sans Wi-Fi
				R.Ssid = ssid;
				R.Psk = psk;
				R.Pays = Carte.Pays[cPays.SelectedIndex][0];
				R.IpFixe = rFixe.Checked;
				if (R.IpFixe)
				{
					if ((e = Regles.AdresseFixe (tIp.Text.Trim (), tMasque.Text.Trim (), tPasserelle.Text.Trim (), tDns.Text.Trim ())) != null) return e;
					R.Ip = tIp.Text.Trim (); R.Masque = tMasque.Text.Trim (); R.Passerelle = tPasserelle.Text.Trim (); R.Dns = tDns.Text.Trim ();
				}
				return null;

			case 3:
				if ((e = Regles.MotDePasse (tMdp1.Text)) != null) return e;
				if (tMdp1.Text != tMdp2.Text) return "Les deux mots de passe sont différents.";
				R.MotDePasse = tMdp1.Text;
				R.Fuseau = (string) cFuseau.SelectedItem;
				return null;

			case 4:
				R.DossierGif = tDossierGif.Text;
				var fournies = kFournies.CheckedItems.Cast<ChoixCategorie> ().Select (c => c.Nom).ToList ();
				R.GifFournies = string.Join (",", fournies);
				var choix = kCategories.Items.Cast<ChoixCategorie> ().ToList ();
				var cochees = kCategories.CheckedItems.Cast<ChoixCategorie> ().ToList ();
				int affichees = fournies.Union (cochees.Select (c => c.Nom), StringComparer.OrdinalIgnoreCase).Count ();
				if (affichees > 20) return "Le panneau affiche 20 catégories au plus : décochez-en " + (affichees - 20) + ".";
				R.CategoriesInactives = string.Join (",", choix.Where (c => !cochees.Contains (c)).Select (c => c.Nom));
				if (m_DossierEssai != null) return null;
				if (cDisque.SelectedItem == null) return "Choisissez la carte micro-SD (insérez-la puis cliquez sur Actualiser).";
				if (!kEffacer.Checked) return "Cochez la case pour confirmer que la carte peut être effacée.";
				var d = (Disque) cDisque.SelectedItem;
				var ok = MessageBox.Show (this,
					"Tout le contenu de ce disque va être effacé définitivement :\n\n" + d + "\n\nContinuer ?",
					"Effacer la carte ?", MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2);
				return ok == DialogResult.Yes ? null : "Création annulée : rien n'a été modifié.";
			}
			return null;
		}

		void ListerDisques ()
		{
			if (cDisque == null) return;
			cDisque.Items.Clear ();
			try
			{
				foreach (Disque d in Systeme.Disques ()) cDisque.Items.Add (d);
			}
			catch (Exception ex)
			{
				lDisqueInfo.Text = "Liste des disques impossible : " + ex.Message;
				return;
			}
			if (cDisque.Items.Count > 0) cDisque.SelectedIndex = 0;
			InfoDisque ();
		}

		void InfoDisque ()
		{
			var d = cDisque.SelectedItem as Disque;
			if (d == null)
			{
				lDisqueInfo.Text = m_DossierEssai != null ? "Mode essai : aucune carte nécessaire." : "Aucune carte trouvée : insérez-la, puis cliquez sur Actualiser.";
				lDisqueInfo.ForeColor = Gris;
				return;
			}
			bool gros = d.Taille > 64e9;
			lDisqueInfo.Text = gros
				? "Ce disque fait plus de 64 Go : vérifiez bien que c'est la carte SD et pas un disque de sauvegarde !"
				: "Une partition FAT32 « ORBITDMD » de " + Math.Min (31, (int) (d.Taille / 1073741824.0)) + " Go sera créée.";
			lDisqueInfo.ForeColor = gros ? Color.FromArgb (210, 59, 59) : Gris;
		}

		// --- creation ---

		void Journal (string ligne)
		{
			if (InvokeRequired) { BeginInvoke (new Action<string> (Journal), ligne); return; }
			m_Journal.AppendText (ligne + "\r\n");
		}

		void Progression (int valeur)
		{
			if (InvokeRequired) { BeginInvoke (new Action<int> (Progression), valeur); return; }
			m_Progression.Value = Math.Max (0, Math.Min (100, valeur));
		}

		async void Creer ()
		{
			var disque = cDisque.SelectedItem as Disque;
			Afficher (m_Pages.Count - 1);
			m_Annuler.Enabled = false;
			string temp = Path.Combine (Path.GetTempPath (), "OrbitDMD-" + DateTime.Now.ToString ("yyyyMMdd-HHmmss"));
			string erreur = null;

			try
			{
				Journal ("Installateur OrbitDMD " + Version + ".");
				Journal ("Préparation du contenu de la carte…");
				Progression (5);
				string contenu = m_DossierEssai ?? Path.Combine (temp, "carte");
				if (m_DossierEssai != null && Directory.Exists (contenu) && Directory.EnumerateFileSystemEntries (contenu).Any ())
					throw new Exception ("Le dossier d'essai n'est pas vide : " + contenu);
				Directory.CreateDirectory (contenu);
				int nbGif = await Task.Run (() => Carte.Preparer (R, contenu, Journal));
				Journal ("Contenu prêt" + (nbGif > 0 ? " (" + nbGif + " GIF ajoutées)." : "."));
				Progression (12);

				if (m_DossierEssai == null)
				{
					m_LettreCarte = await Task.Run (() => Ecrire (disque, contenu, temp));
				}
				Progression (100);
			}
			catch (Exception ex)
			{
				erreur = ex.Message;
			}
			finally
			{
				// le dossier temporaire contient le mot de passe Wi-Fi : on l'efface
				try { if (Directory.Exists (temp)) Directory.Delete (temp, true); } catch { }
			}

			m_Annuler.Enabled = true;
			m_Annuler.Text = "Terminer";
			if (erreur != null)
			{
				Journal ("");
				Journal ("ÉCHEC : " + erreur);
				m_TitrePage.Text = "La carte n'a pas pu être créée";
				m_Precedent.Visible = true;
				return;
			}

			m_TitrePage.Text = "La carte est prête !";
			Journal ("");
			if (m_DossierEssai != null)
			{
				Journal ("Essai terminé : contenu écrit dans " + m_DossierEssai);
				return;
			}
			Journal ("1. Éjectez la carte (bouton ci-dessous) et mettez-la dans le Raspberry Pi 3.");
			Journal ("2. Branchez l'alimentation : l'écran de démarrage s'affiche pendant la connexion au réseau.");
			Journal ("3. Après une trentaine de secondes, ouvrez http://" + R.Nom.ToLowerInvariant () + ".local/");
			Journal ("   (sinon, cherchez l'adresse du panneau dans la page de votre box).");
			bEjecter.Visible = bOuvrir.Visible = true;
		}

		// lance preparer.ps1 en administrateur et suit son journal ; rend la lettre de la carte
		string Ecrire (Disque d, string contenu, string temp)
		{
			string script = Path.Combine (temp, "preparer.ps1");
			string journal = Path.Combine (temp, "journal.txt");
			using (Stream s = Assembly.GetExecutingAssembly ().GetManifestResourceStream ("preparer.ps1"))
			using (var lecteur = new StreamReader (s))
			{
				File.WriteAllText (script, lecteur.ReadToEnd (), new UTF8Encoding (true));
			}
			File.WriteAllText (journal, "", new UTF8Encoding (true));

			Journal ("Windows va demander l'autorisation de modifier la carte…");
			string args = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File \"" + script + "\"" +
				" -Disque " + d.Numero + " -Modele \"" + d.Modele.Replace ("\"", "") + "\" -Taille " + d.Taille +
				" -Source \"" + contenu + "\" -Journal \"" + journal + "\"";
			Process p;
			try
			{
				p = Process.Start (new ProcessStartInfo ("powershell.exe", args) { UseShellExecute = true, Verb = "runas", WindowStyle = ProcessWindowStyle.Hidden });
			}
			catch (System.ComponentModel.Win32Exception)
			{
				throw new Exception ("Autorisation refusée : la carte n'a pas été modifiée.");
			}

			var etapes = new Dictionary<string, string[]>
			{
				{ "effacement", new[] { "Effacement de la carte…", "20" } },
				{ "partition", new[] { "Création de la partition…", "30" } },
				{ "formatage", new[] { "Formatage FAT32…", "40" } },
				{ "copie", new[] { "Copie des fichiers…", "55" } },
				{ "verification", new[] { "Vérification de la copie…", "90" } }
			};
			int vues = 0;
			while (true)
			{
				bool fini = p.HasExited;
				string[] lignes;
				try { lignes = File.ReadAllLines (journal, Encoding.UTF8); } catch (IOException) { lignes = new string[0]; }
				for (; vues < lignes.Length; vues++)
				{
					string l = lignes[vues].Trim ();
					if (l.StartsWith ("ETAPE "))
					{
						string[] e;
						if (etapes.TryGetValue (l.Substring (6), out e)) { Journal (e[0]); Progression (int.Parse (e[1])); }
					}
					else if (l.StartsWith ("OK "))
					{
						string[] parts = l.Split (' ');
						Journal ("Carte écrite et vérifiée (" + parts[2] + " fichiers) sur " + parts[1] + ":");
						return parts[1];
					}
					else if (l.StartsWith ("ERREUR "))
					{
						throw new Exception (l.Substring (7));
					}
				}
				if (fini) throw new Exception ("La préparation s'est arrêtée sans résultat.");
				System.Threading.Thread.Sleep (300);
			}
		}

		void Ejecter ()
		{
			if (m_LettreCarte == null) return;
			Systeme.PowerShell ("(New-Object -ComObject Shell.Application).Namespace(17).ParseName('" + m_LettreCarte + ":').InvokeVerb('Eject')");
			Journal ("Carte éjectée : vous pouvez la retirer.");
			bEjecter.Enabled = false;
		}

		// images de chaque etape (verification de la mise en page)
		public void Captures (string dossier)
		{
			Directory.CreateDirectory (dossier);
			for (int i = 0; i < m_Pages.Count; i++)
			{
				Afficher (i);
				if (i == 2) rFixe.Checked = true;		// montrer aussi les champs de l'adresse fixe
				if (i == 1) cPreset.SelectedIndex = Presets.Length;
				if (i == 4 && GifEssai != null) m_ChoisirGif (GifEssai);
				Application.DoEvents ();
				using (var b = new Bitmap (Width, Height))
				{
					DrawToBitmap (b, new Rectangle (0, 0, Width, Height));
					b.Save (Path.Combine (dossier, "etape-" + (i + 1) + ".png"));
				}
			}
		}
	}

	static class Programme
	{
		[STAThread]
		static int Main (string[] args)
		{
			string dossier = null, auto = null, captures = null;
			foreach (string a in args)
			{
				if (a.StartsWith ("/dossier:", StringComparison.OrdinalIgnoreCase)) dossier = Path.GetFullPath (a.Substring (9));
				else if (a.StartsWith ("/auto:", StringComparison.OrdinalIgnoreCase)) auto = a.Substring (6);
				else if (a.StartsWith ("/captures:", StringComparison.OrdinalIgnoreCase)) captures = a.Substring (10);
			}

			// sans fenetre : reponses lues dans un fichier, contenu ecrit dans un dossier (essais)
			if (auto != null)
			{
				if (dossier == null) return 2;
				try
				{
					Reponses r = Reponses.Lire (auto);
					string e = Regles.Nom (r.Nom) ?? Regles.Wifi (r.Connexion, r.Ssid, r.Psk) ?? Regles.MotDePasse (r.MotDePasse)
						?? (r.IpFixe ? Regles.AdresseFixe (r.Ip, r.Masque, r.Passerelle, r.Dns) : null);
					if (e != null) throw new Exception (e);
					Directory.CreateDirectory (dossier);
					var journal = new List<string> ();
					int n = Carte.Preparer (r, dossier, journal.Add);
					journal.Add ("OK " + n + " GIF");
					File.WriteAllLines (Path.Combine (dossier, "..", Path.GetFileName (dossier) + "-resultat.txt"), journal);
					return 0;
				}
				catch (Exception ex)
				{
					File.WriteAllText (Path.Combine (Path.GetDirectoryName (dossier), Path.GetFileName (dossier) + "-resultat.txt"), "ERREUR " + ex.Message);
					return 1;
				}
			}

			Application.EnableVisualStyles ();
			Application.SetCompatibleTextRenderingDefault (false);
			var f = new Assistant (dossier);
			if (captures != null)
			{
				f.GifEssai = args.Where (a => a.StartsWith ("/gif:", StringComparison.OrdinalIgnoreCase)).Select (a => a.Substring (5)).FirstOrDefault ();
				f.Shown += (s, e) => { f.Captures (captures); f.Close (); };
			}
			Application.Run (f);
			return 0;
		}
	}
}
