// MODULE MQTT - Implémentation

#include "module_mqtt.h"
#include <WiFi.h>

// Constantes NVS (évite d'inclure config.h qui contient des définitions)
#define PREF_MQTT_IP "mqtt_ip"
#define PREF_MQTT_PORT "mqtt_port"
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

// Constantes depuis config.h (valeurs par défaut)
// Note: MQTT_USER et MQTT_PASSWORD sont définis dans config.h
// On les déclare en extern pour éviter d'inclure config.h
extern const char* MQTT_USER;
extern const char* MQTT_PASSWORD;

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
  if (WiFi.status() != WL_CONNECTED) {
    addLog("[MQTT] WiFi non connecté");
    return;
  }
  
  if (!mqttClient->connected()) {
    addLogf("Connexion MQTT... (Broker: %s:%d)", config_mqtt_ip.c_str(), config_mqtt_port);
    
    String clientId = "MSunPV-" + String(random(0xffff), HEX);
    
    bool connected = false;
    if (strlen(MQTT_USER) > 0) {
      connected = mqttClient->connect(clientId.c_str(), MQTT_USER, MQTT_PASSWORD);
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
  addLogf("  Clés JSON - cabane: '%s', water1: '%s', water2: '%s'", 
                config_json_key_cabane.c_str(), 
                config_json_key_water1.c_str(), 
                config_json_key_water2.c_str());
}

// Sauvegarde configuration dans NVS
void mqtt_saveConfig(Preferences* prefs) {
  prefs->putString(PREF_MQTT_IP, config_mqtt_ip);
  prefs->putInt(PREF_MQTT_PORT, config_mqtt_port);
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
  addLogf("  Clés JSON sauvegardées - cabane: '%s', water1: '%s', water2: '%s'", 
                config_json_key_cabane.c_str(), 
                config_json_key_water1.c_str(), 
                config_json_key_water2.c_str());
}

// Handler web - Page de configuration
void mqtt_handleConfig(WebServer* server) {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" type="image/svg+xml" href="/favicon.ico">
  <title>Configuration MQTT - MSunPV</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #0c0a09;
      color: #fff;
      margin: 0;
      padding: 20px;
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
    }
    h1 {
      color: #fbbf24;
      border-bottom: 2px solid #fbbf24;
      padding-bottom: 10px;
    }
    h2 {
      color: #60a5fa;
      margin-top: 30px;
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
    .value-display {
      color: #22c55e;
      font-size: 0.9em;
      margin-top: 5px;
      font-style: italic;
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
    .status {
      background: #1c1917;
      padding: 10px;
      border-radius: 6px;
      margin-bottom: 20px;
    }
    @media (max-width: 768px) {
      body { padding: 12px; }
      h1 { font-size: 1.5em; }
      h2 { font-size: 1.2em; margin-top: 20px; }
      .form-group { padding: 12px; }
      .btn { width: 100%; margin: 5px 0; padding: 14px; min-height: 44px; }
    }
    @media (max-width: 480px) {
      body { padding: 10px; }
      h1 { font-size: 1.3em; }
      input { padding: 12px; font-size: 16px; }
      .btn { font-size: 1em; }
    }
  </style>
  <script>
    function updateValues() {
      fetch('/data')
        .then(r => r.json())
        .then(d => {
          document.getElementById('val_prod').innerText = 'Valeur: ' + d.solarProd.toFixed(1) + ' W';
          document.getElementById('val_cabane').innerText = 'Valeur: ' + d.solarProdCabane.toFixed(1) + ' W';
          document.getElementById('val_conso').innerText = 'Valeur: ' + d.homeConso.toFixed(1) + ' W';
          document.getElementById('val_router').innerText = 'Valeur: ' + d.routerPower.toFixed(1) + ' W';
          document.getElementById('val_water').innerText = 'Valeur: ' + d.waterTemp.toFixed(1) + ' °C';
          document.getElementById('val_ext').innerText = 'Valeur: ' + d.tempExt.toFixed(1) + ' °C';
          document.getElementById('val_salon').innerText = 'Valeur: ' + d.tempSalon.toFixed(1) + ' °C';
          document.getElementById('val_jour').innerText = 'Valeur: ' + d.consoJour.toFixed(2) + ' kWh';
          // val_presence_ben, val_presence_francine, val_presence_victor retirés (Enphase V2)
        });
    }
    setInterval(updateValues, 2000);
    window.onload = updateValues;
  </script>
</head>
<body>
  <div class="container">
    <h1>⚙️ Configuration MQTT</h1>
    
    <div class="status">
      Configuration actuelle chargée depuis NVS. Modifiez et sauvegardez pour mettre à jour.
    </div>
    
    <form method="POST" action="/saveMqtt">
      <h2>📡 Broker MQTT</h2>
      <div class="form-group">
        <label>IP Broker MQTT</label>
        <input type="text" name="mqtt_ip" value=")";
  html += config_mqtt_ip;
  html += R"(" required>
      </div>
      <div class="form-group">
        <label>Port MQTT</label>
        <input type="number" name="mqtt_port" value=")";
  html += String(config_mqtt_port);
  html += R"(" required>
      </div>
      
      <h2>📊 Topics MQTT</h2>
      <div class="form-group">
        <label>Topic Production Shelly Principal</label>
        <input type="text" name="topic_prod" value=")";
  html += config_topic_prod;
  html += R"(" required>
        <div class="value-display" id="val_prod">-</div>
      </div>
      
      <div class="form-group">
        <label>Topic Production Cabane (Zigbee2MQTT)</label>
        <input type="text" name="topic_cabane" value=")";
  html += config_topic_cabane;
  html += R"(" required>
        <div class="value-display" id="val_cabane">-</div>
      </div>
      <div class="form-group">
        <label>Clé JSON Production Cabane (laisser vide pour valeur simple)</label>
        <input type="text" name="json_key_cabane" value=")";
  html += config_json_key_cabane;
  html += R"(" placeholder="power">
        <div class="value-display" style="color: #9ca3af; font-size: 0.8em;">Ex: "power" - Laisser vide si le topic envoie une valeur numérique directe</div>
      </div>
      
      <div class="form-group">
        <label>Topic Consommation Maison</label>
        <input type="text" name="topic_conso" value=")";
  html += config_topic_conso;
  html += R"(" required>
        <div class="value-display" id="val_conso">-</div>
      </div>
      
      <div class="form-group">
        <label>Topic Routeur M'SunPV</label>
        <input type="text" name="topic_router" value=")";
  html += config_topic_router;
  html += R"(" required>
        <div class="value-display" id="val_router">-</div>
      </div>
      
      <div class="form-group">
        <label>Topic Température Cumulus (DS18B20 Tasmota)</label>
        <input type="text" name="topic_water" value=")";
  html += config_topic_water;
  html += R"(" required>
        <div class="value-display" id="val_water">-</div>
      </div>
      <div class="form-group">
        <label>Clé JSON Niveau 1 Température Eau (laisser vide pour valeur simple)</label>
        <input type="text" name="json_key_water1" value=")";
  html += config_json_key_water1;
  html += R"(" placeholder="DS18B20-1">
        <div class="value-display" style="color: #9ca3af; font-size: 0.8em;">Ex: "DS18B20-1" - Première clé JSON (niveau 1)</div>
      </div>
      <div class="form-group">
        <label>Clé JSON Niveau 2 Température Eau (laisser vide pour valeur simple)</label>
        <input type="text" name="json_key_water2" value=")";
  html += config_json_key_water2;
  html += R"(" placeholder="Temperature">
        <div class="value-display" style="color: #9ca3af; font-size: 0.8em;">Ex: "Temperature" - Deuxième clé JSON (niveau 2, dans l'objet niveau 1)</div>
      </div>
      
      <div class="form-group">
        <label>Topic Température Extérieure</label>
        <input type="text" name="topic_ext" value=")";
  html += config_topic_ext;
  html += R"(" required>
        <div class="value-display" id="val_ext">-</div>
      </div>
      
      <div class="form-group">
        <label>Topic Température Salon</label>
        <input type="text" name="topic_salon" value=")";
  html += config_topic_salon;
  html += R"(" required>
        <div class="value-display" id="val_salon">-</div>
      </div>
      
      <div class="form-group">
        <label>Topic Consommation Journalière</label>
        <input type="text" name="topic_jour" value=")";
  html += config_topic_jour;
  html += R"(" required>
        <div class="value-display" id="val_jour">-</div>
      </div>
      
      <!-- Topics présence Ben, Francine, Victor retirés (Enphase V2) -->
      <!-- Topic Alarme retiré (Enphase V2) -->
      
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

// Handler web - Sauvegarde configuration
void mqtt_handleSaveConfig(WebServer* server) {
  if (server->method() == HTTP_POST) {
    // Récupérer les valeurs
    config_mqtt_ip = server->arg("mqtt_ip");
    config_mqtt_port = server->arg("mqtt_port").toInt();
    config_topic_prod = server->arg("topic_prod");
    config_topic_cabane = server->arg("topic_cabane");
    config_topic_conso = server->arg("topic_conso");
    config_topic_router = server->arg("topic_router");
    config_topic_water = server->arg("topic_water");
    config_topic_ext = server->arg("topic_ext");
    config_topic_salon = server->arg("topic_salon");
    config_topic_jour = server->arg("topic_jour");
    // config_topic_presence_* retirés (Enphase V2)
    // config_topic_alarm retiré (Enphase V2)
    
    // Clés JSON configurables
    config_json_key_cabane = server->arg("json_key_cabane");
    config_json_key_water1 = server->arg("json_key_water1");
    config_json_key_water2 = server->arg("json_key_water2");
    
    // Debug : afficher les valeurs reçues
    addLog("[MQTT] Sauvegarde config - Clés JSON:");
    addLogf("  json_key_cabane: '%s'", config_json_key_cabane.c_str());
    addLogf("  json_key_water1: '%s'", config_json_key_water1.c_str());
    addLogf("  json_key_water2: '%s'", config_json_key_water2.c_str());
    
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

