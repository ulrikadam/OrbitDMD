# OrbitDMD ve Home Assistant

[Français](HOME-ASSISTANT.md) · [English](HOME-ASSISTANT.en.md) · [Nederlands](HOME-ASSISTANT.nl.md) · [Español](HOME-ASSISTANT.es.md) · [Русский](HOME-ASSISTANT.ru.md) · [Deutsch](HOME-ASSISTANT.de.md) · [Italiano](HOME-ASSISTANT.it.md) · [Português](HOME-ASSISTANT.pt.md) · [Polski](HOME-ASSISTANT.pl.md) · [Українська](HOME-ASSISTANT.uk.md) · **Türkçe** · [Čeština](HOME-ASSISTANT.cs.md)

1.4.0 sürümünden beri panel Home Assistant'a **MQTT** ile bağlanır ve orada kendiliğinden görünür
(otomatik keşif). Varlık adları panelin dilini izler (Ayarlar → Ekran → Panelin dili).

## Başlarken

1. Home Assistant'ta: **Ayarlar → Eklentiler → Eklenti mağazası**, **Mosquitto broker**'ı kurun ve başlatın.
   Home Assistant kendisi önermezse **MQTT** entegrasyonunu ekleyin.
2. Panel için bir kullanıcı oluşturun (**Ayarlar → Kişiler → Kullanıcılar**), örneğin `orbitdmd`.
3. Panelin web sayfasında: **Ayarlar → Home Assistant**: etkinleştirin, Home Assistant'ın **IP adresini**
   (örneğin `192.168.0.10`), `1883` bağlantı noktasını, kullanıcıyı ve şifresini girin, sonra kaydedin.
   Panel yeniden başlar ve bağlanır.
4. Home Assistant'ta: **Ayarlar → Cihazlar ve hizmetler → MQTT → OrbitDMD**.

Panelin web sayfasında bir **Home Assistant yardımı** sayfası da vardır (`http://<panel>/aide`): tüm komutlar
ve panelinizin gerçek adıyla kopyalanmaya hazır örnekler.

Ayarlar sayfasındaki Home Assistant kartı bağlantının çalışıp çalışmadığını gösterir. Sorun olursa Sistem
sayfasının günlüğünde `homeassistant` satırları bulunur.

## Home Assistant'ta görünenler

| Varlık | Tür | Görev |
|---|---|---|
| Panel | ışık | Açma, kapatma (web sayfasına erişilir), parlaklık |
| Saatlik parlaklık | anahtar | Açık: parlaklık panelin saatlik ayarlarını izler. Home Assistant'tan parlaklık ayarlamak bunu kapatır |
| Duraklat | anahtar | Görüntüyü dondurur |
| Saati göster, Havayı göster | düğmeler | Hemen, 15 saniye boyunca |
| Yeniden başlat | düğme | |
| Mesaj | bildirim | Hemen bir metin gösterir |
| Gösterilecek metin | metin | Home Assistant'ta bir metin yazıp onaylayın: hemen gösterilir |
| Mesaj efekti, rengi, geçişleri, hızı | listeler ve sayılar | Home Assistant'tan gönderilen metinlerin stili (yeniden başlatmadan sonra korunur) |
| Metin: …, Geri sayım: … | anahtarlar | Mesajlar sayfasındaki her metni ve geri sayımı etkinleştirme |
| MQTT komutları | sensör (tanılama) | Tüm komutların listesi, özniteliklerde |
| Gösterilen GIF, Ekran | sensörler | Panelin gösterdiği |
| İşlemci sıcaklığı, IP adresi, Sürüm | sensörler (tanılama) | |

Panel kapalıysa veya fişi çekiliyse Home Assistant onu “kullanılamıyor” olarak gösterir.

## Mesajlar

En kolayı, bir otomasyonda:

```yaml
action: notify.send_message
target:
  entity_id: notify.orbitdmd_message
data:
  message: "Kapı çalıyor!"
```

(varlığın tam adı OrbitDMD cihaz sayfasında görünür).

Stil (efekt, renk, geçişler, hız) cihazın “Mesaj …” öğelerinde ayarlanandır. Tek bir mesaja ayrı bir stil
vermek için `orbitdmd/<ad>/notification` konusuna bir JSON metni yayımlayın (`<ad>`: panelin ağdaki adı,
küçük harfle):

```yaml
action: mqtt.publish
data:
  topic: orbitdmd/orbitdmd/notification
  payload: >
    {"message": "Çamaşır bitti", "effet": "pluie", "couleur": "arc-en-ciel", "passages": 2}
```

| Alan | Değerler | Varsayılan |
|---|---|---|
| `message` | metin (en fazla 200 bayt) | |
| `effet` (efekt) | `defile`, `inverse`, `monte`, `descend`, `lettres`, `pluie`, `fondu` (sola, sağa, yukarı, aşağı, harf harf, harf yağmuru, solma) veya Home Assistant'taki adlar | `defile` |
| `couleur` (renk) | `FF8000` (kırmızı-yeşil-mavi), `arc-en-ciel` (gökkuşağı) veya Home Assistant'taki bir renk adı | beyaz |
| `police` (yazı tipi) | `Pixel` veya paneldeki bir yazı tipi dosyası, örn. `Orbitron.ttf` | `Pixel` |
| `taille` (boyut) | piksel cinsinden boyut, `0` = otomatik | `0` |
| `vitesse` (hız) | 1 (yavaş) ile 10 (hızlı) arası | 5 |
| `passages` (geçişler) | 1 ile 10 arası | 2 |
| `gif` | metinden önce bir GIF: `Kategori/ad.gif` | yok |

Mesaj hemen, GIF'lerin üzerinde gösterilir, sonra dönüşüm sürer. Panel kapalı veya duraklatılmışsa mesaj bekler.

## Örnekler

- Gün batımında parlaklığı azaltın: “Panel” üzerinde `brightness_pct: 20` ile `light.turn_on`.
- Herkes evden çıkınca paneli kapatın: `light.turn_off`.
- Kapı zili: `"gif": "Accueil/zil.gif"` ile “Kapı çalıyor!” mesajı (sizin eklediğiniz bir GIF).

## Sınırlar

- MQTT bağlantısı şifrelenmez (TLS yok): ev ağında kullanın.
- `homeassistant.local` adresi çalışmaz: Home Assistant'ın IP adresini girin.
