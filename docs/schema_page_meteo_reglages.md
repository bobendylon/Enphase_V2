# Schéma – Page météo (Réglages > Meteo)

## Contexte

- **Écran** : 480 × 480 px
- **Onglet** : Réglages → « Meteo » (tab_meteo), scroll activé
- **Padding onglet** : 10 px → zone utile largeur **≈ 460 px** (cartes en 440 px pour marge)

---

## Vue d’ensemble des blocs

```
    0   ┌─────────────────────────────────────────────────────────┐
        │  ONGLET MÉTEO (tab_meteo) – scroll                      │
  10    ├─────────────────────────────────────────────────────────┤
        │  ┌─────────────────────────────────────────────────┐     │
        │  │ CARTE 1 – Météo actuelle        440 × ~110 px   │     │
        │  │  Ville (gauche)            MAJ: HH:MM (droite)  │     │
        │  │  [icône]  24°   Ciel dégagé                     │     │
        │  │  Ressenti: 12°C                                │     │
   ~130 │  └─────────────────────────────────────────────────┘     │
        │  ┌─────────────────────────────────────────────────┐     │
        │  │ CARTE 2 – Détails              440 × ~56 px      │     │
        │  │  Humidité    │   Vent    │   Pression           │     │
        │  │    65 %      │  3.2 m/s  │   1013 hPa           │     │
   ~198 │  └─────────────────────────────────────────────────┘     │
        │  ┌─────────────────────────────────────────────────┐     │
        │  │ CARTE 3 – Prévisions 4 jours   440 × ~80 px      │     │
        │  │   L    M    M    J     (jour)                   │     │
        │  │  [☀] [⛅] [☀] [🌧]    (icône)                   │     │
        │  │  12°  14°  11°  9°    (temp)                    │     │
   ~290 │  └─────────────────────────────────────────────────┘     │
        │  Configuration : portail web (IP/weather)                 │
   ~310 │                                                           │
        └─────────────────────────────────────────────────────────┘
```

---

## Détail carte 1 – Météo actuelle

| Élément        | Position (dans la carte) | Taille / remarque                    |
|----------------|--------------------------|-------------------------------------|
| Ville          | Haut gauche              | Police 20, couleur type #60a5fa     |
| MAJ            | Haut droite              | « MAJ: HH:MM », police 14, gris      |
| Icône météo    | Gauche, centré vertical  | ~56×56 px (zoom 220)                 |
| Température    | À droite de l’icône     | Gros chiffre (26), couleur #fbbf24   |
| Condition      | Sous la temp            | « Ciel dégagé », police 14           |
| Ressenti       | Bas gauche              | « Ressenti: X°C », police 14, gris   |

---

## Détail carte 2 – Détails

| Colonne   | Contenu type | Position X (approx) |
|-----------|---------------|----------------------|
| Humidité  | Label + valeur (ex. 65 %) | 8 px                 |
| Vent      | Label + valeur (ex. 3.2 m/s) | 8 + 140 px        |
| Pression  | Label + valeur (ex. 1013 hPa) | 8 + 280 px        |

Largeur utile ≈ 440 px, 3 colonnes égales ou proportionnelles.

---

## Détail carte 3 – Prévisions 4 jours

| Colonne | Contenu | Largeur approx |
|---------|---------|----------------|
| 1       | Jour (L/M/…) + icône + temp | ~106 px |
| 2       | idem                        | ~106 px |
| 3       | idem                        | ~106 px |
| 4       | idem                        | ~106 px |

Données : `weather_forecast_days[1..4]`, `weather_forecast_codes[1..4]`, `weather_forecast_temps[1..4]`.

---

## Récapitulatif positions (Y depuis haut de l’onglet)

| Bloc                    | Y début | Hauteur | Y fin   |
|-------------------------|---------|---------|---------|
| Carte 1 – Météo actuelle | 0       | ~110–128 | ~130   |
| Carte 2 – Détails      | ~142    | ~56    | ~198   |
| Carte 3 – Prévisions   | ~210    | ~80    | ~290   |
| Ligne « Config »       | ~302    | —      | —      |

---

## Contraintes techniques (pour l’implémentation)

- **Polices LVGL** : utiliser uniquement celles activées (14, 16, 20, 26, 38).
- **Opacité** : pas de `LV_OPA_25` si non définie → utiliser `LV_OPA_20` ou `LV_OPA_30`.
- **Mise à jour** : remplir les widgets dans `updateSettingsUI()` quand `currentPage == 3` (écran Réglages).
- **Données** : `weather_city`, `weather_temp`, `weather_condition`, `weather_feels_like`, `weather_last_update`, `weather_humidity`, `weather_wind_speed`, `weather_pressure`, et pour les 4 jours les tableaux `weather_forecast_*` (index 1 à 4).
