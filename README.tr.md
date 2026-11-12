<p align="center">
  <img src="OrbitDMD.png" alt="OrbitDMD" width="520">
</p>

<p align="center"><a href="README.md">Français</a> · <a href="README.en.md">English</a> · <a href="README.nl.md">Nederlands</a> · <a href="README.es.md">Español</a> · <a href="README.ru.md">Русский</a> · <a href="README.de.md">Deutsch</a> · <a href="README.it.md">Italiano</a> · <a href="README.pt.md">Português</a> · <a href="README.pl.md">Polski</a> · <a href="README.uk.md">Українська</a> · <b>Türkçe</b> · <a href="README.cs.md">Čeština</a></p>

<p align="center">
  <b>Raspberry Pi 3 üzerinde HUB75 LED panelleriniz: animasyonlu GIF'ler, saat ve hava durumu, tarayıcıdan ayarlanır.</b><br>
  Linux yok, birkaç saniyede açılır. Windows'tan 5 adımda kurulum.
</p>

<p align="center">
  <a href="https://github.com/ulrikadam/OrbitDMD/releases/latest"><img alt="İndir" src="https://img.shields.io/github/v/release/ulrikadam/OrbitDMD?label=%C4%B0ndir&style=for-the-badge&color=0156d1"></a>
  <a href="https://github.com/ulrikadam/OrbitDMD/discussions"><img alt="Fikirler ve sorular" src="https://img.shields.io/badge/Fikirler%20ve%20sorular-Discussions-01a7c5?style=for-the-badge"></a>
  <a href="https://ko-fi.com/H1U12A50RN"><img alt="Ko-fi'de destek ol" src="https://img.shields.io/badge/Destek%20ol-Ko--fi-72a4f2?style=for-the-badge&logo=kofi&logoColor=white"></a>
  <a href="LICENSE"><img alt="Lisans GPL v3" src="https://img.shields.io/badge/lisans-GPL%20v3-fcb92b?style=for-the-badge"></a>
</p>

---

## OrbitDMD ne yapar

OrbitDMD bir **Raspberry Pi 3** ile **HUB75 LED panelleri** (örneğin 2 × 64×32) langırt veya atari makinesi
tarzı bir “DMD” ekrana dönüştürür:

- 🎞️ kategorilere ayrılmış **animasyonlu GIF'ler** (562 tane dahil); web sayfasından tek tek, bütün klasör
  olarak veya .zip ile eklenir; favori GIF'ler ve dönüşüm dışı GIF'ler;
- 🕒 **saat**: 14 yazı tipi, renkler, hareketli gökkuşağı, desenler, 13 hareketli arka plan, rastgele seçim,
  tarih, İnternet üzerinden eşitleme;
- 💬 **mesajlar**: kayan yazılar ve geri sayımlar (Noel, Yılbaşı, seçilen tarih), her biri kendi stili ve
  giriş efektiyle (kayma, yükselme, harf harf, harf yağmuru, solma…);
- 🏠 **Home Assistant**: panel orada kendiliğinden görünür (MQTT); açma/kapatma, parlaklık ve otomasyonlarınızdan
  mesajlar ([kılavuz](docs/HOME-ASSISTANT.tr.md));
- 🗓️ **zamanlama**: panel gece kapalı veya belirli saatlerde yalnızca saat, aylara göre GIF kategorileri
  (aralıkta Noel…), yalnızca iki tarih arasında etkin metinler ve geri sayımlar;
- 📐 **çok büyük GIF'ler gönderilirken** panel boyutuna kendiliğinden küçültülür;
- ⛅ şehrinizin **hava durumu** (Open-Meteo, kayıt gerekmez), hareketli simgelerle;
- 🌐 ağınızda **web sayfası** (`http://orbitdmd.local/`): panelin canlı önizlemesi, ayarlar, saatlik parlaklık,
  duraklatma, kapatma, ayar yedeği, güncellemeler;
- 🌍 web sayfası, kurulum, panel metinleri ve Home Assistant için **12 dil** (Türkçe dahil);
- ⚡ **Linux yok**: program doğrudan Raspberry Pi üzerinde ([Circle](https://github.com/rsta2/circle) tabanlı)
  birkaç saniyede başlar.

<p align="center">
  <img src="docs/images/tableau-de-bord.png" alt="Gösterge paneli" width="49%">
  <img src="docs/images/horloge.png" alt="Saat ayarları" width="49%">
</p>
<p align="center">
  <img src="docs/images/meteo.png" alt="Hava durumu" width="49%">
  <img src="docs/images/gif.png" alt="GIF yönetimi" width="49%">
</p>
<p align="center">
  <img src="docs/images/messages.png" alt="Mesajlar: kayan yazılar" width="49%">
  <img src="docs/images/messages-comptes.png" alt="Mesajlar: geri sayımlar" width="49%">
</p>
<p align="center">
  <img src="docs/images/reglages.png" alt="Ayarlar: zamanlama ve Home Assistant" width="49%">
  <img src="docs/images/aide-home-assistant.png" alt="Home Assistant yardımı" width="49%">
</p>

## Donanım

| Parça | Ayrıntı |
|---|---|
| Raspberry Pi | Güç kaynağıyla **Raspberry Pi 3** (B veya B+ modeli) |
| Paneller | HUB75, varsayılan olarak yan yana 2 × 64×32 (128×32); 64×64, 32×16 ve 8 panele kadar mümkün |
| Adaptör kartı | Raspberry Pi için HUB75, rpi-rgb-led-matrix'in **“regular”** bağlantı düzeni (örn. Electrodragon); Adafruit HAT düzeni desteklenmez |
| Panellerin beslemesi | 5 V, panellerinize yetecek güçte (tam parlaklıkta 64×32 panel başına yaklaşık 4 A) |
| micro-SD kart | 32 GB veya daha küçüğü önerilir (en fazla 31 GB'lık bir FAT32 bölümü oluşturulur) |
| Ağ | Ağ kablosu (Ethernet) veya 2,4 GHz Wi-Fi (Raspberry Pi 3, 5 GHz'i görmez) |

## Kurulum

1. `OrbitDMD-Installateur-<sürüm>.exe` dosyasını [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest)
   sayfasından **indirin** (Windows 10 veya 11, başka bir şey kurmak gerekmez).
2. **Çalıştırın.** Program imzalı değildir: Windows “Windows bilgisayarınızı korudu” derse *Ek bilgi*'ye, sonra
   *Yine de çalıştır*'a tıklayın. Kurulum Windows'un dilini kullanır (ilk sayfada seçilebilir).
3. **Soruları yanıtlayın**: panel adı, panellerin düzeni, Wi-Fi, web sayfası şifresi, saat dilimi, hava durumu
   için şehir, kurulacak GIF'ler.
4. **micro-SD kartı seçin**. Yalnızca kart okuyucular ve USB bellekler gösterilir, asla iç diskler değil; seçilen
   kartın tüm içeriği silinir.
5. **Kartı Raspberry Pi'ye takın** ve gücü bağlayın. Yaklaşık otuz saniye sonra `http://<panel adı>.local/`
   adresini (veya modeminizin verdiği IP adresini) açın.

<p align="center">
  <img src="docs/images/installateur-panneaux.png" alt="Kurulum: paneller" width="49%">
  <img src="docs/images/installateur-carte.png" alt="Kurulum: SD kart" width="49%">
</p>

Sonra her şey web sayfasından ayarlanır: GIF'ler (sürükle-bırak), saat, hava durumu, Wi-Fi, sabit IP, şifre,
dil ve hatta yazılım güncellemesi: [Releases](https://github.com/ulrikadam/OrbitDMD/releases/latest) sayfasındaki
son sürümden `kernel8.img` dosyasını indirin, sonra **Sistem → Yeni yazılım**.
Web sayfasının dili her sayfanın sağ üstünden, panelin dili **Ayarlar → Ekran** içinden seçilir.

## Dahil yazı tipleri, arka planlar ve GIF'ler

<p align="center"><img src="docs/images/polices.png" alt="Yazı tipleri" width="90%"></p>
<p align="center"><img src="docs/images/fonds.png" alt="Hareketli arka planlar" width="90%"></p>

- **Yazı tipleri**: Orbitron, Press Start 2P, VT323, Bungee, Audiowide, Black Ops One, Righteous, Russo One,
  Silkscreen, Creepster, Mountains of Christmas (özgür, [Google Fonts](https://fonts.google.com/)) ve
  3 OrbitDMD piksel yazı tipi (LED, Kalın, 7 segment). Kendi TTF yazı tiplerinizi Saat sayfasından ekleyebilirsiniz.
- **Hareketli arka planlar ve desenler**: OrbitDMD için yapıldı (yıldızlar, kar, yağmur, Matrix, ateş, kutup ışıkları, synthwave…).
- **GIF'ler**: OrbitDMD, **RpiTeaM'in ücretsiz temel animasyonlu GIF paketini** (RPI2DMD projesi; eLLuiGi,
  RattenJager ve diğerlerinin piksel çizimleri) eLLuiGi'nin izniyle içerir — teşekkürler!
  Daha fazlasını isteyenler için **11.000 animasyonluk bir paket**
  [Neo-Arcadia'da](https://www.neo-arcadia.com/forum/viewtopic.php?t=67065) veya [rpiteam.carrd.co](https://rpiteam.carrd.co/) adresinde.
  Açılış ekranları ve Noel GIF'lerinin bir kısmı OrbitDMD çalışmasıdır.
  Video oyunu karakterleri hak sahiplerinin mülkü olarak kalır.

## Fikir önerin, sorun bildirin

- 💡 **Bir fikir, bir iyileştirme mi?** Bir [tartışma](https://github.com/ulrikadam/OrbitDMD/discussions)
  veya [öneri](https://github.com/ulrikadam/OrbitDMD/issues/new/choose) açın.
- 🐞 **Bir sorun mu?** Sürümü (gösterge panelinde) ve mümkünse Sistem sayfasının günlüğünü belirterek
  [bildirin](https://github.com/ulrikadam/OrbitDMD/issues/new/choose).
- 🌍 **Daha iyi bir çeviri mi?** Metinler [outils/traductions](outils/traductions) içinde: düzeltmeler memnuniyetle karşılanır.
- 🛠️ **Koda katkı mı?** [CONTRIBUTING.md](CONTRIBUTING.md) ve [docs/CONSTRUIRE.en.md](docs/CONSTRUIRE.en.md) (İngilizce) dosyalarını okuyun.

Her sürümün yenilikleri [sürüm günlüğünde](JOURNAL-DES-VERSIONS.md) (Fransızca).

## Projeyi destekleyin

OrbitDMD ücretsizdir. Beğendiyseniz bir kahve ısmarlayabilirsiniz:

<p align="center"><a href="https://ko-fi.com/H1U12A50RN"><img src="https://ko-fi.com/img/githubbutton_sm.svg" alt="Ko-fi'de destek ol"></a></p>

## Lisans

OrbitDMD, üzerine kurulduğu [Circle](https://github.com/rsta2/circle) gibi **GNU GPL v3** lisanslı özgür
yazılımdır (bkz. [LICENSE](LICENSE)). Diğer öğelerin lisansları (yazı tipleri, Raspberry Pi yazılımları,
hava durumu verileri): bkz. [outils/installateur/carte/LICENCES.txt](outils/installateur/carte/LICENCES.txt).

---

<p align="center"><sub>Geliştiren: <b>Ulrik Adam</b> · <a href="https://www.nexadam.be">www.nexadam.be</a></sub></p>
