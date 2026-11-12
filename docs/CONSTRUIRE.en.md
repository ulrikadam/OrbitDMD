# Building OrbitDMD from source

[Français](CONSTRUIRE.md) · **English** · [Nederlands](CONSTRUIRE.nl.md) · [Español](CONSTRUIRE.es.md) · [Русский](CONSTRUIRE.ru.md)

This guide is for people who want to change the program. To simply install OrbitDMD, use the installer
from the [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) page.

The commands below are meant for **Windows + Git Bash** (the current development environment). On Linux or
macOS the program builds the same way; only the `.exe` installer needs Windows. Code comments are in French.

## 1. Get the code

```bash
git clone --recursive https://github.com/ulrikadam/OrbitDMD.git
cd OrbitDMD
```

`--recursive` also downloads [Circle](https://github.com/rsta2/circle) (version **Step51.1.1**) and
`hostap` (Wi-Fi). If you forgot the option: `git submodule update --init --recursive`.

## 2. Required tools

| Tool | Role |
|---|---|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) **aarch64-none-elf** (tested: 15.2.Rel1) | compiler for the Raspberry Pi |
| GNU make (on Windows: [xPack Windows Build Tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases), tested: 4.4.1-3) | build |
| [Git for Windows](https://git-scm.com/) (Git Bash) | command line |
| Python **3.12** with **Pillow** and **NumPy** (`py -3.12 -m pip install pillow numpy`) | built-in web pages, installer resources |
| Visual Studio 2022 **Build Tools** (C# and C++) | `.exe` installer and PC test bench (optional) |

The file [outils/env.sh](../outils/env.sh) adds the compiler and `make` to the `PATH`; it looks for them in
`%LOCALAPPDATA%\rpi2dmd-outils\arm-aarch64` and `%LOCALAPPDATA%\rpi2dmd-outils\xpack-windows-build-tools-4.4.1-3`.
Adapt it if you install them elsewhere.

## 3. Build Circle (once)

```bash
source outils/env.sh
cp outils/circle/Config.mk circle/Config.mk     # AArch64, Raspberry Pi 3, multi-core
cd circle
./makeall clean && ./makeall
make -C addon/fatfs && make -C addon/SDCard
(cd addon/wlan && ./makeall --nosample)
make -C boot                     # downloads bootcode.bin, start.elf, fixup.dat
make -C addon/wlan/firmware      # downloads the Wi-Fi firmware
cd ..
```

## 4. Build OrbitDMD

```bash
source outils/env.sh
make -C dmd
```

Result: `dmd/kernel8.img`. The version number comes from the [VERSION](../VERSION) file.

To try it: copy it onto a card prepared by the installer, or send it to the panel from the
**System → New firmware** page.

## 5. Build the installer (Windows)

In a **new** Git Bash (without `source outils/env.sh`, which interferes with path conversion):

```bash
bash outils/installateur/construire.sh
```

Result: `outils/installateur/dist/OrbitDMD-Installateur-<version>.exe`.
The GIF collection of the former RPI2DMD project is not in the repository: without it, the installer is
built without included GIFs (the OrbitDMD start-up screens are always included).

## 6. Test without a panel

- **Web page**: `py -3.12 -I dmd/test/mock_web.py`, then open <http://localhost:8765/>. A fake panel
  answers the pages; the simulated card is the `carte-sd/` folder (or the one in the `ORBITDMD_CARTE` variable).
- **Rendering of GIFs, clock and weather**: `dmd/test/build.cmd` builds `dmd/test/out/hosttest.exe`,
  which produces images of the panel on the PC.

## Translations

| File | Content |
|---|---|
| `outils/traductions/web.txt` | web pages: one line per French text, `French \| English \| Dutch \| Spanish \| Russian` |
| `outils/traductions/installateur.txt` | Windows installer, same format |
| `dmd/lang.cpp` | texts drawn on the panel (days, months, weather, status messages) |

`make -C dmd` turns `web.txt` into `dmd/web/lang-xx.js` (by `outils/langues.py`).

## Code layout

| Folder | Content |
|---|---|
| `dmd/` | Raspberry Pi program: HUB75 panels (`matrix`), GIFs, clock, weather, network, web server |
| `dmd/web/` | web interface pages (built into the program at compile time) |
| `dmd/test/` | PC test bench and fake panel for the web page |
| `outils/installateur/` | Windows installer (C#), included fonts, backgrounds and patterns |
| `outils/traductions/` | translations of the web pages and the installer |
| `outils/circle/` | Circle configuration used by OrbitDMD |
| `circle/` | Circle (submodule, unchanged) |
