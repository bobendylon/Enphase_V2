DÉPLOIEMENT NETLIFY — Enphase V2
================================

Avant de déployer, tu DOIS ajouter le fichier firmware.merged.bin dans ce dossier.

ÉTAPE 1 — Construire le firmware
--------------------------------
Dans le dossier racine du projet (Enphase_V2), lance :
  pio run -t merged

Cela génère : .pio/build/esp32-s3-devkitc-1/firmware.merged.bin

ÉTAPE 2 — Copier le firmware
----------------------------
Copie le fichier firmware.merged.bin dans ce dossier (netlify-deploy).
Le dossier doit contenir :
  - index.html
  - manifest.json
  - firmware.merged.bin   ← TU DOIS L'AJOUTER

ÉTAPE 3 — Déployer sur Netlify Drop
-----------------------------------
1. Va sur https://app.netlify.com/drop
2. Glisse-dépose tout le dossier netlify-deploy (avec les 3 fichiers)
3. Netlify va déployer et te donner une URL (ex: random-xyz.netlify.app)

ÉTAPE 4 — Changer le nom du site (pour enphase-install)
-------------------------------------------------------
1. Connecte-toi à Netlify (ou crée un compte gratuit)
2. Va dans Site configuration → Domain management → Options → Edit site name
3. Change le nom en : enphase-install
4. Ton site sera accessible sur : https://enphase-install.netlify.app

POUR METTRE À JOUR
------------------
Après une modification du firmware :
1. pio run -t merged
2. Copie le nouveau firmware.merged.bin dans netlify-deploy
3. Re-déploie sur Netlify Drop (ou utilise Netlify CLI / Git si configuré)
