/*
 * MODULE ENPHASE - MSunPV Monitor V11
 * Gestion de l'Enphase Envoy via HTTPS
 */

#ifndef MODULE_ENPHASE_H
#define MODULE_ENPHASE_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <Preferences.h>

// Variables exposées (extern) - Configuration Enphase
extern String config_enphase_ip;
extern String config_enphase_user;
extern String config_enphase_pwd;
extern String config_enphase_serial;

// Variables exposées (extern) - Données Enphase - Puissances instantanées
extern float enphase_pact_conso;        // Consommation active (W)
extern float enphase_pact_prod;         // Production active (W)
extern float enphase_pact_grid;         // Puissance réseau (W)
extern float enphase_pva_conso;         // VA consommation
extern float enphase_pva_prod;          // VA production
extern float enphase_tension;           // Tension (V)
extern float enphase_intensite;         // Intensité (A)

// Variables exposées (extern) - Données Enphase - Énergies du jour
extern float enphase_energy_produced;   // Energie produite (Wh)
extern float enphase_energy_injected;   // Energie injectée au réseau (Wh)
extern float enphase_energy_consumed;  // Energie consommée (Wh)
extern float enphase_energy_imported;  // Energie importée du réseau (Wh)

// Variables exposées (extern) - Statut Enphase
extern String enphase_status;
extern String enphase_last_error;
extern bool enphase_connected;
extern unsigned long enphase_last_success;

// Fonctions publiques
void enphase_init(WiFiClientSecure* client);
void enphase_update();  // À appeler dans loop()
void enphase_handleWeb(WebServer* server);           // GET /enphase
void enphase_handleData(WebServer* server);           // GET /enphaseData
void enphase_handleStatus(WebServer* server);         // GET /enphaseStatus
void enphase_handleTest(WebServer* server);           // POST /enphaseTest
void enphase_handleSaveConfig(WebServer* server);     // POST /saveEnphaseConfig
void enphase_loadConfig(Preferences* prefs);
void enphase_saveConfig(Preferences* prefs);

#endif




