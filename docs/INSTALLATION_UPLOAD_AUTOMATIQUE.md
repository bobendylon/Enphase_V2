# Installation Enphase V2

## Pour les utilisateurs : installer le firmware

Le firmware peut être flashé directement depuis le navigateur, sans télécharger de fichier.

- **URL** : https://enphase-install.netlify.app
- **Prérequis** : Chrome ou Edge (desktop), ESP32-S3 DevKit connecté en USB

1. Ouvre l'URL ci-dessus
2. Connecte l'ESP32 en USB
3. Clique sur le bouton d'installation et sélectionne le port série
4. Attends la fin du flash

---

## Pour le développeur : publier une mise à jour

### Commandes à exécuter (une par une)

Depuis le dossier racine du projet (`Enphase_V2`), dans PowerShell ou CMD :

```
pio run -t merged
```

```
copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\
```

```
git add extra_script.py netlify-deploy\firmware.merged.bin
```

```
git commit -m "Update firmware"
```

```
git push origin main
```

Netlify redéploie automatiquement en 1 à 2 minutes.

---

### Détail des commandes

| Commande | Rôle |
|----------|------|
| `pio run -t merged` | Compile le projet et génère `firmware.merged.bin` |
| `copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\` | Copie le firmware dans le dossier de déploiement |
| `git add ...` | Prépare les fichiers pour le commit |
| `git commit -m "Update firmware"` | Crée le commit |
| `git push origin main` | Envoie sur GitHub → déclenche le déploiement Netlify |

---

### Première configuration (une seule fois)

1. **Connecter Netlify à GitHub**  
   - https://app.netlify.com → *Add new site* → *Import an existing project*  
   - *Deploy with GitHub* → choisir le dépôt `Enphase_V2`  
   - *Deploy site*

2. **Nom du site**  
   - *Site configuration* → *Domain management* → *Options* → *Edit site name*  
   - Nom : `enphase-install`  
   - URL finale : https://enphase-install.netlify.app

3. **Visibilité du dépôt GitHub**  
   - **Rendre privé** : *Settings* → *Danger Zone* → *Change repository visibility* → *Make private* → confirmer  
   - **Rendre public** (inverser) : *Settings* → *Danger Zone* → *Change repository visibility* → *Make public* → confirmer  
   - La page https://enphase-install.netlify.app fonctionne dans les deux cas  
   - Si privé : Netlify doit garder l'accès au dépôt (*Build & deploy* → *Repository*). Si le déploiement échoue, aller dans *Site configuration* → *Build & deploy* → *Link repository* et réautoriser l'accès.

---

### Déployer une mise à jour (à chaque nouvelle version)

| Étape | Action |
|-------|--------|
| 1 | Modifier le code, tester en local |
| 2 | `pio run -t merged` (compile et génère le merged.bin) |
| 3 | `copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\` |
| 4 | `git add netlify-deploy\firmware.merged.bin` (et autres fichiers modifiés si besoin) |
| 5 | `git commit -m "Update firmware"` |
| 6 | `git push origin main` |
| 7 | Attendre 1–2 min → Netlify redéploie → https://enphase-install.netlify.app est à jour |

---

### Fichiers concernés

| Fichier | Rôle |
|---------|------|
| `netlify-deploy/index.html` | Page d'installation web |
| `netlify-deploy/manifest.json` | Config pour le flasher (ESP32-S3, firmware à 0x0) |
| `netlify-deploy/firmware.merged.bin` | Firmware (généré par build, copié manuellement) |
| `netlify.toml` | Indique à Netlify de publier le dossier `netlify-deploy` |
| `extra_script.py` | Génère le merged.bin (mode flash DIO pour compatibilité web) |

---

### Flash en local (VS Code / PlatformIO)

Pour développer et flasher avec le câble USB :

- Bouton **Upload** dans VS Code, ou
- `pio run -t upload`

Le flash via le web et via PlatformIO utilisent le même `firmware.merged.bin` (mode DIO).
