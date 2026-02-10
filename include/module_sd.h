/*
 * MODULE SD - MSunPV Monitor V13
 * Gestion de la carte SD pour sauvegarde des statistiques
 */

#ifndef MODULE_SD_H
#define MODULE_SD_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// Variables exposées (extern) - État SD
extern bool sd_initialized;
extern bool sd_available;
extern String sd_last_error;

// Fonctions publiques
void sd_init();                                    // Initialisation de la carte SD
bool sd_isAvailable();                             // Vérifie si la SD est disponible
bool sd_saveStatsDaily(String date, float* histoProd, float* histoConso);  // Sauvegarde stats journalières
bool sd_loadStatsDaily(String date, float* histoProd, float* histoConso);   // Charge stats journalières
bool sd_getStatsDates(String* dates, int maxDates); // Liste les dates disponibles
bool sd_deleteOldStats(int daysToKeep);            // Supprime les stats anciennes (> X jours)
String sd_getStatsPath(String date);               // Retourne le chemin du fichier stats pour une date

#endif












