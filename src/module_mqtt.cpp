// MODULE MQTT - Implémentation

#include "module_mqtt.h"
#include "module_enphase.h"
#include <WiFi.h>

// Constantes NVS (évite d'inclure config.h qui contient des définitions)
#define PREF_MQTT_IP "mqtt_ip"
#define PREF_MQTT_PORT "mqtt_port"
#define PREF_MQTT_USER "mqtt_user"
#define PREF_MQTT_PASS "mqtt_pass"
#define PREF_MQTT_TOPIC_PREFIX "mqtt_topic_prefix"
#define PREF_MQTT_PUBLISH_INTERVAL "mqtt_publish_interval"
#define PREF_MQTT_HA_DISCOVERY "mqtt_ha_discovery"
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

// Valeurs par défaut (vides ou génériques, config via portail web)
#define DEFAULT_MQTT_SERVER ""
#define DEFAULT_MQTT_PORT 1883
#define DEFAULT_TOPIC_SOLAR_PROD ""
#define DEFAULT_TOPIC_SOLAR_CABANE ""
#define DEFAULT_TOPIC_HOME_CONSO ""
#define DEFAULT_TOPIC_ROUTER ""
#define DEFAULT_TOPIC_WATER_TEMP ""
#define DEFAULT_TOPIC_TEMP_EXT ""
#define DEFAULT_TOPIC_TEMP_SALON ""
#define DEFAULT_TOPIC_CONSO_JOUR ""
#define DEFAULT_TOPIC_PRESENCE_BEN "jeedom/presence/ben/state"
#define DEFAULT_TOPIC_PRESENCE_FRANCINE "jeedom/presence/francine/state"
#define DEFAULT_TOPIC_PRESENCE_VICTOR "jeedom/presence/victor/state"
#define DEFAULT_TOPIC_ALARM ""
#define DEFAULT_TOPIC_ALARM_COMMAND ""
#define DEFAULT_MSUNPV_IP ""
#define DEFAULT_MQTT_TOPIC_PREFIX "enphase_monitor"
#define DEFAULT_MQTT_PUBLISH_INTERVAL 15

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
String config_mqtt_topic_prefix;
int config_mqtt_publish_interval;
bool config_mqtt_ha_discovery;
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

// Outil écoute MQTT (diagnostic)
static String mqttListenTopic;
static String mqttListenLastTopic;
static String mqttListenLastValue;
static unsigned long mqttListenLastTime = 0;

// Publication Enphase : dernier envoi
static unsigned long lastMqttPublish = 0;

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
  
  // Outil écoute : stocker la dernière valeur reçue sur le topic écouté
  if (mqttListenTopic.length() > 0) {
    mqttListenLastTopic = topicStr;
    mqttListenLastValue = message;
    mqttListenLastTime = millis();
  }
}

// S'abonner au topic d'écoute (si connecté)
static void mqtt_doSubscribeListen() {
  if (!mqttClient || !mqttClient->connected() || mqttListenTopic.length() == 0) return;
  if (mqttClient->subscribe(mqttListenTopic.c_str())) {
    addLogf("[MQTT] Ecoute activee: %s", mqttListenTopic.c_str());
  } else {
    addLog("[MQTT] Echec subscribe ecoute");
  }
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

// Publication des valeurs Enphase vers MQTT (préfixe configurable)
static void mqtt_publish_enphase() {
  if (!mqttClient || !mqttClient->connected()) return;
  String prefix = config_mqtt_topic_prefix.length() > 0 ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
  char buf[32];

  #define PUB(topicSuffix, payload) do { \
    String t = prefix + "/" + (topicSuffix); \
    if (mqttClient->publish(t.c_str(), (payload))) { /* ok */ } \
  } while(0)

  snprintf(buf, sizeof(buf), "%.0f", enphase_pact_prod);
  PUB("power_production", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_pact_conso);
  PUB("power_consumption", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_pact_grid);
  PUB("power_grid", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_energy_produced);
  PUB("energy_produced_today", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_energy_consumed);
  PUB("energy_consumed_today", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_energy_imported);
  PUB("energy_imported_today", buf);
  snprintf(buf, sizeof(buf), "%.0f", enphase_energy_injected);
  PUB("energy_injected_today", buf);
  PUB("status", "online");
  #undef PUB
}

// Publication Home Assistant MQTT Discovery (optionnel)
static void mqtt_publish_ha_discovery() {
  if (!mqttClient || !mqttClient->connected() || !config_mqtt_ha_discovery) return;
  String prefix = config_mqtt_topic_prefix.length() > 0 ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
  char topicBuf[80];
  char payloadBuf[420];
  const char* dev = "{\"identifiers\":[\"";
  const char* dev2 = "\"],\"name\":\"Enphase Monitor\",\"manufacturer\":\"MSunPV\",\"model\":\"Enphase V2\"}";
  String devBlock = String(dev) + prefix + dev2;

  #define DISC_SENSOR(id, name, unit, devClass, stateClass) do { \
    snprintf(topicBuf, sizeof(topicBuf), "homeassistant/sensor/%s/%s/config", prefix.c_str(), id); \
    snprintf(payloadBuf, sizeof(payloadBuf), \
      "{\"name\":\"%s\",\"unique_id\":\"%s_%s\",\"state_topic\":\"%s/%s\",\"unit_of_measurement\":\"%s\",\"device_class\":\"%s\",\"state_class\":\"%s\",\"device\":%s}", \
      name, prefix.c_str(), id, prefix.c_str(), id, unit, devClass, stateClass, devBlock.c_str()); \
    mqttClient->publish(topicBuf, payloadBuf, true); \
  } while(0)

  DISC_SENSOR("power_production",     "Production solaire",  "W",  "power",  "measurement");
  DISC_SENSOR("power_consumption",    "Conso maison",       "W",  "power",  "measurement");
  DISC_SENSOR("power_grid",           "Réseau",             "W",  "power",  "measurement");
  DISC_SENSOR("energy_produced_today","Prod jour",          "Wh", "energy", "total_increasing");
  DISC_SENSOR("energy_consumed_today","Conso jour",         "Wh", "energy", "total_increasing");
  DISC_SENSOR("energy_imported_today","Importé jour",       "Wh", "energy", "total_increasing");
  DISC_SENSOR("energy_injected_today","Injecté jour",       "Wh", "energy", "total_increasing");

  snprintf(topicBuf, sizeof(topicBuf), "homeassistant/sensor/%s/status/config", prefix.c_str());
  snprintf(payloadBuf, sizeof(payloadBuf),
    "{\"name\":\"Statut\",\"unique_id\":\"%s_status\",\"state_topic\":\"%s/status\",\"device\":%s}",
    prefix.c_str(), prefix.c_str(), devBlock.c_str());
  mqttClient->publish(topicBuf, payloadBuf, true);

  #undef DISC_SENSOR
  addLog("[MQTT] Home Assistant Discovery publie");
}

// Reconnexion MQTT (LWT = offline sur <prefix>/status)
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
    
    String clientId = "EnphaseMonitor-" + String(random(0xffff), HEX);
    String prefix = config_mqtt_topic_prefix.length() > 0 ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
    String willTopic = prefix + "/status";
    mqttClient->setBufferSize(2048);
    
    bool connected = false;
    if (config_mqtt_user.length() > 0) {
      connected = mqttClient->connect(clientId.c_str(), config_mqtt_user.c_str(), config_mqtt_pass.c_str(), willTopic.c_str(), 0, true, "offline");
    } else {
      connected = mqttClient->connect(clientId.c_str(), willTopic.c_str(), 0, true, "offline");
    }
    
    if (connected) {
      addLog("Connexion MQTT... OK!");
      mqttConnected = true;
      mqttDataReceived = true;
      mqtt_doSubscribeListen();
      mqtt_publish_enphase();
      mqtt_publish_ha_discovery();
      lastMqttPublish = millis();
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
    unsigned long intervalMs = (unsigned long)(config_mqtt_publish_interval > 0 ? config_mqtt_publish_interval : DEFAULT_MQTT_PUBLISH_INTERVAL) * 1000;
    if (millis() - lastMqttPublish >= intervalMs) {
      mqtt_publish_enphase();
      lastMqttPublish = millis();
    }
  }
}

int mqtt_getState() {
  if (!mqttClient || config_mqtt_ip.length() == 0) return -1;
  return mqttClient->state();
}

String mqtt_getStateMessage() {
  int s = mqtt_getState();
  if (config_mqtt_ip.length() == 0) return "IP vide (MQTT desactive)";
  switch (s) {
    case -4: return "Timeout (broker inaccessible ?)";
    case -2: return "Refuse (IP/port/user/pass ?)";
    case -1: return "Deconnecte";
    case 0:  return "Connecte";
    case 1:  return "Erreur protocole";
    case 2:  return "Client ID invalide";
    case 3:  return "Broker indisponible";
    case 4:  return "User/pass invalide";
    case 5:  return "Non autorise";
    default: return "Code " + String(s);
  }
}

void mqtt_setListenTopic(const String& topic) {
  mqttListenTopic = topic;
  mqttListenTopic.trim();
  mqttListenLastTopic = "";
  mqttListenLastValue = "";
  if (mqttListenTopic.length() > 0) {
    mqtt_doSubscribeListen();
  }
}

// Chargement configuration depuis NVS
void mqtt_loadConfig(Preferences* prefs) {
  config_mqtt_ip = prefs->getString(PREF_MQTT_IP, DEFAULT_MQTT_SERVER);
  config_mqtt_port = prefs->getInt(PREF_MQTT_PORT, DEFAULT_MQTT_PORT);
  config_mqtt_user = prefs->getString(PREF_MQTT_USER, "");
  config_mqtt_pass = prefs->getString(PREF_MQTT_PASS, "");
  config_mqtt_topic_prefix = prefs->getString(PREF_MQTT_TOPIC_PREFIX, DEFAULT_MQTT_TOPIC_PREFIX);
  config_mqtt_publish_interval = prefs->getInt(PREF_MQTT_PUBLISH_INTERVAL, DEFAULT_MQTT_PUBLISH_INTERVAL);
  if (config_mqtt_publish_interval < 5) config_mqtt_publish_interval = 5;
  if (config_mqtt_publish_interval > 300) config_mqtt_publish_interval = 300;
  config_mqtt_ha_discovery = prefs->getBool(PREF_MQTT_HA_DISCOVERY, true);
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
  prefs->putString(PREF_MQTT_TOPIC_PREFIX, config_mqtt_topic_prefix);
  prefs->putInt(PREF_MQTT_PUBLISH_INTERVAL, config_mqtt_publish_interval);
  prefs->putBool(PREF_MQTT_HA_DISCOVERY, config_mqtt_ha_discovery);
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

// GET /mqttPublishData - Valeurs Enphase actuelles (pour tableau de contrôle)
void mqtt_handlePublishData(WebServer* server) {
  char buf[320];
  snprintf(buf, sizeof(buf),
    "{\"power_production\":%.0f,\"power_consumption\":%.0f,\"power_grid\":%.0f,"
    "\"energy_produced_today\":%.0f,\"energy_consumed_today\":%.0f,\"energy_imported_today\":%.0f,\"energy_injected_today\":%.0f,"
    "\"status\":\"%s\"}",
    enphase_pact_prod, enphase_pact_conso, enphase_pact_grid,
    enphase_energy_produced, enphase_energy_consumed, enphase_energy_imported, enphase_energy_injected,
    enphase_connected ? "online" : "offline");
  server->send(200, "application/json", buf);
}

// Handler web - Page de configuration (Enphase V2 : broker + publication Enphase)
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
    body{font-family:system-ui,sans-serif;background:#0c0a09;color:#e7e5e4;margin:0;padding:20px;}
    .container{max-width:720px;margin:0 auto;}
    h1{color:#fbbf24;border-bottom:2px solid rgba(251,191,36,0.4);padding-bottom:10px;}
    h2{color:#60a5fa;margin-top:24px;}
    .form-group{margin-bottom:20px;background:#292524;padding:16px;border-radius:10px;border:1px solid #44403c;}
    label{display:block;color:#a8a29e;margin-bottom:6px;font-size:0.9em;}
    input{width:100%;padding:10px;background:#1c1917;border:1px solid #44403c;color:#fafaf9;border-radius:6px;box-sizing:border-box;}
    input:focus{outline:none;border-color:#fbbf24;}
    .hint{color:#78716c;font-size:0.85em;margin-top:6px;}
    .btn{background:#fbbf24;color:#000;border:none;padding:12px 24px;font-weight:600;border-radius:8px;cursor:pointer;margin-right:10px;}
    .btn:hover{background:#f59e0b;}
    .btn-secondary{background:#44403c;color:#e7e5e4;}
    .btn-secondary:hover{background:#57534e;}
    .nav{margin-top:24px;text-align:center;}
    .status{background:#1c1917;padding:14px;border-radius:8px;margin-bottom:20px;border:1px solid #44403c;}
    .status.connected{border-left:4px solid #22c55e;}
    .status.disconnected{border-left:4px solid #ef4444;}
    .publish-card{background:#292524;border-radius:10px;border:1px solid #44403c;margin-top:20px;}
    .publish-card h2{margin:0;padding:16px;background:rgba(251,191,36,0.08);border-bottom:1px solid #44403c;}
    .publish-table-wrap{overflow-x:auto;-webkit-overflow-scrolling:touch;margin:0 -1px;}
    .publish-table{width:100%;min-width:380px;border-collapse:collapse;}
    .publish-table th{text-align:left;padding:10px 12px;background:#1c1917;color:#a8a29e;font-size:0.8em;text-transform:uppercase;}
    .publish-table td{padding:10px 12px;border-bottom:1px solid #3f3f3f;}
    .publish-table tr:last-child td{border-bottom:none;}
    .publish-table .topic{font-family:monospace;font-size:0.85em;color:#d6d3d1;word-break:break-all;max-width:220px;}
    .publish-table .value{text-align:right;color:#fbbf24;font-weight:600;white-space:nowrap;min-width:5.5em;}
    .publish-table .value.status-online{color:#22c55e;}
    .publish-table .value.status-offline{color:#ef4444;}
    .publish-table .unit{text-align:center;color:#78716c;font-size:0.9em;white-space:nowrap;min-width:2.5em;}
    .publish-badge{display:inline-block;font-size:0.75em;background:rgba(251,191,36,0.2);color:#fbbf24;padding:2px 8px;border-radius:4px;margin-left:8px;}
    @media (max-width:480px){
      .container{padding:0 8px;max-width:100%;}
      body{padding:12px 0;}
    }
  </style>
  <script>
    var publishPrefix = ')";
  html += config_mqtt_topic_prefix.length() ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
  html += R"(';
    function updateStatus() {
      fetch('/data').then(r => r.json()).then(d => {
        var s = document.getElementById('mqttStatus');
        s.className = 'status ' + (d.mqttConnected ? 'connected' : 'disconnected');
        s.innerText = d.mqttConnected ? 'Connecte au broker' : ('Deconnecte - ' + (d.mqttStateMessage || ''));
      });
    }
    function refreshPublishPrefix() {
      var inp = document.getElementById('mqtt_topic_prefix');
      if (inp) publishPrefix = inp.value.trim() || ')";
  html += String(DEFAULT_MQTT_TOPIC_PREFIX);
  html += R"(';
      var cells = document.querySelectorAll('.publish-table .topic');
      var suffixes = ['power_production','power_consumption','power_grid','energy_produced_today','energy_consumed_today','energy_imported_today','energy_injected_today','status'];
      for (var i = 0; i < cells.length && i < suffixes.length; i++) cells[i].textContent = publishPrefix + '/' + suffixes[i];
    }
    function updatePublishTable(data) {
      var ids = ['power_production','power_consumption','power_grid','energy_produced_today','energy_consumed_today','energy_imported_today','energy_injected_today','status'];
      var units = [' W',' W',' W',' Wh',' Wh',' Wh',' Wh',''];
      for (var i = 0; i < ids.length; i++) {
        var el = document.getElementById('val_' + ids[i]);
        if (!el) continue;
        var v = data[ids[i]];
        if (v === undefined) { el.textContent = '-'; el.className = 'value'; continue; }
        if (ids[i] === 'status') {
          el.textContent = v;
          el.className = 'value status-' + v;
        } else {
          el.textContent = (typeof v === 'number' ? (ids[i].indexOf('energy') >= 0 ? Number(v).toFixed(1) : Math.round(v)) : v) + units[i];
          el.className = 'value';
        }
      }
    }
    function startListen() {
      var t = document.getElementById('mqttListenTopic').value.trim();
      if (!t) { alert('Saisissez un topic MQTT'); return; }
      fetch('/mqttListenSet', { method: 'POST', headers: {'Content-Type':'application/x-www-form-urlencoded'}, body: 'topic=' + encodeURIComponent(t) })
        .then(r => r.json()).then(function(d) {
          document.getElementById('mqttListenStatus').textContent = 'Ecoute activee sur: ' + t;
          document.getElementById('mqttListenStatus').style.color = '#22c55e';
        }).catch(function() { document.getElementById('mqttListenStatus').textContent = 'Erreur'; });
    }
    function updateListenData() {
      fetch('/mqttListenData').then(r => r.json()).then(function(d) {
        var el = document.getElementById('mqttListenValue');
        if (d.topic || d.value) {
          el.innerHTML = '<strong>Topic:</strong> ' + (d.topic || '-') + '<br><strong>Valeur:</strong> ' + (d.value || '-');
          el.style.color = '#d1d5db';
        } else {
          el.textContent = 'Aucune valeur recue (attendre une publication sur le topic)';
          el.style.color = '#9ca3af';
        }
      });
    }
    function fetchPublishData() {
      fetch('/mqttPublishData').then(function(r) { return r.text(); }).then(function(t) {
        try { var d = JSON.parse(t); updatePublishTable(d); } catch(e) { updatePublishTable({}); }
      }).catch(function() { updatePublishTable({}); });
    }
    setInterval(updateStatus, 3000);
    setInterval(updateListenData, 2000);
    setInterval(fetchPublishData, 2000);
    window.onload = function() { updateStatus(); updateListenData(); refreshPublishPrefix(); fetchPublishData(); };
  </script>
</head>
<body>
  <div class="container">
    <h1>📡 Configuration MQTT</h1>
    <div class="status" id="mqttStatus">Etat: Chargement...</div>
    <p class="hint">Broker pour publication vers Home Assistant. Laisser IP vide pour desactiver MQTT.</p>
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
  html += R"(" min="1" max="65535" placeholder="1883">
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
      <h2>Publication Enphase</h2>
      <div class="form-group">
        <label>Préfixe des topics</label>
        <input type="text" name="mqtt_topic_prefix" id="mqtt_topic_prefix" value=")";
  html += config_mqtt_topic_prefix.length() ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
  html += F("\" placeholder=\"enphase_monitor\" oninput=\"refreshPublishPrefix()\">\n        <div class=\"hint\">Topics publies: &lt;prefixe&gt;/power_production, etc.</div>\n      </div>\n      <div class=\"form-group\">\n        <label>Intervalle de publication (secondes)</label>\n        <input type=\"number\" name=\"mqtt_publish_interval\" id=\"mqtt_publish_interval\" value=\"");
  html += String(config_mqtt_publish_interval);
  html += R"(" min="5" max="300" step="1" placeholder="15">
        <div class="hint">Entre 5 et 300 s. Defaut: 15 s.</div>
      </div>
      <div class="form-group">
        <label style="display:flex;align-items:center;gap:10px;">
          <input type="checkbox" name="mqtt_ha_discovery" value="1" )";
  html += config_mqtt_ha_discovery ? "checked" : "";
  html += R"(>
          Activer Home Assistant Discovery
        </label>
        <div class="hint">Si coché, les entités seront créées automatiquement dans HA (sans config manuelle).</div>
      </div>
      <button type="submit" class="btn">Enregistrer et Redemarrer</button>
      <button type="button" class="btn btn-secondary" onclick="location.href='/'">Annuler</button>
    </form>
    <div class="publish-card">
      <h2>Valeurs publiees MQTT <span class="publish-badge">Valeur Enphase (contrôle)</span></h2>
      <div class="publish-table-wrap">
      <table class="publish-table">
        <thead><tr><th>Topic publie</th><th>Valeur Enphase</th><th>Unite</th></tr></thead>
        <tbody>)";
  {
    static const char* suf[] = {"power_production","power_consumption","power_grid","energy_produced_today","energy_consumed_today","energy_imported_today","energy_injected_today","status"};
    static const char* unt[] = {"W","W","W","Wh","Wh","Wh","Wh","—"};
    String p = config_mqtt_topic_prefix.length() ? config_mqtt_topic_prefix : String(DEFAULT_MQTT_TOPIC_PREFIX);
    for (int i = 0; i < 8; i++) {
      html += "<tr><td class=\"topic\">";
      html += p;
      html += "/";
      html += suf[i];
      html += "</td><td class=\"value\" id=\"val_";
      html += suf[i];
      html += "\">-</td><td class=\"unit\">";
      html += unt[i];
      html += "</td></tr>";
    }
  }
  html += R"(
        </tbody>
      </table>
      </div>
    </div>
    <h2>Test ecoute MQTT</h2>
    <p class="hint">Connecte au broker uniquement. Saisissez un topic pour ecouter les messages (ex: shellies/+/emeter/0/power).</p>
    <div class="form-group">
      <label>Topic a ecouter</label>
      <input type="text" id="mqttListenTopic" placeholder="ex: shellies/shellyem-xxx/emeter/0/power" style="margin-bottom:8px">
      <button type="button" class="btn" onclick='startListen()'>Ecouter</button>
      <div id="mqttListenStatus" class="hint" style="margin-top:8px"></div>
      <div id="mqttListenValue" class="hint" style="margin-top:8px;min-height:40px">Aucune valeur recue</div>
    </div>
    <div class="nav">
      <a href="/" style="color: #fbbf24; text-decoration: none;">&larr; Retour au Dashboard</a>
    </div>
  </div>
</body>
</html>
)";
  server->send(200, "text/html", html);
}

// Handler web - Sauvegarde configuration (Enphase V2 : broker + publication)
void mqtt_handleSaveConfig(WebServer* server) {
  if (server->method() == HTTP_POST) {
    config_mqtt_ip = server->arg("mqtt_ip");
    config_mqtt_ip.trim();
    config_mqtt_port = server->arg("mqtt_port").toInt();
    if (config_mqtt_port <= 0) config_mqtt_port = 1883;
    config_mqtt_user = server->arg("mqtt_user");
    config_mqtt_user.trim();
    config_mqtt_pass = server->arg("mqtt_pass");
    config_mqtt_topic_prefix = server->arg("mqtt_topic_prefix");
    config_mqtt_topic_prefix.trim();
    if (config_mqtt_topic_prefix.length() == 0) config_mqtt_topic_prefix = String(DEFAULT_MQTT_TOPIC_PREFIX);
    config_mqtt_publish_interval = server->arg("mqtt_publish_interval").toInt();
    if (config_mqtt_publish_interval < 5) config_mqtt_publish_interval = 5;
    if (config_mqtt_publish_interval > 300) config_mqtt_publish_interval = 300;
    config_mqtt_ha_discovery = server->hasArg("mqtt_ha_discovery");
    
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

// POST /mqttListenSet - Définir le topic à écouter
void mqtt_handleListenSet(WebServer* server) {
  String topic = server->hasArg("topic") ? server->arg("topic") : "";
  mqtt_setListenTopic(topic);
  server->send(200, "application/json", "{\"ok\":true,\"topic\":\"" + topic + "\"}");
}

// GET /mqttListenData - Dernière valeur reçue (JSON)
void mqtt_handleListenData(WebServer* server) {
  // Échapper JSON pour topic et value
  String t = mqttListenLastTopic;
  String v = mqttListenLastValue;
  t.replace("\\", "\\\\"); t.replace("\"", "\\\"");
  v.replace("\\", "\\\\"); v.replace("\"", "\\\"");
  String json = "{\"topic\":\"" + t + "\",\"value\":\"" + v + "\",\"time\":" + String(mqttListenLastTime) + "}";
  server->send(200, "application/json", json);
}

