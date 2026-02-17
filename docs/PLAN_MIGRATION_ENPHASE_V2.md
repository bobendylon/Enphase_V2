# Plan de migration — ENPHASE V2

**Objectif :** Créer un nouveau projet *ENPHASE V2* à partir de MSunPV_Monitor_V11_multi, avec :
- **Côté écran LVGL :** un seul affichage = écran Enphase (on retire MQTT, M'SunPV, etc.).
- **Côté Web serveur :** tout est conservé au départ ; nettoyage progressif ensuite.

**Important :** Ce document décrit uniquement la stratégie (copie, structure, quoi garder/supprimer). Aucun code n’est modifié ici.

---

## 1. Créer le nouveau projet dans un nouveau dossier

### 1.1 Emplacement recommandé

Créer le nouveau projet **à côté** du projet actuel, dans le même répertoire parent :

- Actuel : `C:\Users\...\PlatformIO\Projects\MSunPV_Monitor_V11_multi`
- Nouveau : `C:\Users\...\PlatformIO\Projects\Enphase_V2`  
  (ou `MSunPV_Enphase_V2` si vous préférez garder le préfixe)

### 1.2 Méthodes de copie possibles

**Option A — Copie manuelle (Explorateur Windows)**  
1. Fermer Cursor/VS Code sur le projet actuel.  
2. Copier tout le dossier `MSunPV_Monitor_V11_multi`.  
3. Coller dans `PlatformIO\Projects\`.  
4. Renommer le dossier copié en `Enphase_V2` (ou le nom choisi).  
5. Ouvrir ce nouveau dossier dans Cursor/PlatformIO.

**Option B — Ligne de commande (PowerShell)**  
Depuis `C:\Users\...\PlatformIO\Projects\` :

```powershell
Copy-Item -Path "MSunPV_Monitor_V11_multi" -Destination "Enphase_V2" -Recurse
```

Puis ouvrir le dossier `Enphase_V2` dans Cursor.

**Option C — Git (recommandé si vous utilisez Git)**  
1. Cloner le dépôt actuel dans un nouveau dossier (ou copier le dossier puis initialiser un nouveau dépôt dans la copie).  
2. Renommer le dossier en `Enphase_V2`.  
3. Optionnel : nouveau dépôt distant dédié à Enphase V2.

**À exclure de la copie (pour alléger)**  
- Dossier `.pio\build\` (fichiers de compilation, recréés au premier build).  
- Dossier `.pio\libdeps\` (dépendances, réinstallées par PlatformIO).  
- Fichiers `.vscode` peuvent être gardés ou supprimés selon préférence.

Vous pouvez copier d’abord tout le projet, puis supprimer ces dossiers dans `Enphase_V2` après ouverture.

---

## 2. Structure actuelle (rappel)

### 2.1 Côté écran LVGL (ce qui existe aujourd’hui)

- **Écrans :**
  - `screenMain` = écran MQTT (date blanche).
  - `screenEnphase` = écran Enphase (date orange) ; aussi utilisé pour M'SunPV (date verte).
- **Variable :** `activeScreenType` = 0 (MQTT), 1 (Enphase), 2 (M'SunPV).
- **Fichiers concernés :**
  - `src/main.cpp` — création des écrans, chargement selon `activeScreenType`, boucle LVGL.
  - `include/ui_main.h` — déclarations UI, icônes, variables d’écran.
  - `src/module_enphase.cpp` + `include/module_enphase.h` — logique et données Enphase.
  - `src/Enphase_logo.c`, `src/Enphase_logo_gris.c` — icônes Enphase.
  - Autres modules écran : `module_weather`, `module_mqtt`, `module_shelly`, `module_msunpv`, `module_stats`, `module_tempo`, `module_sd`.
  - Nombreux `.c` dans `src/` : icônes LVGL (wifi, mqtt, Shelly, panneaux, maison, etc.).

### 2.2 Côté Web serveur

- Tout est dans `src/main.cpp` : routes HTTP, génération HTML (pages réglages, WiFi, MQTT, écrans, stats, etc.).
- `include/favicon.h` pour l’icône.
- Aucun dossier `data/` : pas de fichiers HTML externes.

---

## 3. Ce qu’il faudra faire dans ENPHASE V2 (après la copie)

### 3.1 Côté LVGL (affichage unique Enphase)

À faire **dans le nouveau projet uniquement** :

1. **Écran unique**  
   - Faire en sorte que l’écran principal soit toujours l’écran Enphase (plus de bascule MQTT / M'SunPV).  
   - Supprimer ou court-circuiter tout chargement de `screenMain` et toute logique basée sur `activeScreenType` pour l’affichage.

2. **Fichiers / modules à garder (minimum pour Enphase + écran)**  
   - `module_enphase.cpp` / `module_enphase.h`  
   - `Enphase_logo.c`, `Enphase_logo_gris.c`  
   - Partie de `main.cpp` et `ui_main.h` qui crée et met à jour l’écran Enphase.  
   - `touch.h` / touch, `config.h` (écran, WiFi, NTP, etc.).  
   - `lv_conf.h`, `weather_icons.h` si l’écran Enphase affiche la météo (footer).  
   - Selon besoin : `module_weather` (si footer météo conservé).

3. **À retirer ou désactiver progressivement (côté écran)**  
   - Création et usage de `screenMain` (écran MQTT).  
   - Modules : `module_mqtt`, `module_shelly`, `module_msunpv`, `module_stats`, `module_tempo`, `module_sd` — selon ce qui n’est plus affiché.  
   - Icônes `.c` inutiles (Shelly, MQTT dédié écran MQTT, etc.) — à identifier au fur et à mesure.  
   - Dans `ui_main.h` : déclarations et références liées à l’écran MQTT et aux autres écrans.

4. **platformio.ini**  
   - Adapter le nom du projet / env si besoin.  
   - `build_src_filter` : plus tard, exclure les `.cpp` des modules supprimés pour éviter les erreurs de link.

### 3.2 Côté Web serveur

- **Phase 1 (dès la copie) :** ne rien supprimer. Garder tout le serveur web tel quel dans `main.cpp` (routes, HTML, réglages WiFi, MQTT, choix d’écran, etc.).
- **Phase 2 (plus tard) :** supprimer petit à petit :
  - Routes ou blocs HTML liés au choix d’écran (MQTT / Enphase / M'SunPV).
  - Pages ou options liées à MQTT, Shelly, M'SunPV, stats, etc., selon ce que vous voulez garder pour Enphase V2.

---

## 4. Ordre recommandé des opérations

1. **Copier** le projet dans le nouveau dossier (ex. `Enphase_V2`) avec l’une des méthodes du § 1.2.  
2. **Ouvrir** uniquement le nouveau projet dans Cursor/PlatformIO.  
3. **Vérifier** qu’il compile et flashe tel quel (même comportement que l’actuel).  
4. **Côté LVGL :** faire de l’écran Enphase l’écran principal et unique, puis retirer progressivement `screenMain` et les modules inutiles.  
5. **Côté Web :** garder tout au début ; supprimer ensuite, par petites étapes, les parties inutiles.

---

## 5. Résumé

- **Copie :** nouveau dossier (ex. `Enphase_V2`) par copie du projet complet, puis suppression optionnelle de `.pio\build` et `.pio\libdeps`.  
- **LVGL :** on ne garde à terme que ce qui sert l’affichage Enphase (écran unique) ; le reste (MQTT, M'SunPV, Shelly, etc.) est retiré progressivement.  
- **Web :** tout est conservé au début ; nettoyage des routes et des options inutiles ensuite, petit à petit.

Si vous voulez, on peut détailler une prochaine étape (par exemple « liste précise des fichiers à toucher pour l’écran unique Enphase » ou « commandes PowerShell exactes pour la copie ») sans écrire de code.
