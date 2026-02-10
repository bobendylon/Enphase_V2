/*
 * MODULE MSunPV - MSunPV Monitor V11
 * Gestion du routeur MSunPV via XML
 */

#ifndef MODULE_MSUNPV_H
#define MODULE_MSUNPV_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>

// Variables exposées (extern) - Configuration
extern String config_msunpv_ip;

// Variables exposées (extern) - État MSunPV
extern String msunpv_status;  // "AUTO", "MANU", "OFF"
extern int msunpv_cmdPos;
extern int msunpv_cumulusState;
extern int msunpv_radiatorState;

// Variables exposées (extern) - Données MSunPV (V12.3 - 7 valeurs simplifiées)
extern float msunpv_powReso;          // POWRESAU (ENEDIS) en W
extern float msunpv_powPV;            // POWP.V (Production solaire) en W
extern int msunpv_outBal;             // OUTBAL en %
extern int msunpv_outRad;             // OUTRAD en %
extern float msunpv_enConso;          // EnConso en Wh
extern float msunpv_tBal1;            // T_Bal1 en °C

// Fonctions publiques
void msunpv_init();
void msunpv_update();  // À appeler dans loop()
void msunpv_sendCommand(int cmd);  // 0=OFF, 1=MANU, 2=AUTO
void msunpv_handleWeb(WebServer* server);           // GET /msunpv
void msunpv_handleCommand(WebServer* server);       // POST /msunpvCmd
void msunpv_handleData(WebServer* server);          // GET /msunpvData
void msunpv_loadConfig(Preferences* prefs);
void msunpv_saveConfig(Preferences* prefs);

#endif

