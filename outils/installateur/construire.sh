#!/bin/bash
# Construit dist/OrbitDMD-Installateur.exe : un seul fichier, qui contient tout ce qu'il faut
# mettre sur la carte (.NET Framework 4.8, deja present sous Windows 10 et 11).
#
# Usage (Git Bash) :  bash outils/installateur/construire.sh
# A relancer apres chaque nouvelle version de dmd/kernel8.img.
#
# Contenu :
#  - programme, fichiers du Raspberry Pi et du Wi-Fi ;
#  - polices : libres (Google Fonts, polices-telechargees/, licence OFL ou Apache) et pixel
#    maison (polices_pixel.py) ; motifs (ressources.py) et fonds animes (fonds.py) maison ;
#  - GIF : collection du projet RPI2DMD + ajouts (gifs.py), sans les GIF "rpi2dmd"/"rpiteam".
# Ne sont PAS fournies (droits d'auteur) : Goudy Stout, les polices Halloween et les motifs
# et fonds de l'ancien RPI2DMD.
set -e
cd "$(dirname "$0")"
ICI=$(pwd)
RACINE=$(cd ../.. && pwd)
B=build
CARTE=$B/carte
CSC="/c/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/MSBuild/Current/Bin/Roslyn/csc.exe"

VERSION=$(tr -d '\r\n' < "$RACINE/VERSION")		# numero de version d'OrbitDMD (fichier VERSION)
SORTIE=${SORTIE:-OrbitDMD-Installateur-$VERSION.exe}

[ -f "$RACINE/dmd/kernel8.img" ] || { echo "dmd/kernel8.img absent : compilez d'abord le programme"; exit 1; }

rm -rf "$B"
mkdir -p "$CARTE/firmware" "$CARTE/gif/Accueil" "$CARTE/fonts" "$CARTE/LICENCES/Polices" dist
py -3.12 -I ressources.py "$RACINE" "$B"
py -3.12 -I polices_pixel.py "$RACINE/dmd/font5x7.cpp" "$CARTE/fonts"
py -3.12 -I fonds.py "$CARTE/fonts" > /dev/null
# collection de GIF : seulement si la sauvegarde de l'ancienne carte RPI2DMD est la (pas dans le depot)
if [ -d "$RACINE/sauvegarde-carte/partition3-gif/gif" ]; then
	py -3.12 -I gifs.py "$RACINE" "$B/gifs" "$CARTE/gif/A_PROPOS_DES_GIF.txt" | head -1
else
	echo "Pas de collection de GIF (sauvegarde-carte absente) : installateur sans GIF fournies"
	mkdir -p "$B/gifs"
fi

# contenu de la carte
# fichiers du Raspberry Pi et du Wi-Fi : telecharges par Circle (make -C circle/boot,
# make -C circle/addon/wlan/firmware)
cp "$RACINE"/circle/boot/{bootcode.bin,start.elf,fixup.dat,LICENCE.broadcom} "$CARTE/"
cp "$RACINE"/circle/addon/wlan/firmware/brcmfmac* "$CARTE/firmware/"
cp "$RACINE/dmd/kernel8.img" "$CARTE/"
cp carte/config.txt carte/LISEZMOI.txt carte/LICENCES.txt "$CARTE/"
printf 'OrbitDMD %s\r\n' "$VERSION" > "$CARTE/VERSION.txt"
cp "$RACINE"/orbitdmd-*.gif "$CARTE/gif/Accueil/"
cp "$B"/fonts/Pattern_*.png "$CARTE/fonts/"
cp polices-telechargees/*.ttf "$CARTE/fonts/"
cp polices-telechargees/*-OFL.txt polices-telechargees/*-Apache.txt "$CARTE/LICENCES/Polices/"
cp "$RACINE/circle/LICENSE" "$CARTE/LICENCES/Circle-GPL3.txt"

# ressources : "carte/<chemin>" (toujours copie) et "gifs/<categorie>/<fichier>" (au choix) ;
# plus de 600 fichiers : la liste passe par un fichier de reponses du compilateur
# traductions : une ligne par texte, 12 langues (outils/traductions/installateur*.txt)
py -3.12 -I ../langues.py --fusion ../traductions/installateur.txt "$B/langues.txt"

RSP=$B/csc.rsp
{
	echo "/nologo"
	echo "/target:winexe"
	echo "/optimize+"
	echo "/codepage:65001"
	echo "/out:\"$(cygpath -w "dist/$SORTIE")\""
	echo "/win32icon:\"$(cygpath -w "$B/orbitdmd.ico")\""
	echo "/win32manifest:\"$(cygpath -w app.manifest)\""
	for r in System.dll System.Core.dll System.Drawing.dll System.Windows.Forms.dll System.Web.Extensions.dll; do
		echo "/reference:$r"
	done
	(cd "$CARTE" && find . -type f | sed 's|^\./||' | sort) | while IFS= read -r f; do
		echo "\"/resource:$(cygpath -w "$CARTE/$f"),carte/$f\""
	done
	(cd "$B/gifs" && find . -type f | sed 's|^\./||' | sort) | while IFS= read -r f; do
		echo "\"/resource:$(cygpath -w "$B/gifs/$f"),gifs/$f\""
	done
	echo "\"/resource:$(cygpath -w "$ICI/preparer.ps1"),preparer.ps1\""
	echo "\"/resource:$(cygpath -w "$B/logo.png"),logo.png\""
	echo "\"/resource:$(cygpath -w "$B/langues.txt"),langues.txt\""
	echo "\"$(cygpath -w Installateur.cs)\""
	echo "\"$(cygpath -w "$B/version.cs")\""
} > "$RSP"

# date de construction, affichee dans la fenetre (pour savoir quelle version tourne)
cat > "$B/version.cs" <<FIN
using System.Reflection;
[assembly: AssemblyVersion ("$VERSION.0")]
[assembly: AssemblyFileVersion ("$VERSION.0")]
[assembly: AssemblyInformationalVersion ("$VERSION")]
namespace OrbitDMD { static class Construction { public const string Numero = "$VERSION"; public const string Date = "$(date '+%d/%m/%Y %H:%M')"; } }
FIN

MSYS_NO_PATHCONV=1 "$CSC" "@$(cygpath -w "$RSP")"

ls -la "dist/$SORTIE"
echo "$(cd "$CARTE" && find . -type f | wc -l) fichiers pour toute carte, $(cd "$B/gifs" && find . -type f | wc -l) GIF au choix"
