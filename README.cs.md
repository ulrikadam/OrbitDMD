<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <b>Čeština</b></p>

<p align="center">
  <b>Vaše LED panely HUB75 na Raspberry Pi 3: animované GIFy, hodiny a počasí, nastavení z prohlížeče.</b><br>
  Bez Linuxu, start za pár sekund. Instalace v 5 krocích z Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Stáhnout" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=St%C3%A1hnout&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Nápady a dotazy" src="https://img.shields.io/badge/N%C3%A1pady%20a%20dotazy-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Podpořit na Ko-fi" src="https://img.shields.io/badge/Podpo%C5%99it-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licence GPL v3" src="https://img.shields.io/badge/licence-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Co OrbitDMD umí

OrbitDMD promění **Raspberry Pi 3** a **LED panely HUB75** (například 2 × 64×32) v displej typu „DMD“
jako u pinballu nebo arkádového automatu:

- 🎞️ **animované GIFy** v kategoriích (562 přiloženo), přidávané z webové stránky jednotlivě, celými složkami
  nebo jako .zip; oblíbené GIFy a GIFy mimo rotaci;
- 🕒 **hodiny**: 14 písem, barvy, animovaná duha, vzory, 13 animovaných pozadí, náhodný výběr, datum,
  synchronizace přes internet;
- 💬 **zprávy**: běžící texty a odpočty (Vánoce, Nový rok, zvolené datum), každý s vlastním stylem a efektem
  příchodu (posun, stoupání, písmeno po písmenu, déšť písmen, prolínání…);
- 🏠 **Home Assistant**: panel se tam objeví sám (MQTT); zapnutí, jas a zprávy z vašich automatizací
  ([návod](docs/HOME-ASSISTANT.cs.md));
- 🗓️ **časový plán**: panel v noci vypnutý nebo jen čas v určených hodinách, kategorie GIFů podle měsíců
  (Vánoce v prosinci…), texty a odpočty aktivní jen mezi dvěma daty;
- 📐 **příliš velké GIFy se při odesílání** samy zmenší na formát panelu;
- ⛅ **počasí** pro vaše město (Open-Meteo, bez registrace) s animovanými ikonami;
- 🌐 **webová stránka** ve vaší síti (`http://orbitdmd.local/`): živý náhled panelu, nastavení, jas podle hodin,
  pauza, vypnutí, záloha nastavení, aktualizace;
- 🌍 **12 jazyků** webové stránky, instalátoru, textů panelu a Home Assistant (včetně češtiny);
- ⚡ **bez Linuxu**: program startuje přímo na Raspberry Pi (postavený na [Circle](https://github.com/rsta2/circle))
  za pár sekund.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Přehled" width="49%">
  <img src="docs/images/horloge.png" alt="Nastavení hodin" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Počasí" width="49%">
  <img src="docs/images/gif.png" alt="Správa GIFů" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Zprávy: běžící texty" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Zprávy: odpočty" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Nastavení: časový plán a Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Nápověda Home Assistant" width="49%">
</p>

## Hardware

| Součást | Podrobnosti |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (model B nebo B+) se zdrojem |
| Panely | HUB75, výchozí 2 × 64×32 vedle sebe (128×32); možné 64×64, 32×16 a až 8 panelů |
| Adaptér | HUB75 pro Raspberry Pi, zapojení **„regular“** z rpi-rgb-led-matrix (např. Electrodragon); zapojení Adafruit HAT není podporováno |
| Napájení panelů | 5 V, dostatečné pro vaše panely (asi 4 A na panel 64×32 při plném jasu) |
| Karta micro-SD | doporučeno 32 GB nebo méně (vytvoří se oddíl FAT32 nejvýše 31 GB) |
| Síť | Síťový kabel (Ethernet) nebo Wi-Fi 2,4 GHz (Raspberry Pi 3 nevidí 5 GHz) |

## Instalace

1. **Stáhněte** `OrbitDMD-Installateur-<verze>.exe` ze stránky
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 nebo 11, nic dalšího není potřeba).
2. **Spusťte ho.** Program není podepsaný: pokud Windows zobrazí „Systém Windows ochránil váš počítač“, klikněte na
   *Další informace* a pak *Přesto spustit*. Instalátor mluví jazykem Windows (lze zvolit na první stránce).
3. **Odpovězte na otázky**: název panelu, uspořádání panelů, Wi-Fi, heslo webové stránky, časové pásmo, město pro
   počasí, GIFy k instalaci.
4. **Vyberte kartu micro-SD**. Nabízejí se jen čtečky karet a USB disky, nikdy interní disky; celý obsah zvolené
   karty se smaže.
5. **Vložte kartu do Raspberry Pi** a zapojte napájení. Asi po třiceti sekundách otevřete
   `http://<název panelu>.local/` (nebo IP adresu přidělenou routerem).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Instalátor: panely" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Instalátor: SD karta" width="49%">
</p>

Pak se vše nastavuje na webové stránce: GIFy (přetažením), hodiny, počasí, Wi-Fi, pevná IP adresa, heslo, jazyk
a dokonce i aktualizace firmwaru: stáhněte `kernel8.img` z poslední verze na
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), pak **Systém → Nový firmware**.
Jazyk webové stránky se volí vpravo nahoře, jazyk panelu v **Nastavení → Displej**.

## Přiložená písma, pozadí a GIFy

<p align="center"><img src="docs/images/polices.png" alt="Písma" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Animovaná pozadí" width="90%"></p>

- **Písma**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (svobodná, [Google Fonts](https://fonts.google.com/)) a
  3 pixelová písma OrbitDMD (LED, Tučné, 7 segmentů). Vlastní písma TTF přidáte na stránce Hodiny.
- **Animovaná pozadí a vzory**: vytvořené pro OrbitDMD (hvězdy, sníh, déšť, Matrix, oheň, polární záře, synthwave…).
- **GIFy**: OrbitDMD obsahuje **bezplatný základní balíček animovaných GIFů od RpiTeaM** (projekt RPI2DMD,
  pixel art od eLLuiGi, RattenJagera a dalších), se souhlasem eLLuiGi — děkujeme!
  Kdo chce víc: **balíček 11 000 animací** je k dispozici
  [na Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) nebo na [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Úvodní obrazovky a část vánočních GIFů jsou výtvory OrbitDMD.
  Postavy z videoher zůstávají majetkem jejich vlastníků.

## Navrhnout nápad, nahlásit problém

- 💡 **Nápad, vylepšení?** Otevřete [diskusi](https://github.com/ulrikadam/OrbitDMD/discussions)
  nebo [návrh](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Problém?** [Nahlaste ho](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) s verzí (je na přehledu)
  a pokud možno s protokolem ze stránky Systém.
- 🌍 **Lepší překlad?** Texty jsou v [outils/traductions](outils/traductions): opravy jsou vítány.
- 🛠️ **Chcete přispět kódem?** Přečtěte si [CONTRIBUTING.md](CONTRIBUTING.md) a [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (anglicky).

Novinky každé verze jsou v [přehledu verzí](JOURNAL-DES-VERSIONS.md) (francouzsky).

## Podpořit projekt

OrbitDMD je zdarma. Pokud se vám líbí, můžete mi koupit kávu:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Podpořit na Ko-fi"></a></p>

## Licence

OrbitDMD je svobodný software pod licencí **GNU GPL v3** (viz [LICENSE](LICENSE)), stejně jako
[Circle](https://github.com/rsta2/circle), na kterém je postaven. Licence ostatních součástí (písma, firmware
Raspberry Pi, data o počasí): viz [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Vyvinul <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
