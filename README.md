# Boyahane Havalandırma Verimliliği İzleme Sistemi

ESP32 tabanlı, boyahane ortamındaki VOC (Uçucu Organik Bileşikler) ve CO (Karbon Monoksit) seviyelerini izleyerek havalandırma sisteminin verimliliğini takip eden IoT temelli izleme sistemi.

## 📋 Proje Özeti

Bu sistem, endüstriyel boyahane ortamlarında:
- ✅ VOC ve CO konsantrasyonlarını gerçek zamanlı olarak izler
- ✅ Havalandırma sisteminin performansını değerlendirir
- ✅ Güvenli seviyeleri aşan durumlarda görsel ve işitsel alarm üretir
- ✅ Verileri hem yerel olarak (SD kart) hem de bulutta (Thingspeak) kaydeder
- ✅ Enerji verimliliği ve iş güvenliği sağlar

## ⚠️ ÖNEMLİ GÜVENLİK UYARISI

**BU SİSTEM BİR PROTOTİPTİR VE TEK BAŞINA CAN GÜVENLİĞİ İÇİN KULLANILMAMALIDIR.**

- 🔴 Gerçek endüstriyel ortamlarda mutlaka sertifikalı (ATEX, IECEx) gaz dedektörleri kullanılmalıdır
- 🔴 Bu sistem sadece destekleyici ve proses takip amaçlı kullanılabilir
- 🔴 Düzenli kalibrasyon ve bakım gerektirir
- 🔴 Yanlış alarm veya alarm vermeme riski vardır

## 🛠️ Donanım Gereksinimleri

### Ana Bileşenler
| Bileşen | Miktar | Açıklama |
|---------|--------|----------|
| ESP32 DevKit | 1 | Ana işlemci ve WiFi iletişimi |
| SGP30 Sensör | 1 | VOC ve eCO2 ölçümü |
| BME280 Sensör | 1 | Sıcaklık, nem, basınç ölçümü |
| MQ-9 Sensör | 1 | CO ve yanıcı gaz ölçümü |
| OLED Ekran (0.96" I2C) | 1 | Yerel veri gösterimi |
| SD Kart Modülü | 1 | Veri kaydı |
| RGB LED | 1 veya 3 | Durum göstergesi |
| Active Buzzer | 1 | Sesli uyarı |

### Bağlantı Şeması
| ESP32 Pin | Bileşen | Pin |
|-----------|---------|-----|
| 3.3V | SGP30 VCC, BME280 VCC, OLED VCC | VCC |
| GND | Tüm GND bağlantıları | GND |
| GPIO 21 (SDA) | SGP30 SDA, BME280 SDA, OLED SDA | SDA |
| GPIO 22 (SCL) | SGP30 SCL, BME280 SCL, OLED SCL | SCL |
| GPIO 34 | MQ-9 Analog Çıkış | AOUT |
| GPIO 25 | Buzzer | + |
| GPIO 26 | Kırmızı LED | + |
| GPIO 27 | Sarı LED | + |
| GPIO 14 | Yeşil LED | + |
| GPIO 5 | SD Kart Modülü | CS |
| GPIO 23 | SD Kart Modülü | MOSI |
| GPIO 19 | SD Kart Modülü | MISO |
| GPIO 18 | SD Kart Modülü | SCK |

## 📦 Yazılım Kurulumu

### Gerekli Kütüphaneler
```cpp
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
```

### Arduino IDE Kurulumu
ESP32 board paketini yükleyin

Gerekli kütüphaneleri Library Manager'dan yükleyin:
  Adafruit SGP30
  Adafruit BME280
  Adafruit GFX
  Adafruit SSD1306
  WiFi
  SD

  ## Yapılandırma
settings.h dosyasını oluşturun veya ana kodda aşağıdaki değişkenleri düzenleyin:
```cpp

// WiFi ayarları
const char* ssid = "WIFI_AG_ADI";
const char* password = "WIFI_SIFRE";

// Thingspeak ayarları
const char* server = "api.thingspeak.com";
String apiKey = "THINGSPEAK_API_KEY";

// Eşik değerleri (ortama göre ayarlayın)
#define TVOC_WARNING 200   // ppb
#define TVOC_DANGER 500    // ppb
#define CO_WARNING 30      // ppm
#define CO_DANGER 50       // ppm
```

## 🚀 Kullanım

Başlangıç
  Donanım bağlantılarını kontrol edin
  
  ESP32'ye kodu yükleyin
  
  Seri monitörü açın (115200 baud)

Kalibrasyon
  Sistem temiz havada 12 dakika çalıştırılmalıdır
  
  SGP30 otomatik baz değer ayarlayacaktır
  
  MQ-9 sensörü için manuel kalibrasyon gerekir

## 📊 Veri Toplama
Yerel Kayıt
Veriler datalog.txt dosyasına CSV formatında kaydedilir

Format: timestamp,tvoc,ppb,eco2,ppm,co_ppm,temp_C,hum_%,pressure_hPa

Bulut Entegrasyonu
Thingspeak platformuna veri gönderimi

Her 2 saniyede bir güncelleme

Field mapping:

Field 1: TVOC (ppb)

Field 2: eCO2 (ppm)

Field 3: CO (ppm)

Field 4: Sıcaklık (°C)

Field 5: Nem (%)

Field 6: Basınç (hPa)

##🔧 Sorun Giderme
Sık Karşılaşılan Sorunlar
Sensörler bulunamıyor: I2C bağlantılarını kontrol edin

WiFi bağlanamıyor: SSID/şifre kontrolü

SD kart yazılamıyor: Kart formatı ve bağlantı kontrolü

Yanlış okumalar: Kalibrasyon ve sensör ısınma süresi

##Bakım
Aylık sensör kalibrasyonu

Haftalık veri yedekleme

Günlük görsel denetim

SD kartın düzenli formatlanması

##📝 Lisans
Bu proje MIT lisansı altında lisanslanmıştır. Detaylar için LICENSE dosyasına bakın.
Not: Bu sistem endüstriyel kullanım için tasarlanmamıştır. Test ve geliştirme amaçlıdır.


