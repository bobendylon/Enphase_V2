# Schéma écran principal LVGL – Dimensions exactes

Référence : `config.h` (SCREEN_WIDTH/HEIGHT), `include/ui_main.h` (`createMainScreen`).

---

## 1. Écran global

| Élément | Valeur |
|--------|--------|
| **Résolution** | **480 × 480** px |
| Fond | `COLOR_BG` (#0c0a09) |

---

## 2. Header (barre du haut)

| Propriété | Valeur |
|-----------|--------|
| **Taille** | **480 × 50** px |
| Position | (0, 0) |
| Fond | `COLOR_HEADER` (#1c1917) |
| Padding | 10 px (tous côtés) |
| Border | 0 |

### Contenu du header

| Élément | Position / alignement | Détail |
|---------|------------------------|--------|
| **Date** | `LV_ALIGN_LEFT_MID` | Police Montserrat **20** |
| **Heure** | `LV_ALIGN_CENTER` | Police Montserrat **20** |
| **Icône WiFi** | `LV_ALIGN_RIGHT_MID`, offset X = **-135** | Source 32×32 px, **zoom 205** → affiché ~25 px |
| **Icône MQTT** | `LV_ALIGN_RIGHT_MID`, offset X = **-105** | 32×31 px, **zoom 205** → ~25 px |
| **Icône Enphase** | `LV_ALIGN_RIGHT_MID`, offset X = **-75** | 32×32 px, **zoom 190** → ~24 px |
| **Icône Réglages** | `LV_ALIGN_RIGHT_MID`, offset X = **-45** | 32×32 px, **zoom 205** → ~25 px |

*Zoom LVGL : 256 = 100 %. Affichage ≈ (dim_source × zoom) / 256.*

---

## 3. Zone principale (sous le header)

- **Début Y** : `main_y = 60`
- **Hauteur commune des 2 cartes** : `main_height = 320` px

---

## 4. Carte gauche (Production / Conso / Flux)

| Propriété | Valeur |
|-----------|--------|
| **Taille** | **225 × 320** px |
| Position | **(10, 60)** |
| Fond | `COLOR_CARD` (#292524) |
| Border | 1 px, #fbbf24, opacité 40 % |
| Radius | 12 px |
| Padding | 15 px |

### Bloc Production solaire

| Élément | Position (X, Y) | Détail |
|---------|-----------------|--------|
| Titre "PRODUCTION SOLAIRE" | (0, **8**) | Gris #d1d5db, Montserrat **16** |
| Icône panneaux | (0, **33**) | 32×32, **zoom 300** → ~37,5 px |
| Valeur (W) | (**60**, 33) | `COLOR_PROD`, Montserrat **38** |
| Unité "W" | (**165**, 46) | `COLOR_PROD`, Montserrat **20** |

### Bloc Conso maison

| Élément | Position (X, Y) | Détail |
|---------|-----------------|--------|
| Titre "CONSO MAISON" | (0, **80**) | Montserrat 16 |
| Icône réseau | (0, **105**) | 32×32, zoom 300 |
| Valeur (W) | (60, 105) | `COLOR_CONSO`, Montserrat 38 |
| Unité "W" | (165, 118) | Montserrat 20 |

### Bloc Conso jour

| Élément | Alignement | Détail |
|---------|------------|--------|
| Titre "CONSO JOUR" | TOP_LEFT, Y = **173** | Montserrat 16 |
| Valeur "X.X kWh" | TOP_RIGHT, Y = **173** | Violet #a78bfa, Montserrat 16 |

### Zone flux PV → Maison → Réseau

| Propriété | Valeur |
|-----------|--------|
| **Container** | **195 × 80** px |
| Position dans carte | **(0, 210)** |
| FLOW_ZOOM | **256** (icônes 32×32 affichées en 32 px) |
| FLOW_Y | **-10** (décalage vertical) |

Positions X des 5 éléments (centre de chaque icône) :

| # | X (px) | Élément |
|---|--------|---------|
| 1 | **5** | Panneaux solaires |
| 2 | **46** | Flèche PV → Maison |
| 3 | **88** | Maison |
| 4 | **129** | Flèche Maison → Réseau |
| 5 | **171** | Réseau électrique / Chauffe-eau |

Labels "X W" sous les flèches : Montserrat 16, alignés sous flèche (BOTTOM_MID + 2 px).

---

## 5. Carte droite (Routage / Cumulus / Météo 4 jours)

| Propriété | Valeur |
|-----------|--------|
| **Taille** | **225 × 320** px |
| Position | **(245, 60)** |
| Style | Identique carte gauche (padding 15, radius 12, border 1 px) |

### Bloc ROUTAGE

| Élément | Position (X, Y) | Détail |
|---------|-----------------|--------|
| Titre "ROUTAGE" | (**60**, 10) | Montserrat 16 |
| Logo "M'Sun\nPV" | (**10**, 35) | Vert #a3e635, Montserrat 16, centré |
| Valeur (W) | (**80**, 50) | `COLOR_ROUTER`, Montserrat **38** |
| Unité "W" | (165, 63) | Montserrat 20 |

### Bloc CUMULUS

| Élément | Taille / position | Détail |
|---------|-------------------|--------|
| Header cliquable | **190 × 28** px, (5, **105**) | Transparent |
| Titre "CUMULUS" | Dans header, LEFT_MID | Montserrat 16 |
| Container | **190 × 100** px, (5, **135**) | Transparent |
| Zone thermo (icône) | **60 × 100** px, (0, 0) | Icône 60×83, **zoom 256** |
| Colonne infos | **120 × 100** px, (75, 0) | Temp + LED |
| Température "XX°C" | TOP_MID, Y = 15 | Montserrat **26**, couleur selon phase |
| LED (cartes_et_drapeaux) | TOP_MID, Y = 55 | Source **40×40**, **zoom 224** → ~35×35 px |

### Météo 4 jours (prévisions)

| Propriété | Valeur |
|-----------|--------|
| **Container** | **190 × 60** px |
| Position | (5, **240**) |
| Cliquable | Oui → ouvre popup météo |

Chaque colonne (4 au total) :

| Propriété | Valeur |
|-----------|--------|
| **Taille colonne** | **47 × 60** px |
| Position X | i × 47 (i = 0..3) |
| Label jour | TOP_MID, Y = 2, Montserrat 16 |
| Icône météo | CENTER, **zoom 128** → 16×16 px (source 32×32) |
| Label température | BOTTOM_MID, Y = -2, Montserrat 16 |

---

## 6. Barre météo (bas d’écran)

| Propriété | Valeur |
|-----------|--------|
| **Taille** | **460 × 50** px |
| Position | **(10, 400)** |
| Fond | `COLOR_WEATHER` (#1c1917) |
| Radius | 12 px |
| Padding | 10 px |

| Élément | Alignement | Détail |
|---------|------------|--------|
| Temp ext + ville | LEFT_MID | Montserrat **20**, format "Ville  XX°C" |
| Icône météo | CENTER | **zoom 256** (taille source selon weather_icons) |
| Temp salon | RIGHT_MID | "SALON  XX°C", Montserrat 20 |

---

## 7. Récapitulatif des icônes (sources C)

| Fichier / ressource | Dimensions source | Zoom typique | Affiché (≈) |
|---------------------|-------------------|--------------|-------------|
| wifi_cercle_vert / wifi_barre_oblique | 32×32 | 205 | ~25 px |
| mqtt_png / mqtt_png_gris | 32×31 | 205 | ~25 px |
| Enphase_logo / Enphase_logo_gris | 32×32 | 190 | ~24 px |
| roue_dentee | 32×32 | 205 | ~25 px |
| panneaux_solaires / reseau_electrique / maison | 32×32 | 300 (cartes) / 256 (flux) | ~37 px / 32 px |
| Chauffeeaucartedroite | 60×83 | 256 | 60×83 px |
| cartes_et_drapeaux | 40×40 | 224 | ~35 px |
| icofleche* (flèches flux) | 32×32 | 256 | 32 px |
| chauffe_eau (réseau → cumulus) | 32×32 | 256 | 32 px |
| Icônes météo (weather) | variable | 128 (4 jours) / 256 (barre) | 16 px / taille source |

---

## 8. Schéma visuel (axes en pixels)

```
    0        100       200       245   345       380       480
 0  ├─────────┬─────────┬─────────┬─────┬─────────┬─────────┤
    │         │  HEADER 480×50    │     │ Date    Heure  🔵🔵🟠⚙ │
50  ├─────────┴─────────┴─────────┴─────┴─────────┴─────────┤
    │
60  ├──────────────┬─── 10 px ───┬──────────────┤
    │ CARTE GAUCHE │             │ CARTE DROITE │
    │  225×320     │   20 px     │   225×320    │
    │  (10,60)     │   gap       │  (245,60)    │
    │              │             │              │
    │ PROD SOL     │             │ ROUTAGE      │
    │ CONSO MAISON │             │ CUMULUS      │
    │ CONSO JOUR   │             │ Météo 4j     │
    │ [Flux 195×80]│             │              │
380 │              │             │              │
    ├──────────────┴─────────────┴──────────────┤
400 ├────────────  Barre météo 460×50 ──────────┤  (10,400)
450 │  Ext °C      [icône météo]      SALON °C   │
480 ├───────────────────────────────────────────┤
```

---

*Document généré à partir du code `include/ui_main.h` et `include/config.h` (Enphase V2).*
