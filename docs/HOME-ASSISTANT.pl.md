# OrbitDMD i Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · **Polski** · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Od wersji 1.4.0 panel łączy się z Home Assistant przez **MQTT** i sam się tam pojawia (automatyczne
wykrywanie). Nazwy encji są w języku panelu (Ustawienia → Wyświetlacz → Język panelu).

## Pierwsze kroki

1. W Home Assistant: **Ustawienia → Dodatki → Sklep z dodatkami**, zainstaluj i uruchom **Mosquitto broker**.
   Potem dodaj integrację **MQTT**, jeśli Home Assistant nie zaproponuje jej sam.
2. Utwórz użytkownika dla panelu (**Ustawienia → Osoby → Użytkownicy**), na przykład `orbitdmd`.
3. Na stronie WWW panelu: **Ustawienia → Home Assistant**: włącz, wpisz **adres IP** Home Assistant
   (na przykład `192.168.0.10`), port `1883`, użytkownika i hasło, a potem zapisz. Panel uruchomi się ponownie
   i połączy.
4. W Home Assistant: **Ustawienia → Urządzenia i usługi → MQTT → OrbitDMD**.

Strona WWW panelu ma też stronę **Pomoc Home Assistant** (`http://<panel>/aide`): wszystkie polecenia
i przykłady do skopiowania, z prawdziwą nazwą twojego panelu.

Karta Home Assistant na stronie Ustawienia pokazuje, czy połączenie działa. W razie problemów dziennik na
stronie System zawiera wiersze `homeassistant`.

## Co pojawia się w Home Assistant

| Encja | Typ | Funkcja |
|---|---|---|
| Panel | światło | Włączanie, wyłączanie (strona WWW pozostaje dostępna), jasność |
| Jasność godzinowa | przełącznik | Włączony: jasność według ustawień godzinowych. Zmiana jasności z Home Assistant go wyłącza |
| Pauza | przełącznik | Zatrzymanie obrazu |
| Pokaż godzinę, Pokaż pogodę | przyciski | Od razu, przez 15 sekund |
| Uruchom ponownie | przycisk | |
| Wiadomość | powiadomienie | Natychmiastowe wyświetlenie tekstu |
| Tekst do wyświetlenia | tekst | Wpisz tekst w Home Assistant i zatwierdź: od razu się pojawi |
| Efekt, Kolor, Przebiegi, Prędkość wiadomości | listy i liczby | Styl tekstów wysyłanych z Home Assistant (zachowany po ponownym uruchomieniu) |
| Tekst: …, Odliczanie: … | przełączniki | Włączanie każdego tekstu i odliczania ze strony Wiadomości |
| Polecenia MQTT | czujnik (diagnostyka) | Lista wszystkich poleceń w atrybutach |
| Wyświetlany GIF, Ekran | czujniki | Co pokazuje panel |
| Temperatura procesora, Adres IP, Wersja | czujniki (diagnostyka) | |

Jeśli panel jest wyłączony lub odłączony, Home Assistant pokazuje go jako „niedostępny”.

## Wiadomości

Najprościej w automatyzacji:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Ktoś dzwoni do drzwi!"
```

(dokładna nazwa encji jest na stronie urządzenia OrbitDMD).

Styl (efekt, kolor, przebiegi, prędkość) to ten z elementów „… wiadomości”. Aby nadać osobny styl jednej
wiadomości, opublikuj tekst JSON w temacie `orbitdmd/<nazwa>/notification` (`<nazwa>`: sieciowa nazwa panelu,
małymi literami):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Pranie skończone", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Pole | Wartości | Domyślnie |
|---|---|---|
| `message` | tekst (najwyżej 200 bajtów; polska litera zajmuje 2) | |
| `effet` (efekt) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (w lewo, w prawo, w górę, w dół, litera po literze, deszcz liter, przenikanie) lub nazwy z Home Assistant | `defile` |
| `couleur` (kolor) | `FF8000` (czerwony-zielony-niebieski), `arc-en-ciel` (tęcza) lub nazwa koloru z Home Assistant | biały |
| `police` (czcionka) | `Pixel` lub plik czcionki panelu, np. `Orbitron.ttf` | `Pixel` |
| `taille` (rozmiar) | rozmiar w pikselach, `0` = automatycznie | `0` |
| `vitesse` (prędkość) | od 1 (wolno) do 10 (szybko) | 5 |
| `passages` (przebiegi) | od 1 do 10 | 2 |
| `gif` | GIF przed tekstem: `Kategoria/nazwa.gif` | brak |

Wiadomość pojawia się od razu, na GIF-ach, potem rotacja trwa dalej. Jeśli panel jest wyłączony lub
wstrzymany, wiadomość czeka.

## Przykłady

- Ściemnianie o zachodzie słońca: `light.turn_on` na „Panel” z `brightness_pct: 20`.
- Wyłączanie panelu, gdy wszyscy wychodzą z domu: `light.turn_off`.
- Dzwonek: wiadomość „Ktoś dzwoni do drzwi!” z `"gif": "Accueil/dzwonek.gif"` (GIF dodany przez ciebie).

## Ograniczenia

- Połączenie MQTT nie jest szyfrowane (bez TLS): używaj w sieci domowej.
- Adres `homeassistant.local` nie działa: wpisz adres IP Home Assistant.
