// MODULE MQTT - Implémentation

#include "module_mqtt.h"
#include <WiFi.h>

// Constantes NVS (évite d'inclure config.h qui contient des définitions)
#define PREF_MQTT_IP "mqtt_ip"
#define PREF_MQTT_PORT "mqtt_port"
#define PREF_MQTT_USER "mqtt_user"
#define PREF_MQTT_PASS "mqtt_pass"
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
#define PREF_JSON_KEY_CABANE "json_key_cabane"
#define PREF_JSON_KEY_WATER1 "json_key_water1"
#define PREF_JSON_KEY_WATER2 "json_key_water2"
#define PREF_MSUNPV_IP "msunpv_ip"

// Déclarations externes pour logging
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// M'SunPV retiré (Enphase V2)
#define MQTT_RECONNECT_INTERVAL 5000

// Valeurs par défaut depuis config.h
#define DEFAULT_MQTT_SERVER "192.168.1.82"
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_TOPIC_SOLAR_PROD "shellies/shellyem-A4E57CBA5ACA/emeter/1/power"
#define DEFAULT_TOPIC_SOLAR_CABANE "zigbee2mqtt/Prize Tongou 16A - Cabanne"
#define DEFAULT_TOPIC_HOME_CONSO "shellies/shellyem-A4E57CBA5ACA/emeter/0/power"
#define DEFAULT_TOPIC_ROUTER "shellies/MSUNPV - shellyem-34945473E575/emeter/0/power"
#define DEFAULT_TOPIC_WATER_TEMP "tele/tasmota_3783F2/SENSOR"
#define DEFAULT_TOPIC_TEMP_EXT "PUBLISH82/TempExt"
#define DEFAULT_TOPIC_TEMP_SALON "PUBLISH82/Salon"
#define DEFAULT_TOPIC_CONSO_JOUR "PUBLISH82/ConsoJour"
#define DEFAULT_TOPIC_PRESENCE_BEN "jeedom/presence/ben/state"
#define DEFAULT_TOPIC_PRESENCE_FRANCINE "jeedom/presence/Francine"
#define DEFAULT_TOPIC_PRESENCE_VICTOR "jeedom/presence/victor/state"
#define DEFAULT_TOPIC_ALARM "PUBLISH82/Alarme"
#define DEFAULT_TOPIC_ALARM_COMMAND "msunpv/alarm/command"
#define DEFAULT_MSUNPV_IP "192.168.1.165"

// Variables exposées (définitions)
bool mqttConnected = false;
bool mqttDataReceived = false;

float solarProdMain = 0;
float solarProdCabane = 0;
float solarProd = 0;
float homeConso = 0;
float routerPower = 0;
float waterTemp = 48;
float tempExt = 0;
float tempSalon = 0;
float consoJour = 0;
// presenceBen, presenceFrancine, presenceVictor retirés (Enphase V2)
// alarmState retiré (Enphase V2)

String config_mqtt_ip;
int config_mqtt_port;
String config_mqtt_user;
String config_mqtt_pass;
String config_topic_prod;
String config_topic_cabane;
String config_topic_conso;
String config_topic_router;
String config_topic_water;
String config_topic_ext;
String config_topic_salon;
String config_topic_jour;
// config_topic_presence_*, config_victor_enabled retirés (Enphase V2)
// config_topic_alarm, config_topic_alarm_command retirés (Enphase V2)
String config_json_key_cabane;
String config_json_key_water1;
String config_json_key_water2;
// config_msunpv_ip est maintenant dans module_msunpv.cpp (extern)

// Variables internes (static)
static PubSubClient* mqttClient = nullptr;
static WiFiClient* wifiClient = nullptr;
static unsigned long lastMqttAttempt = 0;

// Fonction privée pour parser les messages MQTT
static void parseMqttMessage(String topic, String message) {
  (void)topic;
  (void)message;
  // Enphase V2 : plus de données MQTT entrantes, uniquement publication vers HA
  if (!mqttDataReceived) {
    mqttDataReceived = true;
    addLog("[MQTT] Connexion établie (mode publication uniquement)");
  }
}

// Callback MQTT (static)
static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  String topicStr = String(topic);
  parseMqttMessage(topicStr, message);
}

// Initialisation
void mqtt_init(PubSubClient* client, WiFiClient* wifi) {
  mqttClient = client;
  wifiClient = wifi;
  
  // MQTT avec buffer 2048 pour Zigbee2MQTT
  mqttClient->setBufferSize(2048);
  mqttClient->setServer(config_mqtt_ip.c_str(), config_mqtt_port);
  mqttClient->setCallback(mqtt_callback);
  
  addLog("[MQTT] Module initialisé");
}

// Reconnexion MQTT
void mqtt_reconnect() {
  // Enphase V2 : connexion optionnelle — si IP vide, ne pas tenter
  if (config_mqtt_ip.length() == 0) {
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    addLog("[MQTT] WiFi non connecté");
    return;
  }
  
  if (!mqttClient->connected()) {
    mqttClient->setServer(config_mqtt_ip.c_str(), config_mqtt_port);
    addLogf("Connexion MQTT... (Broker: %s:%d)", config_mqtt_ip.c_str(), config_mqtt_port);
    
    String clientId = "MSunPV-" + String(random(0xffff), HEX);
    
    bool connected = false;
    if (config_mqtt_user.length() > 0) {
      connected = mqttClient->connect(clientId.c_str(), config_mqtt_user.c_str(), config_mqtt_pass.c_str());
    } else {
      connected = mqttClient->connect(clientId.c_str());
    }
    
    if (connected) {
      addLog("Connexion MQTT... OK!");
      mqttConnected = true;
      mqttDataReceived = true;  // Pas de souscriptions, connexion = prêt
      
      // Enphase V2 : plus de souscriptions entrantes, uniquement publication vers HA
    } else {
      addLogf("Connexion MQTT échec, code=%d", mqttClient->state());
      mqttConnected = false;
    }
  }
}

// Loop MQTT (à appeler dans loop())
void mqtt_loop() {
  if (!mqttClient) return;
  // Enphase V2 : connexion optionnelle — si IP vide, ne pas tenter
  if (config_mqtt_ip.length() == 0) return;
  
  if (!mqttClient->connected()) {
    unsigned long now = millis();
    if (now - lastMqttAttempt > MQTT_RECONNECT_INTERVAL) {
      lastMqttAttempt = now;
      mqtt_reconnect();
    }
  } else {
    mqttClient->loop();
  }
}

// Chargement configuration depuis NVS
void mqtt_loadConfig(Preferences* prefs) {
  config_mqtt_ip = prefs->getString(PREF_MQTT_IP, DEFAULT_MQTT_SERVER);
  config_mqtt_port = prefs->getInt(PREF_MQTT_PORT, DEFAULT_MQTT_PORT);
  config_mqtt_user = prefs->getString(PREF_MQTT_USER, "");
  config_mqtt_pass = prefs->getString(PREF_MQTT_PASS, "");
  config_topic_prod = prefs->getString(PREF_TOPIC_PROD, DEFAULT_TOPIC_SOLAR_PROD);
  config_topic_cabane = prefs->getString(PREF_TOPIC_CABANE, DEFAULT_TOPIC_SOLAR_CABANE);
  config_topic_conso = prefs->getString(PREF_TOPIC_CONSO, DEFAULT_TOPIC_HOME_CONSO);
  config_topic_router = prefs->getString(PREF_TOPIC_ROUTER, DEFAULT_TOPIC_ROUTER);
  config_topic_water = prefs->getString(PREF_TOPIC_WATER, DEFAULT_TOPIC_WATER_TEMP);
  config_topic_ext = prefs->getString(PREF_TOPIC_EXT, DEFAULT_TOPIC_TEMP_EXT);
  config_topic_salon = prefs->getString(PREF_TOPIC_SALON, DEFAULT_TOPIC_TEMP_SALON);
  config_topic_jour = prefs->getString(PREF_TOPIC_JOUR, DEFAULT_TOPIC_CONSO_JOUR);
  // config_topic_presence_* retirés (Enphase V2)
  // config_topic_alarm retiré (Enphase V2)
  config_json_key_cabane = prefs->getString(PREF_JSON_KEY_CABANE, "");
  config_json_key_water1 = prefs->getString(PREF_JSON_KEY_WATER1, "");
  config_json_key_water2 = prefs->getString(PREF_JSON_KEY_WATER2, "");
  // config_msunpv_ip est maintenant chargée par module_msunpv.cpp
  
  addLog("[MQTT] Configuration chargée depuis NVS");
}

// Sauvegarde configuration dans NVS
void mqtt_saveConfig(Preferences* prefs) {
  prefs->putString(PREF_MQTT_IP, config_mqtt_ip);
  prefs->putInt(PREF_MQTT_PORT, config_mqtt_port);
  prefs->putString(PREF_MQTT_USER, config_mqtt_user);
  prefs->putString(PREF_MQTT_PASS, config_mqtt_pass);
  prefs->putString(PREF_TOPIC_PROD, config_topic_prod);
  prefs->putString(PREF_TOPIC_CABANE, config_topic_cabane);
  prefs->putString(PREF_TOPIC_CONSO, config_topic_conso);
  prefs->putString(PREF_TOPIC_ROUTER, config_topic_router);
  prefs->putString(PREF_TOPIC_WATER, config_topic_water);
  prefs->putString(PREF_TOPIC_EXT, config_topic_ext);
  prefs->putString(PREF_TOPIC_SALON, config_topic_salon);
  prefs->putString(PREF_TOPIC_JOUR, config_topic_jour);
  // config_topic_presence_* retirés (Enphase V2)
  // config_topic_alarm retiré (Enphase V2)
  prefs->putString(PREF_JSON_KEY_CABANE, config_json_key_cabane);
  prefs->putString(PREF_JSON_KEY_WATER1, config_json_key_water1);
  prefs->putString(PREF_JSON_KEY_WATER2, config_json_key_water2);
  // config_msunpv_ip est maintenant sauvegardée par module_msunpv.cpp
  
  addLog("[MQTT] Configuration sauvegardée dans NVS");
}

// Handler web - Page de configuration (Enphase V2 : broker uniquement)
void mqtt_handleConfig(WebServer* server) {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" type="image/svg+xml" href="/favicon.ico">
  <title>Configuration MQTT - Enphase Monitor</title>
  <style>
    body { font-family: Arial, sans-serif; background: #0c0a09; color: #fff; margin: 0; padding: 20px; }
    .container { max-width: 600px; margin: 0 auto; }
    h1 { color: #fbbf24; border-bottom: 2px solid #fbbf24; padding-bottom: 10px; }
    h2 { color: #60a5fa; margin-top: 24px; }
    .form-group { margin-bottom: 20px; background: #292524; padding: 15px; border-radius: 8px; }
    label { display: block; color: #9ca3af; margin-bottom: 5px; font-size: 0.9em; }
    input { width: 100%; padding: 10px; background: #1c1917; border: 1px solid #374151; color: #fff; border-radius: 4px; font-size: 1em; box-sizing: border-box; }
    .hint { color: #9ca3af; font-size: 0.85em; margin-top: 5px; }
    .btn { background: #fbbf24; color: #000; border: none; padding: 12px 30px; font-size: 1.1em; font-weight: bold; border-radius: 6px; cursor: pointer; margin-right: 10px; }
    .btn:hover { background: #f59e0b; }
    .btn-secondary { background: #374151; color: #fff; }
    .btn-secondary:hover { background: #4b5563; }
    .nav { margin-top: 20px; text-align: center; }
    .status { background: #1c1917; padding: 12px; border-radius: 6px; margin-bottom: 20px; }
    .status.connected { border-left: 4px solid #22c55e; }
    .status.disconnected { border-left: 4px solid #ef4444; }
    @media (max-width: 480px) { body { padding: 10px; } .btn { width: 100%; margin: 5px 0; } }
  </style>
  <script>
    function updateStatus() {
      fetch('/data').then(r => r.json()).then(d => {
        var s = document.getElementById('mqttStatus');
        s.className = 'status ' + (d.mqttConnected ? 'connected' : 'disconnected');
        s.innerText = d.mqttConnected ? 'État: Connecté au broker' : 'État: Déconnecté';
      });
    }
    setInterval(updateStatus, 3000);
    window.onload = updateStatus;
  </script>
</head>
<body>
  <div class="container">
    <h1>📡 Configuration MQTT</h1>
    <div class="status" id="mqttStatus">État: Chargement...</div>
    <p class="hint">Broker pour publication vers Home Assistant. Laisser IP vide pour désactiver MQTT.</p>
    <form method="POST" action="/saveMqtt">
      <h2>Broker</h2>
      <div class="form-group">
        <label>IP ou hostname du broker</label>
        <input type="text" name="mqtt_ip" value=")";
  html += config_mqtt_ip;
  html += R"(" placeholder="192.168.1.82">
        <div class="hint">Laisser vide pour ne pas se connecter</div>
      </div>
      <div class="form-group">
        <label>Port</label>
        <input type="number" name="mqtt_port" value=")";
  html += String(config_mqtt_port);
  html += R"(" min="1" max="65535" required>
      </div>
      <div class="form-group">
        <label>Utilisateur (optionnel)</label>
        <input type="text" name="mqtt_user" value=")";
  html += config_mqtt_user;
  html += R"(" placeholder="Laisser vide si pas d'auth">
      </div>
      <div class="form-group">
        <label>Mot de passe (optionnel)</label>
        <input type="password" name="mqtt_pass" value=")";
  html += config_mqtt_pass;
  html += R"(" placeholder="Mot de passe">
      </div>
      <button type="submit" class="btn">💾 Enregistrer et Redémarrer</button>
      <button type="button" class="btn btn-secondary" onclick="location.href='/'">❌ Annuler</button>
    </form>
    <div class="nav">
      <a href="/" style="color: #fbbf24; text-decoration: none;">&larr; Retour au Dashboard</a>
    </div>
  </div>
</body>
</html>
)";
  server->send(200, "text/html", html);
}

// Handler web - Sauvegarde configuration (Enphase V2 : broker uniquement)
void mqtt_handleSaveConfig(WebServer* server) {
  if (server->method() == HTTP_POST) {
    config_mqtt_ip = server->arg("mqtt_ip");
    config_mqtt_ip.trim();
    config_mqtt_port = server->arg("mqtt_port").toInt();
    if (config_mqtt_port <= 0) config_mqtt_port = 1883;
    config_mqtt_user = server->arg("mqtt_user");
    config_mqtt_user.trim();
    config_mqtt_pass = server->arg("mqtt_pass");
    
    // Sauvegarder dans NVS
    extern Preferences preferences;
    preferences.begin("msunpv", false);  // PREF_NAMESPACE depuis config.h
    mqtt_saveConfig(&preferences);
    preferences.end();
    
    // Mettre à jour le serveur MQTT si déjà initialisé
    if (mqttClient) {
      mqttClient->setServer(config_mqtt_ip.c_str(), config_mqtt_port);
    }
    
    // Page de confirmation
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
  <h1>✅ Configuration MQTT Sauvegardée</h1>
  <p>L'ESP32 va redémarrer dans 5 secondes...</p>
  <p>Vous serez redirigé automatiquement.</p>
</body>
</html>
)";
    server->send(200, "text/html", html);
    delay(2000);
    ESP.restart();
  }
}

