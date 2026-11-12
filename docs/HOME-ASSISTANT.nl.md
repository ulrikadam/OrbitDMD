# OrbitDMD en Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · **Nederlands** · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Vanaf versie 1.4.0 verbindt het paneel met Home Assistant via **MQTT** en verschijnt het daar vanzelf
(automatische detectie). De namen van de entiteiten volgen de taal van het paneel (Instellingen → Weergave →
Taal van het paneel).

## Aan de slag

1. In Home Assistant: **Instellingen → Add-ons → Add-on-winkel**, installeer en start **Mosquitto broker**.
   Voeg daarna de integratie **MQTT** toe als Home Assistant die niet zelf voorstelt.
2. Maak een gebruiker voor het paneel (**Instellingen → Personen → Gebruikers**), bijvoorbeeld `orbitdmd`.
3. Op de webpagina van het paneel: **Instellingen → Home Assistant**: inschakelen, het **IP-adres** van
   Home Assistant invullen (bijvoorbeeld `192.168.0.10`), poort `1883`, de gebruiker en zijn wachtwoord,
   daarna opslaan. Het paneel herstart en maakt verbinding.
4. In Home Assistant: **Instellingen → Apparaten en diensten → MQTT → OrbitDMD**.

De webpagina van het paneel heeft ook een pagina **Hulp Home Assistant** (`http://<paneel>/aide`): alle
opdrachten en voorbeelden om te kopiëren, met de echte naam van je paneel.

De kaart Home Assistant op de pagina Instellingen toont of de verbinding werkt. Bij problemen bevat het
logboek op de pagina Systeem regels `homeassistant`.

## Wat er in Home Assistant verschijnt

| Entiteit | Type | Functie |
|---|---|---|
| Paneel | lamp | Aan, uit (de webpagina blijft bereikbaar), helderheid instellen |
| Helderheid per uur | schakelaar | Aan: de helderheid volgt de instellingen per uur van het paneel. Helderheid instellen vanuit Home Assistant schakelt dit uit |
| Pauze | schakelaar | Het beeld bevriezen |
| Tijd tonen, Weer tonen | knoppen | Meteen, 15 seconden lang |
| Herstarten | knop | |
| Bericht | melding | Een tekst meteen tonen |
| Te tonen tekst | tekst | Typ een tekst in Home Assistant en bevestig: hij wordt meteen getoond |
| Effect, Kleur, Aantal keer, Snelheid van berichten | lijsten en getallen | De stijl van teksten die vanuit Home Assistant worden verstuurd (blijft na een herstart) |
| Tekst: …, Afteller: … | schakelaars | Elke tekst en afteller van de pagina Berichten in- of uitschakelen |
| MQTT-opdrachten | sensor (diagnose) | De lijst van alle opdrachten, in de attributen |
| Getoonde GIF, Scherm | sensoren | Wat het paneel toont |
| Processortemperatuur, IP-adres, Versie | sensoren (diagnose) | |

Als het paneel uit staat of losgekoppeld is, toont Home Assistant het als "niet beschikbaar".

## Berichten

Het eenvoudigst, in een automatisering:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Er wordt aangebeld!"
```

(de exacte naam van de entiteit staat op de pagina van het apparaat OrbitDMD).

De stijl (effect, kleur, aantal keren, snelheid) is die van de onderdelen "… van berichten" van het apparaat.
Voor een eigen stijl voor één bericht publiceer je een JSON-tekst op het onderwerp `orbitdmd/<naam>/notification`
(`<naam>`: de netwerknaam van het paneel, in kleine letters):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Wasmachine klaar", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Veld | Waarden | Standaard |
|---|---|---|
| `message` | de tekst (maximaal 200 tekens) | |
| `effet` (effect) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (naar links, naar rechts, omhoog, omlaag, letter voor letter, regen van letters, vervagen) | `defile` |
| `couleur` (kleur) | `FF8000` (rood-groen-blauw) of `arc-en-ciel` (regenboog) | wit |
| `police` (lettertype) | `Pixel` of een lettertypebestand van het paneel, bijvoorbeeld `Orbitron.ttf` | `Pixel` |
| `taille` (grootte) | grootte in pixels, `0` = automatisch | `0` |
| `vitesse` (snelheid) | 1 (traag) tot 10 (snel) | 5 |
| `passages` (aantal keren) | 1 tot 10 | 2 |
| `gif` | een GIF om vóór de tekst af te spelen: `Categorie/naam.gif` | geen |

Het bericht komt meteen, over de GIF's heen, daarna gaat de rotatie verder. Als het paneel uit of
gepauzeerd is, wacht het bericht tot het weer aan is.

## Voorbeelden van automatiseringen

- Helderheid verlagen bij zonsondergang: actie `light.turn_on` op "Paneel" met `brightness_pct: 20`.
- Het paneel uitzetten als iedereen het huis verlaat: `light.turn_off`.
- Deurbel: een bericht "Er wordt aangebeld!" met `"gif": "Accueil/deurbel.gif"` (een GIF die je zelf
  in een categorie naar keuze hebt gezet).

## Beperkingen

- De MQTT-verbinding is niet versleuteld (geen TLS): gebruik ze op het thuisnetwerk.
- Het adres `homeassistant.local` werkt niet: vul het IP-adres van Home Assistant in.
