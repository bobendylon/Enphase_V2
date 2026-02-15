// MSunPV Monitor V12.0 - ESP32-S3 480x480 RGB Display

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <lvgl.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <WebServer.h>
#include <Preferences.h>
#include <stdarg.h>
#include "touch.h"
#include "favicon.h"
#include "config.h"
#include "ui_main.h"
#include "module_weather.h"
#include "module_mqtt.h"
// module_shelly retiré (Enphase V2)
#include "module_enphase.h"
#include "module_stats.h"
// module_sd retiré (Enphase V2)
#include "module_tempo.h"

// VARIABLES GLOBALES
// V3.2 - Preferences
Preferences preferences;

// V3.2 - Configuration dynamique (chargée depuis NVS)
String config_wifi_ssid;
String config_wifi_password;
// Variables MQTT sont maintenant dans module_mqtt (extern)

// États connexion
bool wifiConnected = false;
// mqttConnected est maintenant dans module_mqtt (extern)

// Variables M'SunPV - Maintenant dans module_msunpv.h (extern)

// LED sticky green (V10.0)
bool ledLockedGreen = false;  // ← AJOUTER CETTE LIGNE
bool ledStartupLock = true;   // Verrouille LED rouge pendant démarrage
unsigned long startupTime = 0;  // Temps du démarrage
// mqttDataReceived est maintenant dans module_mqtt (extern)

// V11.0 - Shelly EM (variables maintenant dans module_shelly.h - extern)

// V12.0 - Enphase Envoy (V11.0 - Module)
// Variables déclarées dans module_enphase.h

// V12.1 - Rotation écran
bool screenFlipped = false;

// V15.0 - Écran actif : 0=MQTT (date blanche), 1=Enphase (date orange), 2=M'SunPV (date verte)
// Enphase V2 : forcé à 1 (écran unique Enphase)
uint8_t activeScreenType = 1;

// Verrouillage écran Enphase : MDP requis pour quitter le mode Enphase (web)
bool screenLockEnabled = false;
String screenLockPassword = "";
static String unlockToken = "";
static unsigned long unlockExpiry = 0;

// V14.0 - Format de date (0=complet, 1=abrégé+mois, 2=compact, 3=abrégé+date)
int dateFormatIndex = 1;  // Par défaut: "Dim. 28 Déc. 2025"

// Luminosité écran (0-255), réglable depuis page Info
uint8_t brightnessDay = BACKLIGHT_DAY;
uint8_t brightnessNight = BACKLIGHT_NIGHT;

// SYSTÈME DE LOGS WEB - V12.1
#define MAX_LOGS 100
String systemLogs[MAX_LOGS];
int logIndex = 0;

// Fonction pour obtenir le timestamp format HH:MM:SS.mmm (heure réelle)
String getTimestamp() {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    // Si NTP n'est pas encore synchronisé, utiliser millis() comme fallback
    if (now < 946684800) { // 1er janvier 2000 (avant cette date = NTP non sync)
        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long milliseconds = ms % 1000;
        unsigned long hours = seconds / 3600;
        unsigned long minutes = (seconds % 3600) / 60;
        unsigned long secs = seconds % 60;
        
        char buffer[16];
        sprintf(buffer, "%02lu:%02lu:%02lu.%03lu", hours, minutes, secs, milliseconds);
        return String(buffer);
    }
    
    // Utiliser l'heure réelle
    unsigned long ms = millis() % 1000; // Millisecondes depuis le démarrage
    
    char buffer[16];
    sprintf(buffer, "%02d:%02d:%02d.%03lu", 
            timeinfo->tm_hour, 
            timeinfo->tm_min, 
            timeinfo->tm_sec, 
            ms);
    return String(buffer);
}

void addLog(String message) {
    String timestamp = getTimestamp();
    String logEntry = timestamp + " -> " + message;
    
    systemLogs[logIndex] = logEntry;
    logIndex = (logIndex + 1) % MAX_LOGS;
    
    // Aussi dans Serial pour debug
    Serial.println(message);
}

// Fonction helper pour addLog avec format printf
void addLogf(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    addLog(String(buffer));
}

// Page courante (0=main, 1=stats, 2=info, 3=settings)
int currentPage = 0;

// Statistiques (simulées pour l'instant)
float statsDay[24];
float statsWeek[7];

// Historique pour graphiques (24h) - Maintenant dans module_stats.h (extern)

// Variables pour page INFO
String ipAddress = "";
long rssi = 0;
unsigned long uptimeSeconds = 0;

// V3.3 - Mode AP
bool wifiAPMode = false;
String wifiNetworksJSON = "";

// OBJETS ÉCRAN ET LVGL
Arduino_DataBus *bus = new Arduino_SWSPI(
  GFX_NOT_DEFINED, 39, 48, 47, GFX_NOT_DEFINED);

Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  LCD_DE, LCD_VSYNC, LCD_HSYNC, LCD_PCLK,
  LCD_R0, LCD_R1, LCD_R2, LCD_R3, LCD_R4,
  LCD_G0, LCD_G1, LCD_G2, LCD_G3, LCD_G4, LCD_G5,
  LCD_B0, LCD_B1, LCD_B2, LCD_B3, LCD_B4,
  1, 10, 8, 50,
  1, 10, 8, 20);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
  SCREEN_WIDTH, SCREEN_HEIGHT, rgbpanel, 0, true,
  bus, GFX_NOT_DEFINED, st7701_type9_init_operations, 
  sizeof(st7701_type9_init_operations));

// LVGL
lv_display_t *disp;
lv_color_t *disp_draw_buf;
uint32_t bufSize;

// Objets UI (déclarés dans les fichiers .h)
lv_obj_t *screenMain = NULL;     // Enphase V2 : non utilisé (écran unique Enphase)
lv_obj_t *screenEnphase = NULL;  // V15.0 - Écran Enphase
lv_obj_t *screenStats;
lv_obj_t *screenInfo;
lv_obj_t *screenSettings;

// MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// SERVEUR WEB
WebServer server(80);

// PROTOTYPES
void setupWiFi();
// Fonctions MQTT sont maintenant dans module_mqtt
void updateUI();
void createMainScreen();
void createEnphaseScreen();  // V15.0
void createStatsScreen();
void createInfoScreen();
void createSettingsScreen();
uint32_t millis_cb();
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void setupOTA();
void handleRoot();
void handleFavicon();  // V3.4 - Favicon SVG
void handleData();
void handleFormattedDate();  // V3.4
void handleUpdate();
void handleDoUpdate();
void handleAlarmSet();
// handleStatsWeb() est maintenant dans module_stats (stats_handleWeb)
void handleInfoWeb();
void handleSettingsWeb();
void handleRestart();
// V13.0 - SD Card status
void handleSDStatus();
// V12.1 - Rotation écran
void handleSaveScreenFlip();
// V15.0 - Sélection écran MQTT / Enphase
void handleScreensWeb();
void handleSaveScreens();
void handleUnlockScreen();
void handleSaveScreenLock();
void handleEnphaseMonitorHome();
void handleExportEnphaseConfig();
void handleEnphaseMonitorData();
void handleEnphaseReglages();
// V14.0 - Format de date
void handleSaveDateFormat();
// Luminosité écran (page Info)
void handleSaveBrightness();
// EDF TEMPO (page Info)
void handleSaveTempoConfig();
void applyNightMode();
// saveHistoData() est maintenant dans module_stats (stats_update)
// V3.2 - Preferences et pages web
void loadPreferences();
void savePreferences();
// Handlers MQTT sont maintenant dans module_mqtt
void handleWifiConfig();
void handleSaveWifiConfig();
void handleExportConfig();
void handleImportConfig();

// V11.0 - Shelly EM (fonctions maintenant dans module_shelly)
// V12.0 - Enphase Envoy (V11.0 - Module)
// Fonctions déclarées dans module_enphase.h
// V3.3 - Mode AP
void startWiFiAP();
void scanWiFiNetworks();
void handleWiFiSetupPage();
void handleWiFiScan();
void handleWiFiConnect();

// CALLBACK LVGL
uint32_t millis_cb(void) {
  return millis();
}

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);
  
  if (screenFlipped) {
    // Rotation 180° : inverser les coordonnées
    int32_t x1_flipped = SCREEN_WIDTH - area->x2 - 1;
    int32_t y1_flipped = SCREEN_HEIGHT - area->y2 - 1;
    
    // Créer un buffer temporaire inversé
    uint16_t *flipped_buf = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    if (flipped_buf) {
      uint16_t *src = (uint16_t *)px_map;
      uint16_t *dst = flipped_buf + (w * h - 1);
      
      // Inverser le buffer (rotation 180°)
      for (uint32_t i = 0; i < w * h; i++) {
        *dst-- = *src++;
      }
      
      gfx->draw16bitRGBBitmap(x1_flipped, y1_flipped, flipped_buf, w, h);
      free(flipped_buf);
    } else {
      // Fallback si pas de mémoire : dessiner normalement
      gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
    }
  } else {
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
  }
  
  lv_disp_flush_ready(disp);
}

// CALLBACK TACTILE LVGL
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  if (touch_touched()) {
    data->state = LV_INDEV_STATE_PRESSED;
    if (screenFlipped) {
      // Inverser les coordonnées pour rotation 180°
      data->point.x = SCREEN_WIDTH - touch_last_x - 1;
      data->point.y = SCREEN_HEIGHT - touch_last_y - 1;
    } else {
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

// CALLBACK MQTT - Maintenant dans module_mqtt.cpp

// FONCTIONS M'SunPV - Maintenant dans module_msunpv.cpp

// MODE AP V3.3
void startWiFiAP() {
  Serial.println("\nWiFi failed - AP mode");
  wifiAPMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_AP_SSID);
  IPAddress ip(192, 168, 4, 1);
  WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));
  delay(500);
  Serial.println("AP: " + String(WIFI_AP_SSID) + " -> http://192.168.4.1");
}

void scanWiFiNetworks() {
  int n = WiFi.scanNetworks();
  wifiNetworksJSON = "[";
  for (int i = 0; i < n; i++) {
    if (i > 0) wifiNetworksJSON += ",";
    wifiNetworksJSON += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  wifiNetworksJSON += "]";
}

void handleWiFiSetupPage() {
  String html = "";
  html += "<!DOCTYPE html><html><head><meta charset=utf-8>";
  html += "<meta name=viewport content=\"width=device-width\">";
  html += "<title>MSunPV Setup</title>";
  html += "<style>";
  html += "body{font-family:Arial;background:#0c0a09;color:#fff;display:flex;align-items:center;justify-content:center;min-height:100vh;margin:0;padding:20px}";
  html += ".box{width:100%;max-width:400px;background:rgba(41,37,36,0.95);padding:40px;border-radius:20px;border:2px solid #fbbf24}";
  html += "h1{color:#fbbf24;text-align:center;margin:0}";
  html += "input{width:100%;padding:12px;margin:15px 0;background:#1c1917;border:1px solid #fbbf24;color:#fff;border-radius:8px;box-sizing:border-box}";
  html += "button{width:100%;padding:12px;background:#fbbf24;color:#000;border:none;border-radius:8px;font-weight:600;cursor:pointer}";
  html += ".list{max-height:200px;overflow-y:auto;margin:15px 0;border:1px solid #fbbf24;border-radius:8px}";
  html += ".item{padding:10px;border-bottom:1px solid #333;cursor:pointer}";
  html += ".item:hover{background:rgba(251,191,36,0.1)}";
  html += "@media (max-width: 480px){body{padding:10px}.box{padding:20px;max-width:100%}input,button{padding:14px;font-size:16px;min-height:44px}.list{max-height:150px}}";
  html += "</style></head>";
  html += "<body><div class=box>";
  html += "<h1>MSunPV Setup</h1>";
  html += "<p style=text-align:center>Configuration WiFi</p>";
  html += "<div id=status style=text-align:center>Scanning...</div>";
  html += "<div class=list id=networks></div>";
  html += "<input type=text id=ssid placeholder=SSID>";
  html += "<input type=password id=pwd placeholder=Password>";
  html += "<button onclick=connect()>Connect and Restart</button>";
  html += "</div>";
  html += "<script>";
  html += "let nets=[];";
  html += "function scan(){fetch('/scan').then(r=>r.json()).then(d=>{nets=d;show();}).catch(e=>{document.getElementById('status').innerHTML='ERROR'})}";
  html += "function show(){let h='';nets.forEach(n=>{h+='<div class=item onclick=\"pick(this)\">'+n.ssid+'</div>'});document.getElementById('networks').innerHTML=h;}";
  html += "function pick(el){document.getElementById('ssid').value=el.innerText}";
  html += "function connect(){let s=document.getElementById('ssid').value;let p=document.getElementById('pwd').value;if(!s){alert('Select SSID');return}";
  html += "fetch('/connect',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'ssid='+encodeURIComponent(s)+'&password='+encodeURIComponent(p)}).then(()=>{document.getElementById('status').innerHTML='OK - Restarting...';}).catch(e=>{alert('Error')})}";
  html += "scan();";
  html += "</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleWiFiScan() {
  scanWiFiNetworks();
  server.send(200, "application/json", wifiNetworksJSON);
}

void handleWiFiConnect() {
  String ssid = server.arg("ssid");
  String pwd = server.arg("password");
  if (ssid.length() == 0) {
    server.send(400, "text/plain", "No SSID");
    return;
  }
  config_wifi_ssid = ssid;
  config_wifi_password = pwd;
  savePreferences();
  server.send(200, "text/plain", "OK");
  delay(2000);
  ESP.restart();
}

// ============================================
// V11.0 - SHELLY EM MONITORING (maintenant dans module_shelly)
// ============================================
// ENPHASE ENVOY V12.0 - HTTPS FONCTIONNEL (V11.0 - Module)
// ============================================
// Toute la logique Enphase est maintenant dans module_enphase.cpp

// Client HTTPS pour Enphase (créé dans le .ino, passé au module)
WiFiClientSecure clientEnphase;

// PAGE /logs - Affichage logs système V12.1
void handleLogs() {
    String html = R"(<!DOCTYPE html>
<html>
<head>
<meta charset='UTF-8'>
<meta name='viewport' content='width=device-width,initial-scale=1'>
<title>Logs MSunPV</title>
<style>
body{font-family:'Courier New',monospace;background:#000;color:#0f0;padding:10px;margin:0}
.header{background:#111;padding:15px;border:2px solid #0f0;border-radius:8px;margin-bottom:15px}
h2{margin:0 0 10px 0;color:#0f0}
.btn{background:#0f0;color:#000;border:none;padding:8px 15px;border-radius:5px;cursor:pointer;font-weight:bold;margin:3px;font-size:14px}
.btn:hover{background:#0c0}
.btn:active{background:#090}
.btn.active{background:#ff0;color:#000}
.logs{background:#000;border:2px solid #0f0;border-radius:8px;padding:10px;height:calc(100vh - 180px);overflow-y:auto;font-size:12px;line-height:1.4}
.log-line{padding:4px 0;border-bottom:1px solid #003300;font-family:monospace}
.log-debug{color:#888}
.log-alert{color:#f00;font-weight:bold;background:#330000;padding:2px 4px}
.log-ok{color:#0f0;font-weight:bold}
.log-result{color:#ff0;font-weight:bold;background:#333300;padding:2px 4px}
.log-meteo{color:#60a5fa}
.log-enphase{color:#fbbf24}
.log-shelly{color:#22c55e}
.log-mqtt{color:#a78bfa}
.log-system{color:#9ca3af}
.info{color:#0f0;font-size:13px;margin-top:8px}
@media (max-width: 768px){
  body{padding:8px}
  .header{padding:12px;margin-bottom:12px}
  h2{font-size:1.2em}
  .btn{padding:10px 12px;font-size:13px;min-height:44px;margin:4px 2px}
  .logs{font-size:11px;height:calc(100vh - 220px);padding:8px}
  .info{font-size:12px}
}
@media (max-width: 480px){
  body{padding:6px}
  .header{padding:10px}
  h2{font-size:1.1em;margin-bottom:8px}
  .btn{padding:10px;font-size:12px;width:48%;box-sizing:border-box}
  .logs{font-size:10px;height:calc(100vh - 240px)}
  .info{font-size:11px;margin-top:6px}
}
</style>
</head>
<body>
<div class='header'>
<h2>📊 MSunPV Monitor - Logs Système</h2>
<button class='btn' onclick='location.reload()'>🔄 Actualiser</button>
<button class='btn' onclick='toggleAuto()' id='autoBtn'>▶️ Auto 3s</button>
<button class='btn' onclick='location.href="/"'>🏠 Accueil</button>
<button class='btn' onclick='location.href="/enphase"'>⚡ Enphase</button>
<div class='info'>
Logs affichés: <span id='count'>0</span> | Auto-refresh: <span id='status' style='color:#0f0'>OFF</span>
</div>
</div>
<div class='logs' id='logs'>)";

    // Afficher les logs en ordre chronologique
    int count = 0;
    for (int i = 0; i < MAX_LOGS; i++) {
        int idx = (logIndex + i) % MAX_LOGS;
        if (systemLogs[idx].length() > 0) {
            String logLine = systemLogs[idx];
            String cssClass = "log-line";
            
            // Coloration selon le préfixe de module (priorité aux modules)
            if (logLine.indexOf("[Météo]") >= 0) {
                cssClass += " log-meteo";
            } else if (logLine.indexOf("[Enphase]") >= 0) {
                cssClass += " log-enphase";
            } else if (logLine.indexOf("[Shelly 1]") >= 0 || logLine.indexOf("[Shelly 2]") >= 0 || logLine.indexOf("[Shelly]") >= 0) {
                cssClass += " log-shelly";
            } else if (logLine.indexOf("[MQTT]") >= 0) {
                cssClass += " log-mqtt";
            } else if (logLine.indexOf("[V3.") >= 0 || logLine.indexOf("[V10.") >= 0 || logLine.indexOf("[V11.") >= 0 || logLine.indexOf("[V12.") >= 0) {
                cssClass += " log-system";
            } else if (logLine.indexOf("[DEBUG]") >= 0) {
                cssClass += " log-debug";
            }
            
            // Coloration selon le contenu (erreurs, succès)
            if (logLine.indexOf("✅") >= 0 || logLine.indexOf("OK") >= 0 || logLine.indexOf("réussie") >= 0 || logLine.indexOf("terminée") >= 0) {
                cssClass += " log-ok";
            }
            if (logLine.indexOf("❌") >= 0 || logLine.indexOf("ERREUR") >= 0 || logLine.indexOf("ERROR") >= 0 || logLine.indexOf("FAILED") >= 0) {
                cssClass += " log-alert";
            }
            
            // Échapper les caractères HTML pour sécurité
            logLine.replace("&", "&amp;");
            logLine.replace("<", "&lt;");
            logLine.replace(">", "&gt;");
            
            html += "<div class='" + cssClass + "'>" + logLine + "</div>";
            count++;
        }
    }

    html += R"(</div>
<script>
let autoEnabled=false;
let timer=null;

function toggleAuto(){
  autoEnabled=!autoEnabled;
  const btn=document.getElementById('autoBtn');
  const status=document.getElementById('status');
  
  if(autoEnabled){
    btn.textContent='⏸️ Stop Auto';
    btn.classList.add('active');
    status.textContent='ON';
    status.style.color='#ff0';
    timer=setInterval(()=>location.reload(),3000);
  }else{
    btn.textContent='▶️ Auto 3s';
    btn.classList.remove('active');
    status.textContent='OFF';
    status.style.color='#0f0';
    clearInterval(timer);
  }
}

// Auto-scroll en bas pour voir les derniers logs
const logs=document.getElementById('logs');
logs.scrollTop=logs.scrollHeight;

// Afficher le nombre de logs
document.getElementById('count').textContent=')";
    html += String(count);
    html += R"(';
</script>
</body>
</html>)";

    server.send(200, "text/html", html);
}

// Handlers Enphase déplacés dans module_enphase.cpp

// SETUP
void setup() {
  Serial.begin(115200);
  delay(500);  // Attendre que le port USB CDC s'initialise
  Serial.println("\nMSunPV Monitor V12.0 - Démarrage\n");
  
  // V3.2 - Charger configuration depuis NVS
  loadPreferences();
  
  // WiFi
  setupWiFi();
  
  // NTP
  configTime(3600, 3600, "pool.ntp.org");
  
  // MQTT (V11.0 - Module)
  mqtt_init(&mqttClient, &espClient);
  
  // Stats (V11.0 - Module)
  stats_init();
  
  // SD Card (V13.0 - Module)
  // sd_init() retiré (Enphase V2)
  
  // OTA
  setupOTA();
  
  // Serveur Web
  server.on("/", handleRoot);
  server.on("/favicon.ico", handleFavicon);  // V3.4 - Favicon
  server.on("/data", handleData);
  server.on("/update", handleUpdate);
  server.on("/doUpdate", HTTP_POST, handleDoUpdate, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  // V11.0 - Stats (Module)
  server.on("/stats", []() { stats_handleWeb(&server); });
  server.on("/statsData", []() { stats_handleData(&server); });
  server.on("/info", handleInfoWeb);
  server.on("/export", handleExportConfig);
  server.on("/exportEnphase", handleExportEnphaseConfig);
  server.on("/import", HTTP_POST, handleImportConfig);
  server.on("/settings", handleSettingsWeb);
  server.on("/restart", HTTP_POST, handleRestart);
  server.on("/restart", HTTP_GET, handleRestart);
  // V12.1 - Rotation écran
  server.on("/saveScreenFlip", HTTP_POST, handleSaveScreenFlip);
  // V15.0 - Sélection écran (MQTT / Enphase)
  server.on("/screens", handleScreensWeb);
  server.on("/saveScreens", HTTP_POST, handleSaveScreens);
  server.on("/unlockScreen", HTTP_POST, handleUnlockScreen);
  server.on("/saveScreenLock", HTTP_POST, handleSaveScreenLock);
  // V14.0 - Format de date
  server.on("/saveDateFormat", HTTP_POST, handleSaveDateFormat);
  server.on("/saveBrightness", HTTP_POST, handleSaveBrightness);
  server.on("/saveTempoConfig", HTTP_POST, handleSaveTempoConfig);
  // V12.5 - Alarme
  server.on("/alarm/set", handleAlarmSet);
  // V3.2 - Nouvelles routes configuration
  // V11.0 - MQTT (Module)
  server.on("/mqtt", []() { mqtt_handleConfig(&server); });
  server.on("/saveMqtt", HTTP_POST, []() { mqtt_handleSaveConfig(&server); });
  server.on("/wifi", handleWifiConfig);
  server.on("/saveWifi", HTTP_POST, handleSaveWifiConfig);
  // MSunPV retiré (Enphase V2)
  // V10.0 - Météo
  // V11.0 - Météo (Module)
  server.on("/weather", []() { weather_handleWeb(&server); });
  server.on("/saveWeather", HTTP_POST, []() { weather_handleSaveConfig(&server); });
  server.on("/weatherAPI", []() { weather_handleAPI(&server); });  // V3.4 - API JSON météo
  server.on("/formattedDate", handleFormattedDate);  // V3.4 - Date formatée
  // Shelly EM retiré (Enphase V2)
  // V12.0 - Enphase Envoy
  server.on("/enphase", []() { enphase_handleWeb(&server); });
  server.on("/enphaseData", []() { enphase_handleData(&server); });
  server.on("/enphaseStatus", []() { enphase_handleStatus(&server); });
  server.on("/enphaseTest", HTTP_POST, []() { enphase_handleTest(&server); });
  server.on("/saveEnphaseConfig", HTTP_POST, []() { enphase_handleSaveConfig(&server); });
  server.on("/enphase-monitor", handleEnphaseMonitorHome);
  server.on("/enphaseMonitorData", handleEnphaseMonitorData);
  server.on("/enphase-reglages", handleEnphaseReglages);
  // V12.1 - Logs système
  server.on("/logs", handleLogs);
  // V13.0 - SD Card status
  // /sd retiré (Enphase V2)
  // V3.3 - AP WiFi routes
  server.on("/scan", handleWiFiScan);
  server.on("/connect", HTTP_POST, handleWiFiConnect);
  server.begin();
  
  // Initialisation écran
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // Backlight - ⚡ CRITIQUE : Backlight OFF pendant l'init
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, LOW);  // ← Éteint pendant init
  
  // Touch
  touch_init();
  
  // LVGL
  lv_init();
  lv_tick_set_cb(millis_cb);
  
  bufSize = SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(lv_color_t) / 10;
  disp_draw_buf = (lv_color_t *)heap_caps_malloc(bufSize, MALLOC_CAP_SPIRAM);
  if (!disp_draw_buf) {
    Serial.println("ERREUR: allocation mémoire LVGL");
    while(1);
  }
  
  disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, disp_draw_buf, NULL, bufSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
  
  // Input tactile
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  
  // Création UI - Enphase V2 : écran unique Enphase (screenMain non créé)
  createEnphaseScreen();  // V15.0 - Écran Enphase
  createSettingsScreen();
  lv_screen_load(screenEnphase);  // Enphase V2 : toujours écran Enphase
  
  // ✅ Forcer 3 rafraîchissements AVANT d'allumer (LVGL prêt)
  for (int i = 0; i < 3; i++) {
    lv_timer_handler();
    delay(50);
  }
  
  // 💡 Allumer backlight progressivement (soft start ~500ms)
  for (int b = 0; b <= (int)brightnessDay; b += 5) {
    analogWrite(GFX_BL, b);
    delay(10);
  }
  
  // ✅ Forcer 3 rafraîchissements AVANT d'allumer
  for (int i = 0; i < 3; i++) {
    lv_timer_handler();
    delay(50);
  }

  // 💡 Allumer backlight progressivement (soft start)
  for (int b = 0; b <= (int)brightnessDay; b += 5) {
    analogWrite(GFX_BL, b);
    delay(10);
  }

  // Démarrage (V10.0) - Startup lock pour LED
  startupTime = millis();
  
  Serial.println("Init OK");
}

// SETUP WIFI
void setupWiFi() {
  Serial.print("[V3.3] Connecting WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(config_wifi_ssid.c_str(), config_wifi_password.c_str());
  
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    ipAddress = WiFi.localIP().toString();
    rssi = WiFi.RSSI();
    Serial.println("\nWiFi OK");
    Serial.println("IP: " + ipAddress);
  } else {
    wifiConnected = false;
    Serial.println("\nWiFi FAILED - AP MODE");
    startWiFiAP();
  }
}

// SETUP OTA
void setupOTA() {
  ArduinoOTA.setHostname("MSunPV-Monitor-V3.1");
  ArduinoOTA.setPassword("msunpv2024");
  
  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Début mise à jour OTA: " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nMise à jour OTA terminée");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erreur OTA [%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA OK");
  
  // Météo initiale (V11.0 - Module)
  weather_init();
  // shelly_init() retiré (Enphase V2)
  // msunpv_init() retiré (Enphase V2)
  delay(2000);  // Attendre connexion WiFi stable
  if (wifiConnected) {
    weather_update();
  }
  
  // Enphase Envoy (V12.0 - V11.0 Module)
  if (wifiConnected) {
    enphase_init(&clientEnphase);
  }
}

// LOOP PRINCIPAL
void loop() {
  static unsigned long lastMqttAttempt = 0;
  static unsigned long lastUpdate = 0;
  static unsigned long lastNightCheck = 0;
  static unsigned long lastHistoCheck = 0;
  
  // OTA
  ArduinoOTA.handle();
  
  // Serveur web
  server.handleClient();
  
  // MQTT (V11.0 - Module) — désactivé en mode Enphase (activeScreenType == 1)
  if (activeScreenType != 1) {
    mqtt_loop();
  }
  
  // MSunPV retiré (Enphase V2)
  
  // Météo (V11.0 - Module) - Mise à jour toutes les 10 minutes
  if (wifiConnected) {
    weather_update();
    // Météo Forecast 4 jours (V12.4) - Mise à jour toutes les 30 minutes
    weather_fetchForecast();
  }
  
  // Shelly EM retiré (Enphase V2)

  // Enphase Envoy (V12.0 - V11.0 Module) - Mise à jour toutes les 10 secondes
  if (wifiConnected) {
    enphase_update();
  }
  
  // EDF TEMPO - fetch au démarrage puis toutes les 30 min, rollover à minuit
  if (wifiConnected) {
    tempo_update();
  }
  
  // Mise à jour UI
  unsigned long now = millis();
  if (now - lastUpdate >= UI_UPDATE_INTERVAL) {
    lastUpdate = now;
    updateUI();
    uptimeSeconds = millis() / 1000;
    
    // Mise à jour signal WiFi
    if (WiFi.status() == WL_CONNECTED) {
      rssi = WiFi.RSSI();
    }
  }
  
  // Mode nuit
  if (now - lastNightCheck >= 60000) {
    lastNightCheck = now;
    applyNightMode();
  }
  
  // Sauvegarde historique (V11.0 - Module Stats)
  if (now - lastHistoCheck >= 60000) {
    lastHistoCheck = now;
    stats_update();
  }
  
  // LVGL - ⚡ ESPACÉ À 20ms AU LIEU DE 5ms (stabilité)
  static unsigned long lastLvglUpdate = 0;
  if (now - lastLvglUpdate >= 20) {
    lastLvglUpdate = now;
    lv_timer_handler();
  }
  delay(5);
}

// MISE À JOUR UI - Enphase V2 : toujours écran Enphase
void updateUI() {
  if (currentPage == 0) {
    if (screenEnphase) {
      updateEnphaseUI();
    }
  } else if (currentPage == 3 && screenSettings) {
    updateSettingsUI();
  }
}

// SERVEUR WEB - PAGE PRINCIPALE
void handleRoot() {
  // V3.3 - Mode AP check
  if (wifiAPMode) {
    handleWiFiSetupPage();
    return;
  }
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Enphase Monitor</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.ico">
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
      background: linear-gradient(135deg, #0c0a09 0%, #1c1917 100%);
      color: #fff;
      min-height: 100vh;
      padding: 20px;
    }
    .header {
      padding: 20px 0 30px 0;
      border-bottom: 2px solid rgba(251, 191, 36, 0.2);
      margin-bottom: 30px;
      position: relative;
    }
    .header h1 {
      text-align: center;
      font-size: 2em;
      color: #fbbf24;
      margin-bottom: 20px;
    }
    /* Indicateurs de connexion WiFi/MQTT */
    .connection-status {
      position: absolute;
      top: 20px;
      left: 0;
      display: flex;
      gap: 8px;
      align-items: center;
      z-index: 10;
    }
    .status-icon {
      width: 28px;
      height: 28px;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: all 0.3s ease;
      cursor: pointer;
    }
    .status-icon svg {
      width: 100%;
      height: 100%;
    }
    .wifi-status .wifi-path,
    .mqtt-status .mqtt-path {
      transition: all 0.3s ease;
      fill: #6b7280;
      opacity: 0.5;
    }
    .wifi-status.connected .wifi-path {
      fill: #3b82f6;
      opacity: 1;
    }
    .wifi-status.disconnected .wifi-path {
      fill: #6b7280;
      opacity: 0.5;
    }
    .mqtt-status.connected .mqtt-path {
      fill: #22c55e;
      opacity: 1;
    }
    .mqtt-status.disconnected .mqtt-path {
      fill: #6b7280;
      opacity: 0.5;
    }
    .header-time {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 20px;
    }
    .temp-section {
      flex: 1;
      text-align: center;
    }
    .temp-label {
      font-size: 0.75em;
      color: #9ca3af;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 5px;
    }
    .temp-value {
      font-size: 1em;
      font-weight: 700;
      color:rgb(246, 242, 239);
    }
    .center-section {
      flex: 1;
      text-align: center;
    }
    .date {
      font-size: 0.9em;
      color: #fff;
      margin-bottom: 8px;
      letter-spacing: 0.5px;
    }
    .time {
      font-size: 2.5em;
      font-weight: 700;
      color: #fff;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
    }
    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    .card {
      background: rgba(41, 37, 36, 0.8);
      border-radius: 16px;
      padding: 20px;
      border: 1px solid rgba(251, 191, 36, 0.25);
      backdrop-filter: blur(10px);
    }
    .card-title {
      font-size: 0.85em;
      color: #d1d5db;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 12px;
    }
    .card-value {
      font-size: 2.5em;
      font-weight: 700;
      margin-bottom: 5px;
    }
    .card-unit {
      font-size: 1em;
      color: #9ca3af;
      font-weight: 500;
    }
    .prod { color: #fbbf24; }
    .conso { color: #60a5fa; }
    .router { color: #fb923c; }
    .water { color: #22c55e; }
    .cabane-section {
      margin-top: 15px;
      padding-top: 15px;
      border-top: 1px solid rgba(251, 191, 36, 0.1);
    }
    .cabane-title {
      font-size: 0.85em;
      color: #fbbf24;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 1px;
      margin-bottom: 8px;
    }
    .cabane-value {
      font-size: 0.85em;
      font-weight: 700;
      color: #a78bfa;
      margin-bottom: 5px;
    }
    .cabane-unit {
      font-size: 0.85em;
      color: #9ca3af;
      font-weight: 500;
    }
    .msunpv-controls {
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 12px;
      margin-top: 10px;
    }
    .msunpv-status {
      display: inline-block;
      padding: 5px 12px;
      border-radius: 6px;
      font-size: 0.9em;
      font-weight: 600;
    }
    .status-auto { background: #22c55e; color: white; }
    .status-manu { background: #60a5fa; color: white; }
    .status-off { background: white; color: black; }
    .led {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      box-shadow: 0 0 20px currentColor;
      flex-shrink: 0;
    }
    .led-green { background: #22c55e; }
    .led-red { background: #ef4444; }
    .nav {
      display: flex;
      gap: 10px;
      justify-content: center;
      flex-wrap: wrap;
    }
    .btn {
      background: #374151;
      color: #fff;
      padding: 12px 24px;
      border-radius: 10px;
      text-decoration: none;
      font-weight: 600;
      transition: all 0.2s;
      border: 1px solid rgba(251, 191, 36, 0.2);
    }
    .btn:hover {
      background: #4b5563;
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    }
    .presence-container {
      display: flex;
      gap: 8px;
      justify-content: center;
      margin-top: 12px;
    }
    .person-pill {
      position: relative;
      padding: 6px 14px;
      border-radius: 20px;
      display: flex;
      align-items: center;
      gap: 8px;
      transition: all 0.3s ease;
      font-size: 0.75em;
    }
    .person-pill.present {
      background: rgba(16, 185, 129, 0.15);
      border: 2px solid #10b981;
    }
    .person-pill.absent {
      background: rgba(107, 114, 128, 0.2);
      border: 2px solid #6b7280;
    }
    .person-pill .emoji {
      font-size: 20px;
    }
    .person-pill.absent .emoji {
      opacity: 0.4;
    }
    .status-dot {
      width: 9px;
      height: 9px;
      border-radius: 50%;
      position: absolute;
      top: 6px;
      right: 6px;
    }
    .status-dot.present {
      background: #10b981;
      box-shadow: 0 0 8px rgba(16, 185, 129, 0.8);
      animation: pulse-presence 2s ease-in-out infinite;
    }
    @keyframes pulse-presence {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.5; }
    }
    .status-dot.absent {
      background: #9ca3af;
    }
    .person-name {
      font-size: 12px;
      font-weight: 600;
      color: #f3f4f6;
    }
    .person-pill.absent .person-name {
      color: #9ca3af;
    }
    /* ============================================ */
    /* ALARME - STYLE 1 : ICÔNE + DROPDOWN (2 BOUTONS) */
    /* ============================================ */
    .alarm-section {
      position: relative;
      margin-left: 8px;
    }
    .alarm-icon-container {
      position: relative;
    }
    .alarm-icon {
      width: 40px;
      height: 40px;
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 22px;
      cursor: pointer;
      transition: all 0.3s ease;
      border: 2px solid transparent;
    }
    .alarm-icon:hover {
      transform: scale(1.1);
      box-shadow: 0 4px 12px rgba(0,0,0,0.3);
    }
    .alarm-icon.off {
      background: rgba(209, 213, 219, 0.3);
      border-color: #d1d5db;
    }
    .alarm-icon.on {
      background: linear-gradient(135deg, rgba(254, 226, 226, 0.9) 0%, rgba(254, 202, 202, 0.9) 100%);
      border-color: #dc2626;
    }
    .alarm-icon.on .icon {
      animation: shake 0.5s ease-in-out infinite;
    }
    @keyframes shake {
      0%, 100% { transform: rotate(0deg); }
      25% { transform: rotate(-10deg); }
      75% { transform: rotate(10deg); }
    }
    .alarm-dropdown {
      position: absolute;
      top: 50px;
      right: 0;
      background: rgba(41, 37, 36, 0.98);
      border-radius: 12px;
      box-shadow: 0 8px 24px rgba(0,0,0,0.4);
      padding: 20px;
      min-width: 240px;
      z-index: 1000;
      opacity: 0;
      visibility: hidden;
      transform: translateY(-10px);
      transition: all 0.3s ease;
      border: 1px solid rgba(251, 191, 36, 0.3);
    }
    .alarm-dropdown.show {
      opacity: 1;
      visibility: visible;
      transform: translateY(0);
    }
    .alarm-dropdown::before {
      content: '';
      position: absolute;
      top: -8px;
      right: 15px;
      width: 16px;
      height: 16px;
      background: rgba(41, 37, 36, 0.98);
      transform: rotate(45deg);
      box-shadow: -2px -2px 4px rgba(0,0,0,0.2);
      border-left: 1px solid rgba(251, 191, 36, 0.3);
      border-top: 1px solid rgba(251, 191, 36, 0.3);
    }
    .dropdown-header {
      font-size: 16px;
      font-weight: 600;
      color: #fbbf24;
      margin-bottom: 15px;
      padding-bottom: 10px;
      border-bottom: 1px solid rgba(251, 191, 36, 0.2);
    }
    .alarm-state-display {
      text-align: center;
      margin-bottom: 15px;
    }
    .state-label {
      font-size: 12px;
      color: #9ca3af;
      text-transform: uppercase;
      margin-bottom: 5px;
    }
    .state-value {
      font-size: 18px;
      font-weight: 700;
    }
    .state-value.off {
      color: #9ca3af;
    }
    .state-value.on {
      color: #ef4444;
    }
    .buttons-container {
      display: flex;
      flex-direction: column;
      gap: 10px;
      margin-top: 15px;
    }
    .alarm-btn {
      padding: 12px;
      border: none;
      border-radius: 8px;
      font-size: 14px;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.3s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
      min-height: 44px;
    }
    .alarm-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(0,0,0,0.15);
    }
    .alarm-btn:active {
      transform: translateY(0);
    }
    .btn-activate {
      background: linear-gradient(135deg, #dc2626 0%, #b91c1c 100%);
      color: white;
    }
    .btn-activate:hover {
      background: linear-gradient(135deg, #b91c1c 0%, #991b1b 100%);
    }
    .btn-deactivate {
      background: linear-gradient(135deg, #10b981 0%, #059669 100%);
      color: white;
    }
    .btn-deactivate:hover {
      background: linear-gradient(135deg, #059669 0%, #047857 100%);
    }
    /* Media queries pour mobile */
    @media (max-width: 768px) {
      body { padding: 12px; }
      .header { padding: 15px 0 20px 0; margin-bottom: 20px; }
      .connection-status {
        top: 12px;
        left: 12px;
      }
      .status-icon {
        width: 24px;
        height: 24px;
      }
      .header h1 { font-size: 1.5em; margin-bottom: 15px; }
      .header-time { flex-direction: column; gap: 15px; }
      .temp-section { margin: 0; }
      .time { font-size: 2em; }
      .date { font-size: 0.85em; }
      .container { max-width: 100%; }
      .grid { grid-template-columns: 1fr; gap: 12px; margin-bottom: 20px; }
      .card { padding: 15px; }
      .card-title { font-size: 0.8em; }
      .card-value { font-size: 2em; }
      .nav { gap: 8px; margin-top: 20px; }
      .btn { padding: 14px 20px; font-size: 0.9em; min-height: 44px; }
      .presence-container { gap: 6px; margin-top: 8px; }
      .person-pill { padding: 5px 12px; font-size: 0.7em; }
      .person-pill .emoji { font-size: 18px; }
      .person-name { font-size: 11px; }
      .alarm-icon { width: 35px; height: 35px; font-size: 20px; }
      .alarm-dropdown { min-width: 200px; padding: 15px; right: -10px; }
    }
    @media (max-width: 480px) {
      body { padding: 10px; }
      .header { padding: 12px 0 15px 0; }
      .connection-status {
        top: 10px;
        left: 10px;
      }
      .status-icon {
        width: 22px;
        height: 22px;
      }
      .header h1 { font-size: 1.3em; }
      .header-time { 
        display: grid;
        grid-template-columns: 1fr 1fr;
        grid-template-rows: auto auto auto auto;
        gap: 8px; 
        align-items: center;
        justify-items: center;
        position: relative;
      }
      /* Ligne 2 : Les deux températures côte à côte */
      .header-time > .temp-section:first-child {
        grid-column: 1;
        grid-row: 1;
        justify-self: end;
      }
      /* Température salon - garder à sa place normale (ligne 1, colonne 2) */
      .header-time > .temp-section:last-child {
        grid-column: 2;
        grid-row: 1;
        justify-self: start;
      }
      /* Extraire les présences et les positionner par rapport à header-time */
      .header-time > .temp-section:last-child .presence-container {
        position: absolute;
        left: 50%;
        transform: translateX(-50%);
        display: flex;
        justify-content: center;
        width: 100%;
        z-index: 10;
      }
      /* Positionner après l'heure - calcul basé sur la structure de la grille */
      /* On utilise une valeur qui place après la ligne 3 (heure) */
      .header-time > .temp-section:last-child .presence-container {
        top: calc(3 * (1em + 8px) + 2em + 8px);
      }
      /* Permettre aux enfants de center-section de participer à la grille */
      .header-time .center-section {
        grid-column: 1 / -1;
        display: contents;
      }
      /* Ligne 3 : Date seule */
      .header-time .center-section .date {
        grid-column: 1 / -1;
        grid-row: 2;
        text-align: center;
        margin-bottom: 0;
      }
      /* Ligne 4 : Heure seule */
      .header-time .center-section .time {
        grid-column: 1 / -1;
        grid-row: 3;
        text-align: center;
      }
      /* Espace pour les présences en ligne 5 */
      .header-time {
        padding-bottom: 50px;
      }
      .time { font-size: 1.8em; }
      .date { margin-bottom: 0; font-size: 0.85em; }
      .temp-value { font-size: 0.9em; }
      .temp-label { font-size: 0.7em; }
      .grid { 
        grid-template-columns: repeat(2, 1fr);
        gap: 8px; 
      }
      .card { padding: 12px; border-radius: 12px; }
      .card-value { font-size: 1.8em; }
      .card-unit { font-size: 0.9em; }
      .nav { flex-direction: column; }
      .btn { width: 100%; padding: 14px; }
      .msunpv-controls { gap: 8px; }
      .msunpv-status { padding: 6px 14px; font-size: 0.85em; }
      .led { width: 35px; height: 35px; }
      .alarm-icon { width: 32px; height: 32px; font-size: 18px; }
      .alarm-dropdown { min-width: 180px; padding: 12px; right: -20px; top: 45px; }
      .alarm-btn { padding: 10px; font-size: 12px; min-height: 40px; }
      .presence-container { gap: 4px; }
      .temp-card-text { font-size: 0.9em; }
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="connection-status">
        <div class="status-icon wifi-status disconnected" id="wifiIcon" title="WiFi">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M1 9L3 11C7.97 6.03 16.03 6.03 21 11L23 9C16.93 2.93 7.07 2.93 1 9Z" class="wifi-path"/>
            <path d="M5 13L7 15C9.76 12.24 14.24 12.24 17 15L19 13C15.14 9.14 8.86 9.14 5 13Z" class="wifi-path"/>
            <path d="M9 17L12 20L15 17C13.35 15.35 10.65 15.35 9 17Z" class="wifi-path"/>
          </svg>
        </div>
        <div class="status-icon mqtt-status disconnected" id="mqttIcon" title="MQTT">
          <svg width="24" height="24" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M12 2L2 7L12 12L22 7L12 2Z" class="mqtt-path"/>
            <path d="M2 17L12 22L22 17" class="mqtt-path"/>
            <path d="M2 12L12 17L22 12" class="mqtt-path"/>
          </svg>
        </div>
      </div>
      <h1>☀️ Enphase Monitor</h1>
      <div class="header-time">
        <div class="temp-section">
          <div class="temp-label">🌡️ EXTÉRIEUR</div>
          <div class="temp-value" id="tempExtHeader">0.0°C</div>
        </div>
        <div class="center-section">
          <div class="date" id="date">--</div>
          <div class="time" id="time">--:--</div>
        </div>
        <div class="temp-section">
          <div class="temp-label">🏠 SALON</div>
          <div class="temp-value" id="tempSalonHeader">0.0°C</div>
          <div class="presence-container">
            <div class="person-pill absent" id="presenceBen">
              <span class="emoji">👨</span>
              <span class="person-name">Benoît</span>
              <span class="status-dot absent"></span>
            </div>
            <div class="person-pill absent" id="presenceFrancine">
              <span class="emoji">👱‍♀️</span>
              <span class="person-name">Francine</span>
              <span class="status-dot absent"></span>
            </div>
            <div class="person-pill absent" id="presenceVictor" style="display:none">
              <span class="emoji">👦</span>
              <span class="person-name">Victor</span>
              <span class="status-dot absent"></span>
            </div>
            <div class="alarm-section">
              <div class="alarm-icon-container">
                <div class="alarm-icon off" id="alarmIcon" onclick="toggleAlarmDropdown()">
                  <span class="icon">🔔</span>
                </div>
                <div class="alarm-dropdown" id="alarmDropdown">
                  <div class="dropdown-header" id="alarmHeader">🔔 ALARME</div>
                  <div class="alarm-state-display">
                    <div class="state-label">État actuel</div>
                    <div class="state-value off" id="alarmStateValue">DÉSACTIVÉE</div>
                  </div>
                  <div class="buttons-container">
                    <button class="alarm-btn btn-activate" id="alarmBtnActivate">🚨 ACTIVER</button>
                    <button class="alarm-btn btn-deactivate" id="alarmBtnDeactivate">✅ DÉSACTIVER</button>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    
    <div class="grid">
      <div class="card">
        <div class="card-title">Production Solaire</div>
        <div class="card-value prod"><span id="prod">0</span> <span class="card-unit">W</span></div>
        <div class="cabane-section">
          <div class="cabane-title">Cab.</div>
          <div class="cabane-value"><span id="cabane">0</span> <span class="cabane-unit">W</span></div>
        </div>
      </div>
      
      <div class="card">
        <div class="card-title">Consommation Maison</div>
        <div class="card-value conso"><span id="conso">0</span> <span class="card-unit">W</span></div>
      </div>
      
      <div class="card">
        <div class="card-title">Routage</div>
        <div class="card-value router"><span id="router">0</span> <span class="card-unit">W</span></div>
      </div>
      
      <div class="card">
        <div class="card-title">Température Cumulus</div>
        <div class="card-value water"><span id="water">48</span><span class="card-unit">°C</span></div>
        <div class="msunpv-controls">
          <div class="led" id="led"></div>
        </div>
      </div>
      
      <div class="card">
        <div class="card-title">Conso Jour</div>
        <div class="card-value" style="color:#a78bfa"><span id="consoJour">0.0</span> <span class="card-unit">kWh</span></div>
      </div>
      
      <div class="card">
        <div class="card-title">Températures</div>
        <div class="temp-card-text" style="font-size:1.2em; margin-top:10px">
          <div style="margin-bottom:8px">🌡️ Geaune: <span id="tempExt">--</span>°C</div>
          <div>🏠 Salon: <span id="tempSalon">--</span>°C</div>
        </div>
      </div>
    </div>
    
    <div class="nav">
      <a href="/mqtt" class="btn">📡 Config MQTT</a>
      <a href="/wifi" class="btn">📶 Config WiFi</a>
      <a href="/enphase" class="btn">📡 Enphase Envoy</a>
      <a href="/enphase-monitor" class="btn">☀️ Enphase Monitor</a>
      <a href="/logs" class="btn" style="background:#0f0;color:#000;font-weight:bold">📊 Logs Système</a>
      <a href="/stats" class="btn">📊 Statistiques</a>
      <a href="/info" class="btn">ℹ️ Infos Système</a>
      <a href="/update" class="btn">🔄 Mise à jour OTA</a>
    </div>
  </div>
  
  <script>
    function updateData() {
      fetch('/data')
        .then(r => r.json())
        .then(d => {
          document.getElementById('prod').textContent = d.prod.toFixed(0);
          document.getElementById('conso').textContent = d.conso.toFixed(0);
          document.getElementById('router').textContent = d.router.toFixed(0);
          document.getElementById('water').textContent = d.waterTemp.toFixed(0);
          document.getElementById('consoJour').textContent = d.consoJour.toFixed(1);
          document.getElementById('cabane').textContent = d.solarProdCabane.toFixed(0);
          
          // Mise à jour des températures du header
          document.getElementById('tempExtHeader').textContent = d.tempExt.toFixed(1) + '°C';
          document.getElementById('tempSalonHeader').textContent = d.tempSalon.toFixed(1) + '°C';
          
          // Mise à jour des températures dans la carte
          document.getElementById('tempExt').textContent = d.tempExt.toFixed(1);
          document.getElementById('tempSalon').textContent = d.tempSalon.toFixed(1);
          
          // Mise à jour des présences
          const benEl = document.getElementById('presenceBen');
          const francineEl = document.getElementById('presenceFrancine');
          const victorEl = document.getElementById('presenceVictor');
          if (d.presenceBen) {
            benEl.className = 'person-pill present';
            benEl.querySelector('.status-dot').className = 'status-dot present';
          } else {
            benEl.className = 'person-pill absent';
            benEl.querySelector('.status-dot').className = 'status-dot absent';
          }
          if (d.presenceFrancine) {
            francineEl.className = 'person-pill present';
            francineEl.querySelector('.status-dot').className = 'status-dot present';
          } else {
            francineEl.className = 'person-pill absent';
            francineEl.querySelector('.status-dot').className = 'status-dot absent';
          }
          if (victorEl) {
            if (d.victorEnabled) {
              victorEl.style.display = '';
              if (d.presenceVictor) {
                victorEl.className = 'person-pill present';
                victorEl.querySelector('.status-dot').className = 'status-dot present';
              } else {
                victorEl.className = 'person-pill absent';
                victorEl.querySelector('.status-dot').className = 'status-dot absent';
              }
            } else {
              victorEl.style.display = 'none';
            }
          }
          
          // Mise à jour de l'alarme
          if (d.alarmState !== undefined) {
            updateAlarmState(d.alarmState);
          }
          
          const led = document.getElementById('led');
          led.className = 'led ' + (d.ledGreen ? 'led-green' : 'led-red');
          
          // M'SunPV retiré (Enphase V2)
          
          // Mise à jour des indicateurs WiFi et MQTT
          const wifiIcon = document.getElementById('wifiIcon');
          const mqttIcon = document.getElementById('mqttIcon');
          if (wifiIcon) {
            wifiIcon.className = 'status-icon wifi-status ' + (d.wifiConnected ? 'connected' : 'disconnected');
          }
          if (mqttIcon) {
            mqttIcon.className = 'status-icon mqtt-status ' + (d.mqttConnected ? 'connected' : 'disconnected');
          }
        })
        .catch(e => console.error('Erreur:', e));
    }
    
    function updateTime() {
      const now = new Date();
      const h = String(now.getHours()).padStart(2, '0');
      const m = String(now.getMinutes()).padStart(2, '0');
      document.getElementById('time').textContent = h + ':' + m;
    }
    
    function updateDate() {
      const now = new Date();
      const days = ['Dimanche', 'Lundi', 'Mardi', 'Mercredi', 'Jeudi', 'Vendredi', 'Samedi'];
      const months = ['janvier', 'février', 'mars', 'avril', 'mai', 'juin', 'juillet', 'août', 'septembre', 'octobre', 'novembre', 'décembre'];
      
      const dayName = days[now.getDay()];
      const dayNum = now.getDate();
      const monthName = months[now.getMonth()];
      const year = now.getFullYear();
      
      document.getElementById('date').textContent = dayName + ' ' + dayNum + ' ' + monthName + ' ' + year;
    }
    
    // Fonction pour ouvrir/fermer le dropdown
    function toggleAlarmDropdown() {
      const dropdown = document.getElementById('alarmDropdown');
      if (dropdown) {
        dropdown.classList.toggle('show');
      }
    }
    
    // Fonction pour fermer le dropdown
    function closeAlarmDropdown() {
      const dropdown = document.getElementById('alarmDropdown');
      if (dropdown) {
        dropdown.classList.remove('show');
      }
    }
    
    // Fonction pour mettre à jour l'affichage de l'alarme
    function updateAlarmState(isOn) {
      const icon = document.getElementById('alarmIcon');
      const iconSpan = icon ? icon.querySelector('.icon') : null;
      const header = document.getElementById('alarmHeader');
      const stateValue = document.getElementById('alarmStateValue');
      
      if (isOn) {
        if (icon) {
          icon.classList.remove('off');
          icon.classList.add('on');
        }
        if (iconSpan) iconSpan.textContent = '🚨';
        if (header) header.textContent = '🚨 ALARME';
        if (stateValue) {
          stateValue.textContent = 'ACTIVÉE';
          stateValue.classList.remove('off');
          stateValue.classList.add('on');
        }
      } else {
        if (icon) {
          icon.classList.remove('on');
          icon.classList.add('off');
        }
        if (iconSpan) iconSpan.textContent = '🔔';
        if (header) header.textContent = '🔔 ALARME';
        if (stateValue) {
          stateValue.textContent = 'DÉSACTIVÉE';
          stateValue.classList.remove('on');
          stateValue.classList.add('off');
        }
      }
    }
    
    // Gestion des boutons alarme
    document.addEventListener('DOMContentLoaded', function() {
      const btnActivate = document.getElementById('alarmBtnActivate');
      const btnDeactivate = document.getElementById('alarmBtnDeactivate');
      
      console.log('[Alarme] Initialisation - btnActivate:', btnActivate ? 'trouvé' : 'NON TROUVÉ');
      console.log('[Alarme] Initialisation - btnDeactivate:', btnDeactivate ? 'trouvé' : 'NON TROUVÉ');
      
      if (btnActivate) {
        btnActivate.addEventListener('click', function(e) {
          e.preventDefault();
          e.stopPropagation();
          e.stopImmediatePropagation();
          console.log('[Alarme] Bouton Activer cliqué');
          
          // S'assurer que le dropdown reste ouvert pendant le traitement
          const dropdown = document.getElementById('alarmDropdown');
          if (dropdown) {
            dropdown.classList.add('show');
          }
          
          fetch('/alarm/set?state=ON')
            .then(r => {
              console.log('[Alarme] Réponse HTTP reçue:', r.status);
              return r.json();
            })
            .then(data => {
              console.log('[Alarme] Données reçues:', JSON.stringify(data));
              if (data.success) {
                console.log('[Alarme] Commande ACTIVER envoyée avec succès');
                // Fermer le dropdown après un petit délai pour s'assurer que la requête est bien partie
                setTimeout(function() {
                  closeAlarmDropdown();
                }, 100);
                // L'état sera mis à jour automatiquement par /data
              } else {
                console.error('Erreur activation alarme:', data.error);
              }
            })
            .catch(e => {
              console.error('Erreur fetch:', e);
            });
          
          return false;
        }, true);  // Utiliser capture phase pour intercepter avant les autres listeners
      } else {
        console.error('[Alarme] ERREUR: Bouton Activer non trouvé !');
      }
      
      if (btnDeactivate) {
        btnDeactivate.addEventListener('click', function(e) {
          e.preventDefault();
          e.stopPropagation();
          e.stopImmediatePropagation();
          console.log('[Alarme] Bouton Désactiver cliqué');
          
          // S'assurer que le dropdown reste ouvert pendant le traitement
          const dropdown = document.getElementById('alarmDropdown');
          if (dropdown) {
            dropdown.classList.add('show');
          }
          
          fetch('/alarm/set?state=OFF')
            .then(r => {
              console.log('[Alarme] Réponse HTTP reçue:', r.status);
              return r.json();
            })
            .then(data => {
              console.log('[Alarme] Données reçues:', JSON.stringify(data));
              if (data.success) {
                console.log('[Alarme] Commande DÉSACTIVER envoyée avec succès');
                // Fermer le dropdown après un petit délai pour s'assurer que la requête est bien partie
                setTimeout(function() {
                  closeAlarmDropdown();
                }, 100);
                // L'état sera mis à jour automatiquement par /data
              } else {
                console.error('Erreur désactivation alarme:', data.error);
              }
            })
            .catch(e => {
              console.error('Erreur fetch:', e);
            });
          
          return false;
        }, true);  // Utiliser capture phase pour intercepter avant les autres listeners
      } else {
        console.error('[Alarme] ERREUR: Bouton Désactiver non trouvé !');
      }
      
      // Fermer dropdown si clic dehors (mais pas sur les boutons)
      document.addEventListener('click', function(event) {
        const alarmSection = document.querySelector('.alarm-section');
        const btnActivate = document.getElementById('alarmBtnActivate');
        const btnDeactivate = document.getElementById('alarmBtnDeactivate');
        
        // Ne pas fermer si on clique sur les boutons
        if (event.target === btnActivate || event.target === btnDeactivate) {
          return;
        }
        
        // Ne pas fermer si on clique dans le dropdown
        const dropdown = document.getElementById('alarmDropdown');
        if (dropdown && dropdown.contains(event.target)) {
          return;
        }
        
        // Fermer si on clique ailleurs
        if (alarmSection && !alarmSection.contains(event.target)) {
          closeAlarmDropdown();
        }
      });
    });
    
    updateData();
    updateTime();
    updateDate();
    setInterval(updateData, 1000);
    setInterval(updateTime, 1000);
    setInterval(updateDate, 60000);
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

// SERVEUR WEB - API JSON
void handleData() {
  // Calcul état LED (synchronisé avec updateMainUI)
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  
  // V10.0 - Pendant startup, forcer LED rouge (attendre données MQTT + 30s)
  if (ledStartupLock) {
    if (mqttDataReceived && (millis() - startupTime) > 30000) {
      ledStartupLock = false;
    } else if ((millis() - startupTime) > 60000) {
      ledStartupLock = false;
    }
  }
  
  if (hour == 23 && minute == 0) {
    ledLockedGreen = false;
  }
  
  if (!ledStartupLock) {
    if (!ledLockedGreen) {
      if ((waterTemp > 55) && (routerPower == 0)) {
        ledLockedGreen = true;
      }
    }
  } else {
    ledLockedGreen = false;
  }
  
  String json = "{";
  json += "\"solarProd\":" + String(solarProd, 1) + ",";
  json += "\"solarProdMain\":" + String(solarProdMain, 1) + ",";
  json += "\"solarProdCabane\":" + String(solarProdCabane, 1) + ",";
  json += "\"prod\":" + String(solarProd, 1) + ",";
  json += "\"conso\":" + String(homeConso, 1) + ",";
  json += "\"homeConso\":" + String(homeConso, 1) + ",";
  json += "\"router\":" + String(routerPower, 1) + ",";
  json += "\"routerPower\":" + String(routerPower, 1) + ",";
  json += "\"waterTemp\":" + String(waterTemp, 1) + ",";
  json += "\"consoJour\":" + String(consoJour, 2) + ",";
  json += "\"tempExt\":" + String(tempExt, 1) + ",";
  json += "\"tempSalon\":" + String(tempSalon, 1) + ",";
  json += "\"presenceBen\":" + String(presenceBen ? "true" : "false") + ",";
  json += "\"presenceFrancine\":" + String(presenceFrancine ? "true" : "false") + ",";
  json += "\"victorEnabled\":" + String(config_victor_enabled ? "true" : "false") + ",";
  json += "\"presenceVictor\":" + String(presenceVictor ? "true" : "false") + ",";
  json += "\"alarmState\":" + String(alarmState ? "true" : "false") + ",";
  json += "\"ledGreen\":" + String(ledLockedGreen ? "true" : "false") + ",";
  json += "\"wifiConnected\":" + String(wifiConnected ? "true" : "false") + ",";
  json += "\"mqttConnected\":" + String(mqttConnected ? "true" : "false") + ",";
  json += "\"msunpvStatus\":\"\",";
  json += "\"msunpv_status\":\"\"";
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handler - Retourne la date formatée selon dateFormatIndex (V3.4)
void handleFormattedDate() {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  // Tableaux de jours et mois
  const char* daysFull[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
  const char* daysShort[] = {"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
  const char* months[] = {"Jan.", "Fev.", "Mar.", "Avr.", "Mai", "Juin", "Juil.", "Aout", "Sep.", "Oct.", "Nov.", "Dec."};
  
  String formattedDate = "";
  char buffer[64];
  
  switch(dateFormatIndex) {
    case 0:  // Dimanche 28/12/25
      sprintf(buffer, "%s %02d/%02d/%02d", 
              daysFull[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year % 100);
      break;
    case 1:  // Dim. 28 Dec. 2025
      sprintf(buffer, "%s %02d %s %04d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              months[timeinfo->tm_mon],
              timeinfo->tm_year + 1900);
      break;
    case 2:  // 28/12/2025
      sprintf(buffer, "%02d/%02d/%04d", 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year + 1900);
      break;
    case 3:  // Dim. 28/12/2025
      sprintf(buffer, "%s %02d/%02d/%04d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year + 1900);
      break;
    default:
      sprintf(buffer, "%s %02d %s %04d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              months[timeinfo->tm_mon],
              timeinfo->tm_year + 1900);
  }
  
  formattedDate = String(buffer);
  String json = "{\"date\":\"" + formattedDate + "\"}";
  server.send(200, "application/json", json);
}

// ========== ENPHASE MONITOR (page d'accueil web dédiée) ==========
void handleEnphaseMonitorData() {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char timeBuf[16];
  sprintf(timeBuf, "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
  char dateBuf[64];
  const char* daysShort[] = {"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
  const char* months[] = {"Jan.", "Fev.", "Mar.", "Avr.", "Mai", "Juin", "Juil.", "Aout", "Sep.", "Oct.", "Nov.", "Dec."};
  sprintf(dateBuf, "%s %02d %s %04d", daysShort[t->tm_wday], t->tm_mday, months[t->tm_mon], t->tm_year + 1900);

  String j = "{";
  j += "\"date\":\"" + String(dateBuf) + "\",";
  j += "\"time\":\"" + String(timeBuf) + "\",";
  j += "\"wifi\":" + String(wifiConnected ? "true" : "false") + ",";
  j += "\"enphase_connected\":" + String(enphase_connected ? "true" : "false") + ",";
  j += "\"weather_city\":\"" + weather_city + "\",";
  j += "\"weather_temp\":" + String(weather_temp, 1) + ",";
  j += "\"weather_code\":" + String(weather_code) + ",";
  const char* dayNames[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
  j += "\"forecast\":[";
  for (int i = 1; i <= 4; i++) {
    if (i > 1) j += ",";
    int wday = (t->tm_wday + i) % 7;
    char d[2] = { weather_forecast_days[i], '\0' };
    j += "{\"day\":\"" + String(d) + "\",\"day_name\":\"" + String(dayNames[wday]) + "\",\"temp\":" + String(weather_forecast_temps[i]) + ",\"code\":" + String(weather_forecast_codes[i]) + "}";
  }
  j += "],";
  j += "\"pact_prod\":" + String(enphase_pact_prod, 0) + ",";
  j += "\"pact_conso\":" + String(enphase_pact_conso, 0) + ",";
  j += "\"pact_grid\":" + String(enphase_pact_grid, 0) + ",";
  j += "\"energy_produced\":" + String(enphase_energy_produced / 1000.0f, 1) + ",";
  j += "\"energy_imported\":" + String(enphase_energy_imported / 1000.0f, 1) + "}";
  server.send(200, "application/json", j);
}

void handleEnphaseMonitorHome() {
  if (wifiAPMode) { handleWiFiSetupPage(); return; }
  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Enphase Monitor</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:linear-gradient(135deg,#0c0a09 0%,#1c1917 100%);color:#fff;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Arial,sans-serif;min-height:100vh;padding:12px}
.container{max-width:480px;margin:0 auto}
.header{text-align:center;padding:12px 0;border-bottom:2px solid rgba(251,191,36,0.3);margin-bottom:16px}
.header-title{font-size:1.4em;font-weight:700;color:#fbbf24;display:block;margin-bottom:4px}
.header-date{font-size:0.9em;color:#d1d5db;display:block}
.header-time{font-size:1.5em;font-weight:700;color:#fff;display:block;margin-top:4px}
.header-row{display:flex;align-items:center;justify-content:center;gap:12px;margin-top:10px;flex-wrap:wrap}
.header-icons span{width:28px;height:28px;border-radius:50%;display:inline-flex;align-items:center;justify-content:center;font-size:14px;background:rgba(55,65,81,0.8)}
.header-icons .ok{background:rgba(34,197,94,0.5);color:#22c55e}
.header-icons .ko{opacity:0.5;color:#6b7280}
.header a{color:#fbbf24;text-decoration:none;font-size:0.95em}
.header a:hover{text-decoration:underline}
.barre-meteo{display:flex;align-items:flex-start;gap:16px;padding:14px;background:rgba(41,37,36,0.9);border-radius:12px;border:1px solid rgba(251,191,36,0.25);margin-bottom:16px}
.meteo-gauche{display:flex;flex-direction:column;align-items:flex-start;gap:8px}
.meteo-gauche .ville{font-weight:600;color:#fff;font-size:0.95em}
.meteo-gauche .temp{font-size:1.5em;font-weight:700;color:#fbbf24}
.meteo-gauche .ico{font-size:2.2em;line-height:1}
.meteo-droite{display:flex;gap:10px;flex:1;justify-content:flex-end;flex-wrap:wrap}
.meteo-droite .forecast-day{text-align:center;min-width:52px}
.meteo-droite .forecast-day .d{font-size:0.75em;color:#9ca3af;margin-bottom:2px}
.meteo-droite .forecast-day .t{font-size:0.95em;font-weight:600;color:#fff}
.cartes-deux{display:flex;gap:12px;margin-bottom:16px;flex-wrap:wrap}
.cartes-deux .carte{flex:1;min-width:140px;padding:14px;border-radius:12px;border:1px solid rgba(251,191,36,0.25);background:rgba(41,37,36,0.9)}
.cartes-deux .carte .ligne{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px}
.cartes-deux .carte .ligne:last-child{margin-bottom:0}
.cartes-deux .carte .label{font-size:0.8em;color:#9ca3af}
.cartes-deux .carte .val{font-size:1.1em;font-weight:700}
.cartes-deux .carte .val.prod{color:#fbbf24}
.cartes-deux .carte .val.conso{color:#3b82f6}
.flux{display:flex;flex-wrap:wrap;align-items:center;justify-content:center;gap:4px 12px;padding:16px;background:rgba(41,37,36,0.9);border-radius:12px;border:1px solid rgba(251,191,36,0.25)}
.flux-item{display:flex;flex-direction:column;align-items:center;gap:4px}
.flux-item .ico{font-size:1.6em}
.flux-item .val{font-size:0.9em;color:#9ca3af;font-weight:600}
.flux-chevron{display:flex;align-items:center;justify-content:center;flex-shrink:0}
.flux-chevron svg{width:18px;height:18px;transition:fill .3s}
@media(max-width:400px){.meteo-droite{justify-content:flex-start}.cartes-deux .carte{min-width:100%}}
</style></head><body>
<div class='container'>
<div class='header'>
  <span class='header-title'>☀️ Enphase Monitor</span>
  <span class='header-date' id='dateStr'>--</span>
  <span class='header-time' id='timeStr'>--:--:--</span>
  <div class='header-row'>
    <span id='icoWifi' class='ko' title='WiFi'>📶</span>
    <span id='icoEnphase' class='ko' title='Enphase'>⚡</span>
    <a href='/enphase-reglages'>Réglages</a>
  </div>
</div>
<div class='barre-meteo'>
  <div class='meteo-gauche'>
    <span class='ville' id='weatherCity'>--</span>
    <span class='temp' id='weatherTemp'>--°C</span>
    <span class='ico' id='weatherIco'>--</span>
  </div>
  <div class='meteo-droite' id='forecastRow'></div>
</div>
<div class='cartes-deux'>
  <div class='carte'>
    <div class='ligne'><span class='label'>Production</span><span class='val prod' id='valProd'>0 W</span></div>
    <div class='ligne'><span class='label'>Conso</span><span class='val conso' id='valConso'>0 W</span></div>
  </div>
  <div class='carte'>
    <div class='ligne'><span class='label'>Production jour</span><span class='val prod' id='energyProd'>0.0 kWh</span></div>
    <div class='ligne'><span class='label'>Conso jour</span><span class='val conso' id='energyConso'>0.0 kWh</span></div>
  </div>
</div>
<div class='flux'>
  <div class='flux-item'><span class='ico'>☀️</span><span>PV</span></div>
  <span class='flux-chevron' id='ch1' aria-hidden='true'><svg viewBox='0 0 24 24' fill='currentColor'><path d='M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z'/></svg></span>
  <div class='flux-item'><span class='val' id='valPvMaison'>0 W</span></div>
  <span class='flux-chevron' id='ch2' aria-hidden='true'><svg viewBox='0 0 24 24' fill='currentColor'><path d='M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z'/></svg></span>
  <div class='flux-item'><span class='ico'>🏠</span><span>Maison</span></div>
  <span class='flux-chevron' id='ch3' aria-hidden='true'><svg viewBox='0 0 24 24' fill='currentColor'><path d='M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z'/></svg></span>
  <div class='flux-item'><span class='val' id='valMaisonReseau'>0 W</span></div>
  <span class='flux-chevron' id='ch4' aria-hidden='true'><svg viewBox='0 0 24 24' fill='currentColor'><path d='M10 6L8.59 7.41 13.17 12l-4.58 4.59L10 18l6-6z'/></svg></span>
  <div class='flux-item'><span class='ico'>🔌</span><span>Réseau</span></div>
</div>
</div>
<script>
function codeToEmoji(c){if(c==null)return'--';var x=Number(c);if(x>=800&&x<803)return'☀️';if(x>=803)return'☁️';if(x>=700)return'🌫️';if(x>=600)return'❄️';if(x>=500)return'🌧️';if(x>=300)return'🌧️';if(x>=200)return'⛈️';return'🌤️';}
function upd(){
  fetch('/enphaseMonitorData').then(r=>r.json()).then(d=>{
    document.getElementById('dateStr').textContent=d.date||'--';
    document.getElementById('timeStr').textContent=d.time||'--:--:--';
    document.getElementById('icoWifi').className=d.wifi?'ok':'ko';
    document.getElementById('icoEnphase').className=d.enphase_connected?'ok':'ko';
    document.getElementById('weatherCity').textContent=d.weather_city||'--';
    document.getElementById('weatherTemp').textContent=(d.weather_temp!=null?d.weather_temp.toFixed(0):'--')+'°C';
    document.getElementById('weatherIco').textContent=codeToEmoji(d.weather_code);
    var fr=document.getElementById('forecastRow');
    fr.innerHTML='';
    if(d.forecast&&d.forecast.length){for(var i=0;i<d.forecast.length;i++){var x=d.forecast[i];var n=x.day_name||x.day;var div=document.createElement('div');div.className='forecast-day';div.innerHTML='<div class="d">'+n+'</div><div class="t">'+x.temp+'°</div>';fr.appendChild(div);}}
    document.getElementById('valProd').textContent=(d.pact_prod!=null?Math.round(d.pact_prod):0)+' W';
    document.getElementById('valConso').textContent=(d.pact_conso!=null?Math.round(d.pact_conso):0)+' W';
    document.getElementById('valPvMaison').textContent=(d.pact_prod!=null?Math.round(d.pact_prod):0)+' W';
    document.getElementById('valMaisonReseau').textContent=(d.pact_grid!=null?Math.round(d.pact_grid):0)+' W';
    document.getElementById('energyProd').textContent=(d.energy_produced!=null?d.energy_produced.toFixed(1):'0.0')+' kWh';
    document.getElementById('energyConso').textContent=(d.energy_imported!=null?d.energy_imported.toFixed(1):'0.0')+' kWh';
    var prod=(d.pact_prod!=null?d.pact_prod:0); var grid=(d.pact_grid!=null?d.pact_grid:0);
    var colProd=prod>0?'#fbbf24':'#4b5563'; var colGrid=grid<0?'#22c55e':(grid>0?'#9ca3af':'#4b5563');
    var flip=grid>0;
    document.getElementById('ch1').style.color=colProd; document.getElementById('ch2').style.color=colProd;
    document.getElementById('ch3').style.color=colGrid; document.getElementById('ch4').style.color=colGrid;
    document.getElementById('ch3').style.transform=flip?'scaleX(-1)':'';
    document.getElementById('ch4').style.transform=flip?'scaleX(-1)':'';
  }).catch(function(){});
}
upd();setInterval(upd,5000);
</script></body></html>)";
  server.send(200, "text/html", html);
}

void handleEnphaseReglages() {
  if (wifiAPMode) { handleWiFiSetupPage(); return; }
  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Réglages - Enphase Monitor</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{background:linear-gradient(135deg,#0c0a09 0%,#1c1917 100%);color:#fff;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Arial,sans-serif;min-height:100vh;padding:20px}
.back{display:inline-block;background:#374151;color:#fff;padding:10px 20px;border-radius:8px;text-decoration:none;margin-bottom:24px}
.back:hover{background:#4b5563}
h1{color:#fbbf24;margin-bottom:8px;font-size:1.6em}
.sub{color:#9ca3af;margin-bottom:24px;font-size:0.95em}
.links{display:flex;flex-direction:column;gap:12px;max-width:400px}
.links a{display:block;background:rgba(41,37,36,0.9);border:1px solid rgba(251,191,36,0.3);border-radius:12px;padding:16px 20px;color:#fff;text-decoration:none;font-weight:500;transition:all .2s}
.links a:hover{background:rgba(55,65,81,0.5);border-color:rgba(251,191,36,0.5)}
.card-exp{margin-top:24px;padding:20px;background:rgba(41,37,36,0.9);border:1px solid rgba(251,191,36,0.3);border-radius:12px;max-width:400px}
.card-exp h2{color:#d1d5db;font-size:1.1em;margin-bottom:8px}
.card-exp p{margin-bottom:10px;color:#9ca3af;font-size:0.95em}
.card-exp a.btn-exp{display:inline-block;padding:10px 16px;background:rgba(251,191,36,0.2);color:#fbbf24;border-radius:8px;text-decoration:none;font-weight:500;margin-bottom:12px}
.card-exp a.btn-exp:hover{background:rgba(251,191,36,0.3)}
.card-exp input[type=file]{margin:8px 0;color:#d1d5db;font-size:0.9em}
.card-exp button.btn-imp{background:#16a34a;color:#fff;border:none;padding:10px 16px;border-radius:8px;font-weight:600;cursor:pointer}
.card-exp button.btn-imp:hover{background:#22c55e}
.card-exp .import-status{margin-top:8px;font-size:0.9em;color:#9ca3af}
.compte-dev{display:inline-flex;align-items:center;gap:6px;margin-top:24px;padding:6px 12px;border-radius:20px;font-size:0.8em;color:#6b7280;background:rgba(55,65,81,0.5);border:1px solid rgba(75,85,99,0.5);text-decoration:none;cursor:pointer;transition:all .2s}
.compte-dev:hover{color:#9ca3af;background:rgba(75,85,99,0.5)}
.modal-overlay{display:none;position:fixed;inset:0;background:rgba(0,0,0,0.7);z-index:100;align-items:center;justify-content:center;padding:20px}
.modal-overlay.show{display:flex}
.modal-box{background:#1c1917;border:1px solid rgba(251,191,36,0.3);border-radius:12px;padding:24px;max-width:320px;width:100%}
.modal-box h3{color:#d1d5db;font-size:1em;margin-bottom:12px}
.modal-box input{width:100%;padding:12px;margin-bottom:12px;border-radius:8px;border:1px solid #4b5563;background:#1f2937;color:#fff;font-size:1em;box-sizing:border-box}
.modal-box button{background:#f59e0b;color:#0c0a09;border:none;padding:10px 20px;border-radius:8px;font-weight:600;cursor:pointer;margin-right:8px}
.modal-box button:hover{background:#fbbf24}
.modal-box .err{color:#ef4444;font-size:0.9em;margin-top:8px}
</style></head><body>)";
  html += "<a href='/enphase-monitor' class='back'>&larr; Retour Enphase Monitor</a>";
  html += "<h1>⚙️ Réglages</h1>";
  html += "<p class='sub'>Retour vers l'accueil Enphase ci-dessus. Liens vers les configurations.</p>";
  html += "<div class='links'>";
  html += "<a href='/wifi?from=enphase'>📶 Réglages WiFi (scan, connexion, infos)</a>";
  html += "<a href='/enphase?from=enphase'>📡 Config Envoy (4 paramètres)</a>";
  html += "<a href='/weather?from=enphase'>🌦️ Réglages Météo</a>";
  html += "<a href='/info?from=enphase'>⚡ Réglages Tempo (activable)</a>";
  html += "</div>";
  html += "<div class='card-exp'><h2>💾 Export / Import paramètres</h2>";
  html += "<p>Uniquement ce module : Config Envoy, verrouillage écran, météo.</p>";
  html += "<p><a href='/exportEnphase' class='btn-exp'>📤 Exporter la config</a></p>";
  html += "<p><span class='label'>Importer :</span></p>";
  html += "<input type='file' id='importEnphaseFile' accept='.json'>";
  html += "<p><button type='button' onclick='importEnphaseConfig()' class='btn-imp'>📥 Importer la config</button></p>";
  html += "<p id='importEnphaseStatus' class='import-status'></p></div>";
  html += R"(<script>
  function importEnphaseConfig(){
    var f=document.getElementById('importEnphaseFile');
    var st=document.getElementById('importEnphaseStatus');
    if(!f.files.length){ st.textContent='Choisissez un fichier JSON.'; st.style.color='#ef4444'; return; }
    var r=new FileReader();
    r.onload=function(){
      st.textContent='Import en cours...'; st.style.color='#9ca3af';
      fetch('/import',{method:'POST',headers:{'Content-Type':'application/json'},body:r.result})
        .then(function(res){ return res.json(); })
        .then(function(data){
          if(data.ok){ st.textContent='Config importée. Redémarrage recommandé pour Envoy.'; st.style.color='#22c55e'; setTimeout(function(){ location.reload(); },1500); }
          else{ st.textContent='Erreur: '+(data.err||'inconnue'); st.style.color='#ef4444'; }
        })
        .catch(function(e){ st.textContent='Erreur: '+e; st.style.color='#ef4444'; });
    };
    r.readAsText(f.files[0]);
  }
  </script>)";
  if (screenLockPassword.length() > 0) {
    html += "<span class='compte-dev' onclick='showDevModal()' title='Accès accueil complet'>🔧 Compte DEV</span>";
    html += "<div id='devModal' class='modal-overlay' onclick='if(event.target===this)closeDevModal()'>";
    html += "<div class='modal-box' onclick='event.stopPropagation()'><h3>🔓 Accès accueil</h3>";
    html += "<p style='color:#9ca3af;font-size:0.85em;margin-bottom:12px'>Mot de passe requis.</p>";
    html += "<form method='POST' action='/unlockScreen' id='devUnlockForm'>";
    html += "<input type='hidden' name='next' value='/'>";
    html += "<input type='password' name='pwd' id='devPwd' placeholder='Mot de passe' autocomplete='off' required>";
    html += "<div><button type='submit'>Valider</button><button type='button' onclick='closeDevModal()'>Annuler</button></div>";
    html += "</form>";
    if (server.hasArg("err") && server.arg("err") == "1") html += "<p class='err' style='margin-top:8px'>Mot de passe incorrect.</p>";
    html += "</div></div>";
    html += R"(<script>
    function showDevModal(){ document.getElementById('devModal').classList.add('show'); document.getElementById('devPwd').value=''; document.getElementById('devPwd').focus(); }
    function closeDevModal(){ document.getElementById('devModal').classList.remove('show'); }
    if(document.location.search.indexOf('err=1')!==-1) setTimeout(showDevModal,50);
    </script>)";
  } else {
    html += "<a href='/' class='compte-dev' title='Accès accueil complet'>🔧 Compte DEV</a>";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// SERVEUR WEB - UPLOAD OTA
void handleUpdate() {
  server.sendHeader("Connection", "close");
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>OTA - MSunPV</title>";
  html += "<style>body{font-family:Arial;background:#0c0a09;color:#fff;padding:20px;margin:0}h1{color:#fbbf24;margin-bottom:20px}";
  html += "form{background:#292524;padding:20px;border-radius:12px;border:1px solid rgba(251,191,36,0.25)}";
  html += "input[type='file']{width:100%;padding:12px;margin-bottom:15px;background:#1c1917;border:1px solid #374151;color:#fff;border-radius:6px;box-sizing:border-box}";
  html += "input[type='submit']{width:100%;padding:14px;background:#fbbf24;color:#000;border:none;border-radius:8px;font-weight:600;font-size:16px;cursor:pointer;min-height:44px}";
  html += "input[type='submit']:hover{background:#f59e0b}";
  html += ".back{display:inline-block;margin-bottom:20px;color:#9ca3af;text-decoration:none}";
  html += "@media (max-width: 480px){body{padding:12px}h1{font-size:1.3em}form{padding:15px}}</style></head><body>";
  html += "<a href='/' class='back'>&larr; Retour</a>";
  html += "<h1>🔄 MSunPV V3.1 - Mise à jour OTA</h1>";
  html += "<form method='POST' action='/doUpdate' enctype='multipart/form-data'>";
  html += "<input type='file' name='update' accept='.bin' required>";
  html += "<input type='submit' value='📤 Téléverser et Installer'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleDoUpdate() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/plain", (Update.hasError()) ? "ÉCHEC" : "SUCCÈS - Redémarrage...");
  delay(1000);
  ESP.restart();
}

// Handler Alarme - Changer l'état
void handleAlarmSet() {
  extern String config_topic_alarm_command;
  extern bool alarmState;
  
  if (server.hasArg("state")) {
    String state = server.arg("state");
    addLogf("[Alarme] Requête reçue: state='%s'", state.c_str());
    
    bool newState = (state.equalsIgnoreCase("ON") || state.equals("1"));
    addLogf("[Alarme] État calculé: %s (ON=1, OFF=0)", newState ? "ACTIVER" : "DÉSACTIVER");
    
    // Publier sur MQTT (topic de commande)
    if (mqttClient.connected()) {
      String payload = newState ? "1" : "0";
      bool published = mqttClient.publish(config_topic_alarm_command.c_str(), payload.c_str());
      addLogf("[Alarme] Commande envoyée: %s → Topic: %s, Payload: %s, Published: %s", 
              newState ? "ACTIVER" : "DÉSACTIVER", 
              config_topic_alarm_command.c_str(), 
              payload.c_str(),
              published ? "OK" : "ÉCHEC");
      
      server.send(200, "application/json", 
        "{\"success\":true,\"state\":\"" + String(newState ? "ON" : "OFF") + "\"}");
    } else {
      addLog("[Alarme] ERREUR: MQTT non connecté");
      server.send(500, "application/json", "{\"success\":false,\"error\":\"MQTT not connected\"}");
    }
  } else {
    addLog("[Alarme] ERREUR: Paramètre 'state' manquant");
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameter\"}");
  }
}

// MODE NUIT - Gestion luminosité écran
void applyNightMode() {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  
  // Mode nuit : 22h → 6h
  bool isNight = (hour >= NIGHT_START_HOUR || hour < NIGHT_END_HOUR);
  
  int brightness = isNight ? (int)brightnessNight : (int)brightnessDay;
  analogWrite(GFX_BL, brightness);
}

// SAUVEGARDE HISTORIQUE - Maintenant dans module_stats.cpp (stats_update)
// PAGES WEB - handleStatsWeb() est maintenant dans module_stats.cpp (stats_handleWeb)

void handleInfoWeb() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Info - MSunPV V3.1</title>";
  html += "<style>body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;margin:0}";
  html += ".btn{background:#374151;color:#fff;padding:10px 20px;border-radius:8px;text-decoration:none;display:inline-block;margin-bottom:20px;min-height:44px}";
  html += ".btn:hover{background:#4b5563}";
  html += "h1{color:#22c55e;margin-bottom:20px}";
  html += ".card{background:#292524;padding:20px;margin:15px 0;border-radius:12px;border:1px solid rgba(251,191,36,0.25)}";
  html += ".card h2{color:#fbbf24;margin:0 0 15px 0;font-size:18px}";
  html += ".card p{margin:8px 0;color:#d1d5db}";
  html += ".label{color:#9ca3af;display:inline-block;width:120px}";
  html += ".value{color:#fff;font-weight:600}";
  html += "@media (max-width: 768px){body{padding:12px}h1{font-size:1.5em}.card{padding:15px;margin:12px 0}.card h2{font-size:16px}.btn{width:100%;padding:12px 16px}}";
  html += "@media (max-width: 480px){body{padding:10px}h1{font-size:1.3em}.label{display:block;width:100%;margin-bottom:3px}.card button{width:100%;padding:14px}}</style></head><body>";
  String backUrl = (server.hasArg("from") && server.arg("from") == "enphase") ? "/enphase-monitor" : "/";
  String backLabel = (server.hasArg("from") && server.arg("from") == "enphase") ? "Retour Enphase Monitor" : "Retour";
  html += "<a href='" + backUrl + "' class='btn'>&larr; " + backLabel + "</a>";
  html += "<h1>ℹ️ Informations Système</h1>";
  
  html += "<div class='card'><h2>📶 WiFi</h2>";
  html += "<p><span class='label'>État:</span> <span class='value' style='color:#22c55e'>" + String(wifiConnected?"Connecté":"Déconnecté") + "</span></p>";
  html += "<p><span class='label'>SSID:</span> <span class='value'>" + String(WIFI_SSID) + "</span></p>";
  html += "<p><span class='label'>IP:</span> <span class='value'>" + ipAddress + "</span></p>";
  html += "<p><span class='label'>Signal:</span> <span class='value'>" + String(rssi) + " dBm</span></p></div>";
  
  html += "<div class='card'><h2>📡 MQTT</h2>";
  html += "<p><span class='label'>État:</span> <span class='value' style='color:#22c55e'>" + String(mqttConnected?"Connecté":"Déconnecté") + "</span></p>";
  html += "<p><span class='label'>Broker:</span> <span class='value'>" + String(MQTT_SERVER) + ":" + String(MQTT_PORT) + "</span></p>";
  html += "<p><span class='label'>Buffer:</span> <span class='value'>2048 bytes (Zigbee2MQTT)</span></p></div>";
  
  // M'SunPV retiré (Enphase V2)
  
  // Shelly EM retiré (Enphase V2)
  
  // Carte Enphase Envoy (V12.0)
  html += "<div class='card'><h2>📡 Enphase Envoy</h2>";
  html += "<p style='margin-bottom:15px'><span class='label'>IP:</span> <span class='value'>" + (config_enphase_ip.length() > 0 ? config_enphase_ip : "Non configuré") + "</span></p>";
  html += "<p style='margin-bottom:15px'><span class='label'>User:</span> <span class='value'>" + (config_enphase_user.length() > 0 ? config_enphase_user : "Non configuré") + "</span></p>";
  html += "<p style='margin-bottom:15px'><span class='label'>Serial:</span> <span class='value'>" + (config_enphase_serial.length() > 0 ? config_enphase_serial : "Non configuré") + "</span></p>";
  html += "<p style='margin-top:10px'><button onclick='configEnphase()' class='btn' style='display:inline-block;cursor:pointer;border:none'>⚙️ Configurer</button></p></div>";
  
  html += "<div class='card'><h2>⛅ Météo</h2>";
  html += "<p><span class='label'>Ville:</span> <span class='value'>" + weather_getCity() + "</span></p>";
  html += "<p><span class='label'>Dernière MAJ:</span> <span class='value'>" + weather_last_update + "</span></p>";
  html += "<p><span class='label'>Condition:</span> <span class='value'>" + weather_condition + "</span></p>";
  html += "<p style='margin-top:10px'><a href='/weather' class='btn' style='display:inline-block'>⚙️ Configurer</a></p></div>";
  
  // EDF TEMPO (option activable, affichage vert pour Demain après minuit)
  {
    String tc = tempo_today_color.length() > 0 ? tempo_today_color : "--";
    String tm = tempo_tomorrow_pending ? String("En attente") : (tempo_tomorrow_color.length() > 0 ? tempo_tomorrow_color : "--");
    String colorToday = "#6b7280";
    if (tempo_today_color == "Bleu") colorToday = "#3b82f6";
    else if (tempo_today_color == "Blanc") colorToday = "#94a3b8";
    else if (tempo_today_color == "Rouge") colorToday = "#ef4444";
    String colorTomorrow = tempo_tomorrow_pending ? "#22c55e" : "#6b7280";
    if (!tempo_tomorrow_pending && tempo_tomorrow_color == "Bleu") colorTomorrow = "#3b82f6";
    else if (!tempo_tomorrow_pending && tempo_tomorrow_color == "Blanc") colorTomorrow = "#94a3b8";
    else if (!tempo_tomorrow_pending && tempo_tomorrow_color == "Rouge") colorTomorrow = "#ef4444";
    html += "<div class='card'><h2>⚡ EDF TEMPO</h2>";
    html += "<p><span class='label'>Activer:</span> <button type='button' onclick='saveTempoEnabled(" + String(tempo_enabled ? "0" : "1") + ")' class='btn' style='display:inline-block;cursor:pointer;border:none;background:" + String(tempo_enabled ? "#ef4444" : "#22c55e") + ";padding:8px 14px'>" + String(tempo_enabled ? "Désactiver" : "Activer") + "</button></p>";
    html += "<p><span class='label'>Aujourd'hui:</span> <span class='value' style='color:" + colorToday + ";font-weight:700'>" + tc + "</span></p>";
    html += "<p><span class='label'>Demain:</span> <span class='value' style='color:" + colorTomorrow + ";font-weight:700;background:" + String(tempo_tomorrow_pending ? "#22c55e" : "transparent") + ";padding:2px 8px;border-radius:4px'>" + tm + "</span></p>";
    if (tempo_last_fetch_time > 0) {
      struct tm *t = localtime(&tempo_last_fetch_time);
      if (t) {
        char buf[32];
        sprintf(buf, "%02d/%02d %02d:%02d", t->tm_mday, t->tm_mon + 1, t->tm_hour, t->tm_min);
        html += "<p><span class='label'>Dernière récup.:</span> <span class='value'>" + String(buf) + "</span></p>";
      }
    } else {
      html += "<p><span class='label'>Dernière récup.:</span> <span class='value'>—</span></p>";
    }
    html += "<p style='color:#6b7280;font-size:0.9em'>À minuit, Demain passe en vert jusqu'à réception du nouveau jour. Rafraîchissement toutes les 30 min.</p></div>";
  }
  
  // V12.1 - Rotation écran + V15.0 Sélection écran
  html += "<div class='card'><h2>🖥️ Écran</h2>";
  html += "<p><span class='label'>Écran actif:</span> <span class='value' style='color:#f59e0b'>Enphase (date orange)</span></p>";
  html += "<p style='margin-top:10px'><a href='/screens' class='btn' style='display:inline-block;text-decoration:none;cursor:pointer;border:none'>🖥️ Choisir l'écran</a></p>";
  html += "<p><span class='label'>Rotation:</span> <span class='value' style='color:" + String(screenFlipped ? "#22c55e" : "#9ca3af") + "'>" + String(screenFlipped ? "180° (Retourné)" : "0° (Normal)") + "</span></p>";
  html += "<p style='margin-top:15px'><button onclick='toggleScreenFlip()' class='btn' style='display:inline-block;cursor:pointer;border:none;background:" + String(screenFlipped ? "#ef4444" : "#22c55e") + "'>" + String(screenFlipped ? "↩️ Remettre Normal" : "🔄 Retourner 180°") + "</button></p>";
  html += "<p style='color:#6b7280;margin-top:10px;font-size:0.9em'>L'écran sera retourné immédiatement après sauvegarde</p>";
  // Verrouillage Enphase : MDP requis pour quitter le mode Enphase
  html += "<h3 style='margin-top:20px;margin-bottom:10px;color:#d1d5db;font-size:1em'>🔒 Verrouillage Enphase</h3>";
  html += "<p style='color:#9ca3af;font-size:0.9em;margin-bottom:10px'>Quand l'écran est sur Enphase, exiger un mot de passe pour changer d'écran (web et écran LVGL).</p>";
  html += "<form method='POST' action='/saveScreenLock' style='margin-top:12px' id='formScreenLock' onsubmit='return validateScreenLockPwd()'>";
  html += "<label style='display:flex;align-items:center;gap:10px;margin-bottom:12px;cursor:pointer'>";
  html += "<input type='checkbox' name='lock_enabled' value='1'" + String(screenLockEnabled ? " checked" : "") + ">";
  html += "<span>Activer le verrouillage (mot de passe requis pour quitter le mode Enphase)</span></label>";
  html += "<p><span class='label'>Mot de passe pour déverrouiller:</span></p>";
  html += "<input type='password' name='pwd' id='screenLockPwd' placeholder='Mot de passe (vide = ne pas changer)' style='width:100%;max-width:280px;padding:10px;margin:8px 0;border-radius:8px;border:1px solid #4b5563;background:#1f2937;color:#fff'>";
  html += "<p><span class='label'>Confirmer le mot de passe:</span></p>";
  html += "<input type='password' name='pwd2' id='screenLockPwd2' placeholder='Confirmer le mot de passe' style='width:100%;max-width:280px;padding:10px;margin:8px 0;border-radius:8px;border:1px solid #4b5563;background:#1f2937;color:#fff'>";
  if (server.hasArg("err") && server.arg("err") == "lock_mismatch") html += "<p style='color:#ef4444;font-size:0.9em;margin-top:4px'>Les deux mots de passe ne correspondent pas.</p>";
  html += "<p style='margin-top:12px'><button type='submit' class='btn' style='display:inline-block;cursor:pointer;border:none;background:#f59e0b;color:#0c0a09'>Enregistrer</button></p>";
  html += "</form></div>";
  
  // Luminosité écran (jour / nuit, 0–255)
  html += "<div class='card'><h2>💡 Luminosité écran</h2>";
  html += "<p><span class='label'>Jour (6h–22h):</span> <span id='valDay' class='value'>" + String((int)brightnessDay) + "</span></p>";
  html += "<input type='range' id='brightnessDay' min='0' max='255' value='" + String((int)brightnessDay) + "' style='width:100%;margin:8px 0' oninput=\"document.getElementById('valDay').textContent=this.value\">";
  html += "<p><span class='label'>Nuit (22h–6h):</span> <span id='valNight' class='value'>" + String((int)brightnessNight) + "</span></p>";
  html += "<input type='range' id='brightnessNight' min='0' max='255' value='" + String((int)brightnessNight) + "' style='width:100%;margin:8px 0' oninput=\"document.getElementById('valNight').textContent=this.value\">";
  html += "<p style='margin-top:15px'><button type='button' onclick='saveBrightness()' class='btn' style='display:inline-block;cursor:pointer;border:none;background:#22c55e'>💾 Sauvegarder la luminosité</button></p>";
  html += "<p style='color:#6b7280;font-size:0.9em'>Appliqué immédiatement après sauvegarde. Mode nuit automatique 22h–6h.</p></div>";
  
  // V14.0 - Format de date
  html += "<div class='card'><h2>📅 Format Date</h2>";
  html += "<p><span class='label'>Format actuel:</span></p>";
  html += "<select id='dateFormat' style='width:100%;padding:12px;background:#1c1917;border:1px solid #374151;color:#fff;border-radius:6px;font-size:16px;margin:10px 0' onchange='saveDateFormat(this.value)'>";
  html += "<option value='0'" + String(dateFormatIndex == 0 ? " selected" : "") + ">Dimanche 28/12/25</option>";
  html += "<option value='1'" + String(dateFormatIndex == 1 ? " selected" : "") + ">Dim. 28 Déc. 2025</option>";
  html += "<option value='2'" + String(dateFormatIndex == 2 ? " selected" : "") + ">28/12/2025</option>";
  html += "<option value='3'" + String(dateFormatIndex == 3 ? " selected" : "") + ">Dim. 28/12/2025</option>";
  html += "</select>";
  html += "<p style='color:#6b7280;font-size:0.9em'>Le format s'applique immédiatement à l'écran</p></div>";
  
  unsigned long days = uptimeSeconds / 86400;
  unsigned long hours = (uptimeSeconds % 86400) / 3600;
  unsigned long mins = (uptimeSeconds % 3600) / 60;
  
  html += "<div class='card'><h2>💾 Sauvegarde / Restauration</h2>";
  html += "<p style='margin-bottom:12px;color:#d1d5db'>Exporter toute la config (WiFi, MQTT, Enphase, Météo, écran, date) pour la réimporter après un reset.</p>";
  html += "<p style='margin-bottom:10px'><a href='/export' class='btn' style='display:inline-block;text-decoration:none;cursor:pointer;border:none'>📤 Exporter la config</a></p>";
  html += "<p style='margin-top:15px'><span class='label'>Importer:</span></p>";
  html += "<input type='file' id='importFile' accept='.json' style='margin:8px 0;color:#d1d5db'>";
  html += "<p><button type='button' onclick='importConfig()' class='btn' style='display:inline-block;cursor:pointer;border:none;background:#16a34a'>📥 Importer la config</button></p>";
  html += "<p id='importStatus' style='margin-top:8px;font-size:0.9em;color:#9ca3af'></p></div>";
  
  html += "<div class='card'><h2>🖥️ Système</h2>";
  html += "<p><span class='label'>Version:</span> <span class='value'>MSunPV V3.1</span></p>";
  html += "<p><span class='label'>Uptime:</span> <span class='value'>" + String(days) + "j " + String(hours) + "h " + String(mins) + "m</span></p>";
  html += "<p><span class='label'>RAM libre:</span> <span class='value'>" + String(ESP.getFreeHeap()/1024) + " KB</span></p>";
  html += "<p><span class='label'>PSRAM libre:</span> <span class='value'>" + String(ESP.getFreePsram()/1024) + " KB</span></p>";
  html += "<p style='margin-top:15px'><button onclick='restartDevice()' class='btn' style='display:inline-block;cursor:pointer;border:none;background:#ef4444;color:white'>🔴 Redémarrer</button></p></div>";
  
  html += R"(<script>
function validateScreenLockPwd() {
  var p1 = document.getElementById('screenLockPwd').value;
  var p2 = document.getElementById('screenLockPwd2').value;
  if (p1 !== p2) { alert('Les deux mots de passe ne correspondent pas.'); return false; }
  if (p1.length > 0 && p2.length === 0) { alert('Veuillez confirmer le mot de passe.'); return false; }
  return true;
}
function importConfig() {
  var f = document.getElementById('importFile');
  var st = document.getElementById('importStatus');
  if (!f.files.length) { st.textContent = 'Choisissez un fichier JSON.'; st.style.color = '#ef4444'; return; }
  var r = new FileReader();
  r.onload = function() {
    st.textContent = 'Import en cours...';
    st.style.color = '#9ca3af';
    fetch('/import', {
      method: 'POST',
      headers: {'Content-Type': 'application/json'},
      body: r.result
    })
    .then(res => res.json())
    .then(data => {
      if (data.ok) { st.textContent = 'Config importée. Redémarrage recommandé.'; st.style.color = '#22c55e'; setTimeout(function(){ location.reload(); }, 1500); }
      else { st.textContent = 'Erreur: ' + (data.err || 'inconnue'); st.style.color = '#ef4444'; }
    })
    .catch(function(e){ st.textContent = 'Erreur: ' + e; st.style.color = '#ef4444'; });
  };
  r.readAsText(f.files[0]);
}
</script>
<script>
function configEnphase() {
  let ip = prompt('IP Enphase Envoy:', ')" + config_enphase_ip + R"(');
  if (ip === null) return;
  
  let user = prompt('Identifiant Enphase:', ')" + config_enphase_user + R"(');
  if (user === null) return;
  
  let pwd = prompt('Mot de passe Enphase:', ')" + config_enphase_pwd + R"(');
  if (pwd === null) return;
  
  let serial = prompt('Numéro de série Envoy:', ')" + config_enphase_serial + R"(');
  if (serial === null) return;
  
  fetch('/saveEnphaseConfig', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'ip=' + encodeURIComponent(ip) + 
          '&user=' + encodeURIComponent(user) + 
          '&pwd=' + encodeURIComponent(pwd) + 
          '&serial=' + encodeURIComponent(serial)
  })
  .then(() => {
    alert('Configuration Enphase sauvegardée !');
    location.reload();
  });
}

function toggleScreenFlip() {
  alert('Fonction appelée !'); // Test
  let currentState = )" + String(screenFlipped ? 1 : 0) + R"(;
  let newValue = (currentState === 1) ? 'false' : 'true';
  console.log('Toggle: current=' + currentState + ', new=' + newValue);
  fetch('/saveScreenFlip', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'flipped=' + newValue
  })
  .then(response => response.json())
  .then(data => {
    console.log('Response:', data);
    if (data.success) {
      alert('Rotation écran sauvegardée !');
      location.reload();
    } else {
      alert('Erreur: ' + (data.error || 'Unknown'));
    }
  })
  .catch(error => {
    console.error('Erreur:', error);
    alert('Erreur de connexion');
  });
}

function restartDevice() {
  if (confirm('Êtes-vous sûr ? L\'appareil va redémarrer et sera indisponible quelques secondes.')) {
    fetch('/restart', {
      method: 'POST'
    })
    .then(() => {
      alert('Redémarrage en cours... L\'appareil s\'éteindra dans quelques secondes.');
      // Optionnel : Masquer les boutons pour éviter les clics multiples
      document.querySelectorAll('button').forEach(btn => btn.disabled = true);
    })
    .catch(error => {
      alert('Erreur lors du redémarrage: ' + error);
    });
  }
}

function saveDateFormat(value) {
  fetch('/saveDateFormat', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'format=' + value
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      console.log('Format de date sauvegardé:', data.format);
    } else {
      alert('Erreur: ' + (data.error || 'Unknown'));
    }
  })
  .catch(error => {
    console.error('Erreur:', error);
    alert('Erreur de connexion');
  });
}

function saveBrightness() {
  var day = document.getElementById('brightnessDay').value;
  var night = document.getElementById('brightnessNight').value;
  fetch('/saveBrightness', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'day=' + day + '&night=' + night
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      alert('Luminosité sauvegardée.');
    } else {
      alert('Erreur: ' + (data.error || 'Unknown'));
    }
  })
  .catch(error => {
    console.error('Erreur:', error);
    alert('Erreur de connexion');
  });
}

function saveTempoEnabled(value) {
  fetch('/saveTempoConfig', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'enabled=' + value
  })
  .then(response => response.json())
  .then(data => {
    if (data.success) {
      location.reload();
    } else {
      alert('Erreur: ' + (data.error || 'Unknown'));
    }
  })
  .catch(error => {
    console.error('Erreur:', error);
    alert('Erreur de connexion');
  });
}

</script>)";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// Export config (téléchargement JSON)
void handleExportConfig() {
  JsonDocument doc;
  doc["version"] = 1;
  doc["exported_at"] = (unsigned long)time(nullptr);
  doc[PREF_WIFI_SSID] = config_wifi_ssid;
  doc[PREF_WIFI_PASS] = config_wifi_password;
  doc[PREF_MQTT_IP] = config_mqtt_ip;
  doc[PREF_MQTT_PORT] = config_mqtt_port;
  doc[PREF_TOPIC_PROD] = config_topic_prod;
  doc[PREF_TOPIC_CABANE] = config_topic_cabane;
  doc[PREF_TOPIC_CONSO] = config_topic_conso;
  doc[PREF_TOPIC_ROUTER] = config_topic_router;
  doc[PREF_TOPIC_WATER] = config_topic_water;
  doc[PREF_TOPIC_EXT] = config_topic_ext;
  doc[PREF_TOPIC_SALON] = config_topic_salon;
  doc[PREF_TOPIC_JOUR] = config_topic_jour;
  doc[PREF_TOPIC_PRESENCE_BEN] = config_topic_presence_ben;
  doc[PREF_TOPIC_PRESENCE_FRANCINE] = config_topic_presence_francine;
  doc[PREF_VICTOR_ENABLED] = config_victor_enabled ? "1" : "0";
  doc[PREF_TOPIC_PRESENCE_VICTOR] = config_topic_presence_victor;
  doc[PREF_TOPIC_ALARM] = config_topic_alarm;
  doc[PREF_TOPIC_ALARM_COMMAND] = config_topic_alarm_command;
  doc[PREF_JSON_KEY_CABANE] = config_json_key_cabane;
  doc[PREF_JSON_KEY_WATER1] = config_json_key_water1;
  doc[PREF_JSON_KEY_WATER2] = config_json_key_water2;
  // PREF_MSUNPV_IP retiré (Enphase V2)
  doc[PREF_WEATHER_API] = weather_getApiKey();
  doc[PREF_WEATHER_CITY] = weather_getCity();
  // PREF_SHELLY* retirés (Enphase V2)
  doc[PREF_ENPHASE_IP] = config_enphase_ip;
  doc[PREF_ENPHASE_USER] = config_enphase_user;
  doc[PREF_ENPHASE_PWD] = config_enphase_pwd;
  doc[PREF_ENPHASE_SERIAL] = config_enphase_serial;
  doc[PREF_SCREEN_FLIPPED] = screenFlipped;
  doc[PREF_ACTIVE_SCREEN] = activeScreenType;
  doc[PREF_SCREEN_LOCK_ENABLED] = screenLockEnabled;
  doc[PREF_SCREEN_LOCK_PWD] = screenLockPassword;
  doc[PREF_DATE_FORMAT] = dateFormatIndex;
  doc[PREF_BRIGHTNESS_DAY] = (int)brightnessDay;
  doc[PREF_BRIGHTNESS_NIGHT] = (int)brightnessNight;
  doc[PREF_TEMPO_ENABLED] = tempo_enabled;
  String out;
  serializeJson(doc, out);
  server.sendHeader("Content-Disposition", "attachment; filename=\"msunpv_config.json\"");
  server.send(200, "application/json", out);
}

// Export config Enphase uniquement (paramètres du module : Envoy, verrouillage, météo)
void handleExportEnphaseConfig() {
  JsonDocument doc;
  doc["version"] = 1;
  doc["exported_at"] = (unsigned long)time(nullptr);
  doc[PREF_ENPHASE_IP] = config_enphase_ip;
  doc[PREF_ENPHASE_USER] = config_enphase_user;
  doc[PREF_ENPHASE_PWD] = config_enphase_pwd;
  doc[PREF_ENPHASE_SERIAL] = config_enphase_serial;
  doc[PREF_SCREEN_LOCK_ENABLED] = screenLockEnabled;
  doc[PREF_SCREEN_LOCK_PWD] = screenLockPassword;
  doc[PREF_WEATHER_API] = weather_getApiKey();
  doc[PREF_WEATHER_CITY] = weather_getCity();
  String out;
  serializeJson(doc, out);
  server.sendHeader("Content-Disposition", "attachment; filename=\"msunpv_enphase_config.json\"");
  server.send(200, "application/json", out);
}

// Import config (POST body = JSON)
void handleImportConfig() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"ok\":false,\"err\":\"Method not allowed\"}");
    return;
  }
  String body = server.arg("plain");
  if (body.length() == 0) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"Body vide\"}");
    return;
  }
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    server.send(400, "application/json", "{\"ok\":false,\"err\":\"JSON invalide\"}");
    return;
  }
  preferences.begin(PREF_NAMESPACE, false);
  for (JsonPair p : doc.as<JsonObject>()) {
    const char* k = p.key().c_str();
    if (strcmp(k, "version") == 0 || strcmp(k, "exported_at") == 0) continue;
    JsonVariant v = p.value();
    if (strcmp(k, PREF_MQTT_PORT) == 0 || strcmp(k, PREF_DATE_FORMAT) == 0) {
      if (v.is<int>()) preferences.putInt(k, v.as<int>());
      else if (v.is<long>()) preferences.putInt(k, (int)v.as<long>());
    } else if (strcmp(k, PREF_ACTIVE_SCREEN) == 0) {
      if (v.is<int>()) { uint8_t n = (uint8_t)v.as<int>(); if (n <= 2) preferences.putUChar(k, n); }
      else if (v.is<long>()) { uint8_t n = (uint8_t)v.as<long>(); if (n <= 2) preferences.putUChar(k, n); }
    } else if (strcmp(k, PREF_SCREEN_FLIPPED) == 0) {
      if (v.is<bool>()) preferences.putBool(k, v.as<bool>());
    } else if (strcmp(k, PREF_SCREEN_LOCK_ENABLED) == 0) {
      if (v.is<bool>()) preferences.putBool(k, v.as<bool>());
      else if (v.is<int>() || v.is<long>()) preferences.putBool(k, (v.as<long>() != 0));
    } else if (strcmp(k, PREF_SCREEN_LOCK_PWD) == 0) {
      if (v.is<const char*>()) preferences.putString(k, v.as<const char*>());
      else if (v.is<String>()) preferences.putString(k, v.as<String>());
    } else if (strcmp(k, PREF_BRIGHTNESS_DAY) == 0 || strcmp(k, PREF_BRIGHTNESS_NIGHT) == 0) {
      if (v.is<int>()) { int n = v.as<int>(); if (n >= 0 && n <= 255) preferences.putUChar(k, (uint8_t)n); }
      else if (v.is<long>()) { long n = v.as<long>(); if (n >= 0 && n <= 255) preferences.putUChar(k, (uint8_t)n); }
    } else if (strcmp(k, PREF_TEMPO_ENABLED) == 0) {
      if (v.is<bool>()) { tempo_setEnabled(v.as<bool>()); preferences.putBool(k, v.as<bool>()); }
      else if (v.is<int>()) { bool b = (v.as<int>() != 0); tempo_setEnabled(b); preferences.putBool(k, b); }
      else if (v.is<long>()) { bool b = (v.as<long>() != 0); tempo_setEnabled(b); preferences.putBool(k, b); }
    } else {
      if (v.is<const char*>() || v.is<String>()) preferences.putString(k, v.as<const char*>());
    }
  }
  preferences.end();
  loadPreferences();
  server.send(200, "application/json", "{\"ok\":true}");
}

// V12.1 - Rotation écran
void handleSaveScreenFlip() {
  if (server.hasArg("flipped")) {
    screenFlipped = (server.arg("flipped") == "true");
    savePreferences();
    
    // Effacer l'écran pour éviter le double affichage
    gfx->fillScreen(BLACK);
    
    // Forcer LVGL à redessiner tout l'écran
    if (screenMain) {
      lv_obj_invalidate(screenMain);
    }
    if (screenStats) {
      lv_obj_invalidate(screenStats);
    }
    if (screenInfo) {
      lv_obj_invalidate(screenInfo);
    }
    if (screenSettings) {
      lv_obj_invalidate(screenSettings);
    }
    
    // Forcer un refresh immédiat
    lv_refr_now(disp);
    
    server.send(200, "application/json", "{\"success\":true,\"flipped\":" + String(screenFlipped ? "true" : "false") + "}");
    Serial.println("[V12.1] Rotation écran: " + String(screenFlipped ? "180°" : "0°"));
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameter\"}");
  }
}

// V15.0 - Page sélection écran (MQTT / Enphase)
// Retourne true si le client est déverrouillé (cookie valide)
static bool isUnlockCookieValid() {
  if (unlockToken.length() == 0 || millis() >= unlockExpiry) return false;
  String cookie;
  if (server.hasHeader("Cookie")) cookie = server.header("Cookie");
  else if (server.hasHeader("cookie")) cookie = server.header("cookie");
  else return false;
  int i = cookie.indexOf("unlock_token=");
  if (i < 0) return false;
  i += 13;
  int j = cookie.indexOf(";", i);
  String token = (j > i) ? cookie.substring(i, j) : cookie.substring(i);
  token.trim();
  return (token == unlockToken);
}

void handleScreensWeb() {
  if (wifiAPMode) {
    handleWiFiSetupPage();
    return;
  }
  // Verrouillé = mode Enphase + verrouillage activé + MDP défini
  bool locked = (activeScreenType == 1 && screenLockEnabled && screenLockPassword.length() > 0);
  bool unlocked = isUnlockCookieValid();

  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Écran - MSunPV</title><style>
    *{margin:0;padding:0;box-sizing:border-box}
    body{background:linear-gradient(135deg,#0c0a09 0%,#1c1917 100%);color:#fff;font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Arial,sans-serif;min-height:100vh;padding:24px}
    .back{display:inline-block;background:#374151;color:#fff;padding:10px 20px;border-radius:8px;text-decoration:none;margin-bottom:24px;transition:background .2s}
    .back:hover{background:#4b5563}
    h1{color:#fbbf24;margin-bottom:8px;font-size:1.8em}
    .sub{color:#9ca3af;font-size:0.95em;margin-bottom:28px}
    .grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px;max-width:500px}
    .card{background:rgba(41,37,36,0.9);border:2px solid rgba(251,191,36,0.25);border-radius:16px;padding:28px;cursor:pointer;transition:all .25s;text-align:center}
    .card:hover{border-color:rgba(251,191,36,0.5);transform:translateY(-2px);box-shadow:0 8px 24px rgba(0,0,0,0.3)}
    .card.selected{border-color:#f59e0b;background:rgba(245,158,11,0.1);box-shadow:0 0 0 2px rgba(245,158,11,0.3)}
    .card .icon{font-size:2.5em;margin-bottom:12px}
    .card .title{font-size:1.15em;font-weight:600;color:#fff;margin-bottom:6px}
    .card .desc{font-size:0.85em;color:#9ca3af;line-height:1.4}
    .unlock-box{background:rgba(41,37,36,0.95);border:2px solid rgba(251,191,36,0.4);border-radius:16px;padding:28px;max-width:400px;margin-top:20px}
    .unlock-box input[type=password]{width:100%;padding:12px;margin:12px 0;border-radius:8px;border:1px solid #4b5563;background:#1f2937;color:#fff;font-size:1em}
    .unlock-box button{background:#f59e0b;color:#0c0a09;border:none;padding:12px 24px;border-radius:8px;font-weight:600;cursor:pointer}
    .unlock-box button:hover{background:#fbbf24}
    .unlock-err{color:#ef4444;font-size:0.9em;margin-top:8px}
    @media(max-width:480px){.grid{grid-template-columns:1fr}}
  </style></head><body>)";
  html += "<a href='/info' class='back'>&larr; Retour</a>";

  if (locked && !unlocked) {
    // Afficher formulaire de déverrouillage
    html += "<h1>🔒 Écran verrouillé</h1>";
    html += "<p class='sub'>L'écran est en mode Enphase et verrouillé. Entrez le mot de passe pour changer d'écran.</p>";
    html += "<div class='unlock-box'>";
    html += "<form method='POST' action='/unlockScreen'>";
    html += "<label for='pwd'>Mot de passe</label><br>";
    html += "<input type='password' id='pwd' name='pwd' placeholder='Mot de passe' required autofocus>";
    html += "<button type='submit'>Déverrouiller</button>";
    html += "</form>";
    if (server.hasArg("err")) html += "<p class='unlock-err'>Mot de passe incorrect.</p>";
    html += "</div>";
  } else {
    // Afficher sélection d'écran
    html += "<h1>🖥️ Sélection de l'écran</h1>";
    html += "<p class='sub'>Choisissez l'écran principal. Enphase V2 : écran Enphase uniquement.</p>";
    html += "<div class='grid'>";

    html += "<div class='card' id='card0' onclick='selectScreen(0)'";
    if (activeScreenType == 0) html += " style='border-color:#f59e0b;background:rgba(245,158,11,0.1)'";
    html += "><div class='icon'>📡</div><div class='title'>Écran MQTT</div><div class='desc'>Données Shelly, production, consommation. Date blanche.</div></div>";

    html += "<div class='card' id='card1' onclick='selectScreen(1)'";
    if (activeScreenType == 1) html += " style='border-color:#f59e0b;background:rgba(245,158,11,0.1)'";
    html += "><div class='icon'>⚡</div><div class='title'>Écran Enphase</div><div class='desc'>Affichage Enphase avec date orange.</div></div>";

    html += "</div>";
    html += "<p style='color:#6b7280;margin-top:20px;font-size:0.9em'>Le changement s'applique immédiatement à l'écran.</p>";
    html += R"(<script>
    function selectScreen(n) {
      fetch('/saveScreens', { method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'screen=' + n })
        .then(r => r.json())
        .then(d => {
          if (d.success) {
            for (var i=0;i<=2;i++) { var c=document.getElementById('card'+i); if(c){ c.style.borderColor=(i==n)?'#f59e0b':''; c.style.background=(i==n)?'rgba(245,158,11,0.1)':''; } }
          } else if (d.locked) {
            window.location.href = '/screens?err=1';
          }
        });
    }
  </script>)";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleUnlockScreen() {
  if (!server.hasArg("pwd")) {
    String redir = server.hasArg("next") ? server.arg("next") : "/screens";
    if (redir != "/" && redir != "/info") redir = "/screens";
    server.sendHeader("Location", redir + "?err=1");
    server.send(302, "text/plain", "");
    return;
  }
  if (server.arg("pwd") != screenLockPassword) {
    server.sendHeader("Location", (server.hasArg("next") && server.arg("next") == "/") ? "/enphase-reglages?err=1" : "/screens?err=1");
    server.send(302, "text/plain", "");
    return;
  }
  unlockToken = String((unsigned long)esp_random(), HEX);
  unlockExpiry = millis() + 300000;  // 5 min
  server.sendHeader("Set-Cookie", "unlock_token=" + unlockToken + "; Path=/; Max-Age=300");
  String redir = server.hasArg("next") ? server.arg("next") : "/screens";
  if (redir != "/" && redir != "/info") redir = "/screens";
  server.sendHeader("Location", redir);
  server.send(302, "text/plain", "");
  Serial.println("[Screen] Déverrouillage OK");
}

void handleSaveScreens() {
  if (!server.hasArg("screen")) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameter\"}");
    return;
  }
  uint8_t n = (uint8_t)server.arg("screen").toInt();
  if (n > 2) {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid value\"}");
    return;
  }
  // Enphase V2 : écran unique Enphase - forcer n=1 (ignorer 0 et 2)
  n = 1;
  activeScreenType = n;
  savePreferences();
  if (currentPage == 0 && screenEnphase) {
    lv_screen_load(screenEnphase);
  }
  if (screenEnphase) lv_obj_invalidate(screenEnphase);
  lv_refr_now(disp);
  server.send(200, "application/json", "{\"success\":true,\"screen\":" + String(n) + "}");
  Serial.println("[V15.0] Écran: " + String(n == 0 ? "MQTT" : (n == 1 ? "Enphase" : "M'SunPV")));
}

void handleSaveScreenLock() {
  screenLockEnabled = (server.hasArg("lock_enabled") && server.arg("lock_enabled") == "1");
  if (server.hasArg("pwd")) {
    String p = server.arg("pwd");
    String p2 = server.hasArg("pwd2") ? server.arg("pwd2") : "";
    if (p.length() > 0) {
      if (p != p2) {
        server.sendHeader("Location", "/info?err=lock_mismatch");
        server.send(302, "text/plain", "");
        return;
      }
      screenLockPassword = p;
    }
  }
  savePreferences();
  server.sendHeader("Location", "/info");
  server.send(302, "text/plain", "");
  Serial.println("[Screen] Verrouillage Enphase: " + String(screenLockEnabled ? "activé" : "désactivé"));
}

// Page Réglages (écran LVGL, roue dentée) - Enphase V2 : toujours écran Enphase
void settings_back_to_main(void) {
  currentPage = 0;
  lv_screen_load(screenEnphase);
}

void settings_do_restart(void) {
  ESP.restart();
}

void settings_do_toggle_flip(void) {
  screenFlipped = !screenFlipped;
  preferences.begin(PREF_NAMESPACE, false);
  preferences.putBool(PREF_SCREEN_FLIPPED, screenFlipped);
  preferences.end();
  gfx->fillScreen(BLACK);
  if (screenMain) lv_obj_invalidate(screenMain);
  if (screenSettings) lv_obj_invalidate(screenSettings);
  lv_refr_now(disp);
}

void settings_do_factory_reset(void) {
  preferences.begin(PREF_NAMESPACE, false);
  preferences.clear();
  preferences.end();
  ESP.restart();
}

void settings_get_log_text(char* buf, int maxLen) {
  buf[0] = '\0';
  int len = 0;
  const int maxLines = 10;
  for (int i = 0; i < maxLines && len < maxLen - 1; i++) {
    int idx = (logIndex + MAX_LOGS - 1 - i + MAX_LOGS) % MAX_LOGS;
    if (systemLogs[idx].length() == 0) continue;
    int n = systemLogs[idx].length();
    if (len + n + 2 > maxLen - 1) n = maxLen - 1 - len - 2;
    if (n > 0) {
      memcpy(buf + len, systemLogs[idx].c_str(), (size_t)n);
      len += n;
    }
    if (len < maxLen - 1) { buf[len++] = '\n'; buf[len] = '\0'; }
  }
}

// ============================================
// CALLBACKS CONFIGURATION WiFi (onglet WiFi)
// ============================================
void wifi_scan_clicked(lv_event_t *e) {
  (void)e;
  if (wifi_lbl_status_msg) {
    lv_label_set_text(wifi_lbl_status_msg, "Scanning...");
    lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0xf59e0b), 0);
  }
  lv_refr_now(NULL);
  
  int n = WiFi.scanNetworks(false, false, 5000);
  
  wifi_networks_count = (n > 30) ? 30 : n;
  wifi_networks_str = "";
  
  for (int i = 0; i < wifi_networks_count; i++) {
    String ssid = WiFi.SSID(i);
    int32_t rssi = (int32_t)WiFi.RSSI(i);
    wifi_rssi_values[i] = rssi;
    
    if (i > 0) wifi_networks_str += "\n";
    char buf[64];
    snprintf(buf, sizeof(buf), "%s [%ld dBm]", ssid.c_str(), (long)rssi);
    wifi_networks_str += buf;
  }
  
  if (wifi_dd_networks) {
    lv_dropdown_clear_options(wifi_dd_networks);
    if (wifi_networks_count > 0) {
      lv_dropdown_set_options(wifi_dd_networks, wifi_networks_str.c_str());
      char msg[80];
      snprintf(msg, sizeof(msg), "Scan OK: %d reseaux", wifi_networks_count);
      if (wifi_lbl_status_msg) {
        lv_label_set_text(wifi_lbl_status_msg, msg);
        lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0x22c55e), 0);
      }
    } else {
      if (wifi_lbl_status_msg) {
        lv_label_set_text(wifi_lbl_status_msg, "Aucun reseau trouve");
        lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0x9ca3af), 0);
      }
    }
  }
}

void wifi_network_selected(lv_event_t *e) {
  (void)e;
  if (!wifi_dd_networks || !wifi_ta_ssid) return;
  
  uint16_t idx = lv_dropdown_get_selected(wifi_dd_networks);
  if (idx < wifi_networks_count) {
    String ssid = WiFi.SSID(idx);
    lv_textarea_set_text(wifi_ta_ssid, ssid.c_str());
  }
}

void wifi_connect_clicked(lv_event_t *e) {
  (void)e;
  if (!wifi_ta_ssid || !wifi_ta_pwd) return;
  
  const char *ssid_text = lv_textarea_get_text(wifi_ta_ssid);
  const char *pwd_text = lv_textarea_get_text(wifi_ta_pwd);
  
  String ssid = String(ssid_text ? ssid_text : "");
  String pwd = String(pwd_text ? pwd_text : "");
  
  if (ssid.length() == 0) {
    if (wifi_lbl_status_msg) {
      lv_label_set_text(wifi_lbl_status_msg, "Erreur: SSID requis");
      lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0xef4444), 0);
    }
    return;
  }
  
  if (wifi_lbl_status_msg) {
    lv_label_set_text(wifi_lbl_status_msg, "Connexion...");
    lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0xf59e0b), 0);
  }
  lv_refr_now(NULL);
  
  addLogf("[WiFi Config] Tentative connexion: %s", ssid.c_str());
  
  WiFi.begin(ssid.c_str(), pwd.length() > 0 ? pwd.c_str() : NULL);
  
  unsigned long timeout = millis() + 10000;
  bool connected = false;
  int attempt = 0;
  
  while (millis() < timeout && attempt < 20) {
    delay(500);
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    attempt++;
  }
  
  if (connected) {
    addLog("[WiFi Config] Connexion OK!");
    config_wifi_ssid = ssid;
    config_wifi_password = pwd;
    savePreferences();
    
    if (wifi_lbl_status_msg) {
      lv_label_set_text(wifi_lbl_status_msg, "OK! Redémarrage...");
      lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0x22c55e), 0);
    }
    lv_refr_now(NULL);
    
    delay(2000);
    ESP.restart();
  } else {
    addLog("[WiFi Config] Connexion echouee");
    if (wifi_lbl_status_msg) {
      lv_label_set_text(wifi_lbl_status_msg, "Erreur: connexion echouee");
      lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0xef4444), 0);
    }
  }
}

void wifi_clear_clicked(lv_event_t *e) {
  (void)e;
  if (wifi_ta_ssid) lv_textarea_set_text(wifi_ta_ssid, "");
  if (wifi_ta_pwd) lv_textarea_set_text(wifi_ta_pwd, "");
  if (wifi_lbl_status_msg) {
    lv_label_set_text(wifi_lbl_status_msg, "Champs effacés");
    lv_obj_set_style_text_color(wifi_lbl_status_msg, lv_color_hex(0x9ca3af), 0);
  }
}

// V14.0 - Format de date
void handleSaveDateFormat() {
  if (server.hasArg("format")) {
    int newFormat = server.arg("format").toInt();
    if (newFormat >= 0 && newFormat <= 3) {
      dateFormatIndex = newFormat;
      savePreferences();
      
      server.send(200, "application/json", "{\"success\":true,\"format\":" + String(dateFormatIndex) + "}");
      Serial.println("[V14.0] Format de date: " + String(dateFormatIndex));
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Invalid format\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing parameter\"}");
  }
}

// Luminosité écran (page Info)
void handleSaveBrightness() {
  if (server.hasArg("day") && server.hasArg("night")) {
    int day = server.arg("day").toInt();
    int night = server.arg("night").toInt();
    if (day >= 0 && day <= 255 && night >= 0 && night <= 255) {
      brightnessDay = (uint8_t)day;
      brightnessNight = (uint8_t)night;
      savePreferences();
      applyNightMode();  // applique immédiatement
      server.send(200, "application/json", "{\"success\":true}");
      Serial.println("[Info] Luminosité: jour=" + String(brightnessDay) + " nuit=" + String(brightnessNight));
    } else {
      server.send(400, "application/json", "{\"success\":false,\"error\":\"Valeur 0-255\"}");
    }
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing day/night\"}");
  }
}

// EDF TEMPO (page Info)
void handleSaveTempoConfig() {
  if (server.hasArg("enabled")) {
    tempo_setEnabled(server.arg("enabled") == "1");
    savePreferences();
    server.send(200, "application/json", "{\"success\":true}");
    Serial.println("[Info] TEMPO: " + String(tempo_enabled ? "activé" : "désactivé"));
  } else {
    server.send(400, "application/json", "{\"success\":false,\"error\":\"Missing enabled\"}");
  }
}

void handleSettingsWeb() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Paramètres - MSunPV V3.1</title>";
  html += "<style>body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;margin:0}";
  html += ".btn{background:#374151;color:#fff;padding:10px 20px;border-radius:8px;text-decoration:none;display:inline-block;margin-bottom:20px;min-height:44px}";
  html += ".btn:hover{background:#4b5563}";
  html += ".btn-primary{background:#fbbf24;color:#0c0a09;border:none;padding:12px 24px;border-radius:8px;font-size:16px;font-weight:600;cursor:pointer;margin-top:10px;min-height:44px}";
  html += ".btn-primary:hover{background:#f59e0b}";
  html += "h1{color:#fb923c;margin-bottom:20px}";
  html += ".card{background:#292524;padding:20px;margin:15px 0;border-radius:12px;border:1px solid rgba(251,191,36,0.25)}";
  html += ".card h2{color:#fbbf24;margin:0 0 15px 0;font-size:18px}";
  html += "@media (max-width: 768px){body{padding:12px}h1{font-size:1.5em}.card{padding:15px;margin:12px 0}.card h2{font-size:16px}.btn,.btn-primary{width:100%;padding:14px}}";
  html += "@media (max-width: 480px){body{padding:10px}h1{font-size:1.3em}.btn-primary{font-size:15px}}</style></head><body>";
  html += "<a href='/' class='btn'>&larr; Retour</a>";
  html += "<h1>⚙️ Paramètres</h1>";
  
  html += "<div class='card'><h2>🔄 Actions système</h2>";
  html += "<button class='btn-primary' onclick=\"if(confirm('Redémarrer l\\'ESP32 ?'))location.href='/restart'\">🔄 Redémarrer ESP32</button>";
  html += "<p style='color:#6b7280;margin-top:15px'>Le redémarrage prend environ 10 secondes</p></div>";
  
  html += "<div class='card'><h2>ℹ️ Information</h2>";
  html += "<p style='color:#d1d5db'>La luminosité de l'écran se règle depuis la page <a href='/info' style='color:#fbbf24'>Info</a>. Les autres paramètres avancés (seuils, IP M'SunPV) peuvent nécessiter config.h</p></div>";
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleRestart() {
  server.send(200, "text/plain", "Redémarrage en cours...");
  delay(1000);
  ESP.restart();
}

// handleSDStatus retiré (Enphase V2 - module SD supprimé)

// PREFERENCES (NVS) - V3.2
void loadPreferences() {
  preferences.begin(PREF_NAMESPACE, false);
  
  // WiFi
  config_wifi_ssid = preferences.getString(PREF_WIFI_SSID, WIFI_SSID);
  config_wifi_password = preferences.getString(PREF_WIFI_PASS, WIFI_PASSWORD);
  
  // MQTT (V11.0 - Module)
  mqtt_loadConfig(&preferences);
  
  // MSunPV retiré (Enphase V2)
  
  // Météo (V11.0 - Module)
  weather_loadConfig(&preferences);
  
  // Shelly EM retiré (Enphase V2)
  
  // Enphase Envoy (V12.0 - V11.0 Module)
  enphase_loadConfig(&preferences);
  
  // EDF TEMPO (option activable)
  tempo_loadConfig(preferences);
  
  // V12.1 - Rotation écran
  screenFlipped = preferences.getBool(PREF_SCREEN_FLIPPED, false);
  
  // V15.0 - Écran actif (MQTT vs Enphase) - Enphase V2 : défaut 1 (écran unique)
  activeScreenType = preferences.getUChar(PREF_ACTIVE_SCREEN, 1);
  
  // Verrouillage écran Enphase (MDP pour quitter le mode Enphase)
  screenLockEnabled = preferences.getBool(PREF_SCREEN_LOCK_ENABLED, false);
  screenLockPassword = preferences.getString(PREF_SCREEN_LOCK_PWD, "");
  
  // V14.0 - Format de date
  dateFormatIndex = preferences.getInt(PREF_DATE_FORMAT, 1);  // Défaut: format abrégé
  
  // Luminosité écran (réglable depuis page Info)
  brightnessDay = preferences.getUChar(PREF_BRIGHTNESS_DAY, BACKLIGHT_DAY);
  brightnessNight = preferences.getUChar(PREF_BRIGHTNESS_NIGHT, BACKLIGHT_NIGHT);
  
  preferences.end();
  
  Serial.println("Config NVS OK");
}

void savePreferences() {
  preferences.begin(PREF_NAMESPACE, false);
  
  // WiFi
  preferences.putString(PREF_WIFI_SSID, config_wifi_ssid);
  preferences.putString(PREF_WIFI_PASS, config_wifi_password);
  
  // MQTT (V11.0 - Module)
  mqtt_saveConfig(&preferences);
  
  // MSunPV retiré (Enphase V2)
  
  // Météo (V11.0 - Module)
  weather_saveConfig(&preferences);
  
  // Shelly EM retiré (Enphase V2)
  
  // Enphase Envoy (V12.0 - V11.0 Module)
  enphase_saveConfig(&preferences);
  
  // EDF TEMPO
  tempo_saveConfig(preferences);
  
  // V12.1 - Rotation écran
  preferences.putBool(PREF_SCREEN_FLIPPED, screenFlipped);
  
  // V15.0 - Écran actif
  preferences.putUChar(PREF_ACTIVE_SCREEN, activeScreenType);
  
  // Verrouillage écran Enphase
  preferences.putBool(PREF_SCREEN_LOCK_ENABLED, screenLockEnabled);
  preferences.putString(PREF_SCREEN_LOCK_PWD, screenLockPassword);
  
  // V14.0 - Format de date
  preferences.putInt(PREF_DATE_FORMAT, dateFormatIndex);
  
  // Luminosité écran
  preferences.putUChar(PREF_BRIGHTNESS_DAY, brightnessDay);
  preferences.putUChar(PREF_BRIGHTNESS_NIGHT, brightnessNight);
  
  preferences.end();
  
  Serial.println("Config sauvegardée");
}

// PAGES WEB CONFIGURATION - V3.2

// Page configuration MQTT - Maintenant dans module_mqtt.cpp (supprimée)
// Sauvegarde configuration MQTT - Maintenant dans module_mqtt.cpp (supprimée)

// Page configuration WiFi
void handleWifiConfig() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Configuration WiFi - MSunPV</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #0c0a09;
      color: #fff;
      margin: 0;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
    }
    h1 {
      color: #fbbf24;
      border-bottom: 2px solid #fbbf24;
      padding-bottom: 10px;
    }
    .form-group {
      margin-bottom: 20px;
      background: #292524;
      padding: 15px;
      border-radius: 8px;
    }
    label {
      display: block;
      color: #9ca3af;
      margin-bottom: 5px;
      font-size: 0.9em;
    }
    input {
      width: 100%;
      padding: 10px;
      background: #1c1917;
      border: 1px solid #374151;
      color: #fff;
      border-radius: 4px;
      font-size: 1em;
      box-sizing: border-box;
    }
    .current-value {
      color: #60a5fa;
      font-size: 0.9em;
      margin-top: 5px;
    }
    .btn {
      background: #fbbf24;
      color: #000;
      border: none;
      padding: 12px 30px;
      font-size: 1.1em;
      font-weight: bold;
      border-radius: 6px;
      cursor: pointer;
      margin-right: 10px;
    }
    .btn:hover {
      background: #f59e0b;
    }
    .btn-secondary {
      background: #374151;
      color: #fff;
    }
    .btn-secondary:hover {
      background: #4b5563;
    }
    .nav {
      margin-top: 20px;
      text-align: center;
    }
    .warning {
      background: #7f1d1d;
      padding: 15px;
      border-radius: 6px;
      margin-bottom: 20px;
      border-left: 4px solid #ef4444;
    }
    @media (max-width: 768px) {
      body { padding: 12px; }
      h1 { font-size: 1.5em; }
      .form-group { padding: 12px; }
      .btn { width: 100%; margin: 5px 0; padding: 14px; min-height: 44px; }
      .warning { padding: 12px; font-size: 0.9em; }
    }
    @media (max-width: 480px) {
      body { padding: 10px; }
      h1 { font-size: 1.3em; }
      input { padding: 12px; font-size: 16px; }
      .btn { font-size: 1em; }
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>📶 Configuration WiFi</h1>
    
    <div class="warning">
      ⚠️ <strong>Attention :</strong> Après enregistrement, l'ESP32 va redémarrer et se connecter au nouveau réseau. Assurez-vous que les identifiants sont corrects !
    </div>
    
    <form method="POST" action="/saveWifi">
      <div class="form-group">
        <label>SSID WiFi</label>
        <input type="text" name="wifi_ssid" value=")";
  html += config_wifi_ssid;
  html += R"(" required>
        <div class="current-value">Actuellement connecté à : )";
  html += config_wifi_ssid;
  html += R"(</div>
      </div>
      
      <div class="form-group">
        <label>Mot de Passe WiFi</label>
        <input type="password" name="wifi_pass" value=")";
  html += config_wifi_password;
  html += R"(" required>
        <small style="color: #9ca3af;">Le mot de passe sera enregistré en clair dans la mémoire NVS</small>
      </div>
      
      <button type="submit" class="btn">💾 Enregistrer et Redémarrer</button>
      <button type="button" class="btn btn-secondary" onclick="location.href=')";
  String wifiBackUrl = (server.hasArg("from") && server.arg("from") == "enphase") ? "/enphase-monitor" : "/";
  String wifiBackLabel = (server.hasArg("from") && server.arg("from") == "enphase") ? "Retour Enphase Monitor" : "Retour au Dashboard";
  html += wifiBackUrl;
  html += R"('">❌ Annuler</button>
    </form>
    
    <div class="nav">
      <a href=")";
  html += wifiBackUrl;
  html += R"(" style="color: #fbbf24; text-decoration: none;">&larr; )";
  html += wifiBackLabel;
  html += R"(</a>
    </div>
  </div>
</body>
</html>
)";
  server.send(200, "text/html", html);
}

// Sauvegarde configuration WiFi
void handleSaveWifiConfig() {
  if (server.method() == HTTP_POST) {
    config_wifi_ssid = server.arg("wifi_ssid");
    config_wifi_password = server.arg("wifi_pass");
    
    savePreferences();
    
    String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Configuration Sauvegardée</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #0c0a09;
      color: #fff;
      text-align: center;
      padding: 50px;
    }
    h1 { color: #22c55e; }
    p { font-size: 1.2em; margin: 20px 0; }
  </style>
  <script>
    setTimeout(function() {
      window.location.href = '/';
    }, 5000);
  </script>
</head>
<body>
  <h1>✅ Configuration WiFi Sauvegardée</h1>
  <p>L'ESP32 va redémarrer et se connecter au réseau : <strong>)";
    html += config_wifi_ssid;
    html += R"(</strong></p>
  <p>Reconnectez-vous au même réseau pour accéder de nouveau au dashboard.</p>
</body>
</html>
)";
    server.send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  }
}

// Handlers MSunPV - Maintenant dans module_msunpv.cpp


