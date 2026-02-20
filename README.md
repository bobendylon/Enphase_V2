# Enphase V2

Firmware ESP32-S3 pour tableau de bord Enphase (écran RGB 480×480, LVGL).

## Installation en ligne

👉 **[Installer le firmware dans le navigateur](https://enphase-install.netlify.app)** (Chrome/Edge, ESP32-S3 en USB)

## Publication (développeur)

Commande rapide pour publier une mise à jour :

```
pio run -t merged
copy .pio\build\esp32-s3-devkitc-1\firmware.merged.bin netlify-deploy\
git add netlify-deploy\firmware.merged.bin && git commit -m "Update firmware" && git push origin main
```

📄 **[Documentation complète](docs/INSTALLATION_UPLOAD_AUTOMATIQUE.md)** — toutes les commandes et explications
