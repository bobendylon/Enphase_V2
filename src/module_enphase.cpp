// MODULE ENPHASE - Implémentation

#include "module_enphase.h"
#include <WiFi.h>
#include <time.h>

// Constantes NVS (évite d'inclure config.h qui contient des définitions)
#define PREF_ENPHASE_IP "enphase_ip"
#define PREF_ENPHASE_USER "enphase_user"
#define PREF_ENPHASE_PWD "enphase_pwd"
#define PREF_ENPHASE_SERIAL "enphase_serial"
#define PREF_ENPHASE_TOKEN "enphase_token"
#define PREF_ENPHASE_TOKEN_TS "enphase_token_ts"
#define PREF_NAMESPACE "msunpv"

// Constantes
#define ENPHASE_UPDATE_INTERVAL 10000    // 10 secondes
#define TOKEN_VALIDITY_DAYS 30           // Token valide 30 jours

// Déclarations externes
extern Preferences preferences;
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// Variables exposées (définitions)
String config_enphase_ip = "";
String config_enphase_user = "";
String config_enphase_pwd = "";
String config_enphase_serial = "";

// Données Enphase - Puissances instantanées
float enphase_pact_conso = 0;        // Consommation active (W)
float enphase_pact_prod = 0;         // Production active (W)
float enphase_pact_grid = 0;         // Puissance réseau (W)
float enphase_pva_conso = 0;         // VA consommation
float enphase_pva_prod = 0;          // VA production
float enphase_tension = 0;           // Tension (V)
float enphase_intensite = 0;         // Intensité (A)

// Données Enphase - Énergies du jour
float enphase_energy_produced = 0;   // Energie produite (Wh)
float enphase_energy_injected = 0;   // Energie injectée au réseau (Wh)
float enphase_energy_consumed = 0;   // Energie consommée (Wh)
float enphase_energy_imported = 0;   // Energie importée du réseau (Wh)

// Statut Enphase
String enphase_status = "Non configuré";
String enphase_last_error = "";
bool enphase_connected = false;
unsigned long enphase_last_success = 0;

// Variables internes (static)
static WiFiClientSecure* clientEnphase = nullptr;

// Token et timestamp
static String enphaseToken = "";
static unsigned long tokenTimestamp = 0;

// Timers
static unsigned long lastEnphaseUpdate = 0;

// ============================================
// ENPHASE - Parsing JSON manuel
// ============================================

static float extractJsonFloat(String key, String json) {
    int p = json.indexOf("\"" + key + "\":");
    if (p == -1) return 0.0;
    json = json.substring(p);
    p = json.indexOf(":");
    json = json.substring(p + 1);
    int q = json.indexOf(",");
    int r = json.indexOf("}");
    p = (q != -1 && q < r) ? q : r;
    if (p > 0) {
        json = json.substring(0, p);
        json.trim();
        return json.toFloat();
    }
    return 0.0;
}

static String extractJsonString(String key, String json) {
    int p = json.indexOf("\"" + key + "\":\"");
    if (p == -1) return "";
    p = json.indexOf("\"", p + key.length() + 3);
    if (p == -1) return "";
    int q = json.indexOf("\"", p + 1);
    if (q == -1) return "";
    return json.substring(p + 1, q);
}

static String extractJsonSection(String key, String json) {
    int pos = json.indexOf("\"" + key + "\"");
    if (pos == -1) return "";
    json = json.substring(pos);
    int start = json.indexOf("{");
    if (start == -1) return "";
    return json.substring(start, start + 500);
}

// ============================================
// ENPHASE - Requête HTTPS vers Envoy
// ============================================

static String enphaseHttpsRequest(String endpoint, bool useToken = false) {
    if (!clientEnphase) return "";
    
    clientEnphase->setInsecure();
    
    if (!clientEnphase->connect(config_enphase_ip.c_str(), 443)) {
        addLog("[Enphase] ❌ Connexion HTTPS échouée vers " + config_enphase_ip);
        return "";
    }
    
    addLog("[Enphase] 🔗 Requête: https://" + config_enphase_ip + endpoint);
    
    clientEnphase->println("GET " + endpoint + " HTTP/1.1");
    clientEnphase->println("Host: " + config_enphase_ip);
    clientEnphase->println("Accept: application/json");
    
    if (useToken && enphaseToken.length() > 50) {
        clientEnphase->println("Authorization: Bearer " + enphaseToken);
    }
    
    clientEnphase->println("Connection: close");
    clientEnphase->println();
    
    // Lecture réponse
    String response = "";
    bool headersReceived = false;
    unsigned long timeout = millis();
    
    while ((clientEnphase->connected() || clientEnphase->available()) && (millis() - timeout < 5000)) {
        if (clientEnphase->available()) {
            String line = clientEnphase->readStringUntil('\n');
            
            if (line == "\r") {
                headersReceived = true;
                continue;
            }
            
            if (headersReceived) {
                response += line;
            }
        }
    }
    
    clientEnphase->stop();
    
    if (response.length() < 50) {
        addLog("[Enphase] ⚠️ Réponse vide ou invalide");
        return "";
    }
    
    return response;
}

// ============================================
// ENPHASE - Test connexion Envoy
// ============================================

static bool testEnvoyConnection() {
    if (!clientEnphase) return false;
    
    addLog("[Enphase] 🧪 Test connexion vers " + config_enphase_ip);
    
    clientEnphase->setInsecure();
    
    if (!clientEnphase->connect(config_enphase_ip.c_str(), 443)) {
        addLog("[Enphase] ❌ Envoy non joignable (timeout)");
        return false;
    }
    
    clientEnphase->println("GET /home HTTP/1.1");
    clientEnphase->println("Host: " + config_enphase_ip);
    clientEnphase->println("Connection: close");
    clientEnphase->println();
    
    String response = "";
    unsigned long timeout = millis();
    
    while (clientEnphase->connected() && (millis() - timeout < 3000)) {
        if (clientEnphase->available()) {
            response = clientEnphase->readStringUntil('\n');
            break;
        }
    }
    
    clientEnphase->stop();
    
    bool success = (response.indexOf("200") >= 0 || response.indexOf("HTTP/1") >= 0);
    
    if (success) {
        addLog("[Enphase] ✅ Envoy répond correctement");
    } else {
        addLog("[Enphase] ❌ Réponse invalide: " + response);
    }
    
    return success;
}

// ============================================
// ENPHASE - Authentification (Token Bearer)
// ============================================

static bool getEnphaseToken() {
    // Vérifier si on a déjà un token valide
    if (enphaseToken.length() > 50 && tokenTimestamp > 0) {
        unsigned long tokenAge = (millis() - tokenTimestamp) / 1000; // en secondes
        
        if (tokenAge < (TOKEN_VALIDITY_DAYS * 24 * 3600)) {
            addLogf("[Enphase] ✅ Token existant valide (âge: %lu jours)", tokenAge / 86400);
            return true;
        } else {
            addLog("[Enphase] ⚠️ Token expiré, renouvellement...");
        }
    }
    
    // Si pas de credentials, mode sans authentification (V5)
    if (config_enphase_user.length() == 0 || config_enphase_pwd.length() == 0) {
        addLog("[Enphase] ℹ️ Mode sans authentification (Firmware V5)");
        enphaseToken = "";
        return true;
    }
    
    addLog("[Enphase] 🔐 Authentification Enlighten...");
    
    // Étape 1: Session ID
    WiFiClientSecure client1;
    client1.setInsecure();
    
    if (!client1.connect("enlighten.enphaseenergy.com", 443)) {
        addLog("[Enphase] ❌ Connexion Enlighten échouée");
        return false;
    }
    
    String loginData = "user[email]=" + config_enphase_user + "&user[password]=" + config_enphase_pwd;
    String request = "POST /login/login.json?" + loginData + " HTTP/1.1\r\n";
    request += "Host: enlighten.enphaseenergy.com\r\n";
    request += "Connection: close\r\n\r\n";
    
    client1.print(request);
    
    String response1 = "";
    while (client1.connected() || client1.available()) {
        if (client1.available()) {
            response1 += client1.readString();
        }
    }
    client1.stop();
    
    String sessionId = extractJsonString("session_id", response1);
    
    if (sessionId.length() < 10) {
        addLog("[Enphase] ❌ Session ID non obtenu");
        addLog("Réponse: " + response1.substring(0, 200));
        return false;
    }
    
    addLog("[Enphase] ✅ Session ID: " + sessionId.substring(0, 10) + "...");
    
    // Étape 2: Token
    WiFiClientSecure client2;
    client2.setInsecure();
    
    if (!client2.connect("entrez.enphaseenergy.com", 443)) {
        addLog("[Enphase] ❌ Connexion Entrez échouée");
        return false;
    }
    
    String tokenPayload = "{\"session_id\":\"" + sessionId + "\",\"serial_num\":\"" + config_enphase_serial + "\",\"username\":\"" + config_enphase_user + "\"}";
    
    client2.println("POST /tokens HTTP/1.1");
    client2.println("Host: entrez.enphaseenergy.com");
    client2.println("Content-Type: application/json");
    client2.println("Content-Length: " + String(tokenPayload.length()));
    client2.println("Connection: close");
    client2.println();
    client2.println(tokenPayload);
    
    String response2 = "";
    bool headersReceived = false;
    
    while (client2.connected() || client2.available()) {
        if (client2.available()) {
            String line = client2.readStringUntil('\n');
            if (line == "\r") headersReceived = true;
            if (headersReceived) response2 += line;
        }
    }
    client2.stop();
    
    response2.trim();
    
    if (response2.length() < 50) {
        addLog("[Enphase] ❌ Token non obtenu");
        addLog("Réponse: " + response2);
        return false;
    }
    
    enphaseToken = response2;
    tokenTimestamp = millis();
    
    // Sauvegarder dans NVS
    preferences.begin(PREF_NAMESPACE, false);
    preferences.putString(PREF_ENPHASE_TOKEN, enphaseToken);
    preferences.putULong(PREF_ENPHASE_TOKEN_TS, tokenTimestamp);
    preferences.end();
    
    addLog("[Enphase] ✅ Token obtenu (valide 30 jours)");
    addLog("Token: " + enphaseToken.substring(0, 30) + "...");
    
    return true;
}

// ============================================
// ENPHASE - Mise à jour données temps réel
// ============================================

void enphase_update() {
    if (config_enphase_ip.length() < 7) return;
    
    unsigned long currentMillis = millis();
    if (currentMillis - lastEnphaseUpdate < ENPHASE_UPDATE_INTERVAL) return;
    lastEnphaseUpdate = currentMillis;
    
    addLog("[Enphase] Mise a jour donnees...");
    
    // 1. Données temps réel (livedata)
    String livedata = enphaseHttpsRequest("/ivp/livedata/status", true);
    addLog("[Enphase] Reponse livedata recue, taille=" + String(livedata.length()) + " octets");
    
    if (livedata.length() > 100) {
        // Production PV (en milliwatts, négatif dans l'API)
        int pvIdx = livedata.indexOf("\"pv\"");
        if (pvIdx != -1) {
            String pvSection = livedata.substring(pvIdx, pvIdx + 300);
            float prodMw = extractJsonFloat("agg_p_mw", pvSection);
            enphase_pact_prod = abs(prodMw) / 1000.0;
        }
        
        // Réseau (+ soutirage / - injection)
        int gridIdx = livedata.indexOf("\"grid\"");
        if (gridIdx != -1) {
            String gridSection = livedata.substring(gridIdx, gridIdx + 300);
            float gridMw = extractJsonFloat("agg_p_mw", gridSection);
            enphase_pact_grid = gridMw / 1000.0;
        }
        
        // Consommation (charge)
        int loadIdx = livedata.indexOf("\"load\"");
        if (loadIdx != -1) {
            String loadSection = livedata.substring(loadIdx, loadIdx + 300);
            float loadMw = extractJsonFloat("agg_p_mw", loadSection);
            enphase_pact_conso = loadMw / 1000.0;
        }
        
        enphase_connected = true;
        enphase_status = "✅ Connecté";
        enphase_last_error = "";
        enphase_last_success = millis();
        
        addLog("[Enphase] Prod:" + String(enphase_pact_prod,1) + "W Conso:" + String(enphase_pact_conso,1) + "W Reseau:" + String(enphase_pact_grid,1) + "W");
    } else {
        addLog("[Enphase] ERREUR - Echec recuperation livedata");
        enphase_connected = false;
        enphase_status = "❌ Erreur lecture données";
        return;
    }
    
    // 2. Énergies du jour
    String energydata = enphaseHttpsRequest("/ivp/pdm/energy", true);
    
    addLog("[DEBUG] === DEBUT EXTRACTION ENERGIES ===");
    addLog("[DEBUG] Taille reponse energydata: " + String(energydata.length()));
    
    if (energydata.length() > 100) {
        addLog("[DEBUG] Reponse energydata (premiers 500 chars): " + energydata.substring(0, 500));
        
        // Production jour
        int prodIdx = energydata.indexOf("\"production\"");
        addLog("[DEBUG] Index de 'production': " + String(prodIdx));
        
        if (prodIdx != -1) {
            // Extraire une section plus large pour être sûr d'avoir tout
            String prodSection = energydata.substring(prodIdx);
            // Limiter à 1000 caractères pour éviter les problèmes de mémoire
            if (prodSection.length() > 1000) {
                prodSection = prodSection.substring(0, 1000);
            }
            addLog("[DEBUG] Section production extraite (premiers 800 chars): " + prodSection.substring(0, 800));
            
            // Essayer d'abord "eim" (Envoy IQ Metering - plus précis)
            int eimIdx = prodSection.indexOf("\"eim\"");
            addLog("[DEBUG] Index de 'eim' dans production: " + String(eimIdx));
            
            if (eimIdx != -1) {
                String eimSection = prodSection.substring(eimIdx);
                // Prendre plus de caractères pour être sûr d'avoir la valeur complète
                if (eimSection.length() > 300) {
                    eimSection = eimSection.substring(0, 300);
                }
                addLog("[DEBUG] Section eim extraite: " + eimSection);
                
                // Chercher la clé exacte
                int keyIdx = eimSection.indexOf("\"wattHoursToday\"");
                addLog("[DEBUG] Index de 'wattHoursToday' dans eim: " + String(keyIdx));
                
                if (keyIdx != -1) {
                    enphase_energy_produced = extractJsonFloat("wattHoursToday", eimSection);
                    addLog("[DEBUG] Production wattHoursToday extrait (eim): " + String(enphase_energy_produced, 0) + " Wh");
                } else {
                    addLog("[ERREUR] Clé 'wattHoursToday' NON TROUVEE dans eim !");
                    // Afficher toutes les clés disponibles pour diagnostic
                    addLog("[DEBUG] Contenu eim complet pour diagnostic: " + eimSection);
                }
            } else {
                addLog("[ERREUR] Section 'eim' NON TROUVEE dans production !");
                // Essayer "pcu" comme fallback
                int pcuIdx = prodSection.indexOf("\"pcu\"");
                addLog("[DEBUG] Tentative avec 'pcu', index: " + String(pcuIdx));
                if (pcuIdx != -1) {
                    String pcuSection = prodSection.substring(pcuIdx);
                    if (pcuSection.length() > 300) {
                        pcuSection = pcuSection.substring(0, 300);
                    }
                    addLog("[DEBUG] Section pcu extraite: " + pcuSection);
                    enphase_energy_produced = extractJsonFloat("wattHoursToday", pcuSection);
                    addLog("[DEBUG] Production wattHoursToday extrait (pcu): " + String(enphase_energy_produced, 0) + " Wh");
                }
            }
        } else {
            addLog("[ERREUR] Section 'production' NON TROUVEE dans energydata !");
            // Afficher le début de la réponse pour diagnostic
            addLog("[DEBUG] Debut de energydata: " + energydata.substring(0, 200));
        }
        
        // Consommation jour
        int consIdx = energydata.indexOf("\"consumption\"");
        if (consIdx != -1) {
            String consSection = energydata.substring(consIdx);
            if (consSection.length() > 500) {
                consSection = consSection.substring(0, 500);
            }
            int eimConsIdx = consSection.indexOf("\"eim\"");
            if (eimConsIdx != -1) {
                String eimConsSection = consSection.substring(eimConsIdx);
                if (eimConsSection.length() > 300) {
                    eimConsSection = eimConsSection.substring(0, 300);
                }
                enphase_energy_consumed = extractJsonFloat("wattHoursToday", eimConsSection);
                
                addLog("[DEBUG] Consommation wattHoursToday extrait (eim): " + String(enphase_energy_consumed, 0));
            }
        }
        
        // Energie injectée au réseau (depuis section "grid")
        int gridIdx = energydata.indexOf("\"grid\"");
        addLog("[DEBUG] Index de 'grid': " + String(gridIdx));
        
        if (gridIdx != -1) {
            String gridSection = energydata.substring(gridIdx);
            if (gridSection.length() > 1000) {
                gridSection = gridSection.substring(0, 1000);
            }
            addLog("[DEBUG] Section grid extraite (premiers 800 chars): " + gridSection.substring(0, 800));
            
            // Chercher "eim" dans grid pour l'énergie injectée
            int eimGridIdx = gridSection.indexOf("\"eim\"");
            addLog("[DEBUG] Index de 'eim' dans grid: " + String(eimGridIdx));
            
            if (eimGridIdx != -1) {
                String eimGridSection = gridSection.substring(eimGridIdx);
                if (eimGridSection.length() > 300) {
                    eimGridSection = eimGridSection.substring(0, 300);
                }
                addLog("[DEBUG] Section eim grid extraite: " + eimGridSection);
                
                // L'énergie injectée est généralement négative dans grid, on prend la valeur absolue
                float gridEnergy = extractJsonFloat("wattHoursToday", eimGridSection);
                // Si négatif, c'est de l'injection (on renvoie au réseau)
                if (gridEnergy < 0) {
                    enphase_energy_injected = abs(gridEnergy);
                    addLog("[DEBUG] Energie injectée extraite (grid eim, valeur negative): " + String(enphase_energy_injected, 0) + " Wh");
                } else {
                    // Si positif, c'est du soutirage, injection = 0
                    enphase_energy_injected = 0;
                    addLog("[DEBUG] Grid positif (soutirage), injection = 0");
                }
            } else {
                addLog("[ERREUR] Section 'eim' NON TROUVEE dans grid !");
                // Essayer "pcu" comme fallback
                int pcuGridIdx = gridSection.indexOf("\"pcu\"");
                if (pcuGridIdx != -1) {
                    String pcuGridSection = gridSection.substring(pcuGridIdx);
                    if (pcuGridSection.length() > 300) {
                        pcuGridSection = pcuGridSection.substring(0, 300);
                    }
                    float gridEnergy = extractJsonFloat("wattHoursToday", pcuGridSection);
                    if (gridEnergy < 0) {
                        enphase_energy_injected = abs(gridEnergy);
                        addLog("[DEBUG] Energie injectée extraite (grid pcu): " + String(enphase_energy_injected, 0) + " Wh");
                    } else {
                        enphase_energy_injected = 0;
                    }
                }
            }
        } else {
            addLog("[ERREUR] Section 'grid' NON TROUVEE dans energydata !");
        }
        
        // Calcul des valeurs Import/Export si non disponibles (Fallback Net Metering)
        // Si on n'a pas pu lire l'injection (pas de section grid), on l'estime
        if (gridIdx == -1) {
             if (enphase_energy_produced > enphase_energy_consumed) {
                 enphase_energy_injected = enphase_energy_produced - enphase_energy_consumed;
                 enphase_energy_imported = 0;
             } else {
                 enphase_energy_injected = 0;
                 enphase_energy_imported = enphase_energy_consumed - enphase_energy_produced;
             }
             addLog("[Enphase] Mode Calcul Net: Imp=" + String(enphase_energy_imported,0) + " Exp=" + String(enphase_energy_injected,0));
        } else {
             // Si on a lu l'injection, on déduit l'import (Net Calculation)
             // Import = Conso - Prod + Injection
             float calc_import = enphase_energy_consumed - enphase_energy_produced + enphase_energy_injected;
             if (calc_import < 0) calc_import = 0;
             enphase_energy_imported = calc_import;
        }

        addLog("[Enphase] Energies - Prod:" + String(enphase_energy_produced,0) + "Wh Inj:" + String(enphase_energy_injected,0) + "Wh Conso:" + String(enphase_energy_consumed,0) + "Wh Imp:" + String(enphase_energy_imported,0) + "Wh");
    } else {
        addLog("[ERREUR] Reponse energydata trop courte: " + String(energydata.length()) + " octets");
    }
}

// ============================================
// ENPHASE - Initialisation
// ============================================

void enphase_init(WiFiClientSecure* client) {
    clientEnphase = client;
    
    addLog("[Enphase] 🚀 Initialisation...");
    addLog("[Enphase] Initialisation systeme...");
    
    // Charger config depuis NVS
    preferences.begin(PREF_NAMESPACE, true);
    config_enphase_ip = preferences.getString(PREF_ENPHASE_IP, "");
    config_enphase_user = preferences.getString(PREF_ENPHASE_USER, "");
    config_enphase_pwd = preferences.getString(PREF_ENPHASE_PWD, "");
    config_enphase_serial = preferences.getString(PREF_ENPHASE_SERIAL, "");
    enphaseToken = preferences.getString(PREF_ENPHASE_TOKEN, "");
    tokenTimestamp = preferences.getULong(PREF_ENPHASE_TOKEN_TS, 0);
    preferences.end();
    
    if (config_enphase_ip.length() < 7) {
        addLog("[Enphase] ⚠️ Pas d'IP configurée");
        addLog("[Enphase] ATTENTION - Pas d'IP configuree");
        enphase_status = "Non configuré";
        enphase_connected = false;
        return;
    }
    
    addLog("[Enphase] ✅ Activé - IP: " + config_enphase_ip);
    addLog("[Enphase] IP configuree: " + config_enphase_ip);
    
    // Test connexion
    if (!testEnvoyConnection()) {
        addLog("[Enphase] ⚠️ Envoy non accessible au démarrage");
        addLog("[Enphase] ATTENTION - Envoy non accessible au demarrage");
        enphase_connected = false;
        enphase_status = "❌ Envoy non joignable";
        return;
    } else {
        addLog("[Enphase] Test connexion OK - Envoy repond");
    }
    
    // Obtenir/vérifier token
    if (!getEnphaseToken()) {
        addLog("[Enphase] ⚠️ Échec authentification");
        // Continue quand même, peut-être V5
    }
    
    addLog("[Enphase] ✅ Initialisation terminée");
}

// ============================================
// ENPHASE - Handlers Webserver
// ============================================

void enphase_handleWeb(WebServer* server) {
  bool hasEnphase = (config_enphase_ip.length() > 0);
  
  if (!hasEnphase) {
    String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'>
<title>Enphase Envoy - MSunPV</title>
<style>
body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;text-align:center}
h1{color:#fb923c;margin-top:50px}
.btn{background:#374151;color:#fff;padding:12px 24px;border-radius:8px;text-decoration:none;display:inline-block;margin-top:20px}
.btn:hover{background:#4b5563}
</style></head><body>
<h1>📡 Aucun Enphase Envoy configuré</h1>
<p style='color:#9ca3af'>Configurez l'IP et les identifiants dans la page Info.</p>
<a href='/info' class='btn'>⚙️ Configuration</a>
<a href='/' class='btn'>← Retour</a>
</body></html>)";
    server->send(200, "text/html", html);
    return;
  }
  
  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'>
<title>Enphase Envoy - MSunPV V12</title>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
  background: linear-gradient(135deg, #0c0a09 0%, #1c1917 100%);
  color: #fff;
  min-height: 100vh;
  padding: 20px;
}
.container { max-width: 1200px; margin: 0 auto; }
.header {
  text-align: center;
  padding: 20px 0;
  border-bottom: 2px solid rgba(251, 147, 60, 0.3);
  margin-bottom: 30px;
}
.header h1 { color: #fb923c; font-size: 2em; margin-bottom: 10px; }
.header p { color: #9ca3af; }
.grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 20px;
  margin-bottom: 30px;
}
.card {
  background: rgba(41, 37, 36, 0.8);
  border-radius: 16px;
  padding: 25px;
  border: 1px solid rgba(251, 147, 60, 0.25);
  backdrop-filter: blur(10px);
}
.card-title {
  font-size: 1.2em;
  color: #fb923c;
  font-weight: 600;
  margin-bottom: 20px;
  padding-bottom: 15px;
  border-bottom: 1px solid rgba(251, 147, 60, 0.2);
}
.metric {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #292524;
}
.metric:last-child { border-bottom: none; }
.metric-label { color: #9ca3af; font-size: 0.95em; font-weight: 500; }
.metric-value { font-size: 1.5em; font-weight: 700; color: #fb923c; }
.metric-unit { font-size: 0.8em; color: #d1d5db; margin-left: 5px; }
.status-ok { color: #22c55e; }
.status-error { color: #ef4444; }
.nav {
  display: flex;
  gap: 15px;
  justify-content: center;
  flex-wrap: wrap;
  margin-top: 30px;
}
.btn {
  background: #374151;
  color: #fff;
  padding: 12px 24px;
  border-radius: 8px;
  text-decoration: none;
  display: inline-block;
  font-weight: 500;
  transition: all 0.3s;
}
.btn:hover { background: #4b5563; }
@media (max-width: 768px) {
  body { padding: 12px; }
  .header { padding: 15px 0; margin-bottom: 20px; }
  .header h1 { font-size: 1.5em; }
  .grid { grid-template-columns: 1fr; gap: 15px; }
  .card { padding: 20px; }
  .card-title { font-size: 1.1em; }
  .metric-value { font-size: 1.3em; }
  .nav { gap: 10px; margin-top: 20px; }
  .btn { padding: 14px 20px; font-size: 0.9em; min-height: 44px; }
}
@media (max-width: 480px) {
  body { padding: 10px; }
  .header h1 { font-size: 1.3em; }
  .card { padding: 15px; }
  .card-title { font-size: 1em; margin-bottom: 15px; }
  .metric { padding: 10px 0; }
  .metric-label { font-size: 0.9em; }
  .metric-value { font-size: 1.2em; }
  .nav { flex-direction: column; }
  .btn { width: 100%; padding: 14px; }
}

/* FLOW DIAGRAM STYLES */
.flow-card {
  grid-column: 1 / -1; /* Prend toute la largeur si possible */
  position: relative;
  min-height: 400px;
  display: flex;
  justify-content: center;
  align-items: center;
  background: radial-gradient(circle at center, rgba(41, 37, 36, 0.9) 0%, rgba(28, 25, 23, 0.9) 100%);
}
.flow-container {
  position: relative;
  width: 320px;
  height: 380px;
}
.flow-node {
  position: absolute;
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100px;
  text-align: center;
}
.node-import { top: 0; left: 0; }
.node-prod { top: 0; right: 0; }
.node-conso { top: 120px; left: 50%; transform: translateX(-50%); }
.node-export { bottom: 0; left: 50%; transform: translateX(-50%); }

.icon-circle {
  width: 60px;
  height: 60px;
  border-radius: 50%;
  border: 2px solid;
  display: flex;
  align-items: center;
  justify-content: center;
  margin-bottom: 5px;
  background: rgba(0,0,0,0.5);
}
.icon-circle svg { width: 32px; height: 32px; }

/* Import */
.style-import .icon-circle { border-color: #9ca3af; color: #9ca3af; }
.style-import .val { color: #fff; }
.style-import .sub { color: #9ca3af; }

/* Prod */
.style-prod .icon-circle { border-color: #3b82f6; color: #3b82f6; }
.style-prod .val { color: #3b82f6; }
.style-prod .sub { color: #60a5fa; }

/* Conso (Central) */
.style-conso .icon-circle { 
  width: 100px; height: 100px; 
  border-width: 4px;
  border-color: #f97316; 
  color: #f97316;
}
.style-conso svg { width: 50px; height: 50px; }
.style-conso .val { color: #f97316; font-size: 1.5em; }

/* Export */
.style-export .icon-circle { border-color: #9ca3af; color: #9ca3af; }

.flow-val { font-weight: 700; font-size: 1.2em; }
.flow-sub { font-size: 0.8em; text-transform: uppercase; letter-spacing: 1px; }

/* Arrows (SVG connection lines) */
.flow-arrows {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  pointer-events: none;
  z-index: 0;
}
.arrow-path {
  fill: none;
  stroke: #4b5563;
  stroke-width: 2;
  transition: stroke 0.5s;
}
.arrow-active { stroke: #3b82f6; animation: dash 1s linear infinite; }
@keyframes dash {
  to { stroke-dashoffset: -20; }
}
</style>
</head>
<body>
<div class='container'>
  <div class='header'>
    <h1>📡 Monitoring Enphase Envoy</h1>
    <p>Surveillance passerelle Envoy Metered</p>
  </div>
  
  <div class='grid'>
    <!-- DIAGRAMME DE FLUX -->
    <div class='card flow-card'>
      <div class='flow-container'>
        <svg class='flow-arrows' viewBox='0 0 320 380'>
            <!-- Import -> Conso -->
            <path id='path-import' d='M 50 70 Q 50 170 105 170' class='arrow-path' marker-end='url(#arrowhead)'/>
            <!-- Prod -> Conso -->
            <path id='path-prod' d='M 270 70 Q 270 170 215 170' class='arrow-path' marker-end='url(#arrowhead-blue)'/>
            <!-- Conso -> Export -->
            <path id='path-export' d='M 160 230 L 160 290' class='arrow-path' marker-end='url(#arrowhead)'/>
            
            <defs>
                <marker id='arrowhead' markerWidth='10' markerHeight='7' refX='9' refY='3.5' orient='auto'>
                    <polygon points='0 0, 10 3.5, 0 7' fill='#6b7280'/>
                </marker>
                 <marker id='arrowhead-blue' markerWidth='10' markerHeight='7' refX='9' refY='3.5' orient='auto'>
                    <polygon points='0 0, 10 3.5, 0 7' fill='#3b82f6'/>
                </marker>
            </defs>
        </svg>

        <!-- Import Node -->
        <div class='flow-node node-import style-import'>
            <div class='icon-circle'>
                <svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><path d='M6 6h12M6 12h12M6 18h12M12 6v12'/><path d='M9 3l3-3 3 3'/></svg>
            </div>
            <div class='flow-val'><span id='flow-import'>0.0</span> kWh</div>
            <div class='flow-sub'>Importé</div>
        </div>

        <!-- Prod Node -->
        <div class='flow-node node-prod style-prod'>
            <div class='icon-circle'>
                <svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><circle cx='12' cy='12' r='5'/><path d='M12 1v2M12 21v2M4.22 4.22l1.42 1.42M18.36 18.36l1.42 1.42M1 12h2M21 12h2M4.22 19.78l1.42-1.42M18.36 5.64l1.42-1.42'/></svg>
            </div>
            <div class='flow-val'><span id='flow-prod'>0.0</span> kWh</div>
            <div class='flow-sub'>Produit</div>
        </div>

        <!-- Conso Node (Center) -->
        <div class='flow-node node-conso style-conso'>
            <div class='icon-circle'>
                <svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><path d='M3 9l9-7 9 7v11a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z'/><polyline points='9 22 9 12 15 12 15 22'/></svg>
            </div>
            <div class='flow-val'><span id='flow-conso'>0.0</span> kWh</div>
            <div class='flow-sub'>Consommé</div>
        </div>

        <!-- Export Node -->
        <div class='flow-node node-export style-export'>
            <div class='flow-val'><span id='flow-export'>0.0</span> kWh</div>
            <div class='flow-sub'>Exporté</div>
            <div class='icon-circle' style='margin-top:5px; border-color:#9ca3af; color:#9ca3af;'>
                 <svg viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><path d='M6 6h12M6 12h12M6 18h12M12 21V9'/><path d='M9 21l3 3 3-3'/></svg>
            </div>
        </div>
      </div>
    </div>
    <div class='card'>
      <div class='card-title'>🔌 Statut Connexion</div>
      <div class='metric'>
        <span class='metric-label'>État</span>
        <span id='status-text' class='metric-value' style='color: #ef4444;'>❌ Non connecté</span>
      </div>
      <div class='metric'>
        <span class='metric-label'>IP</span>
        <span class='metric-value' style='color: #d1d5db; font-size: 1em;'>)" + config_enphase_ip + R"(</span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Dernière mise à jour</span>
        <span class='metric-value' style='color: #d1d5db; font-size: 0.95em;' id='last-update'>Jamais</span>
      </div>
      <div style='margin-top: 15px; text-align: center;'>
        <button class='btn' onclick='testConnection()' style='width: 100%; cursor: pointer;'>🔍 Tester Connexion</button>
      </div>
    </div>
    
    <div class='card'>
      <div class='card-title'>⚡ Puissance Instantanée</div>
      <div class='metric'>
        <span class='metric-label'>Prod. Solaire</span>
        <span><span class='metric-value' id='pact-prod'>0</span><span class='metric-unit'>W</span></span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Consommation</span>
        <span><span class='metric-value' id='pact-conso'>0</span><span class='metric-unit'>W</span></span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Enedis</span>
        <span><span class='metric-value' id='pact-grid'>0</span><span class='metric-unit'>W</span></span>
      </div>
    </div>
    
    <div class='card'>
      <div class='card-title'>📊 Énergie Jour</div>
      <div class='metric'>
        <span class='metric-label'>Importé</span>
        <span><span class='metric-value' id='energy-imported'>0</span><span class='metric-unit'>kWh</span></span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Produit</span>
        <span><span class='metric-value' style='color: #3b82f6;' id='energy-produced'>0</span><span class='metric-unit'>kWh</span></span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Consommé</span>
        <span><span class='metric-value' id='energy-consumed'>0</span><span class='metric-unit'>kWh</span></span>
      </div>
       <div class='metric'>
        <span class='metric-label'>Exporté</span>
        <span><span class='metric-value' id='energy-injected'>0</span><span class='metric-unit'>kWh</span></span>
      </div>
    </div>
    
    <!-- CARTE ÉLECTRIQUE - Commentée (données non récupérées actuellement)
    <div class='card'>
      <div class='card-title'>🔌 Électrique</div>
      <div class='metric'>
        <span class='metric-label'>Tension</span>
        <span><span class='metric-value' id='tension'>0</span><span class='metric-unit'>V</span></span>
      </div>
      <div class='metric'>
        <span class='metric-label'>Intensité</span>
        <span><span class='metric-value' id='intensite'>0</span><span class='metric-unit'>A</span></span>
      </div>
    </div>
  </div>
  -->
  <div class='nav'>
    <a href='/info' class='btn'>⚙️ Configuration</a>
    <a href='/' class='btn'>← Dashboard</a>
  </div>
</div>

<script>
function updateData() {
  fetch('/enphaseData')
    .then(r => r.json())
    .then(d => {
      document.getElementById('pact-prod').textContent = d.pact_prod.toFixed(0);
      document.getElementById('pact-conso').textContent = d.pact_conso.toFixed(0);
      document.getElementById('pact-grid').textContent = d.pact_grid.toFixed(0);
      document.getElementById('energy-imported').textContent = (d.energy_imported / 1000).toFixed(1);
      document.getElementById('energy-produced').textContent = (d.energy_produced / 1000).toFixed(1);
      document.getElementById('energy-injected').textContent = (d.energy_injected / 1000).toFixed(1);
      document.getElementById('energy-injected').textContent = (d.energy_injected / 1000).toFixed(1);
      document.getElementById('energy-consumed').textContent = (d.energy_consumed / 1000).toFixed(1);
      
      // Update Flow Diagram
      document.getElementById('flow-import').textContent = (d.energy_imported / 1000).toFixed(1);
      document.getElementById('flow-prod').textContent   = (d.energy_produced / 1000).toFixed(1);
      document.getElementById('flow-conso').textContent = (d.energy_consumed / 1000).toFixed(1);
      document.getElementById('flow-export').textContent = (d.energy_injected / 1000).toFixed(1);
      
      // Animation simple arrows based on values
      document.getElementById('path-import').style.stroke = (d.energy_imported > 0) ? '#9ca3af' : '#333';
      document.getElementById('path-prod').style.stroke = (d.energy_produced > 0) ? '#3b82f6' : '#333';
      document.getElementById('path-export').style.stroke = (d.energy_injected > 0) ? '#9ca3af' : '#333';
      // Électrique - Commenté (non utilisé)
      //document.getElementById('tension').textContent = d.tension.toFixed(1);
      //document.getElementById('intensite').textContent = d.intensite.toFixed(1);
      
      fetch('/enphaseStatus')
        .then(r => r.json())
        .then(s => {
          const statusEl = document.getElementById('status-text');
          if (s.connected) {
            statusEl.textContent = '✅ ' + s.status;
            statusEl.style.color = '#22c55e';
          } else {
            statusEl.textContent = s.status;
            statusEl.style.color = '#ef4444';
          }
          
          if (s.last_update > 0) {
            const now = Math.floor(Date.now() / 1000);
            const diff = now - s.last_update;
            const lastUpEl = document.getElementById('last-update');
            if (diff < 60) {
              lastUpEl.textContent = 'À l\'instant';
            } else if (diff < 3600) {
              lastUpEl.textContent = 'Il y a ' + Math.floor(diff / 60) + 'min';
            } else {
              lastUpEl.textContent = 'Il y a ' + Math.floor(diff / 3600) + 'h';
            }
          }
        });
    })
    .catch(e => console.error('Erreur:', e));
}

function testConnection() {
  const btn = event.target;
  btn.disabled = true;
  btn.textContent = '⏳ Test en cours...';
  
  fetch('/enphaseTest', { method: 'POST' })
    .then(r => r.json())
    .then(result => {
      const statusEl = document.getElementById('status-text');
      if (result.success) {
        statusEl.textContent = '✅ Connexion OK!';
        statusEl.style.color = '#22c55e';
        alert('✅ Connexion à l\'Envoy réussie!\n\n' + result.message);
      } else {
        statusEl.textContent = '❌ ' + result.message;
        statusEl.style.color = '#ef4444';
        alert('❌ Erreur de connexion:\n\n' + result.message);
      }
      btn.disabled = false;
      btn.textContent = '🔍 Tester Connexion';
      updateData();
    })
    .catch(e => {
      alert('❌ Erreur: ' + e);
      btn.disabled = false;
      btn.textContent = '🔍 Tester Connexion';
    });
}

setInterval(updateData, 5000);
window.onload = updateData;
</script>
</body>
</html>)";
  
  server->send(200, "text/html", html);
}

void enphase_handleData(WebServer* server) {
  String json = "{";
  json += "\"pact_conso\":" + String(enphase_pact_conso, 1) + ",";
  json += "\"pact_prod\":" + String(enphase_pact_prod, 1) + ",";
  json += "\"pact_grid\":" + String(enphase_pact_grid, 1) + ",";
  json += "\"energy_imported\":" + String(enphase_energy_imported, 1) + ",";
  json += "\"energy_injected\":" + String(enphase_energy_injected, 1) + ",";
  json += "\"energy_consumed\":" + String(enphase_energy_consumed, 1) + ",";
  json += "\"energy_produced\":" + String(enphase_energy_produced, 1) + ",";
  json += "\"tension\":" + String(enphase_tension, 1) + ",";
  json += "\"intensite\":" + String(enphase_intensite, 1);
  json += "}";
  
  server->send(200, "application/json", json);
}

void enphase_handleStatus(WebServer* server) {
  time_t now = time(NULL);
  unsigned long last_update_s = enphase_last_success > 0 ? (now - (enphase_last_success / 1000)) : 0;
  
  String json = "{";
  json += "\"connected\":" + String(enphase_connected ? "true" : "false") + ",";
  json += "\"status\":\"" + enphase_status + "\",";
  json += "\"error\":\"" + enphase_last_error + "\",";
  json += "\"last_update\":" + String((enphase_last_success / 1000));
  json += "}";
  
  server->send(200, "application/json", json);
}

void enphase_handleTest(WebServer* server) {
  if (server->method() == HTTP_POST) {
    addLog("[Enphase] Test de connexion demandé...");
    
    if (config_enphase_ip.length() == 0) {
      enphase_status = "Non configuré";
      enphase_connected = false;
      enphase_last_error = "IP non configurée";
    } else {
      enphase_status = "Test en cours...";
      
      if (testEnvoyConnection()) {
        enphase_connected = true;
        enphase_status = "✅ Connecté";
        enphase_last_error = "";
        enphase_last_success = millis();
      } else {
        enphase_connected = false;
        enphase_status = "❌ Envoy introuvable";
        enphase_last_error = "Connexion HTTPS échouée";
      }
    }
    
    String json = "{";
    json += "\"success\":" + String(enphase_connected ? "true" : "false") + ",";
    json += "\"message\":\"" + enphase_status + " - " + enphase_last_error + "\",";
    json += "\"ip\":\"" + config_enphase_ip + "\"";
    json += "}";
    
    server->send(200, "application/json", json);
  }
}

void enphase_handleSaveConfig(WebServer* server) {
  if (server->method() == HTTP_POST) {
    config_enphase_ip = server->arg("ip");
    config_enphase_user = server->arg("user");
    config_enphase_pwd = server->arg("pwd");
    config_enphase_serial = server->arg("serial");
    
    // Sauvegarder dans NVS
    extern Preferences preferences;
    preferences.begin(PREF_NAMESPACE, false);
    enphase_saveConfig(&preferences);
    preferences.end();
    
    String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta http-equiv='refresh' content='2;url=/info'>
<title>Sauvegarde - MSunPV</title>
<style>body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;text-align:center}
h1{color:#22c55e;margin-top:50px}
p{color:#9ca3af;margin-top:20px}</style>
</head><body>
<h1>✅ Configuration Enphase Envoy sauvegardée !</h1>
<p>Redirection automatique dans 2 secondes...</p>
</body></html>)";
    
    server->send(200, "text/html", html);
  }
}

// ============================================
// ENPHASE - Configuration NVS
// ============================================

void enphase_loadConfig(Preferences* prefs) {
  config_enphase_ip = prefs->getString(PREF_ENPHASE_IP, "");
  config_enphase_user = prefs->getString(PREF_ENPHASE_USER, "");
  config_enphase_pwd = prefs->getString(PREF_ENPHASE_PWD, "");
  config_enphase_serial = prefs->getString(PREF_ENPHASE_SERIAL, "");
  enphaseToken = prefs->getString(PREF_ENPHASE_TOKEN, "");
  tokenTimestamp = prefs->getULong(PREF_ENPHASE_TOKEN_TS, 0);
}

void enphase_saveConfig(Preferences* prefs) {
  prefs->putString(PREF_ENPHASE_IP, config_enphase_ip);
  prefs->putString(PREF_ENPHASE_USER, config_enphase_user);
  prefs->putString(PREF_ENPHASE_PWD, config_enphase_pwd);
  prefs->putString(PREF_ENPHASE_SERIAL, config_enphase_serial);
  prefs->putString(PREF_ENPHASE_TOKEN, enphaseToken);
  prefs->putULong(PREF_ENPHASE_TOKEN_TS, tokenTimestamp);
}

