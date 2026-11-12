<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <b>Italiano</b> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>I tuoi pannelli LED HUB75 su Raspberry Pi 3: GIF animate, orologio e meteo, regolabili dal browser.</b><br>
  Senza Linux, si avvia in pochi secondi. Installazione in 5 passi da Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Scarica" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Scarica&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Idee e domande" src="https://img.shields.io/badge/Idee%20e%20domande-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Sostieni su Ko-fi" src="https://img.shields.io/badge/Sostieni-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licenza GPL v3" src="https://img.shields.io/badge/licenza-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Cosa fa OrbitDMD

OrbitDMD trasforma un **Raspberry Pi 3** e dei **pannelli LED HUB75** (per esempio 2 × 64×32) in un display
«DMD» come quello di un flipper o di un cabinato arcade:

- 🎞️ **GIF animate** per categorie (562 incluse), aggiunte dalla pagina web una per una, per cartella intera
  o in .zip; GIF preferite e GIF fuori rotazione;
- 🕒 **orologio**: 14 caratteri, colori, arcobaleno animato, motivi, 13 sfondi animati, scelta casuale, data,
  sincronizzazione via Internet;
- 💬 **messaggi**: testi scorrevoli e conti alla rovescia (Natale, Capodanno, data scelta), ognuno con il suo
  stile ed effetto di entrata (scorrimento, salita, lettera per lettera, pioggia di lettere, dissolvenza…);
- 🏠 **Home Assistant**: il pannello compare da solo (MQTT); accensione, luminosità e messaggi dalle tue
  automazioni ([guida](docs/HOME-ASSISTANT.it.md));
- 🗓️ **programma orario**: pannello spento di notte o solo l'ora in certe fasce, categorie di GIF per mese
  (Natale a dicembre…), testi e conti alla rovescia attivi solo tra due date;
- 📐 **GIF troppo grandi ridotte da sole** al formato del pannello all'invio;
- ⛅ **meteo** della tua città (Open-Meteo, senza registrazione) con icone animate;
- 🌐 **pagina web** sulla tua rete (`http://orbitdmd.local/`): anteprima in diretta del pannello, impostazioni,
  luminosità ora per ora, pausa, spegnimento, backup delle impostazioni, aggiornamenti;
- 🌍 **12 lingue** per pagina web, installazione, testi del pannello e Home Assistant (italiano compreso);
- ⚡ **senza Linux**: il programma parte direttamente sul Raspberry Pi (basato su
  [Circle](https://github.com/rsta2/circle)), in pochi secondi.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Pannello di controllo" width="49%">
  <img src="docs/images/horloge.png" alt="Impostazioni dell'orologio" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Meteo" width="49%">
  <img src="docs/images/gif.png" alt="Gestione delle GIF" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Messaggi: testi scorrevoli" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Messaggi: conti alla rovescia" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Impostazioni: programma e Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Aiuto Home Assistant" width="49%">
</p>

## Hardware

| Elemento | Dettaglio |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (modello B o B+) con alimentatore |
| Pannelli | HUB75, di serie 2 × 64×32 affiancati (128×32); possibili 64×64, 32×16 e fino a 8 pannelli |
| Scheda adattatrice | HUB75 per Raspberry Pi, piedinatura **«regular»** di rpi-rgb-led-matrix (es. Electrodragon); la piedinatura dell'HAT Adafruit non è supportata |
| Alimentazione dei pannelli | 5 V, sufficiente per i tuoi pannelli (circa 4 A per pannello 64×32 alla massima luminosità) |
| Scheda micro-SD | consigliata da 32 GB o meno (viene creata una partizione FAT32 di 31 GB al massimo) |
| Rete | Cavo di rete (Ethernet) o Wi-Fi 2,4 GHz (il Raspberry Pi 3 non vede i 5 GHz) |

## Installazione

1. **Scarica** `OrbitDMD-Installateur-<versione>.exe` dalla pagina
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 o 11, nient'altro da installare).
2. **Avvialo.** Il programma non è firmato: se Windows mostra «PC protetto da Windows», clicca su
   *Ulteriori informazioni* e poi *Esegui comunque*. L'installazione usa la lingua di Windows (scegli sulla prima pagina).
3. **Rispondi alle domande**: nome del pannello, disposizione dei pannelli, Wi-Fi, password della pagina web,
   fuso orario, città per il meteo, GIF da installare.
4. **Scegli la scheda micro-SD**. Vengono proposti solo lettori di schede e chiavette USB, mai i dischi interni;
   tutto il contenuto della scheda scelta viene cancellato.
5. **Inserisci la scheda nel Raspberry Pi** e collegalo. Dopo una trentina di secondi apri
   `http://<nome del pannello>.local/` (o l'indirizzo IP dato dal router).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Installazione: pannelli" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Installazione: scheda SD" width="49%">
</p>

Poi tutto si regola dalla pagina web: GIF (trascina e rilascia), orologio, meteo, Wi-Fi, IP fisso, password,
lingua e perfino l'aggiornamento del firmware: scarica `kernel8.img` dall'ultima versione su
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), poi **Sistema → Nuovo firmware**.
La lingua della pagina web si sceglie in alto a destra, quella del pannello in **Impostazioni → Display**.

## Caratteri, sfondi e GIF inclusi

<p align="center"><img src="docs/images/polices.png" alt="Caratteri" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Sfondi animati" width="90%"></p>

- **Caratteri**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (liberi, [Google Fonts](https://fonts.google.com/)) e
  3 caratteri pixel OrbitDMD (LED, Grassetto, 7 segmenti). I tuoi caratteri TTF si aggiungono dalla pagina Orologio.
- **Sfondi animati e motivi**: creati per OrbitDMD (stelle, neve, pioggia, Matrix, fuoco, aurora, synthwave…).
- **GIF**: OrbitDMD include il **pacchetto base gratuito di GIF animate della RpiTeaM** (progetto RPI2DMD,
  pixel art di eLLuiGi, RattenJager e altri), con il consenso di eLLuiGi: grazie!
  Per chi vuole di più, **un pacchetto di 11.000 animazioni** è disponibile
  [su Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) o su [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Le schermate di avvio e parte delle GIF di Natale sono creazioni OrbitDMD.
  I personaggi dei videogiochi restano di proprietà dei rispettivi titolari.

## Proporre un'idea, segnalare un problema

- 💡 **Un'idea, un miglioramento?** Apri una [discussione](https://github.com/ulrikadam/OrbitDMD/discussions)
  o una [proposta](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Un problema?** [Segnalalo](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) indicando la versione
  (sul pannello di controllo) e, se possibile, il registro della pagina Sistema.
- 🌍 **Una traduzione migliore?** I testi sono in [outils/traductions](outils/traductions): le correzioni sono benvenute.
- 🛠️ **Vuoi contribuire al codice?** Leggi [CONTRIBUTING.md](CONTRIBUTING.md) e [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (in inglese).

Le novità di ogni versione sono nel [registro delle versioni](JOURNAL-DES-VERSIONS.md) (in francese).

## Sostenere il progetto

OrbitDMD è gratuito. Se ti piace, puoi offrirmi un caffè:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Sostieni su Ko-fi"></a></p>

## Licenza

OrbitDMD è software libero con licenza **GNU GPL v3** (vedi [LICENSE](LICENSE)), come
[Circle](https://github.com/rsta2/circle) su cui si basa. Licenze degli altri elementi (caratteri, firmware del
Raspberry Pi, dati meteo): vedi [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Sviluppato da <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
