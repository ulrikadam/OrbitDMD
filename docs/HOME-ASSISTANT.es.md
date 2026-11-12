# OrbitDMD y Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · **Español** · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · [Türkçe](HOME-ASSISTANT.tr.md) · [Čeština](HOME-ASSISTANT.cs.md)

Desde la versión 1.4.0, el panel se conecta a Home Assistant por **MQTT** y aparece allí solo
(detección automática). Los nombres de las entidades siguen el idioma del panel (Ajustes → Pantalla → Idioma
del panel).

## Puesta en marcha

1. En Home Assistant: **Ajustes → Complementos → Tienda de complementos**, instale e inicie
   **Mosquitto broker**. Después añada la integración **MQTT** si Home Assistant no la propone sola.
2. Cree un usuario para el panel (**Ajustes → Personas → Usuarios**), por ejemplo `orbitdmd`.
3. En la página web del panel: **Ajustes → Home Assistant**: active, indique la **dirección IP** de
   Home Assistant (por ejemplo `192.168.0.10`), el puerto `1883`, el usuario y su contraseña, y guarde.
   El panel se reinicia y se conecta.
4. En Home Assistant: **Ajustes → Dispositivos y servicios → MQTT → OrbitDMD**.

La página web del panel tiene también una página **Ayuda de Home Assistant** (`http://<panel>/aide`):
todos los comandos y ejemplos listos para copiar, con el nombre real de su panel.

La tarjeta Home Assistant de la página Ajustes indica si la conexión funciona. Si hay algún problema,
el registro de la página Sistema contiene líneas `homeassistant`.

## Qué aparece en Home Assistant

| Entidad | Tipo | Función |
|---|---|---|
| Panel | luz | Encender, apagar (la página web sigue accesible), ajustar el brillo |
| Brillo por horas | interruptor | Activado: el brillo sigue los ajustes hora por hora del panel. Ajustar el brillo desde Home Assistant lo desactiva |
| Pausa | interruptor | Congelar la imagen |
| Mostrar la hora, Mostrar el tiempo | botones | Enseguida, durante 15 segundos |
| Reiniciar | botón | |
| Mensaje | notificación | Mostrar un texto enseguida |
| Texto a mostrar | texto | Escriba un texto en Home Assistant y valide: se muestra enseguida |
| Efecto, Color, Pasadas, Velocidad de los mensajes | listas y números | El estilo de los textos enviados desde Home Assistant (se conserva tras un reinicio) |
| Texto: …, Cuenta atrás: … | interruptores | Activar o no cada texto y cuenta atrás de la página Mensajes |
| Comandos MQTT | sensor (diagnóstico) | La lista de todos los comandos, en sus atributos |
| GIF mostrado, Pantalla | sensores | Lo que muestra el panel |
| Temperatura del procesador, Dirección IP, Versión | sensores (diagnóstico) | |

Si el panel está apagado o desconectado, Home Assistant lo indica como «no disponible».

## Mensajes

Lo más sencillo, en una automatización:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "¡Llaman a la puerta!"
```

(el nombre exacto de la entidad se ve en la página del dispositivo OrbitDMD).

El estilo (efecto, color, pasadas, velocidad) es el ajustado en los elementos «… de los mensajes» del
dispositivo. Para un estilo propio en un solo mensaje, publique un texto JSON en el tema
`orbitdmd/<nombre>/notification` (`<nombre>`: el nombre del panel en la red, en minúsculas):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Lavadora terminada", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Campo | Valores | Por defecto |
|---|---|---|
| `message` | el texto (200 caracteres como máximo) | |
| `effet` (efecto) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (izquierda, derecha, sube, baja, letra a letra, lluvia de letras, fundido) | `defile` |
| `couleur` (color) | `FF8000` (rojo-verde-azul) o `arc-en-ciel` (arcoíris) | blanco |
| `police` (fuente) | `Pixel` o un archivo de fuente del panel, por ejemplo `Orbitron.ttf` | `Pixel` |
| `taille` (tamaño) | tamaño en píxeles, `0` = automático | `0` |
| `vitesse` (velocidad) | 1 (lenta) a 10 (rápida) | 5 |
| `passages` (pasadas) | 1 a 10 | 2 |
| `gif` | un GIF para reproducir antes del texto: `Categoria/nombre.gif` | ninguno |

El mensaje sale enseguida, por encima de los GIF, y luego la rotación continúa. Si el panel está
apagado o en pausa, el mensaje espera a que se vuelva a encender.

## Ejemplos de automatizaciones

- Bajar el brillo al ponerse el sol: acción `light.turn_on` sobre «Panel» con `brightness_pct: 20`.
- Apagar el panel cuando todos salen de casa: `light.turn_off`.
- Timbre: un mensaje «¡Llaman a la puerta!» con `"gif": "Accueil/timbre.gif"` (un GIF añadido por
  usted en la categoría que elija).

## Límites

- Conexión MQTT sin cifrar (sin TLS): úsela en la red de casa.
- La dirección `homeassistant.local` no funciona: indique la dirección IP de Home Assistant.
