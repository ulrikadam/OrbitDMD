<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <b>Polski</b> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Twoje panele LED HUB75 na Raspberry Pi 3: animowane GIF-y, zegar i pogoda, ustawiane z przeglądarki.</b><br>
  Bez Linuksa, start w kilka sekund. Instalacja w 5 krokach z Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Pobierz" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Pobierz&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Pomysły i pytania" src="https://img.shields.io/badge/Pomys%C5%82y%20i%20pytania-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Wesprzyj na Ko-fi" src="https://img.shields.io/badge/Wesprzyj-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licencja GPL v3" src="https://img.shields.io/badge/licencja-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Co robi OrbitDMD

OrbitDMD zamienia **Raspberry Pi 3** i **panele LED HUB75** (na przykład 2 × 64×32) w wyświetlacz „DMD”
jak we flipperze lub automacie arcade:

- 🎞️ **animowane GIF-y** w kategoriach (562 w zestawie), dodawane przez stronę WWW pojedynczo, całymi
  folderami lub jako .zip; ulubione GIF-y i GIF-y poza rotacją;
- 🕒 **zegar**: 14 czcionek, kolory, animowana tęcza, wzory, 13 animowanych teł, losowy wybór, data,
  synchronizacja przez Internet;
- 💬 **wiadomości**: przewijane teksty i odliczania (Boże Narodzenie, Nowy Rok, dowolna data), każde z własnym
  stylem i efektem wejścia (przewijanie, wjazd w górę, litera po literze, deszcz liter, przenikanie…);
- 🏠 **Home Assistant**: panel sam się tam pojawia (MQTT); włączanie, jasność i wiadomości z twoich
  automatyzacji ([poradnik](docs/HOME-ASSISTANT.pl.md));
- 🗓️ **harmonogram**: panel wyłączony w nocy lub tylko godzina w wybranych godzinach, kategorie GIF-ów według
  miesięcy (Boże Narodzenie w grudniu…), teksty i odliczania aktywne tylko między dwiema datami;
- 📐 **za duże GIF-y same zmniejszane** do formatu panelu przy wysyłaniu;
- ⛅ **pogoda** dla twojego miasta (Open-Meteo, bez rejestracji) z animowanymi ikonami;
- 🌐 **strona WWW** w twojej sieci (`http://orbitdmd.local/`): podgląd panelu na żywo, ustawienia, jasność
  godzinowa, pauza, wyłączanie, kopia zapasowa ustawień, aktualizacje;
- 🌍 **12 języków** strony WWW, instalatora, tekstów panelu i Home Assistant (w tym polski);
- ⚡ **bez Linuksa**: program startuje bezpośrednio na Raspberry Pi (oparty na
  [Circle](https://github.com/rsta2/circle)), w kilka sekund.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Pulpit" width="49%">
  <img src="docs/images/horloge.png" alt="Ustawienia zegara" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Pogoda" width="49%">
  <img src="docs/images/gif.png" alt="Zarządzanie GIF-ami" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Wiadomości: przewijane teksty" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Wiadomości: odliczania" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Ustawienia: harmonogram i Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Pomoc Home Assistant" width="49%">
</p>

## Sprzęt

| Element | Szczegóły |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (model B lub B+) z zasilaczem |
| Panele | HUB75, domyślnie 2 × 64×32 obok siebie (128×32); możliwe 64×64, 32×16 i do 8 paneli |
| Płytka adaptera | HUB75 dla Raspberry Pi, układ **„regular”** z rpi-rgb-led-matrix (np. Electrodragon); układ Adafruit HAT nie jest obsługiwany |
| Zasilanie paneli | 5 V, wystarczające dla paneli (ok. 4 A na panel 64×32 przy pełnej jasności) |
| Karta micro-SD | zalecane 32 GB lub mniej (tworzona jest partycja FAT32 do 31 GB) |
| Sieć | Kabel sieciowy (Ethernet) lub Wi-Fi 2,4 GHz (Raspberry Pi 3 nie widzi 5 GHz) |

## Instalacja

1. **Pobierz** `OrbitDMD-Installateur-<wersja>.exe` ze strony
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 lub 11, nic więcej nie trzeba).
2. **Uruchom go.** Program nie jest podpisany: jeśli Windows pokaże „System Windows ochronił ten komputer”, kliknij
   *Więcej informacji*, a potem *Uruchom mimo to*. Instalator mówi w języku Windows (wybór na pierwszej stronie).
3. **Odpowiedz na pytania**: nazwa panelu, układ paneli, Wi-Fi, hasło strony WWW, strefa czasowa, miasto dla
   pogody, GIF-y do instalacji.
4. **Wybierz kartę micro-SD**. Pokazywane są tylko czytniki kart i pamięci USB, nigdy dyski wewnętrzne; cała
   zawartość wybranej karty jest usuwana.
5. **Włóż kartę do Raspberry Pi** i podłącz zasilanie. Po około trzydziestu sekundach otwórz
   `http://<nazwa panelu>.local/` (lub adres IP nadany przez router).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Instalator: panele" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Instalator: karta SD" width="49%">
</p>

Potem wszystko ustawia się na stronie WWW: GIF-y (przeciągnij i upuść), zegar, pogoda, Wi-Fi, stały adres IP,
hasło, język, a nawet aktualizacja oprogramowania: pobierz `kernel8.img` z najnowszej wersji na
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), potem **System → Nowe oprogramowanie**.
Język strony WWW wybiera się w prawym górnym rogu, język panelu w **Ustawienia → Wyświetlacz**.

## Dołączone czcionki, tła i GIF-y

<p align="center"><img src="docs/images/polices.png" alt="Czcionki" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Animowane tła" width="90%"></p>

- **Czcionki**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (wolne, [Google Fonts](https://fonts.google.com/)) oraz
  3 czcionki pikselowe OrbitDMD (LED, Pogrubiona, 7 segmentów). Własne czcionki TTF dodaje się na stronie Zegar.
- **Animowane tła i wzory**: stworzone dla OrbitDMD (gwiazdy, śnieg, deszcz, Matrix, ogień, zorza, synthwave…).
- **GIF-y**: OrbitDMD zawiera **darmowy podstawowy pakiet animowanych GIF-ów od RpiTeaM** (projekt RPI2DMD,
  pixel art eLLuiGi, RattenJagera i innych), za zgodą eLLuiGi — dziękujemy!
  Dla chętnych **pakiet 11 000 animacji** jest dostępny
  [na Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) lub na [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Ekrany startowe i część świątecznych GIF-ów to dzieła OrbitDMD.
  Postacie z gier wideo pozostają własnością ich właścicieli.

## Zaproponuj pomysł, zgłoś problem

- 💡 **Pomysł, ulepszenie?** Otwórz [dyskusję](https://github.com/ulrikadam/OrbitDMD/discussions)
  lub [propozycję](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Problem?** [Zgłoś go](https://github.com/ulrikadam/OrbitDMD/issues/new/choose), podając wersję (na pulpicie)
  i w miarę możliwości dziennik ze strony System.
- 🌍 **Lepsze tłumaczenie?** Teksty są w [outils/traductions](outils/traductions): poprawki mile widziane.
- 🛠️ **Chcesz pomóc w kodzie?** Przeczytaj [CONTRIBUTING.md](CONTRIBUTING.md) i [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (po angielsku).

Nowości każdej wersji są w [dzienniku wersji](JOURNAL-DES-VERSIONS.md) (po francusku).

## Wesprzyj projekt

OrbitDMD jest darmowy. Jeśli ci się podoba, możesz postawić mi kawę:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Wesprzyj na Ko-fi"></a></p>

## Licencja

OrbitDMD to wolne oprogramowanie na licencji **GNU GPL v3** (zob. [LICENSE](LICENSE)), tak jak
[Circle](https://github.com/rsta2/circle), na którym jest zbudowany. Licencje pozostałych elementów (czcionki,
oprogramowanie układowe Raspberry Pi, dane pogodowe): zob. [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Autor: <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
