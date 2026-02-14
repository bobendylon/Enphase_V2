# Schéma – Réglages Web ENPHASE MONITOR

## Contexte

- **Page** : Zone **Réglages** du serveur web en mode **ENPHASE MONITOR** (page d’accueil Enphase dédiée).
- **Accès** : Lien « Réglages » dans le header de l’accueil Enphase.
- **Règle de navigation** : Depuis toute sous-page des Réglages (WiFi, Envoy, Météo, Tempo), le **retour** doit ramener sur **l’accueil ENPHASE MONITOR**, et non sur l’accueil web « classique » (MSunPV complet).

---

## Contenu de la zone Réglages

La zone Réglages contient **4 blocs** (ou 4 entrées vers des pages dédiées) :

| Bloc | Contenu | Remarque |
|------|--------|-----------|
| **Réglages WiFi** | Scan réseaux, liste des SSID, saisie SSID/mot de passe, bouton connexion. Infos de connexion (IP, état, signal RSSI, etc.). | Même logique qu’actuellement (page ou section WiFi existante). |
| **Config Envoy** | Les 4 paramètres Enphase (IP, user, pwd, serial) comme actuellement. **Message explicite** : « Un redémarrage (débrancher / rebrancher) est nécessaire pour la bonne prise en compte de la configuration. » | Réutiliser la config Envoy actuelle + ajouter le message redémarrage. |
| **Réglages Météo** | Même contenu que la page Météo actuelle (ville, API, prévisions, etc.). | Lien depuis Réglages vers cette page. **Retour** → accueil ENPHASE MONITOR. |
| **Réglages Tempo** | EDF Tempo : activable/désactivable, affichage Aujourd’hui/Demain, même page qu’actuellement. | Lien depuis Réglages. **Retour** → accueil ENPHASE MONITOR. |

En plus de ces 4 blocs, la zone Réglages est le lieu où l’on propose **« Revenir au mode d’origine »** (serveur web complet + choix d’écran) avec **saisie du MDP** (voir doc MDP / verrouillage).

---

## Schéma de navigation

```
                    ┌─────────────────────────────────────────────────────────┐
                    │  ACCUEIL ENPHASE MONITOR (☀️ ENPHASE MONITOR)            │
                    │  Header : date + heure, WiFi, Enphase, [Réglages]        │
                    │  Barre météo | Flux réseau | Carte horizontale           │
                    └───────────────────────────┬─────────────────────────────┘
                                                │
                        Clic « Réglages »       ▼
                    ┌─────────────────────────────────────────────────────────┐
                    │  RÉGLAGES (page ou section)                              │
                    │  [Retour → Accueil ENPHASE MONITOR]                     │
                    ├─────────────────────────────────────────────────────────┤
                    │  • Réglages WiFi      → /wifi (ou équivalent)            │
                    │  • Config Envoy      → /enphase ou page Envoy             │
                    │  • Réglages Météo    → /weather (ou équivalent)          │
                    │  • Réglages Tempo    → page Tempo                        │
                    │  • Revenir au mode d’origine (MDP) → déverrouillage + /  │
                    └───────┬─────────────┬─────────────┬─────────────┬───────┘
                            │             │             │             │
              Retour        │             │             │             │     Retour
              accueil      ▼             ▼             ▼             ▼     accueil
              ENPHASE   WiFi          Envoy        Météo          Tempo    ENPHASE
                    ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌──────────┐
                    │ Scan     │ │ 4 params │ │ Ville,   │ │ Activer  │
                    │ SSID     │ │ + msg    │ │ API,     │ │ Auj/Dem  │
                    │ Connexion│ │ redémar- │ │ prévis.  │ │          │
                    │ IP, RSSI │ │ rage     │ │          │ │          │
                    └──────────┘ └──────────┘ └──────────┘ └──────────┘
                            │             │             │             │
                            └─────────────┴──────┬──────┴─────────────┘
                                                │
                                    « Retour » → toujours accueil ENPHASE MONITOR
```

---

## Règles de retour (à implémenter)

| Page atteinte depuis Réglages | Cible du bouton / lien « Retour » |
|-------------------------------|------------------------------------|
| Réglages WiFi                 | Accueil ENPHASE MONITOR           |
| Config Envoy                  | Accueil ENPHASE MONITOR           |
| Réglages Météo                | Accueil ENPHASE MONITOR           |
| Réglages Tempo                | Accueil ENPHASE MONITOR           |

**Méthode possible** :  
- Soit une **query** ou un **paramètre** (ex. `?from=enphase`) sur l’URL des pages WiFi / Envoy / Météo / Tempo quand on y accède depuis Réglages Enphase ; le lien « Retour » pointe alors vers l’accueil Enphase au lieu de `/` ou `/info`.  
- Soit une **page Réglages Enphase** dédiée (ex. `/enphase/settings`) qui embarque ou redirige vers ces réglages, avec un bandeau « Retour → ENPHASE MONITOR » commun.

---

## Config Envoy – message redémarrage

Dans la page (ou le bloc) **Config Envoy**, en plus des 4 paramètres existants, afficher un message du type :

> **Pour une prise en compte correcte de la configuration Envoy, il est recommandé de redémarrer l’appareil (débrancher puis rebrancher).**

À placer après le formulaire ou en encart visible.

---

## Récapitulatif

| Élément | Détail |
|--------|--------|
| **Réglages WiFi** | Scan, connexion, infos (IP, signal, etc.) — comme actuellement. Retour → accueil Enphase. |
| **Config Envoy** | 4 paramètres + message « redémarrage (débrancher/rebrancher) pour bonne prise en compte ». Retour → accueil Enphase. |
| **Réglages Météo** | Même page que l’actuelle. Accès depuis Réglages ; retour → accueil ENPHASE MONITOR. |
| **Réglages Tempo** | Activable, même page qu’actuellement. Retour → accueil ENPHASE MONITOR. |
| **Retour** | Depuis toute sous-page des Réglages → toujours **accueil ENPHASE MONITOR**. |
| **Revenir au mode d’origine** | Dans Réglages : MDP puis redirection vers l’accueil web complet (`/`). |
