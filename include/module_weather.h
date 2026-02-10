/*
 * MODULE MÉTÉO - MSunPV Monitor V11
 * Gestion de la météo via API OpenWeatherMap
 */

#ifndef MODULE_WEATHER_H
#define MODULE_WEATHER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <Preferences.h>
#include "weather_icons.h"

// Variables exposées - Météo actuelle (extern)
extern int weather_code;
extern String weather_condition;
extern String weather_last_update;

// Variables exposées - Données détaillées (V3.4)
extern String weather_city;
extern float weather_temp;
extern float weather_feels_like;
extern int weather_humidity;
extern float weather_wind_speed;
extern int weather_pressure;
extern float weather_visibility;
extern int weather_clouds;
extern String weather_icon_code;

// Variables exposées - Prévisions 4 jours
extern char weather_forecast_days[4];
extern int weather_forecast_temps[4];
extern int weather_forecast_codes[4];

// Variables exposées - Aujourd'hui par créneau (Matin / Midi / Soir)
extern int weather_today_morning_temp;
extern int weather_today_morning_code;
extern int weather_today_noon_temp;
extern int weather_today_noon_code;
extern int weather_today_evening_temp;
extern int weather_today_evening_code;

// Icône LVGL pour un code météo (pour popup)
const lv_image_dsc_t* weather_getIconFromCode(int code);

// Fonctions publiques
void weather_init();
void weather_update();  // À appeler dans loop()
void weather_fetchForecast();  // V12.4 - Forecast 4 jours
void weather_handleWeb(WebServer* server);           // GET /weather
void weather_handleSaveConfig(WebServer* server);    // POST /saveWeather
void weather_handleAPI(WebServer* server);           // GET /weatherAPI (V3.4)
void weather_loadConfig(Preferences* prefs);
void weather_saveConfig(Preferences* prefs);
String weather_getCity();  // Getter pour la ville
String weather_getApiKey();  // Getter pour la clé API (export config)

#endif

