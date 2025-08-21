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
