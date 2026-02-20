# Installation Enphase V2

## Installation en ligne (Netlify)

Le firmware peut être flashé directement depuis le navigateur, sans télécharger de fichier.

- **URL** : https://enphase-install.netlify.app
- **Prérequis** : Chrome ou Edge (desktop), ESP32-S3 connecté en USB

---

## Déployer / mettre à jour le site (Netlify via Git)

### Première fois

1. **Construire le firmware** :
   ```bash
   pio run -t merged
   ```

2. **Copier le firmware** dans `netlify-deploy/` :
   ```bash
   copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\
   ```

3. **Connecter Netlify au dépôt** :
   - https://app.netlify.com → *Add new site* → *Import an existing project*
   - *Deploy with GitHub* → choisir `Enphase_V2`
   - Netlify détecte `netlify.toml` (publish = `netlify-deploy`)
   - *Deploy site*

4. **Nom personnalisé** : *Site configuration* → *Domain management* → *Options* → *Edit site name* → `enphase-install`

### Mise à jour du firmware

1. `pio run -t merged`
2. Copier le nouveau `firmware.merged.bin` dans `netlify-deploy/`
3. `git add netlify-deploy/firmware.merged.bin && git commit -m "Update firmware" && git push`
4. Netlify redéploie automatiquement

---

## Solution de secours (téléchargement manuel)

1. Téléchargez `firmware.merged.bin` depuis [GitHub Releases](https://github.com/bobendylon/Enphase_V2/releases)
2. https://espressif.github.io/esptool-js/ → ESP32-S3, adresse `0x0`, sélectionner le fichier → *Program*
