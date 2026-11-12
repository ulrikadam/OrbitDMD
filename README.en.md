<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <b>English</b> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Your HUB75 LED panels on a Raspberry Pi 3: animated GIFs, clock and weather, set up from your browser.</b><br>
  No Linux, boots in a few seconds. Installed in 5 steps from Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Download" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Download&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Ideas and questions" src="https://img.shields.io/badge/Ideas%20%26%20questions-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Support on Ko-fi" src="https://img.shields.io/badge/Support-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="GPL v3 licence" src="https://img.shields.io/badge/licence-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## What OrbitDMD does

OrbitDMD turns a **Raspberry Pi 3** and **HUB75 LED panels** (for example 2 × 64×32) into a pinball or
arcade-style "DMD" display:

- 🎞️ **animated GIFs** by category (562 included), added from the web page one by one, a whole folder at a
  time or as a .zip; favourite GIFs and GIFs out of rotation;
- 🕒 **clock**: 14 fonts, colours, animated rainbow, patterns, 13 animated backgrounds, random choice, date,
  Internet time sync;
- 💬 **messages**: scrolling texts and countdowns (Christmas, New Year, any date), each with its own style
  and entrance effect (scroll, rise, letter by letter, raining letters, fade…);
- 🏠 **Home Assistant**: the panel shows up by itself (MQTT); power, brightness and messages from your
  automations ([guide](docs/HOME-ASSISTANT.en.md));
- 🗓️ **schedule**: panel off at night or time only at certain hours, GIF categories by month (Christmas in
  December…), texts and countdowns active only between two dates;
- 📐 **oversized GIFs automatically resized** to the panel when uploaded;
- ⛅ **weather** for your town (Open-Meteo, no sign-up) with animated icons;
- 🌐 **web page** on your network (`http://orbitdmd.local/`): live preview of the panel, settings,
  hour-by-hour brightness, pause, display off, settings backup, updates;
- 🌍 **12 languages**: French, English, Dutch, Spanish, Russian, German, Italian, Portuguese, Polish,
  Ukrainian, Turkish and Czech, for the web page, the installer, the panel texts and Home Assistant;
- ⚡ **no Linux**: the program starts directly on the Raspberry Pi (bare metal, built on
  [Circle](https://github.com/rsta2/circle)), in a few seconds.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Dashboard" width="49%">
  <img src="docs/images/horloge.png" alt="Clock settings" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Weather" width="49%">
  <img src="docs/images/gif.png" alt="GIF management" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Messages: scrolling texts" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Messages: countdowns" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Settings: schedule and Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Home Assistant help" width="49%">
</p>

## Hardware

| Item | Details |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (model B or B+) with its power supply |
| Panels | HUB75, by default 2 × 64×32 side by side (128×32); 64×64, 32×16 and up to 8 panels possible |
| Adapter board | HUB75 for Raspberry Pi, **"regular"** pinout of rpi-rgb-led-matrix (e.g. Electrodragon); the Adafruit HAT pinout is not supported |
| Panel power supply | 5 V, enough for your panels (about 4 A per 64×32 panel at full brightness) |
| Micro-SD card | 32 GB or less recommended (a FAT32 partition of at most 31 GB is created) |
| Network | Network cable (Ethernet) or 2.4 GHz Wi-Fi (the Raspberry Pi 3 cannot see 5 GHz) |

## Installation

1. **Download** `OrbitDMD-Installateur-<version>.exe` from the
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) page (Windows 10 or 11, nothing else to install).
2. **Run it.** The program is not signed: if Windows shows "Windows protected your PC", click
   *More info*, then *Run anyway*. The installer speaks your Windows language (or choose it on the first page).
3. **Answer the questions**: panel name, panel layout, Wi-Fi, web page password, time zone, town for the
   weather, GIFs to install.
4. **Choose the micro-SD card**. Only card readers and USB drives are offered, never your internal disks;
   everything on the chosen card is erased.
5. **Put the card in the Raspberry Pi** and power it on. After about thirty seconds, open
   `http://<panel name>.local/` (or the IP address given by your router).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Installer: panels" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Installer: SD card" width="49%">
</p>

After that, everything is set from the web page: GIFs (drag and drop), clock, weather, Wi-Fi, fixed IP
address, password, language, and even firmware updates: download `kernel8.img` from the latest release on
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), then **System → New firmware**.
The web page language is chosen at the top right of each page; the panel language in **Settings → Display**.

## Included fonts, backgrounds and GIFs

<p align="center"><img src="docs/images/polices.png" alt="Fonts" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Animated backgrounds" width="90%"></p>

- **Fonts**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (free, [Google Fonts](https://fonts.google.com/)) and
  3 OrbitDMD pixel fonts (LED, Bold, 7 segments). Your own TTF fonts can be added from the Clock page.
  Fonts without Cyrillic letters automatically fall back to the built-in Pixel font for Russian text.
- **Animated backgrounds and patterns**: made for OrbitDMD (stars, snow, rain, Matrix, fire, aurora, synthwave…).
- **GIFs**: OrbitDMD includes the **free basic animated GIF pack from the RpiTeaM** (RPI2DMD project,
  pixel art by eLLuiGi, RattenJager and others), with eLLuiGi's permission — thanks to them!
  If you want more, **a pack of 11,000 animations** is available
  [on Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) or on [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  The start-up screens and some of the Christmas GIFs are OrbitDMD creations.
  Video game characters remain the property of their respective owners.

## Suggest an idea, report a problem

Your feedback moves OrbitDMD forward:

- 💡 **An idea, an improvement?** Open a [discussion](https://github.com/ulrikadam/OrbitDMD/discussions)
  or a [proposal](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 💬 **French-speaking?** Join the thread on the
  [Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=84762) forum.
- 🐞 **A problem?** [Report it](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) with the version
  (shown on the dashboard) and, if possible, the log from the System page.
- 🌍 **A better translation?** The texts are in [outils/traductions](outils/traductions): corrections are welcome.
- 🛠️ **Want to contribute code?** Read [CONTRIBUTING.md](CONTRIBUTING.md) and
  [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md).

What is new in each version is in the [release notes](JOURNAL-DES-VERSIONS.md) (in French).

## Support the project

OrbitDMD is free. If you like it and want to support its development, you can buy me a coffee:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Support on Ko-fi"></a></p>

## Licence

OrbitDMD is free software under the **GNU GPL v3** licence (see [LICENSE](LICENSE)), like
[Circle](https://github.com/rsta2/circle) on which it is built. Licences of the other components
(fonts, Raspberry Pi firmware, weather data): see
[outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Developed by <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
