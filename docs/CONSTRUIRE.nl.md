# OrbitDMD bouwen vanaf de broncode

[Français](CONSTRUIRE.md) · [English](CONSTRUIRE.en.md) · **Nederlands** · [Español](CONSTRUIRE.es.md) · [Русский](CONSTRUIRE.ru.md)

Deze handleiding is voor wie het programma wil aanpassen. Om OrbitDMD gewoon te installeren, gebruik je
de installatie van de pagina [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest).

De opdrachten hieronder zijn bedoeld voor **Windows + Git Bash** (de huidige ontwikkelomgeving). Onder
Linux of macOS wordt het programma op dezelfde manier gebouwd; alleen de `.exe`-installatie vraagt Windows.
De commentaren in de code zijn in het Frans.

## 1. De code ophalen

```bash
git clone --recursive https://github.com/ulrikadam/OrbitDMD.git
cd OrbitDMD
```

`--recursive` downloadt ook [Circle](https://github.com/rsta2/circle) (versie **Step51.1.1**) en
`hostap` (wifi). Optie vergeten? `git submodule update --init --recursive`.

## 2. Benodigde hulpmiddelen

| Hulpmiddel | Functie |
|---|---|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) **aarch64-none-elf** (getest: 15.2.Rel1) | compiler voor de Raspberry Pi |
| GNU make (onder Windows: [xPack Windows Build Tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases), getest: 4.4.1-3) | compilatie |
| [Git for Windows](https://git-scm.com/) (Git Bash) | opdrachtregel |
| Python **3.12** met **Pillow** en **NumPy** (`py -3.12 -m pip install pillow numpy`) | ingebouwde webpagina's, bronnen van de installatie |
| Visual Studio 2022 **Build Tools** (C# en C++) | `.exe`-installatie en pc-testbank (optioneel) |

Het bestand [outils/env.sh](../outils/env.sh) voegt de compiler en `make` toe aan het `PATH`; het zoekt ze in
`%LOCALAPPDATA%\rpi2dmd-outils\arm-aarch64` en `%LOCALAPPDATA%\rpi2dmd-outils\xpack-windows-build-tools-4.4.1-3`.
Pas het aan als je ze elders installeert.

## 3. Circle compileren (één keer)

```bash
source outils/env.sh
cp outils/circle/Config.mk circle/Config.mk     # AArch64, Raspberry Pi 3, multicore
cd circle
./makeall clean && ./makeall
make -C addon/fatfs && make -C addon/SDCard
(cd addon/wlan && ./makeall --nosample)
make -C boot                     # downloadt bootcode.bin, start.elf, fixup.dat
make -C addon/wlan/firmware      # downloadt de wifi-firmware
cd ..
```

## 4. OrbitDMD compileren

```bash
source outils/env.sh
make -C dmd
```

Resultaat: `dmd/kernel8.img`. Het versienummer komt uit het bestand [VERSION](../VERSION).

Om het te proberen: kopieer het naar een kaart die door de installatie is gemaakt, of stuur het naar het
paneel via de pagina **Systeem → Nieuwe firmware**.

## 5. De installatie bouwen (Windows)

In een **nieuwe** Git Bash (zonder `source outils/env.sh`, dat de omzetting van paden verstoort):

```bash
bash outils/installateur/construire.sh
```

Resultaat: `outils/installateur/dist/OrbitDMD-Installateur-<versie>.exe`.
De GIF-collectie van het vroegere project RPI2DMD staat niet in de repository: zonder die collectie wordt
de installatie gebouwd zonder meegeleverde GIF's (de opstartschermen van OrbitDMD zitten er altijd in).

## 6. Testen zonder paneel

- **Webpagina**: `py -3.12 -I dmd/test/mock_web.py` en open dan <http://localhost:8765/>. Een nep-paneel
  beantwoordt de pagina's; de gesimuleerde kaart is de map `carte-sd/` (of die uit de variabele `ORBITDMD_CARTE`).
- **Weergave van GIF's, klok en weer**: `dmd/test/build.cmd` bouwt `dmd/test/out/hosttest.exe`, dat
  beelden van het paneel op de pc maakt.

## Vertalingen

| Bestand | Inhoud |
|---|---|
| `outils/traductions/web.txt` | webpagina's: één regel per Franse tekst, `Frans \| Engels \| Nederlands \| Spaans \| Russisch` |
| `outils/traductions/installateur.txt` | Windows-installatie, zelfde formaat |
| `dmd/lang.cpp` | teksten op het paneel (dagen, maanden, weer, statusmeldingen) |

`make -C dmd` zet `web.txt` om in `dmd/web/lang-xx.js` (via `outils/langues.py`).

## Indeling van de code

| Map | Inhoud |
|---|---|
| `dmd/` | programma van de Raspberry Pi: HUB75-panelen (`matrix`), GIF's, klok, weer, netwerk, webserver |
| `dmd/web/` | pagina's van de webinterface (bij het compileren in het programma opgenomen) |
| `dmd/test/` | pc-testbank en nep-paneel voor de webpagina |
| `outils/installateur/` | Windows-installatie (C#), meegeleverde lettertypen, achtergronden en patronen |
| `outils/traductions/` | vertalingen van de webpagina's en de installatie |
| `outils/circle/` | configuratie van Circle voor OrbitDMD |
| `circle/` | Circle (submodule, ongewijzigd) |
