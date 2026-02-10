// ============================================
// FAVICON HANDLER - MSunPV V3.4
// Logo Soleil 32x32 SVG en base64
// ============================================

#ifndef FAVICON_H
#define FAVICON_H

// SVG Favicon Base64 (Soleil jaune/orange avec rayons)
const char FAVICON_DATA[] PROGMEM = "PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAzMiAzMiI+PGRlZnM+PGxpbmVhckdyYWRpZW50IGlkPSJzdW5HcmFkIiB4MT0iMCUiIHkxPSIwJSIgeDI9IjEwMCUiIHkyPSIxMDAlIj48c3RvcCBvZmZzZXQ9IjAlIiBzdHlsZT0ic3RvcC1jb2xvcjojRkJENzI0O3N0b3Atb3BhY2l0eToxIiAvPjxzdG9wIG9mZnNldD0iMTAwJSIgc3R5bGU9InN0b3AtY29sb3I6I0ZDQTMxMTtzdG9wLW9wYWNpdHk6MSIgLz48L2xpbmVhckdyYWRpZW50PjwvZGVmcz48Y2lyY2xlIGN4PSIxNiIgY3k9IjE2IiByPSI5IiBmaWxsPSJ1cmwoI3N1bkdyYWQpIi8+PHJlY3QgeD0iMTUiIHk9IjIiIHdpZHRoPSIyIiBoZWlnaHQ9IjQiIGZpbGw9IiNGQkQ3MjQiIHJ4PSIxIi8+PHJlY3QgeD0iMTUiIHk9IjI2IiB3aWR0aD0iMiIgaGVpZ2h0PSI0IiBmaWxsPSIjRkJENzI0IiByeD0iMSIvPjxyZWN0IHg9IjIiIHk9IjE1IiB3aWR0aD0iNCIgaGVpZ2h0PSIyIiBmaWxsPSIjRkJENzI0IiByeD0iMSIvPjxyZWN0IHg9IjI2IiB5PSIxNSIgd2lkdGg9IjQiIGhlaWdodD0iMiIgZmlsbD0iI0ZCRDcyNCIgcng9IjEiLz48cmVjdCB4PSI1IiB5PSI1IiB3aWR0aD0iMyIgaGVpZ2h0PSIzIiBmaWxsPSIjRkJENzI0IiByeD0iMSIgdHJhbnNmb3JtPSJyb3RhdGUoNDUgNi41IDYuNSkiLz48cmVjdCB4PSIyNCIgeT0iMjQiIHdpZHRoPSIzIiBoZWlnaHQ9IjMiIGZpbGw9IiNGQkQ3MjQiIHJ4PSIxIiB0cmFuc2Zvcm09InJvdGF0ZSg0NSAyNS41IDI1LjUpIi8+PHJlY3QgeD0iMjQiIHk9IjUiIHdpZHRoPSIzIiBoZWlnaHQ9IjMiIGZpbGw9IiNGQkQ3MjQiIHJ4PSIxIiB0cmFuc2Zvcm09InJvdGF0ZSgtNDUgMjUuNSA2LjUpIi8+PHJlY3QgeD0iNSIgeT0iMjQiIHdpZHRoPSIzIiBoZWlnaHQ9IjMiIGZpbGw9IiNGQkQ3MjQiIHJ4PSIxIiB0cmFuc2Zvcm09InJvdGF0ZSgtNDUgNi41IDI1LjUpIi8+PC9zdmc+";

// Déclaration externe du serveur WebServer
extern WebServer server;

// Handler pour servir le favicon
void handleFavicon() {
  // Décoder le favicon depuis le PROGMEM
  const char* b64 = FAVICON_DATA;
  
  // Répondre avec le favicon en SVG/base64
  String html = "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 32 32\">";
  html += "<defs><linearGradient id=\"sunGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\">";
  html += "<stop offset=\"0%\" style=\"stop-color:#FBD724;stop-opacity:1\" />";
  html += "<stop offset=\"100%\" style=\"stop-color:#FCA311;stop-opacity:1\" />";
  html += "</linearGradient></defs>";
  html += "<circle cx=\"16\" cy=\"16\" r=\"9\" fill=\"url(#sunGrad)\"/>";
  html += "<rect x=\"15\" y=\"2\" width=\"2\" height=\"4\" fill=\"#FBD724\" rx=\"1\"/>";
  html += "<rect x=\"15\" y=\"26\" width=\"2\" height=\"4\" fill=\"#FBD724\" rx=\"1\"/>";
  html += "<rect x=\"2\" y=\"15\" width=\"4\" height=\"2\" fill=\"#FBD724\" rx=\"1\"/>";
  html += "<rect x=\"26\" y=\"15\" width=\"4\" height=\"2\" fill=\"#FBD724\" rx=\"1\"/>";
  html += "<rect x=\"5\" y=\"5\" width=\"3\" height=\"3\" fill=\"#FBD724\" rx=\"1\" transform=\"rotate(45 6.5 6.5)\"/>";
  html += "<rect x=\"24\" y=\"24\" width=\"3\" height=\"3\" fill=\"#FBD724\" rx=\"1\" transform=\"rotate(45 25.5 25.5)\"/>";
  html += "<rect x=\"24\" y=\"5\" width=\"3\" height=\"3\" fill=\"#FBD724\" rx=\"1\" transform=\"rotate(-45 25.5 6.5)\"/>";
  html += "<rect x=\"5\" y=\"24\" width=\"3\" height=\"3\" fill=\"#FBD724\" rx=\"1\" transform=\"rotate(-45 6.5 25.5)\"/>";
  html += "</svg>";
  
  server.send(200, "image/svg+xml", html);
}

#endif // FAVICON_H

