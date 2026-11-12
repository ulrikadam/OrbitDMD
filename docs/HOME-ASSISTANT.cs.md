# OrbitDMD a Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · **Čeština**

Od verze 1.4.0 se panel připojuje k Home Assistant přes **MQTT** a objeví se tam sám (automatické
zjišťování). Názvy entit odpovídají jazyku panelu (Nastavení → Displej → Jazyk panelu).

## Začínáme

1. V Home Assistant: **Nastavení → Doplňky → Obchod s doplňky**, nainstalujte a spusťte **Mosquitto broker**.
   Pak přidejte integraci **MQTT**, pokud ji Home Assistant nenabídne sám.
2. Vytvořte pro panel uživatele (**Nastavení → Osoby → Uživatelé**), například `orbitdmd`.
3. Na webové stránce panelu: **Nastavení → Home Assistant**: zapněte, zadejte **IP adresu** Home Assistant
   (například `192.168.0.10`), port `1883`, uživatele a heslo, pak uložte. Panel se restartuje a připojí.
4. V Home Assistant: **Nastavení → Zařízení a služby → MQTT → OrbitDMD**.

Webová stránka panelu má také stránku **Nápověda Home Assistant** (`http://<panel>/aide`): všechny příkazy
a příklady ke zkopírování se skutečným názvem vašeho panelu.

Karta Home Assistant na stránce Nastavení ukazuje, zda připojení funguje. Při potížích obsahuje protokol na
stránce Systém řádky `homeassistant`.

## Co se objeví v Home Assistant

| Entita | Typ | Účel |
|---|---|---|
| Panel | světlo | Zapnout, vypnout (webová stránka zůstává dostupná), nastavit jas |
| Jas podle hodin | spínač | Zapnuto: jas se řídí hodinovým nastavením panelu. Změna jasu z Home Assistant ho vypne |
| Pauza | spínač | Zastavit obraz |
| Zobrazit čas, Zobrazit počasí | tlačítka | Hned, na 15 sekund |
| Restartovat | tlačítko | |
| Zpráva | oznámení | Hned zobrazit text |
| Text k zobrazení | text | Napište text v Home Assistant a potvrďte: hned se zobrazí |
| Efekt, Barva, Průchody, Rychlost zpráv | seznamy a čísla | Styl textů posílaných z Home Assistant (zachová se po restartu) |
| Text: …, Odpočet: … | spínače | Zapnout každý text a odpočet ze stránky Zprávy |
| Příkazy MQTT | senzor (diagnostika) | Seznam všech příkazů v atributech |
| Zobrazený GIF, Obrazovka | senzory | Co panel ukazuje |
| Teplota procesoru, IP adresa, Verze | senzory (diagnostika) | |

Pokud je panel vypnutý nebo odpojený, Home Assistant ho ukáže jako „nedostupný“.

## Zprávy

Nejjednodušeji v automatizaci:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Někdo zvoní!"
```

(přesný název entity je vidět na stránce zařízení OrbitDMD).

Styl (efekt, barva, průchody, rychlost) je ten z prvků „… zpráv“. Pro vlastní styl jedné zprávy publikujte
JSON do tématu `orbitdmd/<název>/notification` (`<název>`: síťový název panelu malými písmeny):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Pračka doprala", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Pole | Hodnoty | Výchozí |
|---|---|---|
| `message` | text (nejvýše 200 bajtů) | |
| `effet` (efekt) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (doleva, doprava, nahoru, dolů, písmeno po písmenu, déšť písmen, prolínání) nebo názvy z Home Assistant | `defile` |
| `couleur` (barva) | `FF8000` (červená-zelená-modrá), `arc-en-ciel` (duha) nebo název barvy z Home Assistant | bílá |
| `police` (písmo) | `Pixel` nebo soubor písma na panelu, např. `Orbitron.ttf` | `Pixel` |
| `taille` (velikost) | velikost v pixelech, `0` = automaticky | `0` |
| `vitesse` (rychlost) | 1 (pomalu) až 10 (rychle) | 5 |
| `passages` (průchody) | 1 až 10 | 2 |
| `gif` | GIF před textem: `Kategorie/název.gif` | žádný |

Zpráva se zobrazí hned, přes GIFy, pak rotace pokračuje. Pokud je panel vypnutý nebo pozastavený, zpráva počká.

## Příklady

- Snížit jas při západu slunce: `light.turn_on` pro „Panel“ s `brightness_pct: 20`.
- Vypnout panel, když všichni odejdou z domu: `light.turn_off`.
- Zvonek: zpráva „Někdo zvoní!“ s `"gif": "Accueil/zvonek.gif"` (GIF, který jste přidali).

## Omezení

- Připojení MQTT není šifrované (bez TLS): používejte ho v domácí síti.
- Adresa `homeassistant.local` nefunguje: zadejte IP adresu Home Assistant.
