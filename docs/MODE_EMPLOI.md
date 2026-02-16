# Mode d’emploi — Enphase V2

**Firmware moniteur Enphase pour ESP32-S3 (écran 480×480, LVGL + serveur web)**

---

## 1. Qu’est-ce que c’est ?

Enphase V2 affiche sur un écran tactile les données de production et de consommation issues d’un **Enphase Envoy** (et optionnellement MQTT / météo). Vous pouvez aussi tout piloter et configurer via le **serveur web** intégré (Enphase Monitor, réglages WiFi, Envoy, MQTT, météo, etc.).

- **Écran LVGL** : vue principale Enphase (production, conso, flux PV ↔ maison ↔ réseau).
- **Web** : même vue + réglages (WiFi, Envoy, MQTT, météo, infos, OTA).

![Vue d’ensemble Enphase Monitor (web)](schema_enphase_monitor.png)

*Figure 1 — Schéma type de la page Enphase Monitor (cartes, flux, météo).*

---

## 2. Matériel et prérequis

- **Carte** : ESP32-S3 (ex. DevKitC avec écran RGB 480×480, PSRAM).
- **Fichier firmware** : un `.bin` **merged** (bootloader + table de partitions + application) ou, si vous compilez vous-même, les trois binaires séparés.
- **Navigateur** : Chrome ou Edge (pour le flash via le Web ; **pas Safari**, non supporté par esptool-js).
- **Câble USB** : liaison série vers l’ESP32-S3.

---

## 3. Première utilisation après flash

1. Alimenter l’ESP32-S3 et le connecter en USB si besoin.
2. **WiFi** : au premier démarrage, si le WiFi n’est pas configuré, l’ESP passe en mode point d’accès. Connectez-vous au réseau **Enphase-Monitor-Setup** (sans mot de passe), puis ouvrez **http://192.168.4.1** pour saisir le WiFi de votre box (SSID + mot de passe).
3. **Accès web** : une fois en WiFi, ouvrez dans un navigateur l’URL affichée (ou l’IP du module, ex. `http://192.168.1.xxx`). Vous êtes redirigé vers **Enphase Monitor**.
4. **Config Envoy** : dans Réglages → Config Envoy, renseignez l’IP de votre Envoy (et token si nécessaire). Le moniteur pourra alors afficher production et consommation.
5. **Écran** : l’écran tactile affiche la même vue ; la roue dentée donne accès aux Réglages (WiFi, Infos, Météo, Maint., Logs).
6. **EDF Tempo** (optionnel) : si vous êtes en offre Tempo, vous pouvez activer l’affichage des jours Bleu / Blanc / Rouge (aujourd’hui et demain) depuis **Réglages → Réglages Tempo + Infos** (page Info). Un bouton permet d’activer ou désactiver le suivi Tempo. **Demain en vert** = « en attente » : la couleur du lendemain n’est pas encore connue (RTE la publie en général vers 11 h ; avant cela, ou juste après minuit, l’appli affiche donc « En attente » en vert).

---

## 4. Configuration MQTT (publication vers Home Assistant)

Le moniteur peut **publier** les données Enphase (production, consommation, énergies du jour) sur un broker MQTT pour les utiliser dans **Home Assistant** ou un autre système.

### 4.1 Accès à la page Config MQTT

- **Enphase Monitor** → **Réglages** → **Config MQTT**, ou lien **Config MQTT** depuis la page Info.
- URL directe : `http://<IP-du-module>/mqtt`

### 4.2 Paramètres à configurer

| Paramètre | Description |
|-----------|-------------|
| **IP ou hostname du broker** | Adresse de votre broker MQTT (ex. 192.168.1.82). Laisser vide pour désactiver MQTT. |
| **Port** | En général 1883. |
| **Utilisateur / Mot de passe** | Optionnel, selon la configuration du broker. |
| **Préfixe des topics** | Préfixe commun à tous les topics publiés (défaut : `enphase_monitor`). Les topics sont alors `<prefixe>/power_production`, `<prefixe>/energy_produced_today`, etc. |
| **Intervalle de publication** | Période d’envoi des valeurs en secondes (entre 5 et 300, défaut : 15 s). |

Après modification, cliquer sur **Enregistrer et Redémarrer** pour appliquer.

### 4.3 Topics publiés (valeurs Enphase)

| Topic (sous le préfixe) | Description | Unité |
|------------------------|-------------|--------|
| `power_production` | Production instantanée | W |
| `power_consumption` | Consommation instantanée | W |
| `power_grid` | Puissance réseau (import / export) | W |
| `energy_produced_today` | Énergie produite (jour) | Wh |
| `energy_consumed_today` | Énergie consommée (jour) | Wh |
| `energy_imported_today` | Énergie importée du réseau (jour) | Wh |
| `energy_injected_today` | Énergie injectée au réseau (jour) | Wh |
| `status` | État connexion : `online` / `offline` | — |

Le topic `status` utilise le **LWT** (Last Will Testament) : si l’ESP se déconnecte du broker, le broker publie automatiquement `offline` sur ce topic.

*Exemple de vue dans MQTT Explorer (topic `enphase_monitor` et sous-topics avec valeurs) :*

![Topics MQTT Enphase dans MQTT Explorer](mqtt_explorer_enphase_monitor.png)

*Figure 2 — Arborescence et valeurs des topics publiés, vus dans MQTT Explorer.*

### 4.4 Tableau de contrôle sur la page MQTT

Sur la page Config MQTT, un **tableau** affiche pour chaque topic :

- **Topic publié** : le topic complet (préfixe + nom).
- **Valeur Enphase** : la valeur actuelle lue depuis l’Envoy (rafraîchie environ toutes les 2 secondes). Cela permet de vérifier que les données publiées correspondent bien à celles affichées par l’Enphase.
- **Unité** : W, Wh ou —.

Si les valeurs restent à « - », vérifier que la **Config Envoy** est correcte et que l’Enphase renvoie bien des données.

### 4.5 Test d’écoute MQTT

En bas de la page Config MQTT, un outil permet de **tester l’écoute** d’un topic (ex. `shellies/#`) pour vérifier la connexion au broker. Utile pour le diagnostic.

---

## 5. Flasher le firmware avec le fichier merged (navigateur)

Cette méthode permet de flasher **sans installer Python ni esptool** sur votre PC, en utilisant le **Web Serial** dans le navigateur.

### 5.1 Outil utilisé

**[ESP Tool (esptool-js)](https://espressif.github.io/esptool-js/)**

- Flasher directement depuis Chrome ou Edge.
- **Ne pas utiliser Safari** : l’outil ne fonctionne pas avec Safari.

### 5.2 Étapes

1. **Préparer le fichier merged**  
   Vous devez avoir **un seul fichier .bin** qui contient tout (bootloader + table de partitions + application), généré par votre build ou fourni avec le projet.  
   - Si vous compilez avec PlatformIO : vous pouvez créer ce merged avec `esptool.py merge_bin` (voir section 5).  
   - Sinon, utilisez le fichier `firmware.merged.bin` (ou nom équivalent) fourni.

2. **Ouvrir l’outil**  
   Allez sur : **https://espressif.github.io/esptool-js/**

3. **Connexion**  
   - Connectez l’ESP32-S3 en USB.
   - Cliquez sur **« Program »** (ou l’équivalent pour définir les fichiers à flasher).
   - Cliquez sur **« Connect »** (ou le bouton de connexion série). Le navigateur vous demandera de choisir le **port série** de l’ESP32-S3. Sélectionnez-le et validez.

4. **Vitesse de connexion (baudrate)**  
   - Utilisez **921600** si la connexion est stable (recommandé pour aller plus vite).  
   - En cas d’erreurs ou de échecs de flash, baissez à **460800**, puis éventuellement **230400** ou **115200**.  
   - Un câble USB de mauvaise qualité ou trop long peut obliger à rester à 115200.

5. **Table « Flash » : adresse et fichier**  
   - Dans le tableau **Flash Address | File** :  
     - **Adresse** : **`0x0`** (ou **`0x0000`**, c’est la même chose).  
     - **Fichier** : sélectionnez votre fichier **merged** (ex. `firmware.merged.bin`).  
   - Ne pas ajouter d’autres lignes pour un flash complet avec un seul merged.

6. **Options de flash (souvent par défaut)**  
   - **Flash Mode** : **qio** (ou « keep » si l’outil le propose et que la carte est déjà en qio).  
   - **Flash Size** : **16MB** si votre carte a 16 Mo de flash (comme dans la config du projet). Sinon adapter (ex. 8MB).  
   - Vous pouvez laisser **Flash Frequency** en « keep » ou selon les valeurs proposées.

7. **Lancer le flash**  
   Cliquez sur le bouton pour démarrer la programmation. Attendez la fin sans débrancher. Quand c’est terminé, vous pouvez déconnecter puis redémarrer l’ESP32-S3.

### 5.3 Pourquoi l’adresse **0x0** (0x0000) ?

Sur l’ESP32 (et ESP32-S3), au démarrage, le processeur lit la **flash à l’adresse 0** pour exécuter le **bootloader**. Ensuite le bootloader lit la **table de partitions** (vers 0x8000) puis charge l’**application** (souvent à partir de 0x10000).

Un fichier **merged** est construit exactement pour ça :

- les **premiers octets** du fichier = contenu de la flash à l’adresse **0x0** (bootloader) ;
- puis la table de partitions à l’offset correspondant à **0x8000** ;
- puis l’application à l’offset correspondant à **0x10000**.

Donc quand vous flashez **ce fichier unique à l’adresse 0x0**, vous écrivez d’un coup tout au bon endroit.  
**Résumé** : on met **0x0** (ou 0x0000) parce que le merged est conçu pour être écrit **dès le début de la flash**. Pas d’autre adresse à saisir pour un tel fichier.

---

## 6. Créer le fichier merged (si vous compilez le projet)

Si vous utilisez PlatformIO et voulez générer vous-même le `.bin` merged :

Après `pio run`, les binaires se trouvent en général dans `.pio/build/env:esp32-s3-devkitc-1/` (ou le nom de votre env) :

- `bootloader.bin`
- `partition_table.bin` (ou nom similaire)
- `firmware.bin` (l’application)

Avec **esptool** (Python) :

```bash
esptool.py --chip esp32s3 merge_bin -o firmware.merged.bin --flash_mode qio --flash_size 16MB ^
  0x0 bootloader.bin ^
  0x8000 partition_table.bin ^
  0x10000 firmware.bin
```

Sous Linux/macOS, remplacer `^` par `\` en fin de ligne.

Le fichier **firmware.merged.bin** ainsi obtenu est celui à flasher à l’adresse **0x0** avec [esptool-js](https://espressif.github.io/esptool-js/).

---

## 7. Récap : valeurs recommandées (esptool-js)

| Paramètre        | Valeur        |
|------------------|---------------|
| **Adresse flash**| **0x0**       |
| **Fichier**      | Votre `firmware.merged.bin` |
| **Baudrate**     | **921600** (ou 460800 / 115200 en cas de souci) |
| **Flash size**   | **16MB** (selon votre carte) |
| **Flash mode**   | **qio**       |

---

## 8. Dépannage rapide

- **Le navigateur ne propose pas le port série** : vérifier que le câble USB est bien reconnu (pilote CP210x ou CH340 selon la carte), utiliser Chrome/Edge, et réessayer après un rechargement de la page.
- **Échec au flash** : réduire le baudrate (460800 puis 115200), raccourcir/changer de câble USB, ne pas utiliser de hub USB si possible.
- **Safari** : esptool-js n’est pas supporté ; utiliser Chrome ou Edge.

---

## 9. Avertissement

L’utilisation du firmware et des interfaces (écran et web) est à vos risques et responsabilité. Voir **[Avertissement et limitation de responsabilité](DISCLAIMER.md)**.
