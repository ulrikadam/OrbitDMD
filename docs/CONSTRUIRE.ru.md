# Сборка OrbitDMD из исходного кода

[Français](CONSTRUIRE.md) · [English](CONSTRUIRE.en.md) · [Nederlands](CONSTRUIRE.nl.md) · [Español](CONSTRUIRE.es.md) · **Русский**

Это руководство для тех, кто хочет изменить программу. Чтобы просто установить OrbitDMD, используйте
установщик со страницы [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest).

Команды ниже рассчитаны на **Windows + Git Bash** (текущая среда разработки). В Linux или macOS программа
собирается так же; только установщику `.exe` нужна Windows. Комментарии в коде на французском.

## 1. Получить код

```bash
git clone --recursive https://github.com/ulrikadam/OrbitDMD.git
cd OrbitDMD
```

`--recursive` также скачивает [Circle](https://github.com/rsta2/circle) (версия **Step51.1.1**) и
`hostap` (Wi-Fi). Если вы забыли этот параметр: `git submodule update --init --recursive`.

## 2. Необходимые инструменты

| Инструмент | Назначение |
|---|---|
| [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads) **aarch64-none-elf** (проверено: 15.2.Rel1) | компилятор для Raspberry Pi |
| GNU make (в Windows: [xPack Windows Build Tools](https://github.com/xpack-dev-tools/windows-build-tools-xpack/releases), проверено: 4.4.1-3) | сборка |
| [Git for Windows](https://git-scm.com/) (Git Bash) | командная строка |
| Python **3.12** с **Pillow** и **NumPy** (`py -3.12 -m pip install pillow numpy`) | встроенные веб-страницы, ресурсы установщика |
| Visual Studio 2022 **Build Tools** (C# и C++) | установщик `.exe` и тестовый стенд на ПК (необязательно) |

Файл [outils/env.sh](../outils/env.sh) добавляет компилятор и `make` в `PATH`; он ищет их в
`%LOCALAPPDATA%\rpi2dmd-outils\arm-aarch64` и `%LOCALAPPDATA%\rpi2dmd-outils\xpack-windows-build-tools-4.4.1-3`.
Измените его, если установили их в другое место.

## 3. Собрать Circle (один раз)

```bash
source outils/env.sh
cp outils/circle/Config.mk circle/Config.mk     # AArch64, Raspberry Pi 3, многоядерный режим
cd circle
./makeall clean && ./makeall
make -C addon/fatfs && make -C addon/SDCard
(cd addon/wlan && ./makeall --nosample)
make -C boot                     # скачивает bootcode.bin, start.elf, fixup.dat
make -C addon/wlan/firmware      # скачивает прошивку Wi-Fi
cd ..
```

## 4. Собрать OrbitDMD

```bash
source outils/env.sh
make -C dmd
```

Результат: `dmd/kernel8.img`. Номер версии берётся из файла [VERSION](../VERSION).

Чтобы попробовать: скопируйте его на карту, подготовленную установщиком, или отправьте на панель со
страницы **Система → Новая прошивка**.

## 5. Собрать установщик (Windows)

В **новом** окне Git Bash (без `source outils/env.sh`, который мешает преобразованию путей):

```bash
bash outils/installateur/construire.sh
```

Результат: `outils/installateur/dist/OrbitDMD-Installateur-<версия>.exe`.
Коллекции GIF прежнего проекта RPI2DMD нет в репозитории: без неё установщик собирается без встроенных
GIF (экраны запуска OrbitDMD включаются всегда).

## 6. Проверка без панели

- **Веб-страница**: `py -3.12 -I dmd/test/mock_web.py`, затем откройте <http://localhost:8765/>. Страницам
  отвечает имитация панели; имитируемая карта — папка `carte-sd/` (или папка из переменной `ORBITDMD_CARTE`).
- **Отрисовка GIF, часов и погоды**: `dmd/test/build.cmd` собирает `dmd/test/out/hosttest.exe`, который
  создаёт изображения панели на ПК.

## Переводы

| Файл | Содержимое |
|---|---|
| `outils/traductions/web.txt` | веб-страницы: одна строка на французский текст, `французский \| английский \| нидерландский \| испанский \| русский` |
| `outils/traductions/installateur.txt` | установщик Windows, тот же формат |
| `dmd/lang.cpp` | тексты на панели (дни, месяцы, погода, сообщения о состоянии) |

`make -C dmd` превращает `web.txt` в `dmd/web/lang-xx.js` (с помощью `outils/langues.py`).

## Структура кода

| Папка | Содержимое |
|---|---|
| `dmd/` | программа Raspberry Pi: панели HUB75 (`matrix`), GIF, часы, погода, сеть, веб-сервер |
| `dmd/web/` | страницы веб-интерфейса (встраиваются в программу при сборке) |
| `dmd/test/` | тестовый стенд на ПК и имитация панели для веб-страницы |
| `outils/installateur/` | установщик Windows (C#), встроенные шрифты, фоны и узоры |
| `outils/traductions/` | переводы веб-страниц и установщика |
| `outils/circle/` | конфигурация Circle для OrbitDMD |
| `circle/` | Circle (подмодуль, без изменений) |
