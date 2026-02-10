/**
 * Icônes météo pour LVGL 9.4.0
 * Taille: 64×64 pixels
 * Format: ARGB8888 avec transparence
 * Usage: lv_img_set_src(img, &icon_sun);
 * Zoom: lv_img_set_zoom(img, 128); // 50% = 32×32px
 */

#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

#include "lvgl.h"

extern const lv_image_dsc_t icon_sun;
extern const lv_image_dsc_t icon_cloud;
extern const lv_image_dsc_t icon_cloud_sun;
extern const lv_image_dsc_t icon_cloud_sun2;
extern const lv_image_dsc_t icon_cloud_lightning;
extern const lv_image_dsc_t icon_cloud_lightning2;
extern const lv_image_dsc_t icon_cloud_rain;
extern const lv_image_dsc_t icon_cloud_sun_rain;
extern const lv_image_dsc_t icon_rain_drop;
extern const lv_image_dsc_t icon_rain_drops;
extern const lv_image_dsc_t icon_snow;
extern const lv_image_dsc_t icon_snow2;
extern const lv_image_dsc_t icon_moon;
extern const lv_image_dsc_t icon_moon_cloud;
extern const lv_image_dsc_t icon_moon_cloud2;
extern const lv_image_dsc_t icon_cloud_rain2;
extern const lv_image_dsc_t icon_cloud_sun_rain2;
extern const lv_image_dsc_t icon_cloud_sun_rain3;
extern const lv_image_dsc_t icon_snow_rain;
extern const lv_image_dsc_t icon_cloud_snow;
extern const lv_image_dsc_t icon_moon_cloud_snow;
extern const lv_image_dsc_t icon_cloud_sun_snow;
extern const lv_image_dsc_t icon_cloud_rain3;
extern const lv_image_dsc_t icon_cloud_lightning_rain;
extern const lv_image_dsc_t icon_thermometer_cold;
extern const lv_image_dsc_t icon_thermometer_medium;
extern const lv_image_dsc_t icon_thermometer_hot;
extern const lv_image_dsc_t icon_na;
extern const lv_image_dsc_t icon_wind;
extern const lv_image_dsc_t icon_fog;

#endif // WEATHER_ICONS_H
