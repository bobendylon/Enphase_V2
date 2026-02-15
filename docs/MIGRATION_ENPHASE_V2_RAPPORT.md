# Rapport de migration — ENPHASE V2

**Projet :** Enphase_V2  
**Source :** MSunPV_Monitor_V11_multi  
**Dernière mise à jour :** Février 2025

---

## 1. Objectif de la migration

Transformer le moniteur MSunPV multi-écrans en un affichage unique Enphase :
- **Écran :** uniquement l’écran Enphase (plus MQTT, M'SunPV, etc.)
- **Web :** conservé au départ, nettoyage progressif
- **Modules :** retrait progressif de M'SunPV, Shelly, SD, Alarme, puis intégration Home Assistant

---

## 2. Travaux réalisés

### 2.1 Écran unique Enphase

| Modification | Fichiers | Détail |
|--------------|----------|--------|
| Écran principal unique | `main.cpp`, `ui_main.h` | `screenMain` non créé, `activeScreenType = 1`, chargement direct de `screenEnphase` |
| Pas de bascule d’écran | `main.cpp` | Logique MQTT/M'SunPV/Enphase retirée pour l’affichage |

### 2.2 Module M'SunPV retiré

| Modification | Fichiers | Détail |
|--------------|----------|--------|
| Exclusion du build | `platformio.ini` | `-<module_msunpv.cpp>` dans `build_src_filter` |
| Routes, init, boucle | `main.cpp` | Routes, init, update, prefs, pages web, export, etc. |
| UI | `ui_main.h` | Callbacks, `updateMSunPVUI`, popup, badges |
| Champ IP | `module_mqtt.cpp` | Champ IP M'SunPV retiré |

### 2.3 Module Shelly retiré

| Modification | Fichiers | Détail |
|--------------|----------|--------|
| Exclusion du build | `platformio.ini` | `-<module_shelly.cpp>` dans `build_src_filter` |
| Routes, init, prefs | `main.cpp` | `/shellies`, `/shellyData`, `shelly_init`, `shelly_update`, `configShelly`, export `PREF_SHELLY*`, load/save config |
| UI | `ui_main.h` | Suppression `led_shelly`, `led_ep_shelly` ; décalage des icônes Enphase et Settings ; suppression labels Shelly 1/2 dans `card_infos` ; retrait des mises à jour Shelly dans `updateSettingsUI`, `updateMainUI`, `updateEnphaseUI` |
| Images | `ui_main.h` | Suppression des `extern Shelly32`, `Shelly32_gris` |
| Carte Infos | `ui_main.h` | Enphase et MQTT remontés (y=38, y=76) |

### 2.4 Module carte SD retiré

| Modification | Fichiers | Détail |
|--------------|----------|--------|
| Exclusion du build | `platformio.ini` | `-<module_sd.cpp>` dans `build_src_filter` |
| Dépendances dans Stats | `module_stats.cpp` | Suppression des appels `sd_isAvailable()`, `sd_saveStatsDaily()`, `sd_loadStatsDaily()` ; `stats_loadFromSD()` retourne toujours `false` |
| Variable | `module_stats.cpp` | Suppression de `static String lastSavedDate` |

### 2.5 Intégration Home Assistant (prévue, non implémentée)

- Suppression des souscriptions MQTT entrantes
- Suppression du module Alarme
- Publication des données Enphase vers HA (MQTT Discovery recommandé)
- Connexion MQTT optionnelle
- Météo vers HA : optionnelle
- Stats vers HA : non pour l’instant

---

## 3. Erreurs à ne pas reproduire

### 3.1 Nettoyage du cache après exclusion de modules

**Problème :** Après exclusion d’un fichier via `build_src_filter` (ex. `module_sd.cpp`, `module_shelly.cpp`), la compilation échoue avec des erreurs du type :
- `ar.exe: .../xxx.cpp.o: No such file or directory`
- Erreurs de linking sur des symboles manquants (`undefined reference to 'sd_isAvailable()'`)

**Cause :** Le cache de build (`build`, `libdeps`) reste incohérent avec la nouvelle configuration.

**Règle :** Après toute modification de `build_src_filter` ou suppression de source, exécuter :
```bash
pio run -t clean
pio run -j 1
```
Toujours faire un `clean` avant de recompiler.

### 3.2 Dépendances entre modules exclus et modules restants

**Problème :** Exclure `module_sd.cpp` provoque des erreurs de linking dans `module_stats.cpp` : `undefined reference to 'sd_isAvailable()'`, `sd_saveStatsDaily()`, etc.

**Cause :** `module_stats.cpp` appelle des fonctions définies dans le module exclu. L’exclusion du fichier ne retire pas automatiquement ces appels.

**Règle :** Avant d’exclure un module :
1. Chercher toutes les références au module (fonctions, variables, includes) : `grep -r "nom_module\|fonction_module" src/ include/`
2. Retirer ou remplacer ces références dans les autres fichiers
3. Exclure le module dans `platformio.ini`
4. Faire un `pio run -t clean` puis `pio run -j 1`

### 3.3 Vérifier le contenu réel du fichier sur disque

**Problème :** L’IDE peut afficher une version modifiée non sauvegardée. Les outils (grep, PowerShell, compilateur) lisent le fichier sur disque.

**Règle :** Avant de considérer une modification comme terminée :
- Sauvegarder tous les fichiers (Ctrl+K S ou File → Save All)
- Vérifier sur disque si besoin : `Select-String` (PowerShell) ou `grep` pour confirmer l’absence des symboles supprimés

### 3.4 Ordre des opérations pour retirer un module

**À faire dans cet ordre :**
1. Retirer les appels et références dans les autres modules (main, ui, etc.)
2. Exclure le module dans `platformio.ini`
3. Lancer `pio run -t clean`
4. Lancer `pio run -j 1`
5. Corriger les erreurs restantes éventuelles

**À éviter :** Exclure un module puis oublier de retirer ses usages dans le reste du code.

### 3.5 Compilation ESP32 et mémoire

**Problème :** Sous Windows, compilation parallèle peut provoquer `cc1plus.exe: out of memory`.

**Règle :** Utiliser un seul job : `pio run -j 1`

---

## 4. Configuration actuelle (platformio.ini)

```ini
build_src_filter = +<*> -<weather_icons_NEW.cpp> -<module_msunpv.cpp> -<module_sd.cpp> -<module_shelly.cpp>
```

Modules exclus : `module_msunpv`, `module_sd`, `module_shelly`.

---

## 5. Prochaines étapes prévues

1. **Intégration Home Assistant**
   - Supprimer les souscriptions MQTT entrantes
   - Supprimer le module Alarme
   - Implémenter la publication Enphase vers HA (MQTT Discovery)
   - Connexion MQTT optionnelle

2. **Optionnel**
   - Publication météo vers HA
   - Publication des stats vers HA (à décider plus tard)
