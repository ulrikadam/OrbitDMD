# OrbitDMD and Home Assistant

[Français](HOME-ASSISTANT.md) · **English** · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Since version 1.4.0, the panel connects to Home Assistant through **MQTT** and shows up there by itself
(automatic discovery). The entity names follow the panel language (Settings → Display → Panel language).

## Getting started

1. In Home Assistant: **Settings → Add-ons → Add-on store**, install and start **Mosquitto broker**.
   Then add the **MQTT** integration if Home Assistant does not offer it by itself.
2. Create a user for the panel (**Settings → People → Users**), for example `orbitdmd`.
3. On the panel's web page: **Settings → Home Assistant**: enable it, enter the **IP address** of
   Home Assistant (for example `192.168.0.10`), port `1883`, the user and its password, then save.
   The panel restarts and connects.
4. In Home Assistant: **Settings → Devices & services → MQTT → OrbitDMD**.

The panel's web page also has a **Home Assistant help** page (`http://<panel>/aide`): all the commands
and ready-to-copy examples, with the real name of your panel.

The Home Assistant card on the Settings page shows whether the connection works. If something goes wrong,
the log on the System page contains `homeassistant` lines.

## What appears in Home Assistant

| Entity | Type | Role |
|---|---|---|
| Panel | light | Turn on, turn off (the web page stays reachable), set the brightness |
| Hourly brightness | switch | On: the brightness follows the panel's hour-by-hour settings. Setting the brightness from Home Assistant turns it off |
| Pause | switch | Freeze the image |
| Show time, Show weather | buttons | Right away, for 15 seconds |
| Restart | button | |
| Message | notification | Show a text right away |
| Text to show | text | Type a text in Home Assistant and confirm: it is shown right away |
| Message effect, colour, passes, speed | selects and numbers | The style of texts sent from Home Assistant (kept after a restart) |
| Text: …, Countdown: … | switches | Enable or disable each text and countdown of the Messages page |
| MQTT commands | sensor (diagnostic) | The list of all commands, in its attributes |
| Current GIF, Screen | sensors | What the panel is showing |
| Processor temperature, IP address, Version | sensors (diagnostic) | |

If the panel is off or unplugged, Home Assistant shows it as "unavailable".

## Messages

The simplest way, in an automation:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Someone is at the door!"
```

(the exact entity name is shown on the page of the OrbitDMD device).

The style (effect, colour, passes, speed) is the one set in the "Message …" items of the device.
For a different style for a single message, publish a JSON text on the topic `orbitdmd/<name>/notification`
(`<name>`: the panel's network name, in lower case):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Washing machine done", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Field | Values | Default |
|---|---|---|
| `message` | the text (200 characters at most) | |
| `effet` (effect) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (scroll left, scroll right, rise, fall, letter by letter, raining letters, fade) | `defile` |
| `couleur` (colour) | `FF8000` (red-green-blue) or `arc-en-ciel` (rainbow) | white |
| `police` (font) | `Pixel` or a font file on the panel, for example `Orbitron.ttf` | `Pixel` |
| `taille` (size) | size in pixels, `0` = automatic | `0` |
| `vitesse` (speed) | 1 (slow) to 10 (fast) | 5 |
| `passages` (passes) | 1 to 10 | 2 |
| `gif` | a GIF to play before the text: `Category/name.gif` | none |

The message plays right away, over the GIFs, then the rotation resumes. If the panel is off or paused,
the message waits until it is back on.

## Automation examples

- Dim the panel at sunset: action `light.turn_on` on "Panel" with `brightness_pct: 20`.
- Turn the panel off when everybody leaves home: `light.turn_off`.
- Doorbell: a message "Someone is at the door!" with `"gif": "Accueil/doorbell.gif"` (a GIF you added
  to the category of your choice).

## Limits

- The MQTT connection is not encrypted (no TLS): use it on your home network.
- The address `homeassistant.local` does not work: enter the IP address of Home Assistant.
