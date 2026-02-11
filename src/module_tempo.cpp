/*
 * Module EDF TEMPO - Implémentation
 * Fetch au démarrage + toutes les 30 min. Rollover à minuit (Tomorrow → vert).
 */

#include "module_tempo.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

#define TEMPO_API_HOST "www.api-couleur-tempo.fr"
#define PREF_TEMPO_ENABLED "tempo_enabled"  // clé NVS (évite d'inclure config.h et ses définitions globales)
#define TEMPO_INTERVAL_MS (30 * 60 * 1000)  // 30 minutes

bool tempo_enabled = false;
String tempo_today_color = "";
String tempo_tomorrow_color = "";
bool tempo_tomorrow_pending = false;
time_t tempo_last_fetch_time = 0;

static unsigned long last_tempo_fetch = 0;
static int last_rollover_day = -1;  // jour (tm_mday + mon*32) du dernier rollover

static void tempo_fetch(void) {
  if (!tempo_enabled || WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  bool ok_today = false;
  bool ok_tomorrow = false;

  // Today
  String urlToday = "https://" + String(TEMPO_API_HOST) + "/api/jourTempo/today";
  http.begin(urlToday);
  http.addHeader("Accept", "application/json");
  http.setTimeout(10000);
  int code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      const char *c = doc["libCouleur"];
      if (c) {
        tempo_today_color = String(c);
        ok_today = true;
      }
    }
  }
  http.end();

  // Tomorrow
  String urlTomorrow = "https://" + String(TEMPO_API_HOST) + "/api/jourTempo/tomorrow";
  http.begin(urlTomorrow);
  http.addHeader("Accept", "application/json");
  http.setTimeout(10000);
  code = http.GET();
  if (code == 200) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      const char *c = doc["libCouleur"];
      if (c) {
        tempo_tomorrow_color = String(c);
        tempo_tomorrow_pending = false;
        ok_tomorrow = true;
      }
    }
  }
  http.end();

  if (ok_today || ok_tomorrow) {
    last_tempo_fetch = millis();
    time_t now_sec = time(nullptr);
    if (now_sec >= 946684800) {
      tempo_last_fetch_time = now_sec;
    }
  }
}

// Rollover à minuit : today ← tomorrow, tomorrow en attente (vert)
static void tempo_rollover(void) {
  if (tempo_tomorrow_color.length() > 0) {
    tempo_today_color = tempo_tomorrow_color;
  }
  tempo_tomorrow_color = "";
  tempo_tomorrow_pending = true;
}

void tempo_init(void) {
  last_tempo_fetch = 0;
  last_rollover_day = -1;
}

void tempo_update(void) {
  if (!tempo_enabled) return;
  if (WiFi.status() != WL_CONNECTED) return;

  time_t now = time(nullptr);
  if (now < 946684800) return;  // pas de time NTP valide
  struct tm *t = localtime(&now);
  if (!t) return;

  int current_day = t->tm_mday + t->tm_mon * 32;

  // Rollover à minuit (00:00 à 00:29 on considère nouveau jour)
  if (t->tm_hour == 0 && t->tm_min < 30) {
    if (last_rollover_day != current_day) {
      tempo_rollover();
      last_rollover_day = current_day;
    }
  } else {
    last_rollover_day = -1;  // réarmement pour la prochaine nuit
  }

  // Fetch au démarrage (dès que 30 s passées) ou toutes les 30 min
  unsigned long elapsed = millis() - last_tempo_fetch;
  if (last_tempo_fetch == 0 || elapsed >= TEMPO_INTERVAL_MS) {
    tempo_fetch();
  }
}

void tempo_setEnabled(bool enabled) {
  tempo_enabled = enabled;
  if (!enabled) {
    tempo_today_color = "";
    tempo_tomorrow_color = "";
    tempo_tomorrow_pending = false;
  } else {
    // Forcer un fetch au prochain tempo_update() quand on réactive
    last_tempo_fetch = 0;
  }
}

void tempo_loadConfig(Preferences &prefs) {
  tempo_enabled = prefs.getBool(PREF_TEMPO_ENABLED, false);
  if (!tempo_enabled) {
    tempo_today_color = "";
    tempo_tomorrow_color = "";
    tempo_tomorrow_pending = false;
  }
  tempo_init();
}

void tempo_saveConfig(Preferences &prefs) {
  prefs.putBool(PREF_TEMPO_ENABLED, tempo_enabled);
}
