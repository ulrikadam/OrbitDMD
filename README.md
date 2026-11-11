<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center">
  <b>Vos panneaux LED HUB75 sur Raspberry Pi 3 : GIF animées, horloge et météo, réglables depuis votre navigateur.</b><br>
  Sans Linux, démarrage en quelques secondes. Installation en 5 étapes depuis Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Télécharger" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=T%C3%A9l%C3%A9charger&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Idées et questions" src="https://img.shields.io/badge/Id%C3%A9es%20%26%20questions-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Soutenir sur Ko-fi" src="https://img.shields.io/badge/Soutenir-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licence GPL v3" src="https://img.shields.io/badge/licence-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Ce que fait OrbitDMD

OrbitDMD transforme un **Raspberry Pi 3** et des **panneaux LED HUB75** (par exemple 2 × 64×32) en écran
de type « DMD » de flipper ou de borne d'arcade :

- 🎞️ **GIF animées** par catégories, choisies et ajoutées depuis la page web (562 fournies) ;
- 🕒 **horloge** : 14 polices, couleurs, motifs, 13 fonds animés, date, synchronisation Internet ;
- ⛅ **météo** de votre ville (Open-Meteo, sans inscription) avec icônes animées ;
- 🌐 **page web** sur votre réseau (`http://orbitdmd.local/`) : aperçu en direct du panneau, réglages,
  luminosité heure par heure, pause, extinction, mises à jour ;
- ⚡ **sans Linux** : le programme démarre directement sur le Raspberry Pi (environnement
  [Circle](https://github.com/rsta2/circle)), en quelques secondes.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Tableau de bord" width="49%">
  <img src="docs/images/horloge.png" alt="Réglages de l'horloge" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Météo" width="49%">
  <img src="docs/images/gif.png" alt="Gestion des GIF" width="49%">
</p>

## Matériel

| Élément | Détail |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (modèle B ou B+) avec son alimentation |
| Panneaux | HUB75, par défaut 2 × 64×32 côte à côte (128×32) ; 64×64, 32×16 et jusqu'à 8 panneaux possibles |
| Carte adaptatrice | HUB75 pour Raspberry Pi, brochage **« regular »** de rpi-rgb-led-matrix (par ex. Electrodragon) ; le brochage Adafruit HAT n'est pas géré |
| Alimentation des panneaux | 5 V, suffisante pour vos panneaux (environ 4 A par panneau 64×32 à pleine luminosité) |
| Carte micro-SD | 32 Go ou moins conseillé (une partition FAT32 de 31 Go au plus est créée) |
| Réseau | Wi-Fi 2,4 GHz (le Raspberry Pi 3 ne voit pas le 5 GHz) |

## Installation

1. **Téléchargez** `OrbitDMD-Installateur-<version>.exe` depuis la page
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 ou 11, rien d'autre à installer).
2. **Lancez-le.** L'exécutable n'est pas signé : si Windows affiche « Windows a protégé votre ordinateur »,
   cliquez sur *Informations complémentaires* puis *Exécuter quand même*.
3. **Répondez aux questions** : nom du panneau, disposition des panneaux, Wi-Fi, mot de passe de la page web,
   fuseau horaire, ville pour la météo, GIF à installer.
4. **Choisissez la carte micro-SD**. Seuls les lecteurs de cartes et clés USB sont proposés, jamais vos
   disques internes ; tout le contenu de la carte choisie est effacé.
5. **Mettez la carte dans le Raspberry Pi** et branchez-le. Après une trentaine de secondes, ouvrez
   `http://<nom du panneau>.local/` (ou l'adresse IP donnée par votre box).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Installateur : panneaux" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Installateur : carte SD" width="49%">
</p>

Ensuite, tout se règle depuis la page web : GIF (ajout par glisser-déposer), horloge, météo, Wi-Fi, adresse IP fixe,
mot de passe, et même la mise à jour du programme : téléchargez `kernel8.img` dans la dernière
version sur [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), puis **Système → Nouveau programme**.

## Polices, fonds et GIF fournis

<p align="center"><img src="docs/images/polices.png" alt="Polices" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Fonds animés" width="90%"></p>

- **Polices** : Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (libres, [Google Fonts](https://fonts.google.com/)) et
  3 polices pixel OrbitDMD (LED, Gras, 7 segments). Vos propres polices TTF s'ajoutent depuis la page Horloge.
- **Fonds animés et motifs** : créés pour OrbitDMD (étoiles, neige, pluie, Matrix, feu, aurore, synthwave…).
- **GIF** : OrbitDMD intègre le **pack de GIF animées gratuit de base de la RpiTeaM** (projet RPI2DMD,
  pixel art d'eLLuiGi, RattenJager et d'autres), avec l'accord d'eLLuiGi — merci à eux !
  Pour ceux qui le souhaitent, **un pack de 11 000 animations** est disponible
  [sur Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) ou sur [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Les écrans de démarrage et une partie des GIF de Noël sont des créations OrbitDMD.
  Les personnages de jeux vidéo restent la propriété de leurs ayants droit.

## Proposer une idée, signaler un problème

Vos retours font avancer OrbitDMD :

- 💡 **Une idée, une amélioration ?** Ouvrez une [discussion](https://github.com/ulrikadam/OrbitDMD/discussions)
  ou une [proposition](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 💬 **Envie d'en parler en français ?** Rejoignez la discussion sur le forum
  [Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=84762).
- 🐞 **Un problème ?** [Signalez-le](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) en indiquant la
  version (affichée sur le tableau de bord) et, si possible, le journal de la page Système.
- 🛠️ **Envie de contribuer au code ?** Lisez [CONTRIBUTING.md](CONTRIBUTING.md) et
  [docs/CONSTRUIRE.md](docs/CONSTRUIRE.md).

Les nouveautés de chaque version sont dans le [journal des versions](JOURNAL-DES-VERSIONS.md).

## Soutenir le projet

OrbitDMD est gratuit. Si vous l'aimez et voulez soutenir son développement, vous pouvez offrir un café :

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Soutenir sur Ko-fi"></a></p>

## Licence

OrbitDMD est un logiciel libre sous licence **GNU GPL v3** (voir [LICENSE](LICENSE)), comme
[Circle](https://github.com/rsta2/circle) sur lequel il est construit. Licences des autres éléments
(polices, microprogrammes du Raspberry Pi, données météo) : voir
[outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<details>
<summary><b>English</b></summary>

OrbitDMD turns a **Raspberry Pi 3** and **HUB75 LED panels** (e.g. 2 × 64×32) into a pinball/arcade-style
dot-matrix display: animated GIFs, a customizable clock (fonts, colors, animated backgrounds), weather,
and a full web interface on your network. It runs bare-metal (no Linux, built on
[Circle](https://github.com/rsta2/circle)) and boots in seconds.

Download the Windows installer from [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest):
it asks for your Wi-Fi, panel layout and password, then prepares the micro-SD card. The interface is in
French. Ideas and bug reports are welcome in [Discussions](https://github.com/ulrikadam/OrbitDMD/discussions)
and [Issues](https://github.com/ulrikadam/OrbitDMD/issues). License: GPL v3.
Like it? [Support me on Ko-fi](https://ko-fi.com/H1U12A50RN).
</details>

<p align="center"><sub>Développé par <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
