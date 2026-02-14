/**
 * @file lv_conf.h
 * Configuration LVGL pour MSunPV Monitor (ESP32-S3, écran 480x480 RGB565)
 * Permet à LVGL de trouver ce fichier via LV_CONF_INCLUDE_SIMPLE + include/
 */

#ifndef LV_CONF_H
#define LV_CONF_H

/* Couleur 16 bits (RGB565) pour l'écran */
#define LV_COLOR_DEPTH 16

/* Mémoire pour LVGL (64 Ko) */
#define LV_MEM_SIZE (64 * 1024U)

/* Backend de dessin : SW uniquement (vg_lite = NXP, inutile sur ESP32) */
#define LV_USE_DRAW_SW       1
#define LV_USE_DRAW_VG_LITE  0

/* Polices Montserrat utilisées par l'UI (16, 20, 26, 38) */
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_38 1

#endif /* LV_CONF_H */
