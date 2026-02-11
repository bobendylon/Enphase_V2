/*
 * Module EDF TEMPO - Couleur du jour (Bleu / Blanc / Rouge)
 * API: https://www.api-couleur-tempo.fr
 * Récupération au démarrage puis toutes les 30 min. À minuit, Tomorrow → vert (en attente).
 */

#ifndef MODULE_TEMPO_H
#define MODULE_TEMPO_H

#include <Preferences.h>
#include <time.h>

// Option activable (NVS)
extern bool tempo_enabled;

// Couleurs du jour (libellé API: "Bleu", "Blanc", "Rouge") ou "" si non connu
extern String tempo_today_color;
extern String tempo_tomorrow_color;

// true après rollover minuit jusqu'à obtention du nouveau Tomorrow → affichage vert
extern bool tempo_tomorrow_pending;

// Heure de la dernière récupération réussie (time_t, 0 si jamais)
extern time_t tempo_last_fetch_time;

void tempo_init(void);
void tempo_update(void);   // À appeler dans loop() : fetch 30 min + rollover minuit
void tempo_setEnabled(bool enabled);
void tempo_loadConfig(Preferences &prefs);
void tempo_saveConfig(Preferences &prefs);

#endif
