// MODULE SHELLY - Implémentation

#include "module_shelly.h"
#include <WiFi.h>
#include <time.h>

// Constantes NVS (évite d'inclure config.h qui contient des définitions)
#define PREF_SHELLY1_IP "shelly1_ip"
#define PREF_SHELLY1_NAME "shelly1_name"
#define PREF_SHELLY1_LABEL0 "shelly1_label0"
#define PREF_SHELLY1_LABEL1 "shelly1_label1"
#define PREF_SHELLY1_LABEL_ENERGY "shelly1_label_energy"
#define PREF_SHELLY1_LABEL_RETURNED "shelly1_label_returned"
#define PREF_SHELLY2_IP "shelly2_ip"
#define PREF_SHELLY2_NAME "shelly2_name"
#define PREF_SHELLY2_LABEL0 "shelly2_label0"
#define PREF_SHELLY2_LABEL1 "shelly2_label1"
#define PREF_SHELLY2_LABEL_ENERGY "shelly2_label_energy"
#define PREF_SHELLY2_LABEL_RETURNED "shelly2_label_returned"
#define PREF_NAMESPACE "msunpv"

// Variables exposées (définitions)
String config_shelly1_ip = "";
String config_shelly1_name = "Shelly EM 1";
String config_shelly1_label0 = "Enedis";
String config_shelly1_label1 = "Prod.Solaire";
String config_shelly1_label_energy = "Production Jour";
String config_shelly1_label_returned = "Retourné Réseau Jour";

String config_shelly2_ip = "";
String config_shelly2_name = "Shelly EM 2";
String config_shelly2_label0 = "M'SunPV";
String config_shelly2_label1 = "Pompe Daikin";
String config_shelly2_label_energy = "Production Jour";
String config_shelly2_label_returned = "Retourné Réseau Jour";

// Données Shelly 1
float shelly1_power0 = 0;
float shelly1_power1 = 0;
float shelly1_total0 = 0;
float shelly1_total1 = 0;
float shelly1_returned0 = 0;
float shelly1_returned1 = 0;

// Données Shelly 2
float shelly2_power0 = 0;
float shelly2_power1 = 0;
float shelly2_total0 = 0;
float shelly2_total1 = 0;
float shelly2_returned0 = 0;
float shelly2_returned1 = 0;

// Statut connexion (pour icône header)
bool shelly1_connected = false;
bool shelly2_connected = false;

// Variables internes (static)
static float shelly1_day_start0 = 0;
static float shelly1_day_start1 = 0;
static float shelly2_day_start0 = 0;
static float shelly2_day_start1 = 0;

static float shelly1_day_start_returned0 = 0;
static float shelly1_day_start_returned1 = 0;
static float shelly2_day_start_returned0 = 0;
static float shelly2_day_start_returned1 = 0;

static unsigned long lastShellyUpdate = 0;
static int lastResetDay = -1;
static const unsigned long SHELLY_UPDATE_INTERVAL = 5000;  // 5 secondes

// Déclarations externes
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// Fonction privée pour récupérer les données d'un Shelly
static void fetchShellyData(int shellyNum) {
  String ip = (shellyNum == 1) ? config_shelly1_ip : config_shelly2_ip;
  
  if (ip.length() == 0) return;
  
  if (shellyNum == 1) shelly1_connected = false;
  else shelly2_connected = false;
  
  HTTPClient http;
  String url = "http://" + ip + "/status";
  
  http.begin(url);
  http.setTimeout(3000);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      if (shellyNum == 1) {
        shelly1_connected = true;
        shelly1_power0 = doc["emeters"][0]["power"];
        shelly1_power1 = doc["emeters"][1]["power"];
        shelly1_total0 = doc["emeters"][0]["total"];
        shelly1_total1 = doc["emeters"][1]["total"];
        shelly1_returned0 = doc["emeters"][0]["total_returned"];
        shelly1_returned1 = doc["emeters"][1]["total_returned"];
        
        // Initialisation des compteurs jour au premier appel
        if (shelly1_day_start0 == 0) {
          shelly1_day_start0 = shelly1_total0;
          shelly1_day_start1 = shelly1_total1;
          shelly1_day_start_returned0 = shelly1_returned0;
          shelly1_day_start_returned1 = shelly1_returned1;
        }
        
        addLogf("[Shelly 1] P0=%.1fW P1=%.1fW T0=%.1fWh T1=%.1fWh", 
                      shelly1_power0, shelly1_power1, shelly1_total0, shelly1_total1);
      } else {
        shelly2_connected = true;
        shelly2_power0 = doc["emeters"][0]["power"];
        shelly2_power1 = doc["emeters"][1]["power"];
        shelly2_total0 = doc["emeters"][0]["total"];
        shelly2_total1 = doc["emeters"][1]["total"];
        shelly2_returned0 = doc["emeters"][0]["total_returned"];
        shelly2_returned1 = doc["emeters"][1]["total_returned"];
        
        // Initialisation des compteurs jour au premier appel
        if (shelly2_day_start0 == 0) {
          shelly2_day_start0 = shelly2_total0;
          shelly2_day_start1 = shelly2_total1;
          shelly2_day_start_returned0 = shelly2_returned0;
          shelly2_day_start_returned1 = shelly2_returned1;
        }
        
        addLogf("[Shelly 2] P0=%.1fW P1=%.1fW T0=%.1fWh T1=%.1fWh", 
                      shelly2_power0, shelly2_power1, shelly2_total0, shelly2_total1);
      }
    } else {
      addLogf("[Shelly %d] JSON parse error", shellyNum);
    }
  } else {
    addLogf("[Shelly %d] HTTP Error: %d", shellyNum, httpCode);
  }
  
  http.end();
}

// Fonction privée pour reset journalier
static void resetDayCounters() {
  shelly1_day_start0 = shelly1_total0;
  shelly1_day_start1 = shelly1_total1;
  shelly1_day_start_returned0 = shelly1_returned0;
  shelly1_day_start_returned1 = shelly1_returned1;
  shelly2_day_start0 = shelly2_total0;
  shelly2_day_start1 = shelly2_total1;
  shelly2_day_start_returned0 = shelly2_returned0;
  shelly2_day_start_returned1 = shelly2_returned1;
  addLog("[V11.0] Reset compteurs journaliers Shelly EM");
}

// Initialisation
void shelly_init() {
  addLog("[Shelly] Module initialisé");
}

// Mise à jour des données Shelly
void shelly_update() {
  // Vérifier la connexion WiFi
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }
  
  unsigned long now = millis();
  if (now - lastShellyUpdate < SHELLY_UPDATE_INTERVAL && lastShellyUpdate != 0) {
    return;
  }
  
  // Mise à jour des données
  if (config_shelly1_ip.length() > 0) fetchShellyData(1);
  if (config_shelly2_ip.length() > 0) fetchShellyData(2);
  lastShellyUpdate = now;
  
  // Reset journalier à 23h59
  time_t now_t = time(NULL);
  struct tm *ti = localtime(&now_t);
  int currentDay = ti->tm_mday;
  
  if (ti->tm_hour == 23 && ti->tm_min == 59 && lastResetDay != currentDay) {
    lastResetDay = currentDay;
    resetDayCounters();
  }
}

// Handler web - Page /shellies
void shelly_handleWeb(WebServer* server) {
  bool has1 = (config_shelly1_ip.length() > 0);
  bool has2 = (config_shelly2_ip.length() > 0);
  
  if (!has1 && !has2) {
    String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'>
<title>Shelly EM - MSunPV</title>
<style>
body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;text-align:center}
h1{color:#a78bfa;margin-top:50px;font-size:1.5em}
.btn{background:#374151;color:#fff;padding:12px 24px;border-radius:8px;text-decoration:none;display:inline-block;margin-top:20px;min-height:44px}
.btn:hover{background:#4b5563}
@media (max-width: 480px) {
  body{padding:15px}
  h1{font-size:1.3em;margin-top:30px}
  .btn{width:100%;padding:14px;margin-top:15px}
}
</style></head><body>
<h1>⚡ Aucun Shelly EM configuré</h1>
<p style='color:#9ca3af'>Configurez au moins une adresse IP dans la page Info.</p>
<a href='/info' class='btn'>⚙️ Configuration</a>
<a href='/' class='btn'>← Retour</a>
</body></html>)";
    server->send(200, "text/html", html);
    return;
  }
  
  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'>
<title>Shelly EM - MSunPV V11</title>
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
  border-bottom: 2px solid rgba(167, 139, 250, 0.3);
  margin-bottom: 30px;
}
.header h1 { color: #a78bfa; font-size: 2em; margin-bottom: 10px; }
.header p { color: #9ca3af; }
.grid {
  display: grid;
  grid-template-columns: )";
  
  if (has1 && has2) {
    html += "repeat(2, 1fr)";
  } else {
    html += "1fr";
  }
  
  html += R"(;
  gap: 20px;
  margin-bottom: 30px;
}
.card {
  background: rgba(41, 37, 36, 0.8);
  border-radius: 16px;
  padding: 25px;
  border: 1px solid rgba(167, 139, 250, 0.25);
  backdrop-filter: blur(10px);
}
.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 15px;
  border-bottom: 1px solid rgba(167, 139, 250, 0.2);
}
.card-title {
  font-size: 1.5em;
  color: #a78bfa;
  font-weight: 600;
}
.btn-edit {
  background: #374151;
  color: #fff;
  border: none;
  padding: 8px 15px;
  border-radius: 6px;
  cursor: pointer;
  font-size: 0.9em;
}
.btn-edit:hover { background: #4b5563; }
.power-section {
  background: #1c1917;
  padding: 20px;
  border-radius: 12px;
  margin-bottom: 20px;
}
.power-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 12px 0;
  border-bottom: 1px solid #292524;
}
.power-item:last-child { border-bottom: none; }
.power-label {
  color: #9ca3af;
  font-size: 1em;
  font-weight: 500;
}
.power-value {
  font-size: 1.8em;
  font-weight: 700;
  color: #a78bfa;
}
.power-unit {
  font-size: 0.9em;
  color: #d1d5db;
  margin-left: 5px;
}
.energy-section {
  margin-top: 20px;
}
.energy-label {
  color: #9ca3af;
  font-size: 0.85em;
  text-transform: uppercase;
  letter-spacing: 1px;
  margin-bottom: 10px;
}
.energy-value {
  font-size: 1.3em;
  color: #fbbf24;
  font-weight: 600;
}
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
.btn-primary { background: #a78bfa; color: #0c0a09; }
.btn-primary:hover { background: #9171f8; }
.popup {
  display: none;
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: rgba(0,0,0,0.8);
  justify-content: center;
  align-items: center;
  z-index: 1000;
}
.popup-content {
  background: #292524;
  padding: 30px;
  border-radius: 16px;
  border: 2px solid #a78bfa;
  max-width: 400px;
  width: 90%;
}
.popup-content h2 {
  color: #a78bfa;
  margin-bottom: 20px;
}
.popup-content input {
  width: 100%;
  padding: 12px;
  background: #1c1917;
  border: 1px solid #44403c;
  border-radius: 8px;
  color: #fff;
  font-size: 1em;
  margin-bottom: 20px;
}
.popup-content input:focus {
  outline: none;
  border-color: #a78bfa;
}
.popup-buttons {
  display: flex;
  gap: 10px;
}
.popup-buttons button {
  flex: 1;
  padding: 12px;
  border: none;
  border-radius: 8px;
  font-weight: 600;
  cursor: pointer;
  font-size: 1em;
}
.btn-save { background: #22c55e; color: white; }
.btn-cancel { background: #ef4444; color: white; }
@media (max-width: 768px) {
  body { padding: 12px; }
  .header { padding: 15px 0; margin-bottom: 20px; }
  .header h1 { font-size: 1.5em; }
  .grid { grid-template-columns: 1fr; gap: 15px; }
  .card { padding: 20px; }
  .card-header { flex-direction: column; align-items: flex-start; gap: 10px; }
  .card-title { font-size: 1.3em; }
  .btn-edit { width: 100%; }
  .power-section { padding: 15px; }
  .power-item { padding: 10px 0; }
  .power-value { font-size: 1.5em; }
  .nav { gap: 10px; margin-top: 20px; }
  .btn { padding: 14px 20px; font-size: 0.9em; min-height: 44px; }
  .popup-content { padding: 20px; max-width: 95%; }
}
@media (max-width: 480px) {
  body { padding: 10px; }
  .header h1 { font-size: 1.3em; }
  .card { padding: 15px; }
  .card-title { font-size: 1.1em; }
  .power-value { font-size: 1.3em; }
  .power-label { font-size: 0.9em; }
  .energy-value { font-size: 1.1em; }
  .nav { flex-direction: column; }
  .btn { width: 100%; padding: 14px; }
  .popup-content { padding: 15px; }
  .popup-buttons { flex-direction: column; }
  .popup-buttons button { min-height: 44px; }
}
</style>
</head>
<body>
<div class='container'>
  <div class='header'>
    <h1>⚡ Monitoring Shelly EM</h1>
    <p>Surveillance temps réel des capteurs de puissance</p>
  </div>
  
  <div class='grid'>)";
  
  if (has1) {
    html += R"(
    <div class='card'>
      <div class='card-header'>
        <span class='card-title' id='shelly1-name'>)" + config_shelly1_name + R"(</span>
        <button class='btn-edit' onclick='editName(1)'>✏️ Renommer</button>
      </div>
      
      <div class='power-section'>
        <div class='power-item'>
          <span class='power-label' id='s1-label0'>)" + config_shelly1_label0 + R"(</span>
          <div>
            <span class='power-value' id='s1p0'>0</span>
            <span class='power-unit'>W</span>
          </div>
        </div>
        <div class='power-item'>
          <span class='power-label' id='s1-label1'>)" + config_shelly1_label1 + R"(</span>
          <div>
            <span class='power-value' id='s1p1'>0</span>
            <span class='power-unit'>W</span>
          </div>
        </div>
      </div>
      
      <div class='energy-section'>
        <div class='energy-label' id='s1-label-energy'>)" + config_shelly1_label_energy + R"(</div>
        <div class='energy-value' id='s1-energy'>0.0 kWh</div>
      </div>
      
      <div class='energy-section' style='margin-top:15px'>
        <div class='energy-label' id='s1-label-returned'>)" + config_shelly1_label_returned + R"(</div>
        <div class='energy-value' id='s1-returned' style='color:#22c55e'>0.0 kWh</div>
      </div>
    </div>)";
  }
  
  if (has2) {
    html += R"(
    <div class='card'>
      <div class='card-header'>
        <span class='card-title' id='shelly2-name'>)" + config_shelly2_name + R"(</span>
        <button class='btn-edit' onclick='editName(2)'>✏️ Renommer</button>
      </div>
      
      <div class='power-section'>
        <div class='power-item'>
          <span class='power-label' id='s2-label0'>)" + config_shelly2_label0 + R"(</span>
          <div>
            <span class='power-value' id='s2p0'>0</span>
            <span class='power-unit'>W</span>
          </div>
        </div>
        <div class='power-item'>
          <span class='power-label' id='s2-label1'>)" + config_shelly2_label1 + R"(</span>
          <div>
            <span class='power-value' id='s2p1'>0</span>
            <span class='power-unit'>W</span>
          </div>
        </div>
      </div>
      
      <div class='energy-section'>
        <div class='energy-label' id='s2-label-energy'>)" + config_shelly2_label_energy + R"(</div>
        <div class='energy-value' id='s2-energy'>0.0 kWh</div>
      </div>
      
      <div class='energy-section' style='margin-top:15px'>
        <div class='energy-label' id='s2-label-returned'>)" + config_shelly2_label_returned + R"(</div>
        <div class='energy-value' id='s2-returned' style='color:#22c55e'>0.0 kWh</div>
      </div>
    </div>)";
  }
  
  html += R"(
  </div>
  
  <div class='nav'>
    <a href='/info' class='btn'>⚙️ Configuration</a>
    <a href='/' class='btn'>← Dashboard</a>
  </div>
</div>

<div id='popup-rename' class='popup'>
  <div class='popup-content'>
    <h2>Configurer Shelly EM</h2>
    
    <div style='margin-bottom:15px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Adresse IP Shelly:</label>
      <input type='text' id='input-ip' placeholder='Ex: 192.168.1.100' style='margin-bottom:10px'>
    </div>
    
    <div style='margin-bottom:15px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Nom du Shelly:</label>
      <input type='text' id='input-name' placeholder='Nom du Shelly' style='margin-bottom:10px'>
    </div>
    
    <div style='margin-bottom:15px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Premier label:</label>
      <input type='text' id='input-label0' placeholder='Ex: Enedis' style='margin-bottom:10px'>
    </div>
    
    <div style='margin-bottom:15px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Deuxième label:</label>
      <input type='text' id='input-label1' placeholder='Ex: Prod.Solaire' style='margin-bottom:10px'>
    </div>
    
    <div style='margin-bottom:15px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Label Production Jour:</label>
      <input type='text' id='input-label-energy' placeholder='Ex: Production Jour' style='margin-bottom:10px'>
    </div>
    
    <div style='margin-bottom:20px'>
      <label style='color:#9ca3af; font-size:0.85em; display:block; margin-bottom:5px'>Label Retourné Réseau Jour:</label>
      <input type='text' id='input-label-returned' placeholder='Ex: Retourné Réseau Jour'>
    </div>
    
    <div class='popup-buttons'>
      <button class='btn-save' onclick='saveConfig()'>💾 Enregistrer</button>
      <button class='btn-cancel' onclick='closePopup()'>❌ Annuler</button>
    </div>
  </div>
</div>

<script>
let currentShellyEdit = 0;

function updateData() {
  fetch('/shellyData')
    .then(r => r.json())
    .then(d => {)";
  
  if (has1) {
    html += R"(
      document.getElementById('s1p0').textContent = d.s1p0.toFixed(0);
      document.getElementById('s1p1').textContent = d.s1p1.toFixed(0);
      document.getElementById('s1-energy').textContent = d.s1_energy.toFixed(2) + ' kWh';
      document.getElementById('s1-returned').textContent = d.s1_returned.toFixed(2) + ' kWh';)";
  }
  
  if (has2) {
    html += R"(
      document.getElementById('s2p0').textContent = d.s2p0.toFixed(0);
      document.getElementById('s2p1').textContent = d.s2p1.toFixed(0);
      document.getElementById('s2-energy').textContent = d.s2_energy.toFixed(2) + ' kWh';
      document.getElementById('s2-returned').textContent = d.s2_returned.toFixed(2) + ' kWh';)";
  }
  
  html += R"(
    })
    .catch(e => console.error('Erreur:', e));
}

function editName(shellyNum) {
  currentShellyEdit = shellyNum;
  document.getElementById('popup-rename').style.display = 'flex';
  
  // Récupérer les valeurs actuelles
  fetch('/getShellyConfig?shelly=' + shellyNum)
    .then(r => r.json())
    .then(data => {
      document.getElementById('input-ip').value = data.ip || '';
      document.getElementById('input-name').value = data.name || '';
      document.getElementById('input-label0').value = data.label0 || '';
      document.getElementById('input-label1').value = data.label1 || '';
      document.getElementById('input-label-energy').value = data.label_energy || '';
      document.getElementById('input-label-returned').value = data.label_returned || '';
    });
}

function saveConfig() {
  const ip = document.getElementById('input-ip').value;
  const name = document.getElementById('input-name').value;
  const label0 = document.getElementById('input-label0').value;
  const label1 = document.getElementById('input-label1').value;
  const label_energy = document.getElementById('input-label-energy').value;
  const label_returned = document.getElementById('input-label-returned').value;
  
  if (!name.trim()) {
    alert('Le nom ne peut pas être vide');
    return;
  }
  
  console.log('Envoi sauvegarde Shelly', currentShellyEdit, ip, name);
  
  fetch('/saveShellyConfig', {
    method: 'POST',
    headers: {'Content-Type': 'application/x-www-form-urlencoded'},
    body: 'shelly=' + currentShellyEdit + 
          '&ip=' + encodeURIComponent(ip) +
          '&name=' + encodeURIComponent(name) +
          '&label0=' + encodeURIComponent(label0) +
          '&label1=' + encodeURIComponent(label1) +
          '&label_energy=' + encodeURIComponent(label_energy) +
          '&label_returned=' + encodeURIComponent(label_returned)
  })
  .then(response => {
    console.log('Réponse reçue:', response.status, response.statusText);
    if (!response.ok) {
      throw new Error('Erreur HTTP: ' + response.status);
    }
    return response.text();
  })
  .then(data => {
    console.log('Données reçues:', data);
    location.reload();
  })
  .catch(error => {
    console.error('Erreur lors de la sauvegarde:', error);
    alert('Erreur lors de la sauvegarde: ' + error.message);
  });
}

function closePopup() {
  document.getElementById('popup-rename').style.display = 'none';
}

setInterval(updateData, 5000);
window.onload = updateData;
</script>
</body>
</html>)";
  
  server->send(200, "text/html", html);
}

// Handler web - API JSON /shellyData
void shelly_handleData(WebServer* server) {
  // Shelly 1: Production Jour = cumul label0 (total0), Retourné = cumul label1 (returned1)
  float s1_energy_day = max(0.0f, (shelly1_total0 - shelly1_day_start0) / 1000.0f);
  float s1_returned_day = max(0.0f, (shelly1_returned1 - shelly1_day_start_returned1) / 1000.0f);
  
  // Shelly 2: Production Jour = cumul label0 (total0), Retourné = cumul label1 (returned1)
  float s2_energy_day = max(0.0f, (shelly2_total0 - shelly2_day_start0) / 1000.0f);
  float s2_returned_day = max(0.0f, (shelly2_returned1 - shelly2_day_start_returned1) / 1000.0f);
  
  String json = "{";
  json += "\"s1p0\":" + String(shelly1_power0, 1) + ",";
  json += "\"s1p1\":" + String(shelly1_power1, 1) + ",";
  json += "\"s1_energy\":" + String(s1_energy_day, 2) + ",";
  json += "\"s1_returned\":" + String(s1_returned_day, 2) + ",";
  json += "\"s2p0\":" + String(shelly2_power0, 1) + ",";
  json += "\"s2p1\":" + String(shelly2_power1, 1) + ",";
  json += "\"s2_energy\":" + String(s2_energy_day, 2) + ",";
  json += "\"s2_returned\":" + String(s2_returned_day, 2);
  json += "}";
  
  server->send(200, "application/json", json);
}

// Handler web - Récupère la configuration d'un Shelly (JSON)
void shelly_handleGetConfig(WebServer* server) {
  int shellyNum = server->arg("shelly").toInt();
  String json = "{";
  
  if (shellyNum == 1) {
    json += "\"ip\":\"" + config_shelly1_ip + "\",";
    json += "\"name\":\"" + config_shelly1_name + "\",";
    json += "\"label0\":\"" + config_shelly1_label0 + "\",";
    json += "\"label1\":\"" + config_shelly1_label1 + "\",";
    json += "\"label_energy\":\"" + config_shelly1_label_energy + "\",";
    json += "\"label_returned\":\"" + config_shelly1_label_returned + "\"";
  } else if (shellyNum == 2) {
    json += "\"ip\":\"" + config_shelly2_ip + "\",";
    json += "\"name\":\"" + config_shelly2_name + "\",";
    json += "\"label0\":\"" + config_shelly2_label0 + "\",";
    json += "\"label1\":\"" + config_shelly2_label1 + "\",";
    json += "\"label_energy\":\"" + config_shelly2_label_energy + "\",";
    json += "\"label_returned\":\"" + config_shelly2_label_returned + "\"";
  }
  
  json += "}";
  server->send(200, "application/json", json);
}

// Handler web - Sauvegarde configuration complète Shelly
void shelly_handleSaveConfig(WebServer* server) {
  addLogf("[Shelly] handleSaveConfig appelé - Méthode: %d (HTTP_POST=%d)", 
          server->method(), HTTP_POST);
  
  if (server->method() == HTTP_POST) {
    // Debug: afficher tous les arguments reçus
    addLogf("[Shelly] Nombre d'arguments: %d", server->args());
    for (int i = 0; i < server->args(); i++) {
      addLogf("  Arg[%d]: '%s' = '%s'", i, server->argName(i).c_str(), server->arg(i).c_str());
    }
    
    // Vérifier le format : ancien format depuis /info (shelly1_ip, shelly2_ip) ou nouveau format depuis /shellies (shelly, ip, name, etc.)
    if (server->hasArg("shelly1_ip") || server->hasArg("shelly2_ip")) {
      // Format ancien depuis /info : juste les IP
      String ip1 = server->arg("shelly1_ip");
      String ip2 = server->arg("shelly2_ip");
      
      addLogf("[Shelly] Format ancien détecté - IP1: '%s', IP2: '%s'", ip1.c_str(), ip2.c_str());
      
      if (ip1.length() > 0) {
        config_shelly1_ip = ip1;
        addLogf("[Shelly] Shelly 1 IP mise à jour: '%s'", config_shelly1_ip.c_str());
      }
      if (ip2.length() > 0) {
        config_shelly2_ip = ip2;
        addLogf("[Shelly] Shelly 2 IP mise à jour: '%s'", config_shelly2_ip.c_str());
      }
    } else {
      // Format nouveau depuis /shellies : configuration complète
      int shellyNum = server->arg("shelly").toInt();
      String ip = server->arg("ip");
      String name = server->arg("name");
      String label0 = server->arg("label0");
      String label1 = server->arg("label1");
      String label_energy = server->arg("label_energy");
      String label_returned = server->arg("label_returned");
      
      // Debug: afficher les valeurs reçues
      addLogf("[Shelly] Format nouveau détecté - Shelly %d", shellyNum);
      addLogf("  IP reçue: '%s' (longueur: %d)", ip.c_str(), ip.length());
      addLogf("  Nom: '%s'", name.c_str());
      
      // Validation: shellyNum doit être 1 ou 2
      if (shellyNum != 1 && shellyNum != 2) {
        addLogf("[Shelly] ERREUR: shellyNum invalide: %d (doit être 1 ou 2)", shellyNum);
        server->send(400, "text/plain", "Invalid shelly number");
        return;
      }
      
      if (shellyNum == 1) {
        config_shelly1_ip = ip;
        config_shelly1_name = name;
        config_shelly1_label0 = label0;
        config_shelly1_label1 = label1;
        config_shelly1_label_energy = label_energy;
        config_shelly1_label_returned = label_returned;
        addLogf("[Shelly] Variables Shelly 1 mises à jour - IP: '%s' (longueur: %d)", 
                config_shelly1_ip.c_str(), config_shelly1_ip.length());
      } else if (shellyNum == 2) {
        config_shelly2_ip = ip;
        config_shelly2_name = name;
        config_shelly2_label0 = label0;
        config_shelly2_label1 = label1;
        config_shelly2_label_energy = label_energy;
        config_shelly2_label_returned = label_returned;
        addLogf("[Shelly] Variables Shelly 2 mises à jour - IP: '%s' (longueur: %d)", 
                config_shelly2_ip.c_str(), config_shelly2_ip.length());
      }
    }
    
    // Sauvegarder dans NVS
    extern Preferences preferences;
    preferences.begin(PREF_NAMESPACE, false);
    shelly_saveConfig(&preferences);
    preferences.end();
    
    // Vérifier que la sauvegarde a fonctionné en relisant
    preferences.begin(PREF_NAMESPACE, true);
    String saved_ip1 = preferences.getString(PREF_SHELLY1_IP, "");
    String saved_ip2 = preferences.getString(PREF_SHELLY2_IP, "");
    preferences.end();
    
    addLogf("[Shelly] Sauvegarde NVS - IP1 relue: '%s' (longueur: %d), IP2 relue: '%s' (longueur: %d)", 
            saved_ip1.c_str(), saved_ip1.length(), saved_ip2.c_str(), saved_ip2.length());
    
    if (server->hasArg("shelly1_ip") || server->hasArg("shelly2_ip")) {
      // Format ancien
      addLogf("[V11.0] Configuration Shelly sauvegardée (format ancien)");
      addLogf("  Shelly 1 IP: %s", config_shelly1_ip.c_str());
      addLogf("  Shelly 2 IP: %s", config_shelly2_ip.c_str());
    } else {
      // Format nouveau
      int shellyNum = server->arg("shelly").toInt();
      String name = server->arg("name");
      String ip = server->arg("ip");
      String label0 = server->arg("label0");
      String label1 = server->arg("label1");
      addLogf("[V11.0] Shelly %d configuré: %s", shellyNum, name.c_str());
      addLogf("  IP: %s", ip.c_str());
      addLogf("  Label0: %s, Label1: %s", label0.c_str(), label1.c_str());
    }
    
    server->send(200, "text/plain", "OK");
  } else {
    addLogf("[Shelly] ERREUR: handleSaveConfig appelé avec méthode non-POST: %d", server->method());
    server->send(405, "text/plain", "Method Not Allowed");
  }
}

// Handler web - Sauvegarde nom Shelly (ancienne fonction, deprecated)
void shelly_handleSaveName(WebServer* server) {
  if (server->method() == HTTP_POST) {
    int shellyNum = server->arg("shelly").toInt();
    String name = server->arg("name");
    
    if (shellyNum == 1) {
      config_shelly1_name = name;
    } else if (shellyNum == 2) {
      config_shelly2_name = name;
    }
    
    extern Preferences preferences;
    preferences.begin(PREF_NAMESPACE, false);
    shelly_saveConfig(&preferences);
    preferences.end();
    
    addLogf("[V11.0] Shelly %d renommé: %s", shellyNum, name.c_str());
    
    server->send(200, "text/plain", "OK");
  }
}

// Chargement configuration depuis NVS
void shelly_loadConfig(Preferences* prefs) {
  config_shelly1_ip = prefs->getString(PREF_SHELLY1_IP, "");
  config_shelly1_name = prefs->getString(PREF_SHELLY1_NAME, "Shelly EM 1");
  config_shelly1_label0 = prefs->getString(PREF_SHELLY1_LABEL0, "Enedis");
  config_shelly1_label1 = prefs->getString(PREF_SHELLY1_LABEL1, "Prod.Solaire");
  config_shelly1_label_energy = prefs->getString(PREF_SHELLY1_LABEL_ENERGY, "Production Jour");
  config_shelly1_label_returned = prefs->getString(PREF_SHELLY1_LABEL_RETURNED, "Retourné Réseau Jour");
  
  config_shelly2_ip = prefs->getString(PREF_SHELLY2_IP, "");
  config_shelly2_name = prefs->getString(PREF_SHELLY2_NAME, "Shelly EM 2");
  config_shelly2_label0 = prefs->getString(PREF_SHELLY2_LABEL0, "M'SunPV");
  config_shelly2_label1 = prefs->getString(PREF_SHELLY2_LABEL1, "Pompe Daikin");
  config_shelly2_label_energy = prefs->getString(PREF_SHELLY2_LABEL_ENERGY, "Production Jour");
  config_shelly2_label_returned = prefs->getString(PREF_SHELLY2_LABEL_RETURNED, "Retourné Réseau Jour");
  
  addLog("[Shelly] Configuration chargée depuis NVS");
  addLogf("  Shelly 1 - IP: '%s' (longueur: %d), Nom: '%s'", 
          config_shelly1_ip.c_str(), config_shelly1_ip.length(), config_shelly1_name.c_str());
  addLogf("  Shelly 2 - IP: '%s' (longueur: %d), Nom: '%s'", 
          config_shelly2_ip.c_str(), config_shelly2_ip.length(), config_shelly2_name.c_str());
}

// Sauvegarde configuration dans NVS
void shelly_saveConfig(Preferences* prefs) {
  bool ok1 = prefs->putString(PREF_SHELLY1_IP, config_shelly1_ip);
  bool ok2 = prefs->putString(PREF_SHELLY1_NAME, config_shelly1_name);
  bool ok3 = prefs->putString(PREF_SHELLY1_LABEL0, config_shelly1_label0);
  bool ok4 = prefs->putString(PREF_SHELLY1_LABEL1, config_shelly1_label1);
  bool ok5 = prefs->putString(PREF_SHELLY1_LABEL_ENERGY, config_shelly1_label_energy);
  bool ok6 = prefs->putString(PREF_SHELLY1_LABEL_RETURNED, config_shelly1_label_returned);
  
  bool ok7 = prefs->putString(PREF_SHELLY2_IP, config_shelly2_ip);
  bool ok8 = prefs->putString(PREF_SHELLY2_NAME, config_shelly2_name);
  bool ok9 = prefs->putString(PREF_SHELLY2_LABEL0, config_shelly2_label0);
  bool ok10 = prefs->putString(PREF_SHELLY2_LABEL1, config_shelly2_label1);
  bool ok11 = prefs->putString(PREF_SHELLY2_LABEL_ENERGY, config_shelly2_label_energy);
  bool ok12 = prefs->putString(PREF_SHELLY2_LABEL_RETURNED, config_shelly2_label_returned);
  
  addLog("[Shelly] Configuration sauvegardée dans NVS");
  addLogf("  Shelly 1 IP sauvegardée: %s (valeur: '%s')", ok1 ? "OK" : "ERREUR", config_shelly1_ip.c_str());
  addLogf("  Shelly 2 IP sauvegardée: %s (valeur: '%s')", ok7 ? "OK" : "ERREUR", config_shelly2_ip.c_str());
}

