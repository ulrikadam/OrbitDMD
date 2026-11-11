# Construire OrbitDMD depuis les sources

Ce guide sert aux personnes qui veulent modifier le programme. Pour simplement installer OrbitDMD,
utilisez l'installateur de la page [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest).

Les commandes ci-dessous sont prévues pour **Windows + Git Bash** (c'est l'environnement de développement
actuel). Sous Linux ou macOS, le programme se compile de la même façon ; seul l'installateur `.exe` demande Windows.

## 1. Récupérer le code

```bash
git clone --recursive https://github.com/ulrikadam/OrbitDMD.git
cd OrbitDMD
```

`--recursive` télécharge aussi [Circle](https://github.com/rsta2/circle) (version **Step51.1.1**) et
`hostap` (le Wi-Fi). Si vous avez oublié l'option : `git submodule update --init --recursive`.

## 2. Outils nécessaires

| Outil | Rôle |
|---|---|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) **aarch64-none-elf** (testé : 15.2.Rel1) | compilateur pour le Raspberry Pi |
| GNU make (sous Windows : [xPack Windows Build Tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases), testé : 4.4.1-3) | compilation |
| [Git for Windows](https://git-scm.com/) (Git Bash) | ligne de commande |
| Python **3.12** avec **Pillow** et **NumPy** (`py -3.12 -m pip install pillow numpy`) | pages web intégrées, ressources de l'installateur |
| Visual Studio 2022 **Build Tools** (C# et C++) | installateur `.exe` et banc de test PC (facultatif) |

Le fichier [outils/env.sh](../outils/env.sh) ajoute le compilateur et `make` au `PATH` ; il les cherche dans
`%LOCALAPPDATA%\rpi2dmd-outils\arm-aarch64` et `%LOCALAPPDATA%\rpi2dmd-outils\xpack-windows-build-tools-4.4.1-3`.
Adaptez-le si vous les installez ailleurs.

## 3. Compiler Circle (une seule fois)

```bash
source outils/env.sh
cp outils/circle/Config.mk circle/Config.mk     # AArch64, Raspberry Pi 3, multicœur
cd circle
./makeall clean && ./makeall
make -C addon/fatfs && make -C addon/SDCard
(cd addon/wlan && ./makeall --nosample)
make -C boot                     # télécharge bootcode.bin, start.elf, fixup.dat
make -C addon/wlan/firmware      # télécharge le microprogramme Wi-Fi
cd ..
```

## 4. Compiler OrbitDMD

```bash
source outils/env.sh
make -C dmd
```

Résultat : `dmd/kernel8.img`. Le numéro de version vient du fichier [VERSION](../VERSION).

Pour l'essayer : copiez-le sur une carte préparée par l'installateur, ou envoyez-le au panneau depuis la page
**Système → Mettre à jour le programme**.

## 5. Construire l'installateur (Windows)

Dans un **nouveau** Git Bash (sans `source outils/env.sh`, qui gêne la conversion des chemins) :

```bash
bash outils/installateur/construire.sh
```

Résultat : `outils/installateur/dist/OrbitDMD-Installateur-<version>.exe`.
La collection de GIF de l'ancien projet RPI2DMD n'est pas dans le dépôt : sans elle, l'installateur est
construit sans GIF fournies (les écrans de démarrage OrbitDMD sont toujours inclus).

## 6. Tester sans panneau

- **Page web** : `py -3.12 -I dmd/test/mock_web.py` puis ouvrez <http://localhost:8765/>. Un faux panneau
  répond aux pages ; la carte simulée est le dossier `carte-sd/` (ou celui de la variable `ORBITDMD_CARTE`).
- **Rendu des GIF, de l'horloge et de la météo** : `dmd/test/build.cmd` compile `dmd/test/out/hosttest.exe`,
  qui produit des images du panneau sur le PC.

## Organisation du code

| Dossier | Contenu |
|---|---|
| `dmd/` | programme du Raspberry Pi : panneaux HUB75 (`matrix`), GIF, horloge, météo, réseau, serveur web |
| `dmd/web/` | pages de l'interface web (intégrées au programme à la compilation) |
| `dmd/test/` | banc de test PC et faux panneau pour la page web |
| `outils/installateur/` | installateur Windows (C#), polices, fonds et motifs fournis |
| `outils/circle/` | configuration de Circle utilisée par OrbitDMD |
| `circle/` | Circle (sous-module, non modifié) |
