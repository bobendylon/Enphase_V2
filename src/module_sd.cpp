// MODULE SD - Implémentation

#include "module_sd.h"
#include <WiFi.h>
#include <time.h>

// Pins SD (définis dans config.h mais on les redéfinit ici pour éviter les conflits)
// Si vous modifiez les pins dans config.h, modifiez-les aussi ici
#ifndef SD_CS
#define SD_CS 2
#endif
#ifndef SD_MOSI
#define SD_MOSI 1
#endif
#ifndef SD_MISO
#define SD_MISO 40
#endif
#ifndef SD_SCK
#define SD_SCK 41
#endif

// Déclarations externes
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// Variables exposées (définitions)
bool sd_initialized = false;
bool sd_available = false;
String sd_last_error = "";

// Variables internes (static)
static SPIClass* sd_spi = nullptr;

// ============================================
// SD - Initialisation
// ============================================

void sd_init() {
  sd_available = false;
  sd_last_error = "";
  
  addLogf("[SD] Initialisation - CS:%d MOSI:%d MISO:%d SCK:%d", SD_CS, SD_MOSI, SD_MISO, SD_SCK);
  
  // Initialiser le bus SPI pour la SD
  // Utiliser SPI2 (HSPI) pour éviter conflit avec l'écran
  sd_spi = new SPIClass(HSPI);
  
  // Configuration des pins SPI
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH); // CS inactif (HIGH)
  
  addLog("[SD] Configuration des pins SPI...");
  sd_spi->begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  
  // Configuration SPI pour la carte SD
  // Essayer différentes fréquences pour trouver celle qui fonctionne
  bool sdOk = false;
  
  // Essai 1 : Fréquence très basse (1 MHz) pour test
  addLog("[SD] Essai 1: Fréquence 1 MHz...");
  sd_spi->setFrequency(1000000); // 1 MHz
  sd_spi->setDataMode(SPI_MODE0);
  delay(200);
  
  if (SD.begin(SD_CS, *sd_spi)) {
    sdOk = true;
    addLog("[SD] ✅ Initialisation réussie à 1 MHz");
  } else {
    // Essai 2 : Fréquence moyenne (4 MHz)
    addLog("[SD] Essai 1 échoué, essai 2: Fréquence 4 MHz...");
    sd_spi->setFrequency(4000000); // 4 MHz
    delay(200);
    
    if (SD.begin(SD_CS, *sd_spi)) {
      sdOk = true;
      addLog("[SD] ✅ Initialisation réussie à 4 MHz");
    } else {
      // Essai 3 : Fréquence standard (10 MHz)
      addLog("[SD] Essai 2 échoué, essai 3: Fréquence 10 MHz...");
      sd_spi->setFrequency(10000000); // 10 MHz
      delay(200);
      
      if (SD.begin(SD_CS, *sd_spi)) {
        sdOk = true;
        addLog("[SD] ✅ Initialisation réussie à 10 MHz");
      } else {
        addLog("[SD] ❌ Tous les essais d'initialisation ont échoué");
      }
    }
  }
  
  if (!sdOk) {
    sd_last_error = "Échec initialisation - Vérifiez pins et connexions";
    addLog("[SD] ERREUR: " + sd_last_error);
    addLog("[SD] Pins utilisés: CS=" + String(SD_CS) + " MOSI=" + String(SD_MOSI) + " MISO=" + String(SD_MISO) + " SCK=" + String(SD_SCK));
    addLog("[SD] Solutions possibles:");
    addLog("[SD] 1. Vérifiez que la carte est bien insérée (clic audible)");
    addLog("[SD] 2. Testez les autres configurations de pins dans config.h");
    addLog("[SD] 3. Vérifiez les connexions physiques");
    addLog("[SD] 4. Essayez une carte ≤32 Go si disponible");
    sd_available = false;
    sd_initialized = true;
    return;
  }
  
  addLog("[SD] SD.begin() réussi, vérification du type...");
  
  // Vérifier le type de carte
  uint8_t cardType = SD.cardType();
  addLogf("[SD] Type de carte détecté: %d (0=None, 1=MMC, 2=SD, 3=SDHC)", cardType);
  
  if (cardType == CARD_NONE) {
    sd_last_error = "Aucune carte SD détectée (CARD_NONE)";
    addLog("[SD] ERREUR: " + sd_last_error);
    addLog("[SD] Vérifiez que la carte est bien insérée et formatée en FAT32");
    sd_available = false;
    sd_initialized = true;
    return;
  }
  
  // Carte détectée
  sd_available = true;
  sd_initialized = true;
  
  // Afficher les infos de la carte
  uint64_t cardSize = SD.cardSize() / (1024 * 1024); // Taille en MB
  uint64_t totalBytes = SD.totalBytes() / (1024 * 1024);
  uint64_t usedBytes = SD.usedBytes() / (1024 * 1024);
  
  String typeStr = "Inconnu";
  if (cardType == CARD_MMC) typeStr = "MMC";
  else if (cardType == CARD_SD) typeStr = "SD";
  else if (cardType == CARD_SDHC) typeStr = "SDHC/SDXC"; // SDHC inclut aussi SDXC sur ESP32
  else typeStr = "Type " + String(cardType);
  
  addLogf("[SD] ✅ Carte détectée - Type: %s, Taille: %llu MB", typeStr.c_str(), cardSize);
  addLogf("[SD] Espace: %llu MB utilisé / %llu MB total", usedBytes, totalBytes);
  
  // Créer le dossier stats s'il n'existe pas
  if (!SD.exists("/stats")) {
    if (SD.mkdir("/stats")) {
      addLog("[SD] Dossier /stats créé");
    } else {
      addLog("[SD] ERREUR: Impossible de créer /stats");
    }
  } else {
    addLog("[SD] Dossier /stats existe déjà");
  }
}

// ============================================
// SD - Vérification disponibilité
// ============================================

bool sd_isAvailable() {
  if (!sd_initialized) {
    return false;
  }
  
  // Vérifier que la carte est toujours présente
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    sd_available = false;
    return false;
  }
  
  sd_available = true;
  return true;
}

// ============================================
// SD - Chemin fichier stats
// ============================================

String sd_getStatsPath(String date) {
  // Format: /stats/YYYY-MM-DD.csv
  return "/stats/" + date + ".csv";
}

// ============================================
// SD - Sauvegarde stats journalières
// ============================================

bool sd_saveStatsDaily(String date, float* histoProd, float* histoConso) {
  if (!sd_isAvailable()) {
    sd_last_error = "Carte SD non disponible";
    return false;
  }
  
  String filepath = sd_getStatsPath(date);
  File file = SD.open(filepath, FILE_WRITE);
  
  if (!file) {
    sd_last_error = "Impossible d'ouvrir " + filepath + " en écriture";
    addLog("[SD] ERREUR: " + sd_last_error);
    return false;
  }
  
  // En-tête CSV
  file.println("Heure,Production_W,Consommation_W");
  
  // Données horaires (24h)
  for (int i = 0; i < 24; i++) {
    file.printf("%d,%.2f,%.2f\n", i, histoProd[i], histoConso[i]);
  }
  
  file.close();
  addLogf("[SD] Stats sauvegardées: %s", filepath.c_str());
  return true;
}

// ============================================
// SD - Chargement stats journalières
// ============================================

bool sd_loadStatsDaily(String date, float* histoProd, float* histoConso) {
  if (!sd_isAvailable()) {
    sd_last_error = "Carte SD non disponible";
    return false;
  }
  
  String filepath = sd_getStatsPath(date);
  
  if (!SD.exists(filepath)) {
    sd_last_error = "Fichier " + filepath + " introuvable";
    return false;
  }
  
  File file = SD.open(filepath, FILE_READ);
  if (!file) {
    sd_last_error = "Impossible d'ouvrir " + filepath + " en lecture";
    return false;
  }
  
  // Initialiser les tableaux
  for (int i = 0; i < 24; i++) {
    histoProd[i] = 0;
    histoConso[i] = 0;
  }
  
  // Lire l'en-tête (ignorer)
  String line = file.readStringUntil('\n');
  
  // Lire les données
  int hour = 0;
  while (file.available() && hour < 24) {
    line = file.readStringUntil('\n');
    line.trim();
    
    if (line.length() == 0) continue;
    
    // Parser CSV: heure,prod,conso
    int comma1 = line.indexOf(',');
    int comma2 = line.indexOf(',', comma1 + 1);
    
    if (comma1 > 0 && comma2 > 0) {
      int h = line.substring(0, comma1).toInt();
      float prod = line.substring(comma1 + 1, comma2).toFloat();
      float conso = line.substring(comma2 + 1).toFloat();
      
      if (h >= 0 && h < 24) {
        histoProd[h] = prod;
        histoConso[h] = conso;
        hour++;
      }
    }
  }
  
  file.close();
  addLogf("[SD] Stats chargées: %s (%d heures)", filepath.c_str(), hour);
  return true;
}

// ============================================
// SD - Liste des dates disponibles
// ============================================

bool sd_getStatsDates(String* dates, int maxDates) {
  if (!sd_isAvailable()) {
    return false;
  }
  
  File root = SD.open("/stats");
  if (!root) {
    return false;
  }
  
  if (!root.isDirectory()) {
    root.close();
    return false;
  }
  
  int count = 0;
  File file = root.openNextFile();
  
  while (file && count < maxDates) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      // Extraire la date du nom de fichier (YYYY-MM-DD.csv)
      if (filename.endsWith(".csv")) {
        String date = filename.substring(0, filename.length() - 4);
        dates[count] = date;
        count++;
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  return (count > 0);
}

// ============================================
// SD - Suppression stats anciennes
// ============================================

bool sd_deleteOldStats(int daysToKeep) {
  if (!sd_isAvailable()) {
    return false;
  }
  
  time_t now = time(NULL);
  if (now < 946684800) { // NTP pas synchronisé
    addLog("[SD] ERREUR: Heure système non synchronisée");
    return false;
  }
  
  File root = SD.open("/stats");
  if (!root || !root.isDirectory()) {
    return false;
  }
  
  int deleted = 0;
  File file = root.openNextFile();
  
  while (file) {
    if (!file.isDirectory()) {
      String filename = String(file.name());
      if (filename.endsWith(".csv")) {
        String dateStr = filename.substring(0, filename.length() - 4);
        
        // Parser la date YYYY-MM-DD
        int year = dateStr.substring(0, 4).toInt();
        int month = dateStr.substring(5, 7).toInt();
        int day = dateStr.substring(8, 10).toInt();
        
        struct tm fileDate = {0};
        fileDate.tm_year = year - 1900;
        fileDate.tm_mon = month - 1;
        fileDate.tm_mday = day;
        time_t fileTime = mktime(&fileDate);
        
        // Calculer l'âge en jours
        double diffSeconds = difftime(now, fileTime);
        int ageDays = (int)(diffSeconds / 86400);
        
        if (ageDays > daysToKeep) {
          String filepath = "/stats/" + filename;
          if (SD.remove(filepath)) {
            deleted++;
            addLogf("[SD] Fichier supprimé: %s (âge: %d jours)", filepath.c_str(), ageDays);
          }
        }
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  addLogf("[SD] %d fichiers anciens supprimés", deleted);
  return true;
}

