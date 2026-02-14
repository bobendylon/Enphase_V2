/*
 * Module EDF TEMPO - Implémentation
 * Fetch au démarrage + toutes les 30 min. Rollover à minuit (Tomorrow → vert).
 */

#include "module_tempo.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

extern void addLog(String message);
extern void addLogf(const char* format, ...);

#define TEMPO_API_HOST "www.api-couleur-tempo.fr"
#define PREF_TEMPO_ENABLED "tempo_enabled"  // clé NVS (évite d'inclure config.h et ses définitions globales)
#define TEMPO_INTERVAL_MS (30 * 60 * 1000)  // 30 minutes

bool tempo_enabled = false;
String tempo_today_color = "";
String tempo_tomorrow_color = "";
bool tempo_tomorrow_pending = false;
time_t tempo_last_fetch_time = 0;

static unsigned long last_tempo_fetch = 0;
static int last_rollover_day = -1;  // jour unique (année*400 + mois*32 + jour) du dernier rollover

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

  // Tomorrow — RTE publie vers 11h ; avant ça l'API peut renvoyer "Couleur à venir" ou autre
  String urlTomorrow = "https://" + String(TEMPO_API_HOST) + "/api/jourTempo/tomorrow";
  addLog("[TEMPO] Fetch tomorrow: " + urlTomorrow);
  http.begin(urlTomorrow);
  http.addHeader("Accept", "application/json");
  http.setTimeout(10000);
  code = http.GET();
  addLogf("[TEMPO] Tomorrow HTTP code=%d", code);
  if (code == 200) {
    String payload = http.getString();
    addLog("[TEMPO] Tomorrow reponse: " + payload.substring(0, payload.length() > 120 ? 120 : payload.length()) + (payload.length() > 120 ? "..." : ""));
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      const char *c = doc["libCouleur"];
      if (c) {
        String color = String(c);
        // Seules les 3 couleurs EDF = connu ; "Couleur à venir" ou autre = rester en vert
        if (color == "Bleu" || color == "Blanc" || color == "Rouge") {
          tempo_tomorrow_color = color;
          tempo_tomorrow_pending = false;
          ok_tomorrow = true;
          addLogf("[TEMPO] Demain = %s (couleur connue)", color.c_str());
        } else {
          tempo_tomorrow_color = "";
          tempo_tomorrow_pending = true;
          addLogf("[TEMPO] Demain libCouleur='%s' -> vert (non reconnu, avant 11h?)", color.c_str());
        }
      } else {
        tempo_tomorrow_pending = true;
        addLog("[TEMPO] Demain pas de libCouleur -> vert");
      }
    } else {
      addLog("[TEMPO] Demain JSON invalide -> vert");
      tempo_tomorrow_pending = true;
    }
  } else {
    tempo_tomorrow_pending = true;
    addLogf("[TEMPO] Demain erreur HTTP %d -> vert", code);
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

  // Un jour unique par date (année, mois, jour) pour détecter le changement de jour à tout moment
  int current_day = (t->tm_year + 1900) * 400 + t->tm_mon * 32 + t->tm_mday;

  // Rollover dès qu'on détecte un nouveau jour (depuis minuit) : today ← tomorrow, demain → vert (en attente)
  if (last_rollover_day != current_day) {
    tempo_rollover();
    last_rollover_day = current_day;
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
    // Forcer un fetch au prochain tempo_update() ; demain = vert jusqu'à réponse EDF
    last_tempo_fetch = 0;
    tempo_tomorrow_pending = true;
    tempo_tomorrow_color = "";
  }
}

void tempo_loadConfig(Preferences &prefs) {
  tempo_enabled = prefs.getBool(PREF_TEMPO_ENABLED, false);
  if (!tempo_enabled) {
    tempo_today_color = "";
    tempo_tomorrow_color = "";
    tempo_tomorrow_pending = false;
  } else {
    // Au démarrage : demain = vert (en attente) tant qu'EDF n'a pas donné la couleur
    tempo_tomorrow_pending = true;
    tempo_tomorrow_color = "";
    addLog("[TEMPO] Demarrage: demain = vert (en attente)");
  }
  tempo_init();
}

void tempo_saveConfig(Preferences &prefs) {
  prefs.putBool(PREF_TEMPO_ENABLED, tempo_enabled);
}
