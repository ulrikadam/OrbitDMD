<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <b>Русский</b> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Ваши светодиодные панели HUB75 на Raspberry Pi 3: анимированные GIF, часы и погода, настройка из браузера.</b><br>
  Без Linux, запуск за несколько секунд. Установка в 5 шагов из Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Скачать" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=%D0%A1%D0%BA%D0%B0%D1%87%D0%B0%D1%82%D1%8C&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Идеи и вопросы" src="https://img.shields.io/badge/%D0%98%D0%B4%D0%B5%D0%B8%20%D0%B8%20%D0%B2%D0%BE%D0%BF%D1%80%D0%BE%D1%81%D1%8B-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Поддержать на Ko-fi" src="https://img.shields.io/badge/%D0%9F%D0%BE%D0%B4%D0%B4%D0%B5%D1%80%D0%B6%D0%B0%D1%82%D1%8C-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Лицензия GPL v3" src="https://img.shields.io/badge/licence-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Что делает OrbitDMD

OrbitDMD превращает **Raspberry Pi 3** и **светодиодные панели HUB75** (например, 2 × 64×32) в табло
в стиле «DMD», как на пинболе или игровом автомате:

- 🎞️ **анимированные GIF** по категориям (562 в комплекте); добавляются через веб-страницу по одному,
  целыми папками или в .zip; избранные GIF и GIF вне ротации;
- 🕒 **часы**: 14 шрифтов, цвета, анимированная радуга, узоры, 13 анимированных фонов, случайный выбор,
  дата, синхронизация через Интернет;
- 💬 **сообщения**: бегущие строки и обратные отсчёты (Рождество, Новый год, любая дата), у каждого свой
  стиль и эффект появления (прокрутка, подъём, по буквам, дождь из букв, плавное появление…);
- 🏠 **Home Assistant**: панель появляется там сама (MQTT); включение, яркость и сообщения из ваших
  автоматизаций ([руководство](docs/HOME-ASSISTANT.ru.md));
- 🗓️ **расписание**: панель выключается ночью или показывает только время в заданные часы, категории GIF
  по месяцам (Рождество в декабре…), тексты и отсчёты активны только между двумя датами;
- 📐 **слишком большие GIF уменьшаются** до размера панели автоматически при загрузке;
- ⛅ **погода** в вашем городе (Open-Meteo, без регистрации) с анимированными значками;
- 🌐 **веб-страница** в вашей сети (`http://orbitdmd.local/`): живой предпросмотр панели, настройки,
  яркость по часам, пауза, выключение, резервная копия настроек, обновления;
- 🌍 **12 языков** (включая русский и украинский) — для веб-страницы, установщика, текстов на панели и Home Assistant;
- ⚡ **без Linux**: программа запускается прямо на Raspberry Pi (на основе
  [Circle](https://github.com/rsta2/circle)) за несколько секунд.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Панель" width="49%">
  <img src="docs/images/horloge.png" alt="Настройки часов" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Погода" width="49%">
  <img src="docs/images/gif.png" alt="Управление GIF" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Сообщения: бегущие строки" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Сообщения: обратные отсчёты" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Настройки: расписание и Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Справка Home Assistant" width="49%">
</p>

## Оборудование

| Элемент | Подробности |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (модель B или B+) с блоком питания |
| Панели | HUB75, по умолчанию 2 × 64×32 рядом (128×32); возможны 64×64, 32×16 и до 8 панелей |
| Плата-адаптер | HUB75 для Raspberry Pi, распиновка **«regular»** из rpi-rgb-led-matrix (например, Electrodragon); распиновка Adafruit HAT не поддерживается |
| Питание панелей | 5 В, достаточной мощности (около 4 А на панель 64×32 при полной яркости) |
| Карта micro-SD | лучше 32 ГБ или меньше (создаётся раздел FAT32 не более 31 ГБ) |
| Сеть | Сетевой кабель (Ethernet) или Wi-Fi 2,4 ГГц (Raspberry Pi 3 не видит 5 ГГц) |

## Установка

1. **Скачайте** `OrbitDMD-Installateur-<версия>.exe` со страницы
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 или 11, больше ничего не нужно).
2. **Запустите его.** Программа не подписана: если Windows покажет «Система Windows защитила ваш компьютер»,
   нажмите *Подробнее*, затем *Выполнить в любом случае*. Установщик говорит на языке Windows
   (язык можно выбрать на первой странице).
3. **Ответьте на вопросы**: имя панели, расположение панелей, Wi-Fi, пароль веб-страницы, часовой пояс,
   город для погоды, GIF для установки.
4. **Выберите карту micro-SD**. Предлагаются только картридеры и USB-накопители, никогда не внутренние
   диски; всё содержимое выбранной карты стирается.
5. **Вставьте карту в Raspberry Pi** и включите питание. Примерно через полминуты откройте
   `http://<имя панели>.local/` (или IP-адрес, выданный роутером).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Установщик: панели" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Установщик: SD-карта" width="49%">
</p>

Дальше всё настраивается на веб-странице: GIF (перетаскиванием), часы, погода, Wi-Fi, постоянный
IP-адрес, пароль, язык и даже обновление прошивки: скачайте `kernel8.img` из последней версии на
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), затем **Система → Новая прошивка**.
Язык веб-страницы выбирается справа вверху каждой страницы; язык панели — в **Настройки → Экран**.

## Шрифты, фоны и GIF в комплекте

<p align="center"><img src="docs/images/polices.png" alt="Шрифты" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Анимированные фоны" width="90%"></p>

- **Шрифты**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (свободные, [Google Fonts](https://fonts.google.com/)) и
  3 пиксельных шрифта OrbitDMD (LED, Жирный, 7 сегментов). Свои шрифты TTF добавляются на странице «Часы».
  Кириллицу содержат Press Start 2P, Russo One и встроенный шрифт Pixel; для остальных шрифтов русский
  текст автоматически показывается шрифтом Pixel.
- **Анимированные фоны и узоры**: созданы для OrbitDMD (звёзды, снег, дождь, Matrix, огонь, полярное
  сияние, synthwave…).
- **GIF**: OrbitDMD включает **бесплатный базовый набор анимированных GIF от RpiTeaM** (проект RPI2DMD,
  пиксель-арт eLLuiGi, RattenJager и других) с согласия eLLuiGi — спасибо им!
  Для желающих **набор из 11 000 анимаций** доступен
  [на Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) или на [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Экраны запуска и часть рождественских GIF созданы для OrbitDMD.
  Персонажи видеоигр остаются собственностью их правообладателей.

## Предложить идею, сообщить о проблеме

Ваши отзывы помогают OrbitDMD развиваться:

- 💡 **Идея, улучшение?** Откройте [обсуждение](https://github.com/ulrikadam/OrbitDMD/discussions)
  или [предложение](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Проблема?** [Сообщите о ней](https://github.com/ulrikadam/OrbitDMD/issues/new/choose), указав версию
  (видна на панели) и, по возможности, журнал со страницы «Система».
- 🌍 **Перевод можно улучшить?** Тексты лежат в [outils/traductions](outils/traductions): исправления приветствуются.
- 🛠️ **Хотите помочь с кодом?** Прочитайте [CONTRIBUTING.md](CONTRIBUTING.md) и
  [docs/CONSTRUIRE.ru.md](docs/CONSTRUIRE.ru.md).

Изменения в каждой версии описаны в [журнале версий](JOURNAL-DES-VERSIONS.md) (на французском).

## Поддержать проект

OrbitDMD бесплатен. Если он вам нравится и вы хотите поддержать разработку, можно угостить автора кофе:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Поддержать на Ko-fi"></a></p>

## Лицензия

OrbitDMD — свободная программа под лицензией **GNU GPL v3** (см. [LICENSE](LICENSE)), как и
[Circle](https://github.com/rsta2/circle), на котором она построена. Лицензии остальных компонентов
(шрифты, прошивки Raspberry Pi, данные о погоде): см.
[outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Разработчик: <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
