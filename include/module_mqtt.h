/*
 * MODULE MQTT - MSunPV Monitor V11
 * Gestion de la connexion MQTT et parsing des messages
 */

#ifndef MODULE_MQTT_H
#define MODULE_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>

// Variables exposées (extern) - États
extern bool mqttConnected;
extern bool mqttDataReceived;

// Variables exposées (extern) - Données mises à jour par le callback
extern float solarProdMain;
extern float solarProdCabane;
extern float solarProd;
extern float homeConso;
extern float routerPower;
extern float waterTemp;
extern float tempExt;
extern float tempSalon;
extern float consoJour;
// presenceBen, presenceFrancine, presenceVictor retirés (Enphase V2)
// alarmState retiré (Enphase V2)

// Variables exposées (extern) - Configuration broker
extern String config_mqtt_ip;
extern int config_mqtt_port;
extern String config_mqtt_user;
extern String config_mqtt_pass;
extern String config_mqtt_topic_prefix;
extern int config_mqtt_publish_interval;
extern String config_topic_prod;
extern String config_topic_cabane;
extern String config_topic_conso;
extern String config_topic_router;
extern String config_topic_water;
extern String config_topic_ext;
extern String config_topic_salon;
extern String config_topic_jour;
// config_topic_presence_*, config_victor_enabled retirés (Enphase V2)
// config_topic_alarm, config_topic_alarm_command retirés (Enphase V2)
extern String config_json_key_cabane;
extern String config_json_key_water1;
extern String config_json_key_water2;
extern String config_msunpv_ip;  // Géré dans la page MQTT

// Fonctions publiques
void mqtt_init(PubSubClient* client, WiFiClient* wifiClient);
void mqtt_loop();  // À appeler dans loop()
void mqtt_reconnect();
int mqtt_getState();              // Code état PubSubClient (-4 timeout, -2 refuse, etc.)
String mqtt_getStateMessage();    // Message lisible pour diagnostic
void mqtt_setListenTopic(const String& topic);  // Topic à écouter (outil test)
void mqtt_handleListenSet(WebServer* server);   // POST /mqttListenSet
void mqtt_handleListenData(WebServer* server);  // GET /mqttListenData
void mqtt_handleConfig(WebServer* server);           // GET /mqtt
void mqtt_handleSaveConfig(WebServer* server);       // POST /saveMqtt
void mqtt_handlePublishData(WebServer* server);      // GET /mqttPublishData (JSON valeurs Enphase)
void mqtt_loadConfig(Preferences* prefs);
void mqtt_saveConfig(Preferences* prefs);

#endif



