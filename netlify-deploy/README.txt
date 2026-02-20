NETLIFY DEPLOY — Enphase V2
===========================

Ce dossier est publié sur https://enphase-install.netlify.app via Netlify (Git).

CONTENU REQUIS
--------------
  - index.html      (page d'installation)
  - manifest.json   (config du flasher)
  - firmware.merged.bin   (À COPIER après chaque build)

MISE À JOUR DU FIRMWARE
-----------------------
Depuis le dossier racine du projet :

  1. pio run -t merged

  2. copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\

  3. git add netlify-deploy\firmware.merged.bin

  4. git commit -m "Update firmware"

  5. git push origin main

→ Netlify redéploie automatiquement.

Voir docs/INSTALLATION_UPLOAD_AUTOMATIQUE.md pour la doc complète.
