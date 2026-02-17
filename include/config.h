/*
 * Configuration - Moniteur Solaire MSunPV
 * VERSION 11.0 - Ajout Shelly EM monitoring
 * Modifiez vos paramètres ici
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// CONFIGURATION WIFI (DÉFAUTS - vides, config via portail web)
// ============================================
const char* WIFI_SSID = "";
const char* WIFI_PASSWORD = "";

// ============================================
// CONFIGURATION MQTT (DÉFAUTS)
// ============================================
const char* MQTT_SERVER = "";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "";
const char* MQTT_PASSWORD = "";

// Topics MQTT (DÉFAUTS - exemples, à configurer via portail web)
const char* TOPIC_SOLAR_PROD = "";
const char* TOPIC_SOLAR_CABANE = "";
const char* TOPIC_HOME_CONSO = "";
const char* TOPIC_ROUTER = "";
const char* TOPIC_WATER_TEMP = "";
const char* TOPIC_TEMP_EXT = "";
const char* TOPIC_TEMP_SALON = "";
const char* TOPIC_CONSO_JOUR = "";

// ============================================
// CONFIGURATION M'SunPV ROUTER (DÉFAUT)
// ============================================
const char* MSUNPV_IP = "";
const int MSUNPV_PORT = 80;
const int MSUNPV_UPDATE_INTERVAL = 10000;       // 10 secondes

// ============================================
// CLÉS PREFERENCES (NVS) - V3.2
// ============================================
#define PREF_NAMESPACE "msunpv"

// WiFi
#define PREF_WIFI_SSID "wifi_ssid"
#define PREF_WIFI_PASS "wifi_pass"

// MQTT Broker
#define PREF_MQTT_IP "mqtt_ip"
#define PREF_MQTT_PORT "mqtt_port"
#define PREF_MQTT_USER "mqtt_user"
#define PREF_MQTT_PASS "mqtt_pass"

// M'SunPV
#define PREF_MSUNPV_IP "msunpv_ip"

// Topics MQTT
#define PREF_TOPIC_PROD "topic_prod"
#define PREF_TOPIC_CABANE "topic_cabane"
#define PREF_TOPIC_CONSO "topic_conso"
#define PREF_TOPIC_ROUTER "topic_router"
#define PREF_TOPIC_WATER "topic_water"
#define PREF_TOPIC_EXT "topic_ext"
#define PREF_TOPIC_SALON "topic_salon"
#define PREF_TOPIC_JOUR "topic_jour"
#define PREF_TOPIC_PRESENCE_BEN "topic_presence_ben"
#define PREF_TOPIC_PRESENCE_FRANCINE "topic_presence_francine"
#define PREF_VICTOR_ENABLED "victor_enabled"
#define PREF_TOPIC_PRESENCE_VICTOR "topic_presence_victor"
#define PREF_TOPIC_ALARM "topic_alarm"
#define PREF_TOPIC_ALARM_COMMAND "topic_alarm_command"

// Clés JSON configurables pour topics MQTT
#define PREF_JSON_KEY_CABANE "json_key_cabane"
#define PREF_JSON_KEY_WATER1 "json_key_water1"
#define PREF_JSON_KEY_WATER2 "json_key_water2"

// Météo (V10.0)
#define PREF_WEATHER_API "weather_api"
#define PREF_WEATHER_CITY "weather_city"

// Shelly EM (V11.0)
#define PREF_SHELLY1_IP "shelly1_ip"
#define PREF_SHELLY1_NAME "shelly1_name"
#define PREF_SHELLY1_LABEL0 "shelly1_label0"   // Premier label Shelly 1
#define PREF_SHELLY1_LABEL1 "shelly1_label1"   // Deuxième label Shelly 1
#define PREF_SHELLY1_LABEL_ENERGY "shelly1_label_energy"      // Label Production Jour
#define PREF_SHELLY1_LABEL_RETURNED "shelly1_label_returned"  // Label Retourné Réseau
#define PREF_SHELLY2_IP "shelly2_ip"
#define PREF_SHELLY2_NAME "shelly2_name"
#define PREF_SHELLY2_LABEL0 "shelly2_label0"   // Premier label Shelly 2
#define PREF_SHELLY2_LABEL1 "shelly2_label1"   // Deuxième label Shelly 2
#define PREF_SHELLY2_LABEL_ENERGY "shelly2_label_energy"      // Label Production Jour
#define PREF_SHELLY2_LABEL_RETURNED "shelly2_label_returned"  // Label Retourné Réseau

// ============================================
// CONFIGURATION ÉCRAN
// ============================================
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480
#define GFX_BL 38

// Pins écran RGB
#define LCD_DE 18
#define LCD_VSYNC 17
#define LCD_HSYNC 16
#define LCD_PCLK 21
#define LCD_R0 11
#define LCD_R1 12
#define LCD_R2 13
#define LCD_R3 14
#define LCD_R4 0
#define LCD_G0 8
#define LCD_G1 20
#define LCD_G2 3
#define LCD_G3 46
#define LCD_G4 9
#define LCD_G5 10
#define LCD_B0 4
#define LCD_B1 5
#define LCD_B2 6
#define LCD_B3 7
#define LCD_B4 15

// ============================================
// CONFIGURATION CARTE SD (V13.0)
// ============================================
// Pins pour le slot SD intégré à l'écran
// ⚠️ À ajuster selon votre écran si nécessaire
// Pins libres sur ESP32-S3 (non utilisés par écran/touch) :
// Configuration 1 (par défaut - pins libres) :
#define SD_CS 2              // Chip Select (CS)
#define SD_MOSI 1            // MOSI (Master Out Slave In)
#define SD_MISO 40           // MISO (Master In Slave Out)
#define SD_SCK 41            // SCK (Serial Clock)
//
// Configuration 2 (alternative - décommentez si config 1 ne fonctionne pas) :
// #define SD_CS 42
// #define SD_MOSI 43
// #define SD_MISO 44
// #define SD_SCK 35
//
// Configuration 3 (pins SD standard ESP32-S3 - souvent utilisés par les écrans) :
// #define SD_CS 5
// #define SD_MOSI 23
// #define SD_MISO 2    // Note: 19 est utilisé par touch
// #define SD_SCK 12    // Note: 18 est utilisé par LCD_DE
//
// Configuration 4 (pins courants pour écrans 480x480 avec slot SD) :
// #define SD_CS 5
// #define SD_MOSI 11    // Note: LCD_R0 utilise 11, mais peut être partagé
// #define SD_MISO 2
// #define SD_SCK 12    // Note: LCD_R1 utilise 12, mais peut être partagé
//
// Configuration 5 (pins GPIO libres ESP32-S3 - éviter 38=backlight) :
// #define SD_CS 35
// #define SD_MOSI 36
// #define SD_MISO 37
// #define SD_SCK 39    // Note: vérifiez si 39 est libre (utilisé par bus SWSPI)

// ============================================
// SEUILS ET LIMITES
// ============================================
#define TEMP_MIN 20
#define TEMP_MAX 65
#define TEMP_HOT_THRESHOLD 55
#define HOUR_NIGHT_END 23

// Mode nuit
#define NIGHT_START_HOUR 22
#define NIGHT_END_HOUR 6
#define BACKLIGHT_DAY 255        // Luminosité jour (100%)
#define BACKLIGHT_NIGHT 50       // Luminosité nuit (~20%)

#define MQTT_RECONNECT_INTERVAL 5000     // 5 secondes
#define UI_UPDATE_INTERVAL 2000           // 2000ms

// Shelly EM (V11.0)
#define SHELLY_UPDATE_INTERVAL 5000      // 5 secondes

// ============================================
// COULEURS LVGL (format hex)
// ============================================
#define COLOR_BG 0x0c0a09
#define COLOR_HEADER 0x1c1917
#define COLOR_CARD 0x292524
#define COLOR_WEATHER 0x1c1917
#define COLOR_TEXT 0xFFFFFF
#define COLOR_PROD 0xFBBF24
#define COLOR_CONSO 0x60A5FA
#define COLOR_ROUTER 0xFB923C
#define COLOR_EXPORT 0x22C55E
#define COLOR_IMPORT 0xEF4444
#define COLOR_GRAY 0x6B7280

// Couleurs statuts M'SunPV (V3.0)
#define COLOR_MSUNPV_AUTO 0x22C55E      // Vert
#define COLOR_MSUNPV_MANU 0x60A5FA      // Bleu
#define COLOR_MSUNPV_OFF 0xFFFFFF       // Blanc

// Couleurs Shelly EM (V11.0)
#define COLOR_SHELLY 0xA78BFA           // Violet

// Enphase Envoy (V12.0)
#define PREF_ENPHASE_IP "enphase_ip"
#define PREF_ENPHASE_USER "enphase_user"
#define PREF_ENPHASE_PWD "enphase_pwd"
#define PREF_ENPHASE_SERIAL "enphase_serial"
#define COLOR_ENPHASE 0xFB923C           // Orange (inspiré du style Shelly)

// Rotation écran (V12.1)
#define PREF_SCREEN_FLIPPED "screen_flipped"

// Écran actif principal (V15.0) : 0=MQTT, 1=Enphase
#define PREF_ACTIVE_SCREEN "active_screen"

// Verrouillage écran Enphase : MDP requis pour quitter le mode Enphase (web + LVGL)
#define PREF_SCREEN_LOCK_ENABLED "screen_lock_enabled"
#define PREF_SCREEN_LOCK_PWD "screen_lock_pwd"

// Format de date (V14.0)
#define PREF_DATE_FORMAT "date_format"

// Luminosité écran (réglable depuis page Info)
#define PREF_BRIGHTNESS_DAY "brightness_day"
#define PREF_BRIGHTNESS_NIGHT "brightness_night"

// EDF TEMPO (couleur du jour, option activable)
#define PREF_TEMPO_ENABLED "tempo_enabled"

// ============================================
// MODE AP (FALLBACK) - V3.3
// ============================================
#define WIFI_AP_SSID "Enphase-Monitor-Setup"
#define WIFI_CONNECT_TIMEOUT 15000

#endif
