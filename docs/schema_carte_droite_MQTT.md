# Schéma – Carte droite écran MQTT

## Position et taille de la carte sur l’écran

- **Position** : x = 245 px, y = 60 px (sous le header)
- **Taille** : **225 px** (largeur) × **320 px** (hauteur)
- **Padding** : 15 px (tous côtés) → zone utile interne : **195 × 290 px**

---

## Dimensions verticales et horizontales (intérieur carte)

```
                    ┌─────────────────────────────────────────┐
                    │  CARTE DROITE  (225 × 320 px)           │
                    │  padding 15 px                          │
    0               ├─────────────────────────────────────────┤
                    │                                         │
   10 px            │     "ROUTAGE" (titre)     X=60          │
                    │                                         │
   35 px            │  "M'Sun"   (logo) X=10                   │
                    │   "PV"                                   │
   50 px            │            valeur routage (38)  X=80     │
   63 px            │                            "W"  X=165   │
                    │                                         │
  ───────────────────────────────────────────────────────── 105
   105              │  ┌───────────────────────────────┐      │
                    │  │ CUMULUS header    190 × 28     │      │
   133              │  │ "CUMULUS"          [AUTO]     │      │
  ───────────────────────────────────────────────────────── 133
   135              │  ┌───────────────────────────────┐      │
                    │  │ cumulus_container  190 × 100    │      │
                    │  │ ┌──────┐  ┌─────────────────┐  │      │
                    │  │ │ 60×100│  │ 120×100         │  │      │
                    │  │ │icône  │  │ "48°C"  Y=15   │  │      │
                    │  │ │chaud. │  │ LED (35×35) Y=55│  │      │
                    │  │ └──────┘  └─────────────────┘  │      │
   235              │  └───────────────────────────────┘      │
  ───────────────────────────────────────────────────────── 235
   240              │  ┌───────────────────────────────┐      │
                    │  │ Météo 4 jours    190 × 60     │      │
                    │  │ [J1] [J2] [J3] [J4]  47px×4   │      │
   300              │  └───────────────────────────────┘      │
  ───────────────────────────────────────────────────────── 240
                    │                                         │
   320              └─────────────────────────────────────────┘
```

---

## Récapitulatif des blocs (coordonnées dans la carte)

| Élément | X | Y | Largeur | Hauteur |
|--------|----|----|--------|--------|
| **Carte** (sur écran) | 245 | 60 | **225** | **320** |
| Titre "ROUTAGE" | 60 | 10 | — | — |
| Logo M'Sun PV | 10 | 35 | — | — |
| Valeur routage (W) | 80 | 50 | — | — |
| Unité "W" | 165 | 63 | — | — |
| **cumulus_header** | 5 | 105 | **190** | **28** |
| **cumulus_container** | 5 | 135 | **190** | **100** |
| ↳ thermo_bar (icône) | 0* | 0* | **60** | **100** |
| ↳ info_column (temp + LED) | 75* | 0* | **120** | **100** |
| **weather_forecast_container** | 5 | 240 | **190** | **60** |
| ↳ chaque colonne jour | i×47 | 0* | **47** | **60** |

\* position relative au parent.

---

## Largeurs utiles

- **Carte** : 225 px (zone contenu après padding : 195 px).
- **Blocs pleine largeur** : 190 px (marge 5 px à gauche, 225 − 5 − 190 = 30 px à droite avec padding).
- **Météo 4 jours** : 4 colonnes de 47 px = 188 px dans un conteneur 190 px.

## Hauteurs utiles

- **De 0 à 105** : ROUTAGE + logo + valeur (≈ 105 px).
- **105 à 133** : En-tête CUMULUS (28 px).
- **135 à 235** : Bloc cumulus (100 px).
- **240 à 300** : Météo 4 jours (60 px).

Total utilisé ≈ 300 px dans une zone utile de 290 px (léger chevauchement possible selon LVGL).
