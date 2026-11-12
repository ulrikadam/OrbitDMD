# OrbitDMD e Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · **Italiano** · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Dalla versione 1.4.0 il pannello si collega a Home Assistant tramite **MQTT** e vi compare da solo
(rilevamento automatico). I nomi delle entità seguono la lingua del pannello (Impostazioni → Display →
Lingua del pannello).

## Primi passi

1. In Home Assistant: **Impostazioni → Componenti aggiuntivi → Negozio**, installa e avvia **Mosquitto broker**.
   Poi aggiungi l'integrazione **MQTT** se Home Assistant non la propone da solo.
2. Crea un utente per il pannello (**Impostazioni → Persone → Utenti**), per esempio `orbitdmd`.
3. Sulla pagina web del pannello: **Impostazioni → Home Assistant**: attiva, inserisci l'**indirizzo IP** di
   Home Assistant (per esempio `192.168.0.10`), la porta `1883`, l'utente e la sua password, poi salva.
   Il pannello si riavvia e si connette.
4. In Home Assistant: **Impostazioni → Dispositivi e servizi → MQTT → OrbitDMD**.

La pagina web del pannello ha anche una pagina **Aiuto Home Assistant** (`http://<pannello>/aide`): tutti i
comandi ed esempi pronti da copiare, con il vero nome del tuo pannello.

La scheda Home Assistant della pagina Impostazioni indica se la connessione funziona. In caso di problemi,
il registro della pagina Sistema contiene righe `homeassistant`.

## Cosa compare in Home Assistant

| Entità | Tipo | Funzione |
|---|---|---|
| Pannello | luce | Accendere, spegnere (la pagina web resta raggiungibile), regolare la luminosità |
| Luminosità oraria | interruttore | Attivo: la luminosità segue le impostazioni ora per ora. Regolarla da Home Assistant lo disattiva |
| Pausa | interruttore | Bloccare l'immagine |
| Mostra l'ora, Mostra il meteo | pulsanti | Subito, per 15 secondi |
| Riavvia | pulsante | |
| Messaggio | notifica | Mostrare subito un testo |
| Testo da mostrare | testo | Scrivi un testo in Home Assistant e conferma: viene mostrato subito |
| Effetto, Colore, Passaggi, Velocità dei messaggi | elenchi e numeri | Lo stile dei testi inviati da Home Assistant (conservato dopo un riavvio) |
| Testo: …, Conto alla rovescia: … | interruttori | Attivare o no ogni testo e conto alla rovescia della pagina Messaggi |
| Comandi MQTT | sensore (diagnostica) | L'elenco di tutti i comandi, negli attributi |
| GIF mostrata, Schermo | sensori | Ciò che mostra il pannello |
| Temperatura del processore, Indirizzo IP, Versione | sensori (diagnostica) | |

Se il pannello è spento o scollegato, Home Assistant lo indica come «non disponibile».

## Messaggi

Il modo più semplice, in un'automazione:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Suonano alla porta!"
```

(il nome esatto dell'entità è visibile nella pagina del dispositivo OrbitDMD).

Lo stile (effetto, colore, passaggi, velocità) è quello impostato negli elementi «… dei messaggi». Per uno stile
a parte in un solo messaggio, pubblica un testo JSON sul topic `orbitdmd/<nome>/notification`
(`<nome>`: il nome di rete del pannello, in minuscolo):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Lavatrice finita", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Campo | Valori | Predefinito |
|---|---|---|
| `message` | il testo (200 byte al massimo) | |
| `effet` (effetto) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (sinistra, destra, sale, scende, lettera per lettera, pioggia di lettere, dissolvenza) o i nomi di Home Assistant | `defile` |
| `couleur` (colore) | `FF8000` (rosso-verde-blu), `arc-en-ciel` (arcobaleno) o un nome di colore di Home Assistant | bianco |
| `police` (carattere) | `Pixel` o un file di carattere del pannello, es. `Orbitron.ttf` | `Pixel` |
| `taille` (dimensione) | dimensione in pixel, `0` = automatica | `0` |
| `vitesse` (velocità) | da 1 (lenta) a 10 (veloce) | 5 |
| `passages` (passaggi) | da 1 a 10 | 2 |
| `gif` | una GIF prima del testo: `Categoria/nome.gif` | nessuna |

Il messaggio passa subito, sopra le GIF, poi la rotazione riprende. Se il pannello è spento o in pausa, il
messaggio aspetta.

## Esempi

- Abbassare la luminosità al tramonto: `light.turn_on` su «Pannello» con `brightness_pct: 20`.
- Spegnere il pannello quando tutti escono di casa: `light.turn_off`.
- Campanello: messaggio «Suonano alla porta!» con `"gif": "Accueil/campanello.gif"` (una GIF aggiunta da te).

## Limiti

- Connessione MQTT non cifrata (niente TLS): da usare sulla rete di casa.
- L'indirizzo `homeassistant.local` non funziona: inserisci l'indirizzo IP di Home Assistant.
