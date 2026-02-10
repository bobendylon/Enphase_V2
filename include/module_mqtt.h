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
extern bool presenceBen;
extern bool presenceFrancine;
extern bool presenceVictor;
extern bool alarmState;

// Variables exposées (extern) - Configuration (pour compatibilité)
extern String config_mqtt_ip;
extern int config_mqtt_port;
extern String config_topic_prod;
extern String config_topic_cabane;
extern String config_topic_conso;
extern String config_topic_router;
extern String config_topic_water;
extern String config_topic_ext;
extern String config_topic_salon;
extern String config_topic_jour;
extern String config_topic_presence_ben;
extern String config_topic_presence_francine;
extern bool config_victor_enabled;
extern String config_topic_presence_victor;
extern String config_topic_alarm;
extern String config_topic_alarm_command;
extern String config_json_key_cabane;
extern String config_json_key_water1;
extern String config_json_key_water2;
extern String config_msunpv_ip;  // Géré dans la page MQTT

// Fonctions publiques
void mqtt_init(PubSubClient* client, WiFiClient* wifiClient);
void mqtt_loop();  // À appeler dans loop()
void mqtt_reconnect();
void mqtt_handleConfig(WebServer* server);           // GET /mqtt
void mqtt_handleSaveConfig(WebServer* server);       // POST /saveMqtt
void mqtt_loadConfig(Preferences* prefs);
void mqtt_saveConfig(Preferences* prefs);

#endif



