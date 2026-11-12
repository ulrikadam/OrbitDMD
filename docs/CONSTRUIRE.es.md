# Compilar OrbitDMD desde el código fuente

[Français](CONSTRUIRE.md) · [English](CONSTRUIRE.en.md) · [Nederlands](CONSTRUIRE.nl.md) · **Español** · [Русский](CONSTRUIRE.ru.md)

Esta guía es para quien quiera modificar el programa. Para instalar OrbitDMD sin más, use el instalador
de la página [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest).

Los comandos de abajo están pensados para **Windows + Git Bash** (el entorno de desarrollo actual). En Linux
o macOS el programa se compila igual; solo el instalador `.exe` requiere Windows. Los comentarios del código
están en francés.

## 1. Obtener el código

```bash
git clone --recursive https://github.com/ulrikadam/OrbitDMD.git
cd OrbitDMD
```

`--recursive` descarga también [Circle](https://github.com/rsta2/circle) (versión **Step51.1.1**) y
`hostap` (el Wi-Fi). Si olvidó la opción: `git submodule update --init --recursive`.

## 2. Herramientas necesarias

| Herramienta | Función |
|---|---|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) **aarch64-none-elf** (probado: 15.2.Rel1) | compilador para la Raspberry Pi |
| GNU make (en Windows: [xPack Windows Build Tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases), probado: 4.4.1-3) | compilación |
| [Git for Windows](https://git-scm.com/) (Git Bash) | línea de comandos |
| Python **3.12** con **Pillow** y **NumPy** (`py -3.12 -m pip install pillow numpy`) | páginas web integradas, recursos del instalador |
| Visual Studio 2022 **Build Tools** (C# y C++) | instalador `.exe` y banco de pruebas en PC (opcional) |

El archivo [outils/env.sh](../outils/env.sh) añade el compilador y `make` al `PATH`; los busca en
`%LOCALAPPDATA%\rpi2dmd-outils\arm-aarch64` y `%LOCALAPPDATA%\rpi2dmd-outils\xpack-windows-build-tools-4.4.1-3`.
Adáptelo si los instala en otro lugar.

## 3. Compilar Circle (una sola vez)

```bash
source outils/env.sh
cp outils/circle/Config.mk circle/Config.mk     # AArch64, Raspberry Pi 3, multinúcleo
cd circle
./makeall clean && ./makeall
make -C addon/fatfs && make -C addon/SDCard
(cd addon/wlan && ./makeall --nosample)
make -C boot                     # descarga bootcode.bin, start.elf, fixup.dat
make -C addon/wlan/firmware      # descarga el firmware Wi-Fi
cd ..
```

## 4. Compilar OrbitDMD

```bash
source outils/env.sh
make -C dmd
```

Resultado: `dmd/kernel8.img`. El número de versión viene del archivo [VERSION](../VERSION).

Para probarlo: cópielo en una tarjeta preparada por el instalador o envíelo al panel desde la página
**Sistema → Nuevo firmware**.

## 5. Construir el instalador (Windows)

En un **nuevo** Git Bash (sin `source outils/env.sh`, que estorba la conversión de rutas):

```bash
bash outils/installateur/construire.sh
```

Resultado: `outils/installateur/dist/OrbitDMD-Installateur-<versión>.exe`.
La colección de GIF del antiguo proyecto RPI2DMD no está en el repositorio: sin ella, el instalador se
construye sin GIF incluidos (las pantallas de arranque de OrbitDMD siempre se incluyen).

## 6. Probar sin panel

- **Página web**: `py -3.12 -I dmd/test/mock_web.py` y abra <http://localhost:8765/>. Un panel falso
  responde a las páginas; la tarjeta simulada es la carpeta `carte-sd/` (o la de la variable `ORBITDMD_CARTE`).
- **Dibujo de los GIF, el reloj y el tiempo**: `dmd/test/build.cmd` compila `dmd/test/out/hosttest.exe`,
  que produce imágenes del panel en el PC.

## Traducciones

| Archivo | Contenido |
|---|---|
| `outils/traductions/web.txt` | páginas web: una línea por texto francés, `francés \| inglés \| neerlandés \| español \| ruso` |
| `outils/traductions/installateur.txt` | instalador de Windows, mismo formato |
| `dmd/lang.cpp` | textos dibujados en el panel (días, meses, tiempo, mensajes de estado) |

`make -C dmd` convierte `web.txt` en `dmd/web/lang-xx.js` (con `outils/langues.py`).

## Organización del código

| Carpeta | Contenido |
|---|---|
| `dmd/` | programa de la Raspberry Pi: paneles HUB75 (`matrix`), GIF, reloj, tiempo, red, servidor web |
| `dmd/web/` | páginas de la interfaz web (integradas en el programa al compilar) |
| `dmd/test/` | banco de pruebas en PC y panel falso para la página web |
| `outils/installateur/` | instalador de Windows (C#), fuentes, fondos y motivos incluidos |
| `outils/traductions/` | traducciones de las páginas web y del instalador |
| `outils/circle/` | configuración de Circle usada por OrbitDMD |
| `circle/` | Circle (submódulo, sin modificar) |
