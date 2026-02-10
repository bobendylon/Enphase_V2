// MODULE STATS - Implémentation

#include "module_stats.h"
#include "module_sd.h"
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>

// Variables exposées (définitions)
float histoProd[24] = {0};
float histoConso[24] = {0};

// Variables internes (static)
static int currentHour = -1;
static unsigned long lastHistoSave = 0;
static String lastSavedDate = "";

// Déclarations externes
extern float solarProd;   // Production solaire (W)
extern float homeConso;   // Consommation (W)
extern void addLog(String message);
extern void addLogf(const char* format, ...);

// ============================================
// STATS - Initialisation
// ============================================

void stats_init() {
  // Initialisation si nécessaire
  addLog("Module Stats initialisé");
}

// ============================================
// STATS - Sauvegarde historique
// ============================================

void stats_update() {
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  int hour = timeinfo->tm_hour;
  
  if (hour != currentHour) {
    currentHour = hour;
    histoProd[hour] = solarProd;
    histoConso[hour] = homeConso;
    addLogf("[STATS] H%02d: Prod=%.0fW Conso=%.0fW", hour, solarProd, homeConso);
    
    // Sauvegarder sur SD si disponible (toutes les heures)
    if (sd_isAvailable()) {
      // Formater la date YYYY-MM-DD
      char dateStr[11];
      sprintf(dateStr, "%04d-%02d-%02d", 
              timeinfo->tm_year + 1900, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_mday);
      String date = String(dateStr);
      
      // Sauvegarder seulement si la date a changé (nouveau jour)
      if (date != lastSavedDate) {
        if (sd_saveStatsDaily(date, histoProd, histoConso)) {
          lastSavedDate = date;
          addLogf("[STATS] Sauvegarde SD réussie: %s", date.c_str());
        }
      } else {
        // Mettre à jour le fichier du jour en cours
        sd_saveStatsDaily(date, histoProd, histoConso);
      }
    }
  }
}

// ============================================
// STATS - Page Web
// ============================================

void stats_handleWeb(WebServer* server) {
  // Calculer les statistiques
  float prodTotal = stats_getTotalProd();
  float consoTotal = stats_getTotalConso();
  float autoCons = stats_getAutoconsommation();
  float injection = stats_getInjection();
  float picProd = stats_getPicProd();
  int picProdHour = stats_getPicProdHour();
  float picConso = stats_getPicConso();
  int picConsoHour = stats_getPicConsoHour();
  float tauxAuto = stats_getTauxAuto();
  
  // Obtenir l'heure actuelle pour affichage
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'><link rel='icon' type='image/svg+xml' href='/favicon.ico'><title>Stats - MSunPV</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js'></script>";
  html += "<style>*{margin:0;padding:0;box-sizing:border-box}body{background:#0c0a09;color:#fff;font-family:Arial;padding:20px;min-height:100vh}.container{max-width:1200px;margin:0 auto}.btn-back{background:#374151;color:#fff;padding:10px 20px;border-radius:8px;text-decoration:none;display:inline-block;margin-bottom:20px;min-height:44px;border:none;cursor:pointer}.btn-back:hover{background:#4b5563}h1{color:#60a5fa;margin-bottom:20px;font-size:28px}.stats-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(200px,1fr));gap:20px;margin-bottom:30px}.stat-card{background:#292524;padding:20px;border-radius:12px;border:1px solid #374151}.stat-card.primary{border-color:#fbbf24;background:linear-gradient(135deg,#292524 0%,#1c1917 100%)}.stat-label{color:#9ca3af;font-size:14px;margin-bottom:8px}.stat-value{color:#fff;font-size:32px;font-weight:bold;margin-bottom:4px}.stat-value.prod{color:#fbbf24}.stat-value.conso{color:#60a5fa}.stat-value.auto{color:#22c55e}.stat-unit{color:#6b7280;font-size:14px}.chart-container{background:#292524;padding:25px;border-radius:12px;margin-bottom:30px;border:1px solid #374151}.chart-title{color:#d1d5db;margin-bottom:20px;font-size:18px}.table-container{background:#292524;border-radius:12px;overflow:hidden;border:1px solid #374151;margin-top:30px}table{width:100%;border-collapse:collapse}th{background:#1c1917;color:#d1d5db;padding:15px;text-align:left;font-weight:600;border-bottom:2px solid #374151}td{padding:12px 15px;color:#fff;border-bottom:1px solid #374151}tr:hover{background:#1c1917}.hour-cell{color:#9ca3af;font-weight:600}.prod-cell{color:#fbbf24}.conso-cell{color:#60a5fa}.footer{text-align:center;color:#6b7280;font-size:14px;margin-top:20px}@media (max-width:768px){body{padding:12px}h1{font-size:1.5em}.stats-grid{grid-template-columns:repeat(2,1fr);gap:12px}.stat-value{font-size:24px}.btn-back{width:100%;padding:12px 16px}table{font-size:0.9em}th,td{padding:8px 6px}}@media (max-width:480px){.stats-grid{grid-template-columns:1fr}table{font-size:0.85em}th,td{padding:6px 4px}}</style>";
  html += "</head><body><div class='container'><a href='/' class='btn-back'>&larr; Retour</a><h1>📊 Statistiques</h1>";
  
  // Cartes statistiques
  html += "<div class='stats-grid'>";
  
  // Production totale
  html += "<div class='stat-card primary'><div class='stat-label'>Production Totale</div>";
  html += "<div class='stat-value prod' id='prodTotal'>" + String(prodTotal, 2) + "</div>";
  html += "<div class='stat-unit'>kWh</div></div>";
  
  // Consommation totale
  html += "<div class='stat-card'><div class='stat-label'>Consommation Totale</div>";
  html += "<div class='stat-value conso' id='consoTotal'>" + String(consoTotal, 2) + "</div>";
  html += "<div class='stat-unit'>kWh</div></div>";
  
  // Autoconsommation
  html += "<div class='stat-card'><div class='stat-label'>Autoconsommation</div>";
  html += "<div class='stat-value auto' id='autoCons'>" + String(autoCons, 2) + "</div>";
  html += "<div class='stat-unit'>kWh (" + String(tauxAuto, 1) + "%)</div></div>";
  
  // Injection réseau
  html += "<div class='stat-card'><div class='stat-label'>Injection Réseau</div>";
  html += "<div class='stat-value prod' id='injection'>" + String(injection, 2) + "</div>";
  html += "<div class='stat-unit'>kWh</div></div>";
  
  // Pic production
  html += "<div class='stat-card'><div class='stat-label'>Pic Production</div>";
  html += "<div class='stat-value prod' id='picProd'>" + String(picProd / 1000.0, 2) + "</div>";
  html += "<div class='stat-unit'>kW à " + String(picProdHour) + "h</div></div>";
  
  // Pic consommation
  html += "<div class='stat-card'><div class='stat-label'>Pic Consommation</div>";
  html += "<div class='stat-value conso' id='picConso'>" + String(picConso / 1000.0, 2) + "</div>";
  html += "<div class='stat-unit'>kW à " + String(picConsoHour) + "h</div></div>";
  
  html += "</div>";
  
  // Graphique
  html += "<div class='chart-container'><div class='chart-title'>Production et Consommation sur 24h</div>";
  html += "<canvas id='chart24h' height='80'></canvas></div>";
  
  // Tableau détaillé
  html += "<div class='table-container'><table><thead><tr><th>Heure</th><th>Production (W)</th><th>Consommation (W)</th><th>Bilan (W)</th></tr></thead><tbody id='tableBody'>";
  
  for(int i = 0; i < 24; i++) {
    float bilan = histoProd[i] - histoConso[i];
    String bilanColor = bilan >= 0 ? "#22c55e" : "#ef4444";
    String bilanSign = bilan >= 0 ? "+" : "";
    
    html += "<tr><td class='hour-cell'>" + String(i) + "h</td>";
    html += "<td class='prod-cell'>" + String((int)histoProd[i]) + "</td>";
    html += "<td class='conso-cell'>" + String((int)histoConso[i]) + "</td>";
    html += "<td style='color:" + bilanColor + "'>" + bilanSign + String((int)bilan) + "</td></tr>";
  }
  
  html += "</tbody></table></div>";
  
  // Footer
  html += "<p class='footer'>Données mises à jour toutes les heures • Dernière mise à jour: " + String(timeStr) + "</p>";
  
  html += "</div><script>";
  
  // JavaScript pour le graphique et rafraîchissement
  html += "const ctx=document.getElementById('chart24h').getContext('2d');";
  html += "let chart=null;";
  html += "function updateChart(data){";
  html += "const labels=[];for(let i=0;i<24;i++)labels.push(i+'h');";
  html += "if(chart)chart.destroy();";
  html += "chart=new Chart(ctx,{type:'line',data:{labels:labels,datasets:[{label:'Production (W)',data:data.histoProd,borderColor:'#fbbf24',backgroundColor:'rgba(251,191,36,0.1)',fill:true,tension:0.4},{label:'Consommation (W)',data:data.histoConso,borderColor:'#60a5fa',backgroundColor:'rgba(96,165,250,0.1)',fill:true,tension:0.4}]},options:{responsive:true,maintainAspectRatio:true,plugins:{legend:{labels:{color:'#d1d5db'},position:'top'}},scales:{x:{ticks:{color:'#9ca3af'},grid:{color:'#374151'}},y:{ticks:{color:'#9ca3af'},grid:{color:'#374151'}}}}});";
  html += "}";
  
  html += "function updatePage(data){";
  html += "document.getElementById('prodTotal').textContent=data.prodTotal.toFixed(2);";
  html += "document.getElementById('consoTotal').textContent=data.consoTotal.toFixed(2);";
  html += "document.getElementById('autoCons').textContent=data.autoconsommation.toFixed(2);";
  html += "document.getElementById('injection').textContent=data.injection.toFixed(2);";
  html += "document.getElementById('picProd').textContent=(data.picProd/1000).toFixed(2);";
  html += "document.getElementById('picConso').textContent=(data.picConso/1000).toFixed(2);";
  html += "updateChart(data);";
  html += "const tbody=document.getElementById('tableBody');";
  html += "tbody.innerHTML='';";
  html += "for(let i=0;i<24;i++){";
  html += "const bilan=data.histoProd[i]-data.histoConso[i];";
  html += "const color=bilan>=0?'#22c55e':'#ef4444';";
  html += "const sign=bilan>=0?'+':'';";
  html += "tbody.innerHTML+='<tr><td class=\"hour-cell\">'+i+'h</td><td class=\"prod-cell\">'+Math.round(data.histoProd[i])+'</td><td class=\"conso-cell\">'+Math.round(data.histoConso[i])+'</td><td style=\"color:'+color+'\">'+sign+Math.round(bilan)+'</td></tr>';";
  html += "}";
  html += "}";
  
  html += "function fetchData(){";
  html += "fetch('/statsData').then(r=>r.json()).then(d=>updatePage(d)).catch(e=>console.error('Erreur:',e));";
  html += "}";
  
  // Initialiser le graphique avec les données actuelles
  html += "const initialData={histoProd:[";
  for(int i = 0; i < 24; i++) {
    html += String((int)histoProd[i]);
    if(i < 23) html += ",";
  }
  html += "],histoConso:[";
  for(int i = 0; i < 24; i++) {
    html += String((int)histoConso[i]);
    if(i < 23) html += ",";
  }
  html += "],prodTotal:" + String(prodTotal, 2) + ",consoTotal:" + String(consoTotal, 2);
  html += ",autoconsommation:" + String(autoCons, 2) + ",injection:" + String(injection, 2);
  html += ",picProd:" + String(picProd, 1) + ",picConso:" + String(picConso, 1) + "};";
  html += "updateChart(initialData);";
  
  // Rafraîchissement automatique toutes les 30 secondes
  html += "setInterval(fetchData,30000);";
  html += "</script></body></html>";
  
  server->send(200, "text/html", html);
}

// ============================================
// STATS - Date actuelle
// ============================================

String stats_getCurrentDate() {
  time_t now = time(NULL);
  if (now < 946684800) { // NTP pas synchronisé
    return "";
  }
  
  struct tm *timeinfo = localtime(&now);
  char dateStr[11];
  sprintf(dateStr, "%04d-%02d-%02d", 
          timeinfo->tm_year + 1900, 
          timeinfo->tm_mon + 1, 
          timeinfo->tm_mday);
  return String(dateStr);
}

// ============================================
// STATS - Chargement depuis SD
// ============================================

bool stats_loadFromSD(String date) {
  if (!sd_isAvailable()) {
    return false;
  }
  
  return sd_loadStatsDaily(date, histoProd, histoConso);
}

// ============================================
// STATS - Fonctions de calcul statistiques
// ============================================

float stats_getTotalProd() {
  float total = 0;
  for(int i = 0; i < 24; i++) {
    total += histoProd[i];  // W
  }
  return total / 1000.0;  // kWh
}

float stats_getTotalConso() {
  float total = 0;
  for(int i = 0; i < 24; i++) {
    total += histoConso[i];  // W
  }
  return total / 1000.0;  // kWh
}

float stats_getAutoconsommation() {
  float total = 0;
  for(int i = 0; i < 24; i++) {
    float autoCons = min(histoProd[i], histoConso[i]);
    total += autoCons;  // W
  }
  return total / 1000.0;  // kWh
}

float stats_getInjection() {
  float total = 0;
  for(int i = 0; i < 24; i++) {
    float inj = max(0.0f, histoProd[i] - histoConso[i]);
    total += inj;  // W
  }
  return total / 1000.0;  // kWh
}

float stats_getPicProd() {
  float maxVal = 0;
  for(int i = 0; i < 24; i++) {
    if(histoProd[i] > maxVal) {
      maxVal = histoProd[i];
    }
  }
  return maxVal;  // W
}

int stats_getPicProdHour() {
  float maxVal = 0;
  int hour = 0;
  for(int i = 0; i < 24; i++) {
    if(histoProd[i] > maxVal) {
      maxVal = histoProd[i];
      hour = i;
    }
  }
  return hour;
}

float stats_getPicConso() {
  float maxVal = 0;
  for(int i = 0; i < 24; i++) {
    if(histoConso[i] > maxVal) {
      maxVal = histoConso[i];
    }
  }
  return maxVal;  // W
}

int stats_getPicConsoHour() {
  float maxVal = 0;
  int hour = 0;
  for(int i = 0; i < 24; i++) {
    if(histoConso[i] > maxVal) {
      maxVal = histoConso[i];
      hour = i;
    }
  }
  return hour;
}

float stats_getTauxAuto() {
  float prodTotal = stats_getTotalProd();
  if(prodTotal <= 0) return 0.0;
  float autoCons = stats_getAutoconsommation();
  return (autoCons / prodTotal) * 100.0;  // %
}

// ============================================
// STATS - Endpoint JSON /statsData
// ============================================

void stats_handleData(WebServer* server) {
  String json = "{";
  
  // Données du jour
  json += "\"prodTotal\":" + String(stats_getTotalProd(), 2) + ",";
  json += "\"consoTotal\":" + String(stats_getTotalConso(), 2) + ",";
  json += "\"autoconsommation\":" + String(stats_getAutoconsommation(), 2) + ",";
  json += "\"injection\":" + String(stats_getInjection(), 2) + ",";
  json += "\"picProd\":" + String(stats_getPicProd(), 1) + ",";
  json += "\"picProdHour\":" + String(stats_getPicProdHour()) + ",";
  json += "\"picConso\":" + String(stats_getPicConso(), 1) + ",";
  json += "\"picConsoHour\":" + String(stats_getPicConsoHour()) + ",";
  json += "\"tauxAuto\":" + String(stats_getTauxAuto(), 1) + ",";
  
  // Tableaux horaires
  json += "\"histoProd\":[";
  for(int i = 0; i < 24; i++) {
    json += String((int)histoProd[i]);
    if(i < 23) json += ",";
  }
  json += "],";
  
  json += "\"histoConso\":[";
  for(int i = 0; i < 24; i++) {
    json += String((int)histoConso[i]);
    if(i < 23) json += ",";
  }
  json += "]";
  
  json += "}";
  
  server->send(200, "application/json", json);
}

