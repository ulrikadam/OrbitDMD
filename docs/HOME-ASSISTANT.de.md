# OrbitDMD und Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · **Deutsch** · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Seit Version 1.4.0 verbindet sich das Panel per **MQTT** mit Home Assistant und erscheint dort von selbst
(automatische Erkennung). Die Namen der Entitäten folgen der Sprache des Panels (Einstellungen → Anzeige →
Sprache des Panels).

## Einrichtung

1. In Home Assistant: **Einstellungen → Add-ons → Add-on-Store**, **Mosquitto broker** installieren und starten.
   Danach die Integration **MQTT** hinzufügen, falls Home Assistant sie nicht selbst anbietet.
2. Einen Benutzer für das Panel anlegen (**Einstellungen → Personen → Benutzer**), zum Beispiel `orbitdmd`.
3. Auf der Webseite des Panels: **Einstellungen → Home Assistant**: aktivieren, die **IP-Adresse** von
   Home Assistant eingeben (zum Beispiel `192.168.0.10`), Port `1883`, Benutzer und Passwort, dann speichern.
   Das Panel startet neu und verbindet sich.
4. In Home Assistant: **Einstellungen → Geräte & Dienste → MQTT → OrbitDMD**.

Die Webseite des Panels hat auch eine Seite **Home-Assistant-Hilfe** (`http://<Panel>/aide`): alle Befehle
und Beispiele zum Kopieren, mit dem echten Namen Ihres Panels.

Die Home-Assistant-Karte der Seite Einstellungen zeigt, ob die Verbindung steht. Bei Problemen enthält das
Protokoll der Seite System Zeilen mit `homeassistant`.

## Was in Home Assistant erscheint

| Entität | Typ | Funktion |
|---|---|---|
| Panel | Licht | Ein, aus (die Webseite bleibt erreichbar), Helligkeit einstellen |
| Helligkeit nach Uhrzeit | Schalter | An: die Helligkeit folgt den stündlichen Einstellungen des Panels. Helligkeit aus Home Assistant schaltet ihn aus |
| Pause | Schalter | Bild einfrieren |
| Uhrzeit anzeigen, Wetter anzeigen | Tasten | Sofort, für 15 Sekunden |
| Neustart | Taste | |
| Nachricht | Benachrichtigung | Einen Text sofort anzeigen |
| Anzuzeigender Text | Text | Text in Home Assistant eingeben und bestätigen: er wird sofort angezeigt |
| Nachrichteneffekt, -farbe, -durchläufe, -tempo | Listen und Zahlen | Stil der aus Home Assistant gesendeten Texte (bleibt nach einem Neustart) |
| Text: …, Countdown: … | Schalter | Jeden Text und Countdown der Seite Nachrichten aktivieren oder nicht |
| MQTT-Befehle | Sensor (Diagnose) | Liste aller Befehle, in den Attributen |
| Aktuelles GIF, Bildschirm | Sensoren | Was das Panel zeigt |
| Prozessortemperatur, IP-Adresse, Version | Sensoren (Diagnose) | |

Ist das Panel aus oder abgesteckt, zeigt Home Assistant es als „nicht verfügbar“.

## Nachrichten

Am einfachsten in einer Automatisierung:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Es hat geklingelt!"
```

(der genaue Name der Entität steht auf der Geräteseite von OrbitDMD).

Der Stil (Effekt, Farbe, Durchläufe, Tempo) ist der der Elemente „Nachrichten…“ des Geräts. Für einen eigenen
Stil einer einzelnen Nachricht veröffentlichen Sie einen JSON-Text im Topic `orbitdmd/<Name>/notification`
(`<Name>`: Netzwerkname des Panels in Kleinbuchstaben):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Waschmaschine fertig", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Feld | Werte | Standard |
|---|---|---|
| `message` | der Text (höchstens 200 Bytes) | |
| `effet` (Effekt) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (links, rechts, hoch, runter, Buchstabe für Buchstabe, Buchstabenregen, Einblenden) oder die Namen aus Home Assistant | `defile` |
| `couleur` (Farbe) | `FF8000` (Rot-Grün-Blau), `arc-en-ciel` (Regenbogen) oder ein Farbname aus Home Assistant | weiß |
| `police` (Schrift) | `Pixel` oder eine Schriftdatei des Panels, z. B. `Orbitron.ttf` | `Pixel` |
| `taille` (Größe) | Größe in Pixeln, `0` = automatisch | `0` |
| `vitesse` (Tempo) | 1 (langsam) bis 10 (schnell) | 5 |
| `passages` (Durchläufe) | 1 bis 10 | 2 |
| `gif` | ein GIF vor dem Text: `Kategorie/name.gif` | keins |

Die Nachricht läuft sofort über die GIFs, danach geht die Rotation weiter. Ist das Panel aus oder pausiert,
wartet die Nachricht.

## Beispiele

- Helligkeit bei Sonnenuntergang senken: `light.turn_on` auf „Panel“ mit `brightness_pct: 20`.
- Panel ausschalten, wenn alle das Haus verlassen: `light.turn_off`.
- Türklingel: Nachricht „Es hat geklingelt!“ mit `"gif": "Accueil/klingel.gif"` (ein von Ihnen hinzugefügtes GIF).

## Grenzen

- MQTT-Verbindung unverschlüsselt (kein TLS): im Heimnetz verwenden.
- Die Adresse `homeassistant.local` funktioniert nicht: die IP-Adresse von Home Assistant eingeben.
