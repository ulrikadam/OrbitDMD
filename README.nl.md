<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <b>Nederlands</b> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Je HUB75-ledpanelen op een Raspberry Pi 3: geanimeerde GIF's, klok en weer, in te stellen vanuit je browser.</b><br>
  Zonder Linux, opgestart in enkele seconden. Installatie in 5 stappen vanuit Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Downloaden" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Downloaden&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Ideeën en vragen" src="https://img.shields.io/badge/Idee%C3%ABn%20%26%20vragen-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Steun op Ko-fi" src="https://img.shields.io/badge/Steun-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licentie GPL v3" src="https://img.shields.io/badge/licentie-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Wat OrbitDMD doet

OrbitDMD maakt van een **Raspberry Pi 3** en **HUB75-ledpanelen** (bijvoorbeeld 2 × 64×32) een
"DMD"-scherm zoals op een flipperkast of arcadekast:

- 🎞️ **geanimeerde GIF's** per categorie (562 meegeleverd), toe te voegen via de webpagina, één voor één,
  per map of als .zip; favoriete GIF's en GIF's buiten de rotatie;
- 🕒 **klok**: 14 lettertypen, kleuren, bewegende regenboog, patronen, 13 bewegende achtergronden,
  willekeurige keuze, datum, tijd via internet;
- 💬 **berichten**: lopende teksten en aftellers (Kerstmis, Nieuwjaar, gekozen datum), elk met een eigen
  stijl en binnenkomsteffect (lopen, omhoog, letter voor letter, regen van letters, vervagen…);
- 🏠 **Home Assistant**: het paneel verschijnt er vanzelf (MQTT); aan/uit, helderheid en berichten vanuit
  je automatiseringen ([handleiding](docs/HOME-ASSISTANT.nl.md));
- 🗓️ **schema**: paneel 's nachts uit of alleen de tijd op bepaalde uren, GIF-categorieën per maand
  (Kerstmis in december…), teksten en aftellers alleen actief tussen twee datums;
- 📐 **te grote GIF's automatisch verkleind** tot het paneelformaat bij het uploaden;
- ⛅ **weer** van je woonplaats (Open-Meteo, zonder registratie) met bewegende pictogrammen;
- 🌐 **webpagina** op je netwerk (`http://orbitdmd.local/`): live voorbeeld van het paneel, instellingen,
  helderheid per uur, pauze, uitschakelen, back-up van de instellingen, updates;
- 🌍 **12 talen** (waaronder Nederlands) voor de webpagina, de installatie, de teksten op het paneel en Home Assistant;
- ⚡ **zonder Linux**: het programma start rechtstreeks op de Raspberry Pi (gebouwd op
  [Circle](https://github.com/rsta2/circle)), in enkele seconden.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Dashboard" width="49%">
  <img src="docs/images/horloge.png" alt="Instellingen van de klok" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Weer" width="49%">
  <img src="docs/images/gif.png" alt="Beheer van de GIF's" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Berichten: lopende teksten" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Berichten: aftellers" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Instellingen: schema en Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Hulp Home Assistant" width="49%">
</p>

## Hardware

| Onderdeel | Details |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (model B of B+) met voeding |
| Panelen | HUB75, standaard 2 × 64×32 naast elkaar (128×32); 64×64, 32×16 en tot 8 panelen mogelijk |
| Adapterbord | HUB75 voor Raspberry Pi, **"regular"**-pinout van rpi-rgb-led-matrix (bv. Electrodragon); de pinout van de Adafruit HAT wordt niet ondersteund |
| Voeding van de panelen | 5 V, voldoende voor je panelen (ongeveer 4 A per paneel van 64×32 op volle helderheid) |
| Micro-SD-kaart | 32 GB of minder aanbevolen (er wordt een FAT32-partitie van maximaal 31 GB gemaakt) |
| Netwerk | Netwerkkabel (Ethernet) of wifi op 2,4 GHz (de Raspberry Pi 3 ziet geen 5 GHz) |

## Installatie

1. **Download** `OrbitDMD-Installateur-<versie>.exe` van de pagina
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 of 11, verder niets te installeren).
2. **Start het.** Het programma is niet ondertekend: als Windows "Windows heeft uw pc beschermd" toont,
   klik dan op *Meer informatie* en daarna op *Toch uitvoeren*. De installatie gebruikt de taal van Windows
   (of kies ze op de eerste pagina).
3. **Beantwoord de vragen**: naam van het paneel, opstelling van de panelen, wifi, wachtwoord van de
   webpagina, tijdzone, plaats voor het weer, te installeren GIF's.
4. **Kies de micro-SD-kaart**. Alleen kaartlezers en USB-sticks worden getoond, nooit je interne schijven;
   alle inhoud van de gekozen kaart wordt gewist.
5. **Steek de kaart in de Raspberry Pi** en sluit de voeding aan. Open na een halve minuut
   `http://<naam van het paneel>.local/` (of het IP-adres dat je router gegeven heeft).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Installatie: panelen" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Installatie: SD-kaart" width="49%">
</p>

Daarna stel je alles in via de webpagina: GIF's (slepen en neerzetten), klok, weer, wifi, vast IP-adres,
wachtwoord, taal en zelfs de update van de firmware: download `kernel8.img` uit de laatste versie op
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) en kies dan **Systeem → Nieuwe firmware**.
De taal van de webpagina kies je rechtsboven op elke pagina; de taal van het paneel bij **Instellingen → Weergave**.

## Meegeleverde lettertypen, achtergronden en GIF's

<p align="center"><img src="docs/images/polices.png" alt="Lettertypen" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Bewegende achtergronden" width="90%"></p>

- **Lettertypen**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (vrij, [Google Fonts](https://fonts.google.com/)) en
  3 pixellettertypen van OrbitDMD (LED, Vet, 7 segmenten). Eigen TTF-lettertypen voeg je toe via de pagina Klok.
- **Bewegende achtergronden en patronen**: gemaakt voor OrbitDMD (sterren, sneeuw, regen, Matrix, vuur,
  noorderlicht, synthwave…).
- **GIF's**: OrbitDMD bevat het **gratis basispakket geanimeerde GIF's van het RpiTeaM** (RPI2DMD-project,
  pixelart van eLLuiGi, RattenJager en anderen), met toestemming van eLLuiGi — bedankt!
  Wie meer wil: **een pakket van 11.000 animaties** is beschikbaar
  [op Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) of op [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  De opstartschermen en een deel van de kerst-GIF's zijn creaties van OrbitDMD.
  Personages uit videogames blijven eigendom van hun rechthebbenden.

## Een idee voorstellen, een probleem melden

Je feedback helpt OrbitDMD vooruit:

- 💡 **Een idee, een verbetering?** Open een [discussie](https://github.com/ulrikadam/OrbitDMD/discussions)
  of een [voorstel](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Een probleem?** [Meld het](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) met de versie
  (te zien op het dashboard) en, indien mogelijk, het logboek van de pagina Systeem.
- 🌍 **Een betere vertaling?** De teksten staan in [outils/traductions](outils/traductions): correcties zijn welkom.
- 🛠️ **Zin om mee te programmeren?** Lees [CONTRIBUTING.md](CONTRIBUTING.md) en
  [docs/CONSTRUIRE.nl.md](docs/CONSTRUIRE.nl.md).

De nieuwigheden van elke versie staan in het [versielogboek](JOURNAL-DES-VERSIONS.md) (in het Frans).

## Het project steunen

OrbitDMD is gratis. Vind je het leuk en wil je de ontwikkeling steunen, dan kun je een koffie trakteren:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Steun op Ko-fi"></a></p>

## Licentie

OrbitDMD is vrije software onder de licentie **GNU GPL v3** (zie [LICENSE](LICENSE)), net als
[Circle](https://github.com/rsta2/circle) waarop het gebouwd is. Licenties van de andere onderdelen
(lettertypen, firmware van de Raspberry Pi, weergegevens): zie
[outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Ontwikkeld door <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
