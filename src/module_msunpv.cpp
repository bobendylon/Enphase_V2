// MODULE MSunPV - Gestion routeur MSunPV via XML

#include "module_msunpv.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <Preferences.h>

// Déclarations externes (évite d'inclure des fichiers avec définitions)
extern Preferences preferences;
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// Constantes NVS (évite d'inclure config.h)
#define PREF_MSUNPV_IP "msunpv_ip"

// Valeur par défaut (évite d'inclure config.h)
#define DEFAULT_MSUNPV_IP "192.168.1.165"

// Intervalle de mise à jour (défini dans le module)
static const int MSUNPV_UPDATE_INTERVAL = 10000;  // 10 secondes

// Variables exposées (extern) - Définies ici
String msunpv_status = "AUTO";  // AUTO, MANU, OFF
int msunpv_cmdPos = 0;
int msunpv_cumulusState = 0;
int msunpv_radiatorState = 0;

// Variables exposées (extern) - Données MSunPV (V12.3)
float msunpv_powReso = 0;
float msunpv_powPV = 0;
int msunpv_outBal = 0;
int msunpv_outRad = 0;
float msunpv_enConso = 0;
float msunpv_tBal1 = 0;

// Variables exposées (extern) - Configuration
String config_msunpv_ip = "";

// Variables internes (static)
static unsigned long lastMsunpvUpdate = 0;

// ============================================
// DÉCLARATIONS ANTICIPÉES (forward declarations)
// ============================================
static void parseMsunpvXML(String xml);

// ============================================
// FONCTIONS HELPER (static)
// ============================================

// Fonction helper pour extraire une valeur XML
static String extractXMLValue(String xml, String tag) {
  String startTag = "<" + tag + ">";
  String endTag = "</" + tag + ">";
  int startPos = xml.indexOf(startTag);
  if (startPos < 0) return "";
  startPos += startTag.length();
  int endPos = xml.indexOf(endTag, startPos);
  if (endPos < 0) return "";
  String value = xml.substring(startPos, endPos);
  value.trim();
  return value;
}

// Fonction helper pour parser une liste séparée par ';'
static void parseSemicolonList(String data, float* output, int maxCount) {
  int count = 0;
  int startPos = 0;
  for (int i = 0; i < data.length() && count < maxCount; i++) {
    if (data.charAt(i) == ';' || i == data.length() - 1) {
      String value = data.substring(startPos, i);
      value.trim();
      if (value.length() > 0) {
        // Remplacer ',' par '.' pour les nombres décimaux
        value.replace(',', '.');
        output[count] = value.toFloat();
        count++;
      }
      startPos = i + 1;
    }
  }
}

// Fonction helper pour parser une liste séparée par ';' (int)
static void parseSemicolonListInt(String data, int* output, int maxCount) {
  int count = 0;
  int startPos = 0;
  for (int i = 0; i < data.length() && count < maxCount; i++) {
    if (data.charAt(i) == ';' || i == data.length() - 1) {
      String value = data.substring(startPos, i);
      value.trim();
      if (value.length() > 0) {
        output[count] = value.toInt();
        count++;
      }
      startPos = i + 1;
    }
  }
}

// ============================================
// FONCTIONS PRIVÉES (static)
// ============================================

// Récupérer l'état du routeur via XML
static void fetchMsunpvStatus() {
  if (millis() - lastMsunpvUpdate < MSUNPV_UPDATE_INTERVAL) {
    return;  // Pas encore temps de mettre à jour
  }
  
  if (config_msunpv_ip.length() == 0) {
    return;  // IP non configurée
  }
  
  HTTPClient http;
  String url = "http://" + config_msunpv_ip + "/status.xml";
  
  http.begin(url);
  http.setTimeout(2000);  // Timeout 2 secondes
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String xml = http.getString();
    parseMsunpvXML(xml);
    lastMsunpvUpdate = millis();
    addLog("[MSunPV] Status updated");
  } else {
    addLogf("[MSunPV] HTTP error: %d", httpCode);
  }
  
  http.end();
}

// Parser le XML pour extraire seulement les 7 valeurs nécessaires (V12.3)
static void parseMsunpvXML(String xml) {
  // 1. <inAns> - PowReso[0], PowPV[1], OutBal[2], OutRad[3], VoltRes[4], T_Bal1[5]
  String inAns = extractXMLValue(xml, "inAns");
  inAns.replace(',', '.');
  float inAnsVals[8] = {0};
  parseSemicolonList(inAns, inAnsVals, 8);
  msunpv_powReso = inAnsVals[0];
  msunpv_powPV = inAnsVals[1];
  msunpv_outBal = (int)inAnsVals[2];  // OUTBAL est à la position 2 de inAns
  msunpv_outRad = (int)inAnsVals[3];  // OUTRAD est à la position 3 de inAns
  msunpv_tBal1 = inAnsVals[5];
  
  // 3. <cptVals> - EnConso[0] (hex → dec / 10)
  String cptVals = extractXMLValue(xml, "cptVals");
  int sep = cptVals.indexOf(';');
  if (sep > 0) {
    String hexVal = cptVals.substring(0, sep);
    hexVal.trim();
    if (hexVal.length() > 0) {
      msunpv_enConso = strtoul(hexVal.c_str(), NULL, 16) / 10.0;
    }
  }
  
  // 4. <cmdPos> - État Cumulus et Radiateur
  String cmdPos = extractXMLValue(xml, "cmdPos");
  int sep2 = cmdPos.indexOf(';');
  if (sep2 > 0) {
    String cmdStr = cmdPos.substring(0, sep2);
    cmdStr.trim();
    if (cmdStr.length() > 0) {
      msunpv_cmdPos = (int)strtol(cmdStr.c_str(), NULL, 16);
      msunpv_cumulusState = msunpv_cmdPos & 0x03;
      msunpv_radiatorState = (msunpv_cmdPos >> 2) & 0x03;
      if (msunpv_cumulusState == 0) msunpv_status = "OFF";
      else if (msunpv_cumulusState == 1) msunpv_status = "MANU";
      else if (msunpv_cumulusState == 2) msunpv_status = "AUTO";
      else msunpv_status = "OFF";
    }
  }
}

// ============================================
// FONCTIONS PUBLIQUES
// ============================================

// Initialisation
void msunpv_init() {
  addLog("[MSunPV] Initialisation...");
  // L'initialisation réelle se fait dans loadConfig()
}

// Mise à jour (à appeler dans loop())
void msunpv_update() {
  if (WiFi.status() != WL_CONNECTED) {
    return;  // Pas de WiFi, pas de mise à jour
  }
  fetchMsunpvStatus();
}

// Envoyer une commande au routeur
void msunpv_sendCommand(int cmd) {
  if (config_msunpv_ip.length() == 0) {
    addLog("[MSunPV] ❌ IP non configurée");
    return;
  }
  
  HTTPClient http;
  String url = "http://" + config_msunpv_ip + "/index.xml";
  
  // V3.3 - Mapper les commandes aux valeurs hex
  String cmdValue;
  if (cmd == 0) {
    cmdValue = "0";    // OFF
  } else if (cmd == 1) {
    cmdValue = "9";    // MANU
  } else if (cmd == 2) {
    cmdValue = "10";   // AUTO
  }
  
  // Construire la commande parS=cmdValue;0;0;0;0;0;0;2;
  String command = "parS=" + cmdValue + ";0;0;0;0;0;0;2;";
  
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(2000);
  
  int httpCode = http.POST(command);
  
  if (httpCode == 200) {
    addLogf("[MSunPV] ✅ Commande envoyée: %s (cmd=%d OK)", command.c_str(), cmd);
    
    // Mettre à jour le statut immédiatement
    if (cmd == 0) {
      msunpv_status = "OFF";
    } else if (cmd == 1) {
      msunpv_status = "MANU";
    } else if (cmd == 2) {
      msunpv_status = "AUTO";
    }
  } else {
    addLogf("[MSunPV] ❌ Erreur commande: %d", httpCode);
  }
  
  http.end();
  
  // Forcer une mise à jour dans 2 secondes
  lastMsunpvUpdate = millis() - MSUNPV_UPDATE_INTERVAL + 2000;
}

// Handler web - Page /msunpv
void msunpv_handleWeb(WebServer* server) {
  if (config_msunpv_ip.length() == 0) {
    String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<link rel='icon' type='image/svg+xml' href='/favicon.ico'>
<title>M'SunPV - MSunPV</title>
<style>
body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;text-align:center}
h1{color:#fbbf24;margin-top:50px;font-size:1.5em}
.btn{background:#374151;color:#fff;padding:12px 24px;border-radius:8px;text-decoration:none;display:inline-block;margin-top:20px;min-height:44px}
.btn:hover{background:#4b5563}
@media (max-width: 480px) {
  body{padding:15px}
  h1{font-size:1.3em;margin-top:30px}
  .btn{width:100%;padding:14px;margin-top:15px}
}
</style></head><body>
<h1>⚡ M'SunPV non configuré</h1>
<p style='color:#9ca3af'>Configurez l'adresse IP du routeur dans la page Info.</p>
<a href='/info' class='btn'>⚙️ Configuration</a>
<a href='/' class='btn'>← Retour</a>
</body></html>)";
    server->send(200, "text/html", html);
    return;
  }
  
  String html = R"(<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>M'SunPV</title>
<style>
body{font-family:Arial;background:#0c0a09;color:#fff;padding:15px;margin:0}
.c{max-width:600px;margin:0 auto}
.h{text-align:center;padding:10px 0;border-bottom:2px solid #fbbf24;margin-bottom:15px}
.h h1{color:#fbbf24;font-size:1.5em;margin:0}
.h p{color:#9ca3af;font-size:0.85em;margin:5px 0}
.di{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #292524;font-size:0.95em}
.di:last-child{border-bottom:none}
.dl{color:#9ca3af}
.dv{font-weight:600;color:#fff}
.btns{display:flex;gap:8px;justify-content:center;flex-wrap:wrap;margin:20px 0}
.btn{padding:12px 24px;font-size:1em;font-weight:bold;border:none;border-radius:6px;cursor:pointer;min-width:90px}
.btn:hover{opacity:0.8}
.ba{background:#22c55e;color:white}
.bm{background:#60a5fa;color:white}
.bo{background:#fff;color:#000}
.nav{text-align:center;margin-top:15px}
.nav a{color:#fbbf24;text-decoration:none}
.up{text-align:center;color:#9ca3af;font-size:0.75em;margin-top:10px}
@media (max-width: 768px) {
  body{padding:12px}
  .h{padding:8px 0;margin-bottom:12px}
  .h h1{font-size:1.3em}
  .di{font-size:0.9em;padding:8px 0}
  .btns{flex-direction:column;gap:10px}
  .btn{width:100%;padding:14px;min-height:44px}
}
@media (max-width: 480px) {
  body{padding:10px}
  .h h1{font-size:1.1em}
  .di{font-size:0.85em}
  .btn{font-size:0.9em;padding:12px}
}
</style>
<script>
function sendCmd(c){fetch('/msunpvCmd',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'cmd='+c}).then(()=>setTimeout(update,500))}
function update(){fetch('/msunpvData').then(r=>r.json()).then(d=>{
if(d.powReso!==undefined)document.getElementById('powReso').textContent=d.powReso.toFixed(1)+' W';
if(d.powPV!==undefined)document.getElementById('powPV').textContent=d.powPV.toFixed(1)+' W';
if(d.outBal!==undefined)document.getElementById('outBal').textContent=d.outBal+'%';
if(d.outRad!==undefined)document.getElementById('outRad').textContent=d.outRad+'%';
if(d.enConso!==undefined)document.getElementById('enConso').textContent=(d.enConso/1000).toFixed(3)+' kWh';
if(d.tBal1!==undefined)document.getElementById('tBal1').textContent=d.tBal1.toFixed(1)+' °C';
if(d.lastUpdate)document.getElementById('up').textContent='MAJ: '+d.lastUpdate
}).catch(e=>console.error('Err:',e))}
setInterval(update,5000);window.onload=update
</script>
</head>
<body>
<div class='c'>
<div class='h'><h1>🔌 M'SunPV</h1><p>IP: )";
  html += config_msunpv_ip;
  html += R"(</p></div>
<div class='di'><span class='dl'>ENEDIS</span><span class='dv' id='powReso'>)";
  html += String(msunpv_powReso, 1);
  html += R"( W</span></div>
<div class='di'><span class='dl'>Prod. solaire</span><span class='dv' id='powPV'>)";
  html += String(msunpv_powPV, 1);
  html += R"( W</span></div>
<div class='di'><span class='dl'>OUTBAL</span><span class='dv' id='outBal'>)";
  html += String(msunpv_outBal);
  html += R"(%</span></div>
<div class='di'><span class='dl'>OUTRAD</span><span class='dv' id='outRad'>)";
  html += String(msunpv_outRad);
  html += R"(%</span></div>
<div class='di'><span class='dl'>Conso Jour</span><span class='dv' id='enConso'>)";
  html += String(msunpv_enConso / 1000.0, 3);
  html += R"( kWh</span></div>
<div class='di'><span class='dl'>T° Routeur</span><span class='dv' id='tBal1'>)";
  html += String(msunpv_tBal1, 1);
  html += R"( °C</span></div>
<div class='btns'>
<button class='btn ba' onclick='sendCmd(2)'>AUTO</button>
<button class='btn bm' onclick='sendCmd(1)'>MANU</button>
<button class='btn bo' onclick='sendCmd(0)'>OFF</button>
</div>
<div class='up' id='up'>Chargement...</div>
<div class='nav'><a href='/'>← Retour</a></div>
</div></body></html>)";
  server->send(200, "text/html", html);
}

// Handler commande M'SunPV via web
void msunpv_handleCommand(WebServer* server) {
  if (server->method() == HTTP_POST) {
    int cmd = server->arg("cmd").toInt();
    msunpv_sendCommand(cmd);
    server->send(200, "text/plain", "OK");
  }
}

// Handler données M'SunPV en JSON (V12.3) - Seulement les 7 valeurs
void msunpv_handleData(WebServer* server) {
  unsigned long secondsSinceUpdate = (millis() - lastMsunpvUpdate) / 1000;
  String json = "{";
  json += "\"powReso\":" + String(msunpv_powReso, 1) + ",";
  json += "\"powPV\":" + String(msunpv_powPV, 1) + ",";
  json += "\"outBal\":" + String(msunpv_outBal) + ",";
  json += "\"outRad\":" + String(msunpv_outRad) + ",";
  json += "\"enConso\":" + String(msunpv_enConso, 1) + ",";
  json += "\"tBal1\":" + String(msunpv_tBal1, 1) + ",";
  json += "\"lastUpdate\":\"" + String(secondsSinceUpdate) + "s\"";
  json += "}";
  server->send(200, "application/json", json);
}

// Chargement configuration depuis NVS
void msunpv_loadConfig(Preferences* prefs) {
  config_msunpv_ip = prefs->getString(PREF_MSUNPV_IP, DEFAULT_MSUNPV_IP);
  addLogf("[MSunPV] Configuration chargée - IP: %s", config_msunpv_ip.c_str());
}

// Sauvegarde configuration dans NVS
void msunpv_saveConfig(Preferences* prefs) {
  prefs->putString(PREF_MSUNPV_IP, config_msunpv_ip);
  addLogf("[MSunPV] Configuration sauvegardée - IP: %s", config_msunpv_ip.c_str());
}

