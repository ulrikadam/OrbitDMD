<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <b>Українська</b> · <a href="README.tr.md">Türkçe</a> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Ваші світлодіодні панелі HUB75 на Raspberry Pi 3: анімовані GIF, годинник і погода, налаштування з браузера.</b><br>
  Без Linux, запуск за кілька секунд. Установлення за 5 кроків із Windows.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="Завантажити" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=%D0%97%D0%B0%D0%B2%D0%B0%D0%BD%D1%82%D0%B0%D0%B6%D0%B8%D1%82%D0%B8&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Ідеї та питання" src="https://img.shields.io/badge/%D0%86%D0%B4%D0%B5%D1%97%20%D1%82%D0%B0%20%D0%BF%D0%B8%D1%82%D0%B0%D0%BD%D0%BD%D1%8F-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Підтримати на Ko-fi" src="https://img.shields.io/badge/%D0%9F%D1%96%D0%B4%D1%82%D1%80%D0%B8%D0%BC%D0%B0%D1%82%D0%B8-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Ліцензія GPL v3" src="https://img.shields.io/badge/licence-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## Що робить OrbitDMD

OrbitDMD перетворює **Raspberry Pi 3** і **світлодіодні панелі HUB75** (наприклад, 2 × 64×32) на табло
в стилі «DMD», як на пінболі чи ігровому автоматі:

- 🎞️ **анімовані GIF** за категоріями (562 у комплекті); додаються через вебсторінку по одному, цілими
  теками або в .zip; улюблені GIF і GIF поза ротацією;
- 🕒 **годинник**: 14 шрифтів, кольори, анімована веселка, візерунки, 13 анімованих фонів, випадковий вибір,
  дата, синхронізація через Інтернет;
- 💬 **повідомлення**: біжучі рядки й зворотні відліки (Різдво, Новий рік, будь-яка дата), кожен зі своїм
  стилем і ефектом появи (прокрутка, підйом, по літерах, дощ із літер, плавна поява…);
- 🏠 **Home Assistant**: панель з'являється там сама (MQTT); увімкнення, яскравість і повідомлення з ваших
  автоматизацій ([посібник](docs/HOME-ASSISTANT.uk.md));
- 🗓️ **розклад**: панель вимикається вночі або показує лише час у задані години, категорії GIF за місяцями
  (Різдво в грудні…), тексти й відліки активні лише між двома датами;
- 📐 **завеликі GIF самі зменшуються** до розміру панелі під час надсилання;
- ⛅ **погода** у вашому місті (Open-Meteo, без реєстрації) з анімованими значками;
- 🌐 **вебсторінка** у вашій мережі (`http://orbitdmd.local/`): живий перегляд панелі, налаштування, яскравість
  по годинах, пауза, вимкнення, резервна копія налаштувань, оновлення;
- 🌍 **12 мов** вебсторінки, установника, текстів панелі та Home Assistant (зокрема українська);
- ⚡ **без Linux**: програма запускається просто на Raspberry Pi (на основі
  [Circle](https://github.com/rsta2/circle)) за кілька секунд.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Панель" width="49%">
  <img src="docs/images/horloge.png" alt="Налаштування годинника" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Погода" width="49%">
  <img src="docs/images/gif.png" alt="Керування GIF" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Повідомлення: біжучі рядки" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Повідомлення: зворотні відліки" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Налаштування: розклад і Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Довідка Home Assistant" width="49%">
</p>

## Обладнання

| Елемент | Подробиці |
|---|---|
| Raspberry Pi | **Raspberry Pi 3** (модель B або B+) з блоком живлення |
| Панелі | HUB75, типово 2 × 64×32 поруч (128×32); можливі 64×64, 32×16 і до 8 панелей |
| Плата-адаптер | HUB75 для Raspberry Pi, розпіновка **«regular»** з rpi-rgb-led-matrix (наприклад, Electrodragon); розпіновка Adafruit HAT не підтримується |
| Живлення панелей | 5 В, достатньої потужності (близько 4 А на панель 64×32 за повної яскравості) |
| Картка micro-SD | краще 32 ГБ або менше (створюється розділ FAT32 до 31 ГБ) |
| Мережа | Мережевий кабель (Ethernet) або Wi-Fi 2,4 ГГц (Raspberry Pi 3 не бачить 5 ГГц) |

## Установлення

1. **Завантажте** `OrbitDMD-Installateur-<версія>.exe` зі сторінки
   [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) (Windows 10 або 11, більше нічого не потрібно).
2. **Запустіть його.** Програма не підписана: якщо Windows покаже «Windows захистила ваш ПК», натисніть
   *Докладніше*, потім *Усе одно запустити*. Установник говорить мовою Windows (вибір на першій сторінці).
3. **Дайте відповіді на запитання**: ім'я панелі, розташування панелей, Wi-Fi, пароль вебсторінки, часовий
   пояс, місто для погоди, GIF для встановлення.
4. **Виберіть картку micro-SD**. Пропонуються лише кардрідери та USB-накопичувачі, ніколи не внутрішні диски;
   увесь вміст вибраної картки стирається.
5. **Вставте картку в Raspberry Pi** і ввімкніть живлення. Приблизно за пів хвилини відкрийте
   `http://<ім'я панелі>.local/` (або IP-адресу, видану роутером).

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Установник: панелі" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Установник: SD-картка" width="49%">
</p>

Далі все налаштовується на вебсторінці: GIF (перетягуванням), годинник, погода, Wi-Fi, постійна IP-адреса,
пароль, мова і навіть оновлення прошивки: завантажте `kernel8.img` з останньої версії на
[Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest), потім **Система → Нова прошивка**.
Мова вебсторінки вибирається праворуч угорі, мова панелі — в **Налаштування → Екран**.

## Шрифти, фони й GIF у комплекті

<p align="center"><img src="docs/images/polices.png" alt="Шрифти" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Анімовані фони" width="90%"></p>

- **Шрифти**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (вільні, [Google Fonts](https://fonts.google.com/)) і
  3 піксельні шрифти OrbitDMD (LED, Жирний, 7 сегментів). Свої шрифти TTF додаються на сторінці «Годинник».
  Кирилицю містять Press Start 2P, Russo One і вбудований шрифт Pixel; для інших шрифтів кириличний текст
  автоматично показується шрифтом Pixel.
- **Анімовані фони й візерунки**: створені для OrbitDMD (зорі, сніг, дощ, Matrix, вогонь, полярне сяйво, synthwave…).
- **GIF**: OrbitDMD містить **безкоштовний базовий набір анімованих GIF від RpiTeaM** (проєкт RPI2DMD,
  піксель-арт eLLuiGi, RattenJager та інших) за згодою eLLuiGi — дякуємо!
  Для охочих **набір із 11 000 анімацій** доступний
  [на Neo-Arcadia](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) або на [rpiteam.carrd.co](https://rpiteam.carrd.co/).
  Екрани запуску й частина різдвяних GIF створені для OrbitDMD.
  Персонажі відеоігор залишаються власністю їхніх правовласників.

## Запропонувати ідею, повідомити про проблему

- 💡 **Ідея, покращення?** Відкрийте [обговорення](https://github.com/ulrikadam/OrbitDMD/discussions)
  або [пропозицію](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🐞 **Проблема?** [Повідомте про неї](https://github.com/ulrikadam/OrbitDMD/issues/new/choose), указавши версію
  (видно на панелі) і, по можливості, журнал зі сторінки «Система».
- 🌍 **Переклад можна покращити?** Тексти лежать в [outils/traductions](outils/traductions): виправлення вітаються.
- 🛠️ **Хочете допомогти з кодом?** Прочитайте [CONTRIBUTING.md](CONTRIBUTING.md) і [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (англійською).

Зміни в кожній версії описано в [журналі версій](JOURNAL-DES-VERSIONS.md) (французькою).

## Підтримати проєкт

OrbitDMD безкоштовний. Якщо він вам подобається, можна пригостити автора кавою:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Підтримати на Ko-fi"></a></p>

## Ліцензія

OrbitDMD — вільна програма під ліцензією **GNU GPL v3** (див. [LICENSE](LICENSE)), як і
[Circle](https://github.com/rsta2/circle), на якому вона побудована. Ліцензії інших складників (шрифти,
прошивки Raspberry Pi, дані про погоду): див. [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Розробник: <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
