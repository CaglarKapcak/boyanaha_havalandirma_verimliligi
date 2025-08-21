#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>

// Sensör nesneleri
Adafruit_SGP30 sgp;
Adafruit_BME280 bme;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// WiFi bilgileri
const char* ssid = "WIFI_AG_ADI";
const char* password = "WIFI_SIFRE";

// Thingspeak bilgileri
const char* server = "api.thingspeak.com";
String apiKey = "THINGSPEAK_API_KEY";

// Pin tanımlamaları
#define MQ9_PIN 34
#define BUZZER_PIN 25
#define RED_LED 26
#define YELLOW_LED 27
#define GREEN_LED 14
#define SD_CS 5

// Eşik değerleri
#define TVOC_WARNING 200   // ppb
#define TVOC_DANGER 500    // ppb
#define CO_WARNING 30      // ppm (kalibre edilmeli)
#define CO_DANGER 50       // ppm (kalibre edilmeli)

// Değişkenler
unsigned long lastMsg = 0;
unsigned long measurementInterval = 2000;
int baselineSet = 0;
String dataString = "";

void setup() {
  Serial.begin(115200);
  
  // Pin modları
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  
  // LED'leri başlangıçta kapat
  digitalWrite(RED_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  // I2C başlatma
  Wire.begin();
  
  // BME280 başlatma
  if (!bme.begin(0x76)) {
    Serial.println("BME280 bulunamadı!");
    while (1);
  }
  
  // SGP30 başlatma
  if (!sgp.begin()) {
    Serial.println("SGP30 bulunamadı!");
    while (1);
  }
  
  // OLED ekran başlatma
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED ekran bulunamadı!");
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Sistem baslatiliyor");
  display.display();
  delay(2000);
  
  // SD kart başlatma
  if (!SD.begin(SD_CS)) {
    Serial.println("SD kart baslatilamadi!");
  }
  
  // WiFi bağlantısı
  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi baglaniyor...");
  display.display();
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi baglandi!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
  } else {
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi baglanamadi!");
    display.display();
    delay(2000);
  }
  
  // SGP30 için baz değer ayarı
  Serial.println("SGP30 icin baz degerler ayarlaniyor...");
  establishBaseline();
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastMsg > measurementInterval) {
    lastMsg = now;
    
    // Sensör verilerini oku
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    
    // Mutlak nem hesaplama (SGP30 için gerekli)
    double absoluteHumidity = getAbsoluteHumidity(temperature, humidity);
    sgp.setHumidity(absoluteHumidity);
    
    // SGP30 ölçümü
    if (!sgp.IAQmeasure()) {
      Serial.println("SGP30 olcum hatasi!");
      return;
    }
    
    // MQ-9 ölçümü (ham değer)
    int mq9Value = analogRead(MQ9_PIN);
    
    // CO seviyesini hesapla (kalibrasyon gerektirir)
    float co_ppm = convertToPPM(mq9Value, temperature, humidity);
    
    // Verileri işle ve göster
    processData(sgp.TVOC, sgp.eCO2, co_ppm, temperature, humidity, pressure);
    
    // Verileri kaydet ve gönder
    logData(sgp.TVOC, sgp.eCO2, co_ppm, temperature, humidity, pressure);
  }
}

double getAbsoluteHumidity(float temperature, float humidity) {
  // Magnus formülü ile mutlak nem hesaplama (g/m³)
  const double mw = 18.01534;    // suyun moleküler ağırlığı (g/mol)
  const double R = 8.31447215;   // evrensel gaz sabiti (J/mol·K)
  
  double tempKelvin = temperature + 273.15;
  double saturationPressure = 6.1078 * pow(10, (7.5 * temperature) / (237.3 + temperature));
  double actualPressure = (humidity / 100.0) * saturationPressure;
  double absoluteHumidity = (mw * actualPressure * 100) / (R * tempKelvin);
  
  return absoluteHumidity;
}

float convertToPPM(int rawValue, float temperature, float humidity) {
  // MQ-9 için ham değeri ppm'e dönüştürme (kalibrasyon gerektirir)
  // Bu formül örnek amaçlıdır, gerçek kalibrasyon yapılmalıdır
  
  float sensor_volt = (float)rawValue / 4095.0 * 3.3;
  float RS_gas = (3.3 - sensor_volt) / sensor_volt;
  float ratio = RS_gas / 9.9; // Temiz havada RS/R0 oranı (R0 kalibre edilmeli)
  
  // Sıcaklık ve nem düzeltmeleri
  float temp_correction = 1.0 + 0.02 * (temperature - 20.0);
  float hum_correction = 1.0 - 0.04 * (humidity - 40.0);
  
  float ppm = 10.0 * pow(ratio, -2.0) * temp_correction * hum_correction;
  return ppm;
}

void processData(uint16_t tvoc, uint16_t eco2, float co_ppm, float temp, float hum, float pressure) {
  // Ekrana verileri yazdır
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("TVOC: "); display.print(tvoc); display.println(" ppb");
  display.print("eCO2: "); display.print(eco2); display.println(" ppm");
  display.print("CO: "); display.print(co_ppm, 1); display.println(" ppm");
  display.print("Temp: "); display.print(temp, 1); display.println(" C");
  display.print("Hum: "); display.print(hum, 1); display.println(" %");
  display.display();
  
  // Alarm durumunu kontrol et
  if (tvoc > TVOC_DANGER || co_ppm > CO_DANGER) {
    // Tehlike durumu - kırmızı LED ve sürekli buzzer
    digitalWrite(RED_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER_PIN, 1000);
    display.setCursor(0,56);
    display.print("!!! TEHLIKE !!!");
    display.display();
  } 
  else if (tvoc > TVOC_WARNING || co_ppm > CO_WARNING) {
    // Uyarı durumu - sarı LED ve aralıklı buzzer
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER_PIN, 1000, 500);
    display.setCursor(0,56);
    display.print("--- UYARI ---");
    display.display();
  } 
  else {
    // Normal durum - yeşil LED
    digitalWrite(RED_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    noTone(BUZZER_PIN);
    display.setCursor(0,56);
    display.print("NORMAL");
    display.display();
  }
}

void logData(uint16_t tvoc, uint16_t eco2, float co_ppm, float temp, float hum, float pressure) {
  // Seri porta yazdır
  Serial.print("TVOC: "); Serial.print(tvoc); Serial.print(" ppb\t");
  Serial.print("eCO2: "); Serial.print(eco2); Serial.print(" ppm\t");
  Serial.print("CO: "); Serial.print(co_ppm, 1); Serial.print(" ppm\t");
  Serial.print("Temp: "); Serial.print(temp, 1); Serial.print(" C\t");
  Serial.print("Hum: "); Serial.print(hum, 1); Serial.print(" %\t");
  Serial.print("Pressure: "); Serial.print(pressure, 1); Serial.println(" hPa");
  
  // SD karta yazdır
  File dataFile = SD.open("/datalog.txt", FILE_APPEND);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(tvoc);
    dataFile.print(",");
    dataFile.print(eco2);
    dataFile.print(",");
    dataFile.print(co_ppm, 1);
    dataFile.print(",");
    dataFile.print(temp, 1);
    dataFile.print(",");
    dataFile.print(hum, 1);
    dataFile.print(",");
    dataFile.println(pressure, 1);
    dataFile.close();
  }
  
  // Thingspeak'e gönder
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    
    if (client.connect(server, 80)) {
      String postStr = apiKey;
      postStr += "&field1=" + String(tvoc);
      postStr += "&field2=" + String(eco2);
      postStr += "&field3=" + String(co_ppm, 1);
      postStr += "&field4=" + String(temp, 1);
      postStr += "&field5=" + String(hum, 1);
      postStr += "&field6=" + String(pressure, 1);
      postStr += "\r\n\r\n";
      
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
      
      client.stop();
    }
  }
}

void establishBaseline() {
  // SGP30 için baz değerleri ayarlama
  Serial.println("Baz degerler ayarlaniyor... 12 sn bekleyin.");
  
  int baselineTVOC = 0;
  int baselineeCO2 = 0;
  int counter = 0;
  
  while (counter < 12) {
    if (sgp.IAQmeasure()) {
      baselineTVOC = sgp.TVOC;
      baselineeCO2 = sgp.eCO2;
      counter++;
    }
    delay(1000);
  }
  
  // Baz değerleri kaydet
  uint16_t TVOC_base, eCO2_base;
  if (sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
    Serial.print("Baz degerler ayarlandi: eCO2: 0x"); Serial.print(eCO2_base, HEX);
    Serial.print(" & TVOC: 0x"); Serial.println(TVOC_base, HEX);
    
    // SD karta baz değerleri kaydet
    File baseFile = SD.open("/baseline.txt", FILE_WRITE);
    if (baseFile) {
      baseFile.print("eCO2_base: ");
      baseFile.println(eCO2_base, HEX);
      baseFile.print("TVOC_base: ");
      baseFile.println(TVOC_base, HEX);
      baseFile.close();
    }
  } else {
    Serial.println("Baz degerler alinamadi!");
  }
  
  baselineSet = 1;
}
