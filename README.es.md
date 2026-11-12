<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <b>Español</b> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Sus paneles LED HUB75 en una Raspberry Pi 3: GIF animados, reloj y tiempo, ajustables desde su navegador.</b><br>
  Sin Linux, arranca en unos segundos. Instalación en 5 pasos desde Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Descargar" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=Descargar&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Ideas y preguntas" src="https://img.shields.io/badge/Ideas%20y%20preguntas-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Apoyar en Ko-fi" src="https://img.shields.io/badge/Apoyar-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Licencia GPL v3" src="https://img.shields.io/badge/licencia-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Qué hace OrbitDMD

OrbitDMD convierte una **Raspberry Pi 3** y unos **paneles LED HUB75** (por ejemplo 2 × 64×32) en una
pantalla «DMD» como la de un pinball o una máquina arcade:

- 🎞️ **GIF animados** por categorías (562 incluidos), añadidos desde la página web uno a uno, por carpeta
  entera o en .zip; GIF favoritos y GIF fuera de rotación;
- 🕒 **reloj**: 14 fuentes, colores, arcoíris animado, motivos, 13 fondos animados, elección al azar,
  fecha, sincronización por Internet;
- 💬 **mensajes**: textos que se desplazan y cuentas atrás (Navidad, Año Nuevo, fecha elegida), cada uno
  con su estilo y su efecto de entrada (desplazamiento, subida, letra a letra, lluvia de letras, fundido…);
- 🏠 **Home Assistant**: el panel aparece solo (MQTT); encendido, brillo y mensajes desde sus
  automatizaciones ([guía](docs/HOME-ASSISTANT.es.md));
- 🗓️ **programa horario**: panel apagado de noche o solo la hora a ciertas horas, categorías de GIF por mes
  (Navidad en diciembre…), textos y cuentas atrás activos solo entre dos fechas;
- 📐 **GIF demasiado grandes reducidos solos** al tamaño del panel al enviarlos;
- ⛅ **tiempo** de su ciudad (Open-Meteo, sin registro) con iconos animados;
- 🌐 **página web** en su red (`http://orbitdmd.local/`): vista previa en directo del panel, ajustes,
  brillo hora por hora, pausa, apagado, copia de seguridad de los ajustes, actualizaciones;
- 🌍 **12 idiomas** (español incluido) para la página web, el instalador, los textos del panel y Home Assistant;
- ⚡ **sin Linux**: el programa arranca directamente en la Raspberry Pi (basado en
  [Circle](https://github.com/rsta2/circle)), en unos segundos.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Panel de control" width="49%">
  <img src="docs/images/horloge.png" alt="Ajustes del reloj" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Tiempo" width="49%">
  <img src="docs/images/gif.png" alt="Gestión de los GIF" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Mensajes: textos que se desplazan" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Mensajes: cuentas atrás" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Ajustes: programa y Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Ayuda de Home Assistant" width="49%">
</p>

## Material

| Elemento | Detalle |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (modelo B o B+) con su alimentación |
| Paneles | HUB75, por defecto 2 × 64×32 uno al lado del otro (128×32); 64×64, 32×16 y hasta 8 paneles posibles |
| Placa adaptadora | HUB75 para Raspberry Pi, conexión **«regular»** de rpi-rgb-led-matrix (p. ej. Electrodragon); la conexión del HAT de Adafruit no es compatible |
| Alimentación de los paneles | 5 V, suficiente para sus paneles (unos 4 A por panel 64×32 a pleno brillo) |
| Tarjeta micro-SD | 32 GB o menos recomendado (se crea una partición FAT32 de 31 GB como máximo) |
| Red | Cable de red (Ethernet) o Wi-Fi de 2,4 GHz (la Raspberry Pi 3 no ve los 5 GHz) |

## Instalación

1. **Descargue** `OrbitDMD-Installateur-<versión>.exe` desde la página
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 u 11, nada más que instalar).
2. **Ejecútelo.** El programa no está firmado: si Windows muestra «Windows protegió su PC», haga clic en
   *Más información* y luego en *Ejecutar de todas formas*. El instalador usa el idioma de Windows
   (o elíjalo en la primera página).
3. **Responda a las preguntas**: nombre del panel, disposición de los paneles, Wi-Fi, contraseña de la
   página web, zona horaria, ciudad para el tiempo, GIF a instalar.
4. **Elija la tarjeta micro-SD**. Solo se proponen lectores de tarjetas y memorias USB, nunca sus discos
   internos; se borra todo el contenido de la tarjeta elegida.
5. **Ponga la tarjeta en la Raspberry Pi** y conéctela. Tras unos treinta segundos, abra
   `http://<nombre del panel>.local/` (o la dirección IP que le haya dado su router).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Instalador: paneles" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Instalador: tarjeta SD" width="49%">
</p>

Después, todo se ajusta desde la página web: GIF (arrastrar y soltar), reloj, tiempo, Wi-Fi, dirección IP
fija, contraseña, idioma e incluso la actualización del firmware: descargue `kernel8.img` de la última
versión en [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) y luego **Sistema → Nuevo firmware**.
El idioma de la página web se elige arriba a la derecha de cada página; el del panel en **Ajustes → Pantalla**.

## Fuentes, fondos y GIF incluidos

<p align="center"><img src="docs/images/polices.png" alt="Fuentes" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Fondos animados" width="90%"></p>

- **Fuentes**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (libres, [Google Fonts](https://fonts.google.com/)) y
  3 fuentes de píxeles de OrbitDMD (LED, Negrita, 7 segmentos). Sus propias fuentes TTF se añaden desde la página Reloj.
- **Fondos animados y motivos**: creados para OrbitDMD (estrellas, nieve, lluvia, Matrix, fuego, aurora, synthwave…).
- **GIF**: OrbitDMD incluye el **pack básico gratuito de GIF animados del RpiTeaM** (proyecto RPI2DMD,
  pixel art de eLLuiGi, RattenJager y otros), con el permiso de eLLuiGi: ¡gracias!
  Para quien lo desee, **un pack de 11 000 animaciones** está disponible
  [en Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) o en [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Las pantallas de arranque y parte de los GIF de Navidad son creaciones de OrbitDMD.
  Los personajes de videojuegos siguen siendo propiedad de sus titulares.

## Proponer una idea, informar de un problema

Sus comentarios hacen avanzar OrbitDMD:

- 💡 **¿Una idea, una mejora?** Abra una [discusión](https://github.com/ulrikadam/OrbitDMD/discussions)
  o una [propuesta](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **¿Un problema?** [Infórmelo](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) indicando la
  versión (mostrada en el panel de control) y, si es posible, el registro de la página Sistema.
- 🌍 **¿Una traducción mejor?** Los textos están en [outils/traductions](outils/traductions): las correcciones son bienvenidas.
- 🛠️ **¿Quiere contribuir al código?** Lea [CONTRIBUTING.md](CONTRIBUTING.md) y
  [docs/CONSTRUIRE.es.md](docs/CONSTRUIRE.es.md).

Las novedades de cada versión están en el [registro de versiones](JOURNAL-DES-VERSIONS.md) (en francés).

## Apoyar el proyecto

OrbitDMD es gratuito. Si le gusta y quiere apoyar su desarrollo, puede invitarme a un café:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Apoyar en Ko-fi"></a></p>

## Licencia

OrbitDMD es software libre bajo licencia **GNU GPL v3** (ver [LICENSE](LICENSE)), como
[Circle](https://github.com/rsta2/circle), sobre el que está construido. Licencias de los demás elementos
(fuentes, firmware de la Raspberry Pi, datos meteorológicos): ver
[outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Desarrollado por <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
