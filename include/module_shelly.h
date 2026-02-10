/*
 * MODULE SHELLY - MSunPV Monitor V11
 * Gestion des Shelly EM via HTTP
 */

#ifndef MODULE_SHELLY_H
#define MODULE_SHELLY_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <Preferences.h>

// Variables exposées (extern) - Configuration Shelly 1
extern String config_shelly1_ip;
extern String config_shelly1_name;
extern String config_shelly1_label0;
extern String config_shelly1_label1;
extern String config_shelly1_label_energy;
extern String config_shelly1_label_returned;

// Variables exposées (extern) - Configuration Shelly 2
extern String config_shelly2_ip;
extern String config_shelly2_name;
extern String config_shelly2_label0;
extern String config_shelly2_label1;
extern String config_shelly2_label_energy;
extern String config_shelly2_label_returned;

// Variables exposées (extern) - Données Shelly 1
extern float shelly1_power0;      // Thor 0 (W)
extern float shelly1_power1;      // Thor 1 (W)
extern float shelly1_total0;      // Total Thor 0 (Wh)
extern float shelly1_total1;      // Total Thor 1 (Wh)
extern float shelly1_returned0;   // Retourné Thor 0 (Wh)
extern float shelly1_returned1;   // Retourné Thor 1 (Wh)

// Variables exposées (extern) - Données Shelly 2
extern float shelly2_power0;
extern float shelly2_power1;
extern float shelly2_total0;
extern float shelly2_total1;
extern float shelly2_returned0;
extern float shelly2_returned1;

// Statut connexion (pour icônes header)
extern bool shelly1_connected;
extern bool shelly2_connected;

// Fonctions publiques
void shelly_init();
void shelly_update();  // À appeler dans loop()
void shelly_handleWeb(WebServer* server);           // GET /shellies
void shelly_handleData(WebServer* server);           // GET /shellyData
void shelly_handleGetConfig(WebServer* server);      // GET /getShellyConfig
void shelly_handleSaveConfig(WebServer* server);     // POST /saveShellyConfig
void shelly_handleSaveName(WebServer* server);       // POST /saveShellyName (deprecated)
void shelly_loadConfig(Preferences* prefs);
void shelly_saveConfig(Preferences* prefs);

#endif




