/*
 * MODULE STATS - MSunPV Monitor V11
 * Gestion des statistiques et historique 24h
 */

#ifndef MODULE_STATS_H
#define MODULE_STATS_H

#include <Arduino.h>
#include <WebServer.h>
#include <time.h>

// Variables exposées (extern) - Historique 24h
extern float histoProd[24];
extern float histoConso[24];

// Fonctions publiques
void stats_init();
void stats_update();  // À appeler dans loop() pour sauvegarder les données
void stats_handleWeb(WebServer* server);  // GET /stats
void stats_handleData(WebServer* server);  // GET /statsData (JSON)
bool stats_loadFromSD(String date);       // Charge les stats d'une date depuis la SD
String stats_getCurrentDate();            // Retourne la date actuelle (YYYY-MM-DD)

// Fonctions de calcul statistiques
float stats_getTotalProd();              // Production totale jour (kWh)
float stats_getTotalConso();             // Consommation totale jour (kWh)
float stats_getAutoconsommation();       // Autoconsommation (kWh)
float stats_getInjection();              // Injection réseau (kWh)
float stats_getPicProd();                // Pic production (W)
int stats_getPicProdHour();              // Heure pic production (0-23)
float stats_getPicConso();               // Pic consommation (W)
int stats_getPicConsoHour();             // Heure pic consommation (0-23)
float stats_getTauxAuto();               // Taux autoconsommation (%)

#endif

