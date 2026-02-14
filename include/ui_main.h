/*
 * UI PAGE PRINCIPALE - Moniteur Solaire MSunPV
 * VERSION 3.2 - Badge décalé 8px à gauche (3px de plus)
 */

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include "weather_icons.h"
#include "module_weather.h"
#include "module_enphase.h"
#include "module_msunpv.h"
#include "module_tempo.h"

// Icônes header WiFi / MQTT / Shelly / Enphase / Réglages (déclarations extern des .c du projet)
extern const lv_image_dsc_t wifi_cercle_vert;
extern const lv_image_dsc_t wifi_barre_oblique;
extern const lv_image_dsc_t mqtt_png;
extern const lv_image_dsc_t mqtt_png_gris;
extern const lv_image_dsc_t Shelly32;
extern const lv_image_dsc_t Shelly32_gris;
extern const lv_image_dsc_t Enphase_logo;
extern const lv_image_dsc_t Enphase_logo_gris;
extern const lv_image_dsc_t roue_dentee;

// Icônes carte flux PV → Maison → Réseau
extern const lv_image_dsc_t panneau_solaire;
extern const lv_image_dsc_t panneaux_solaires;
extern const lv_image_dsc_t maison;
extern const lv_image_dsc_t reseau_electrique;
extern const lv_image_dsc_t chauffe_eau;
extern const lv_image_dsc_t Chauffeeaucartedroite;
extern const lv_image_dsc_t icoflechedroiteverte;
extern const lv_image_dsc_t icoflechedroiteorange;
extern const lv_image_dsc_t icoflechegaucheorange;
extern const lv_image_dsc_t icoflechegaucheverte;
extern const lv_image_dsc_t cartes_et_drapeaux;

// Structure météo
#ifndef WEATHERDAY_DEFINED
#define WEATHERDAY_DEFINED
struct WeatherDay {
  int day;
  int icon;
  int tempMin;
  int tempMax;
};
#endif

// Variables externes
extern lv_obj_t *screenMain;
extern lv_obj_t *screenEnphase;  // V15.0 - Écran Enphase (layout réduit + footer météo)
extern float solarProd;
extern float homeConso;
extern float routerPower;
extern float waterTemp;
extern float tempExt;
extern float tempSalon;
extern float consoJour;

// Variables M'SunPV (V3.0)
extern String msunpv_status;
extern void msunpv_sendCommand(int cmd);

// Variable icône météo (V10.0)
lv_obj_t *weather_icon = NULL;

// LED sticky green (V10.0)
extern bool ledLockedGreen;  // ← AJOUTER CETTE LIGNE

// V15.0 - Écran actif : 0=MQTT, 1=Enphase (date orange)
extern uint8_t activeScreenType;
extern bool ledStartupLock;  // V10.0 - Verrouillage au démarrage
extern unsigned long startupTime;  // V10.0 - Temps du démarrage
extern bool mqttDataReceived;  // V10.0 - Flag données MQTT reçues

// Labels dynamiques
lv_obj_t *label_time;
lv_obj_t *label_date;
lv_obj_t *label_prod_value;
lv_obj_t *label_prod_unit;
lv_obj_t *label_conso_value;
lv_obj_t *label_conso_unit;
lv_obj_t *label_conso_jour_value;
lv_obj_t *label_router_value;
lv_obj_t *label_router_unit;
lv_obj_t *label_water_temp_value;
lv_obj_t *label_temp_ext;
lv_obj_t *label_temp_salon;
lv_obj_t *img_cumulus_right = NULL;
lv_obj_t *obj_led_indicator;
lv_obj_t *led_wifi;
lv_obj_t *led_mqtt;
lv_obj_t *led_shelly;
lv_obj_t *led_enphase;
lv_obj_t *led_settings;

// Objets M'SunPV (V3.0)
lv_obj_t *label_msunpv_status;
lv_obj_t *popup_msunpv;
static lv_timer_t * msunpv_autoclose_timer = NULL;

// Popup météo (détail jour + phase lune)
lv_obj_t *popup_weather = NULL;
static lv_timer_t * weather_autoclose_timer = NULL;

// Objets Météo 4 jours (V12.4)
lv_obj_t *weather_forecast_container;
lv_obj_t *weather_day_label[4];
lv_obj_t *weather_icon_img[4];
lv_obj_t *weather_temp_label[4];

// V15.0 - Écran Enphase (labels et icônes)
lv_obj_t *label_ep_date;
lv_obj_t *label_ep_time;
lv_obj_t *label_ep_prod;
lv_obj_t *label_ep_conso;
lv_obj_t *label_ep_prod_jour;
lv_obj_t *label_ep_conso_jour;
lv_obj_t *label_ep_prod_jour_title = NULL;  // Titre ligne 1 carte droite (PRODUCTION JOUR / ROUTAGE)
lv_obj_t *label_ep_prod_jour_unit = NULL;  // Unité ligne 1 (kWh / %)
lv_obj_t *led_ep_wifi;
lv_obj_t *led_ep_mqtt;
lv_obj_t *led_ep_shelly;
lv_obj_t *led_ep_enphase;
lv_obj_t *led_ep_settings;
lv_obj_t *label_ep_weather_city;
lv_obj_t *img_ep_weather_icon;
lv_obj_t *label_ep_weather_temp;
lv_obj_t *label_ep_weather_day[5];
lv_obj_t *img_ep_weather_icon_day[5];
lv_obj_t *label_ep_weather_temp_day[5];
// Zone flux Enphase (PV → Maison → Réseau)
lv_obj_t *zone_flow_left_ep = NULL;
lv_obj_t *img_flow_pv_ep = NULL;
lv_obj_t *img_flow_maison_ep = NULL;
lv_obj_t *img_flow_reseau_ep = NULL;
lv_obj_t *img_arrow_pv_house_ep = NULL;
lv_obj_t *img_arrow_house_grid_ep = NULL;
lv_obj_t *label_flow_pv_val_ep = NULL;
lv_obj_t *label_flow_grid_val_ep = NULL;
lv_obj_t *obj_flow_state_ep = NULL;   // Badge Import / Auto / Export (tiers droit flux)
lv_obj_t *label_flow_state_ep = NULL;

// Carte flux PV → Maison → Réseau (en bas de la carte gauche)
#define FLOW_THRESHOLD_PV_W 100
lv_obj_t *zone_flow_left = NULL;
lv_obj_t *img_flow_pv = NULL;
lv_obj_t *img_flow_maison = NULL;
lv_obj_t *img_flow_reseau = NULL;
lv_obj_t *img_arrow_pv_house = NULL;
lv_obj_t *img_arrow_house_grid = NULL;
lv_obj_t *label_flow_pv_val = NULL;
lv_obj_t *label_flow_grid_val = NULL;

// Pages
extern lv_obj_t *screenStats;
extern lv_obj_t *screenInfo;
extern lv_obj_t *screenSettings;
extern int currentPage;

// Callbacks boutons
void btn_stats_clicked(lv_event_t * e) {
  currentPage = 1;
  lv_screen_load(screenStats);
}

void btn_info_clicked(lv_event_t * e) {
  currentPage = 2;
  lv_screen_load(screenInfo);
}

void btn_settings_clicked(lv_event_t * e) {
  currentPage = 3;
  lv_screen_load(screenSettings);
}

// Forward declarations - Callbacks WiFi
void wifi_scan_clicked(lv_event_t *e);
void wifi_network_selected(lv_event_t *e);
void wifi_connect_clicked(lv_event_t *e);
void wifi_clear_clicked(lv_event_t *e);

// Réglages (page ouverte par la roue dentée) — callbacks fournis par le .ino
extern void settings_back_to_main(void);
extern void settings_do_restart(void);
extern void settings_do_toggle_flip(void);
extern void settings_do_factory_reset(void);
extern void settings_get_log_text(char* buf, int maxLen);

static lv_obj_t *label_settings_wifi = NULL;
static lv_obj_t *label_settings_meteo = NULL;
static lv_obj_t *label_settings_logs = NULL;
// Onglet Infos : labels séparés pour couleurs (nom + valeur par ligne)
static lv_obj_t *label_infos_ip_val = NULL;
static lv_obj_t *label_infos_shelly1_val = NULL;
static lv_obj_t *label_infos_shelly2_val = NULL;
static lv_obj_t *label_infos_enphase_val = NULL;
static lv_obj_t *label_infos_msunpv_val = NULL;
static lv_obj_t *label_infos_mqtt_val = NULL;
static lv_obj_t *reset_confirm_popup = NULL;

void settings_open_clicked(lv_event_t * e) {
  (void)e;
  currentPage = 3;
  lv_screen_load(screenSettings);
}

void settings_back_clicked(lv_event_t * e) {
  (void)e;
  settings_back_to_main();
}

void settings_restart_clicked(lv_event_t * e) {
  (void)e;
  settings_do_restart();
}

void settings_flip_clicked(lv_event_t * e) {
  (void)e;
  settings_do_toggle_flip();
}

void settings_reset_cancel_clicked(lv_event_t * e) {
  (void)e;
  if (reset_confirm_popup) {
    lv_obj_del(reset_confirm_popup);
    reset_confirm_popup = NULL;
  }
}

void settings_reset_confirm_clicked(lv_event_t * e) {
  (void)e;
  lv_obj_t *popup = (lv_obj_t*)lv_event_get_user_data(e);
  if (!popup) return;
  lv_obj_del(popup);
  if (reset_confirm_popup) reset_confirm_popup = NULL;
  settings_do_factory_reset();
}

static void settings_reset_show_second_confirm(lv_event_t * e) {
  (void)e;
  if (reset_confirm_popup) {
    lv_obj_del(reset_confirm_popup);
    reset_confirm_popup = NULL;
  }
  reset_confirm_popup = lv_obj_create(lv_screen_active());
  lv_obj_set_size(reset_confirm_popup, 400, 220);
  lv_obj_center(reset_confirm_popup);
  lv_obj_set_style_bg_color(reset_confirm_popup, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(reset_confirm_popup, 2, 0);
  lv_obj_set_style_border_color(reset_confirm_popup, lv_color_hex(0xef4444), 0);
  lv_obj_set_style_radius(reset_confirm_popup, 12, 0);
  lv_obj_set_style_pad_all(reset_confirm_popup, 16, 0);
  lv_obj_t *msg = lv_label_create(reset_confirm_popup);
  lv_label_set_text(msg, "Derniere confirmation.\nTout sera efface.");
  lv_obj_set_style_text_font(msg, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(msg, lv_color_hex(0xffffff), 0);
  lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(msg, 360);
  lv_obj_t *btn_cancel = lv_btn_create(reset_confirm_popup);
  lv_obj_set_size(btn_cancel, 160, 44);
  lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 16, -16);
  lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x374151), 0);
  lv_obj_add_event_cb(btn_cancel, settings_reset_cancel_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lc = lv_label_create(btn_cancel);
  lv_label_set_text(lc, "Annuler");
  lv_obj_set_style_text_font(lc, &lv_font_montserrat_16, 0);
  lv_obj_center(lc);
  lv_obj_t *btn_yes = lv_btn_create(reset_confirm_popup);
  lv_obj_set_size(btn_yes, 160, 44);
  lv_obj_align(btn_yes, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
  lv_obj_set_style_bg_color(btn_yes, lv_color_hex(0xef4444), 0);
  lv_obj_add_event_cb(btn_yes, settings_reset_confirm_clicked, LV_EVENT_CLICKED, reset_confirm_popup);
  lv_obj_t *ly = lv_label_create(btn_yes);
  lv_label_set_text(ly, "OUI tout effacer");
  lv_obj_set_style_text_font(ly, &lv_font_montserrat_16, 0);
  lv_obj_center(ly);
}

void settings_reset_clicked(lv_event_t * e) {
  (void)e;
  if (reset_confirm_popup) return;
  reset_confirm_popup = lv_obj_create(lv_screen_active());
  lv_obj_set_size(reset_confirm_popup, 400, 200);
  lv_obj_center(reset_confirm_popup);
  lv_obj_set_style_bg_color(reset_confirm_popup, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(reset_confirm_popup, 2, 0);
  lv_obj_set_style_border_color(reset_confirm_popup, lv_color_hex(0xf59e0b), 0);
  lv_obj_set_style_radius(reset_confirm_popup, 12, 0);
  lv_obj_set_style_pad_all(reset_confirm_popup, 16, 0);
  lv_obj_t *msg = lv_label_create(reset_confirm_popup);
  lv_label_set_text(msg, "Tout sera efface.\nConfirmer ?");
  lv_obj_set_style_text_font(msg, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(msg, lv_color_hex(0xffffff), 0);
  lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_width(msg, 360);
  lv_obj_t *btn_cancel = lv_btn_create(reset_confirm_popup);
  lv_obj_set_size(btn_cancel, 160, 44);
  lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 16, -16);
  lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x374151), 0);
  lv_obj_add_event_cb(btn_cancel, settings_reset_cancel_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lc = lv_label_create(btn_cancel);
  lv_label_set_text(lc, "Annuler");
  lv_obj_set_style_text_font(lc, &lv_font_montserrat_16, 0);
  lv_obj_center(lc);
  lv_obj_t *btn_ok = lv_btn_create(reset_confirm_popup);
  lv_obj_set_size(btn_ok, 160, 44);
  lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -16, -16);
  lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xf59e0b), 0);
  lv_obj_add_event_cb(btn_ok, settings_reset_show_second_confirm, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lo = lv_label_create(btn_ok);
  lv_label_set_text(lo, "Confirmer");
  lv_obj_set_style_text_font(lo, &lv_font_montserrat_16, 0);
  lv_obj_center(lo);
}

// ============================================
// CALLBACKS M'SunPV (V3.0)
// ============================================
static void msunpv_autoclose_cb(lv_timer_t * t) {
  (void)t;
  if (msunpv_autoclose_timer) {
    lv_timer_del(msunpv_autoclose_timer);
    msunpv_autoclose_timer = NULL;
  }
  if (popup_msunpv) {
    lv_obj_del(popup_msunpv);
    popup_msunpv = NULL;
  }
}

void msunpv_close_popup(lv_event_t * e) {
  if (msunpv_autoclose_timer) {
    lv_timer_del(msunpv_autoclose_timer);
    msunpv_autoclose_timer = NULL;
  }
  if (popup_msunpv) {
    lv_obj_del(popup_msunpv);
    popup_msunpv = NULL;
  }
}

void msunpv_cmd_auto(lv_event_t * e) {
  msunpv_sendCommand(2);  // AUTO = 2
  msunpv_status = "AUTO";
  Serial.println("[V3.0] Commande AUTO envoyée");
  
  // Fermer popup après 500ms et annuler l'autoclose 10s
  lv_timer_create([](lv_timer_t * timer) {
    if (msunpv_autoclose_timer) { lv_timer_del(msunpv_autoclose_timer); msunpv_autoclose_timer = NULL; }
    if (popup_msunpv) {
      lv_obj_del(popup_msunpv);
      popup_msunpv = NULL;
    }
    lv_timer_del(timer);
  }, 500, NULL);
}

void msunpv_cmd_manu(lv_event_t * e) {
  msunpv_sendCommand(1);  // MANU = 1
  msunpv_status = "MANU";
  Serial.println("[V3.0] Commande MANU envoyée");
  
  // Fermer popup après 500ms et annuler l'autoclose 10s
  lv_timer_create([](lv_timer_t * timer) {
    if (msunpv_autoclose_timer) { lv_timer_del(msunpv_autoclose_timer); msunpv_autoclose_timer = NULL; }
    if (popup_msunpv) {
      lv_obj_del(popup_msunpv);
      popup_msunpv = NULL;
    }
    lv_timer_del(timer);
  }, 500, NULL);
}

void msunpv_cmd_off(lv_event_t * e) {
  msunpv_sendCommand(0);  // OFF = 0
  msunpv_status = "OFF";
  Serial.println("[V3.0] Commande OFF envoyée");
  
  // Fermer popup après 500ms et annuler l'autoclose 10s
  lv_timer_create([](lv_timer_t * timer) {
    if (msunpv_autoclose_timer) { lv_timer_del(msunpv_autoclose_timer); msunpv_autoclose_timer = NULL; }
    if (popup_msunpv) {
      lv_obj_del(popup_msunpv);
      popup_msunpv = NULL;
    }
    lv_timer_del(timer);
  }, 500, NULL);
}

void msunpv_open_popup(lv_event_t * e) {
  if (popup_msunpv) return;  // Déjà ouvert
  
  // Overlay
  popup_msunpv = lv_obj_create(lv_screen_active());
  lv_obj_set_size(popup_msunpv, 480, 480);
  lv_obj_set_pos(popup_msunpv, 0, 0);
  lv_obj_set_style_bg_color(popup_msunpv, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(popup_msunpv, LV_OPA_70, 0);
  lv_obj_set_style_border_width(popup_msunpv, 0, 0);
  lv_obj_clear_flag(popup_msunpv, LV_OBJ_FLAG_SCROLLABLE);
  
  // Dialog box
  lv_obj_t *dialog = lv_obj_create(popup_msunpv);
  lv_obj_set_size(dialog, 380, 320);
  lv_obj_center(dialog);
  lv_obj_set_style_bg_color(dialog, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(dialog, 2, 0);
  lv_obj_set_style_border_color(dialog, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_radius(dialog, 16, 0);
  lv_obj_set_style_pad_all(dialog, 20, 0);
  
  // Titre
  lv_obj_t *title = lv_label_create(dialog);
  lv_label_set_text(title, "CONTROLE CUMULUS");
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);
  
  // Info état actuel
  lv_obj_t *info = lv_label_create(dialog);
  char infoText[64];
  sprintf(infoText, "Etat: %s | Temp: %.0f°C", msunpv_status.c_str(), waterTemp);
  lv_label_set_text(info, infoText);
  lv_obj_set_style_text_color(info, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(info, &lv_font_montserrat_16, 0);
  lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 35);
  
  // Bouton AUTO
  lv_obj_t *btn_auto = lv_btn_create(dialog);
  lv_obj_set_size(btn_auto, 340, 50);
  lv_obj_set_pos(btn_auto, 0, 70);
  lv_obj_set_style_bg_color(btn_auto, lv_color_hex(COLOR_MSUNPV_AUTO), 0);
  lv_obj_set_style_radius(btn_auto, 10, 0);
  lv_obj_add_event_cb(btn_auto, msunpv_cmd_auto, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *label_auto = lv_label_create(btn_auto);
  lv_label_set_text(label_auto, "AUTO - Gestion automatique");
  lv_obj_set_style_text_font(label_auto, &lv_font_montserrat_16, 0);
  lv_obj_center(label_auto);
  
  // Bouton MANU
  lv_obj_t *btn_manu = lv_btn_create(dialog);
  lv_obj_set_size(btn_manu, 340, 50);
  lv_obj_set_pos(btn_manu, 0, 130);
  lv_obj_set_style_bg_color(btn_manu, lv_color_hex(COLOR_MSUNPV_MANU), 0);
  lv_obj_set_style_radius(btn_manu, 10, 0);
  lv_obj_add_event_cb(btn_manu, msunpv_cmd_manu, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *label_manu = lv_label_create(btn_manu);
  lv_label_set_text(label_manu, "MANU - Marche forcee");
  lv_obj_set_style_text_font(label_manu, &lv_font_montserrat_16, 0);
  lv_obj_center(label_manu);
  
  // Bouton OFF
  lv_obj_t *btn_off = lv_btn_create(dialog);
  lv_obj_set_size(btn_off, 340, 50);
  lv_obj_set_pos(btn_off, 0, 190);
  lv_obj_set_style_bg_color(btn_off, lv_color_hex(COLOR_MSUNPV_OFF), 0);
  lv_obj_set_style_radius(btn_off, 10, 0);
  lv_obj_add_event_cb(btn_off, msunpv_cmd_off, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *label_off = lv_label_create(btn_off);
  lv_label_set_text(label_off, "OFF - Arret");
  lv_obj_set_style_text_color(label_off, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_off, &lv_font_montserrat_16, 0);
  lv_obj_center(label_off);
  
  // Bouton Fermer
  lv_obj_t *btn_close = lv_btn_create(dialog);
  lv_obj_set_size(btn_close, 340, 40);
  lv_obj_set_pos(btn_close, 0, 255);
  lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x374151), 0);
  lv_obj_set_style_radius(btn_close, 10, 0);
  lv_obj_add_event_cb(btn_close, msunpv_close_popup, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t *label_close = lv_label_create(btn_close);
  lv_label_set_text(label_close, "Fermer");
  lv_obj_set_style_text_font(label_close, &lv_font_montserrat_16, 0);
  lv_obj_center(label_close);

  msunpv_autoclose_timer = lv_timer_create(msunpv_autoclose_cb, 10000, NULL);
  lv_timer_set_repeat_count(msunpv_autoclose_timer, 1);
}

static void weather_autoclose_cb(lv_timer_t * t) {
  (void)t;
  if (weather_autoclose_timer) {
    lv_timer_del(weather_autoclose_timer);
    weather_autoclose_timer = NULL;
  }
  if (popup_weather) {
    lv_obj_del(popup_weather);
    popup_weather = NULL;
  }
}

void weather_close_popup(lv_event_t * e) {
  (void)e;
  if (weather_autoclose_timer) {
    lv_timer_del(weather_autoclose_timer);
    weather_autoclose_timer = NULL;
  }
  if (popup_weather) {
    lv_obj_del(popup_weather);
    popup_weather = NULL;
  }
}

void weather_open_popup(lv_event_t * e) {
  if (popup_weather) return;
  (void)e;

  popup_weather = lv_obj_create(lv_screen_active());
  lv_obj_set_size(popup_weather, 480, 480);
  lv_obj_set_pos(popup_weather, 0, 0);
  lv_obj_set_style_bg_color(popup_weather, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(popup_weather, LV_OPA_70, 0);
  lv_obj_set_style_border_width(popup_weather, 0, 0);
  lv_obj_clear_flag(popup_weather, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(popup_weather, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(popup_weather, weather_close_popup, LV_EVENT_CLICKED, NULL);

  lv_obj_t *dialog = lv_obj_create(popup_weather);
  lv_obj_set_size(dialog, 400, 340);
  lv_obj_center(dialog);
  lv_obj_set_style_bg_color(dialog, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(dialog, 2, 0);
  lv_obj_set_style_border_color(dialog, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_radius(dialog, 16, 0);
  lv_obj_set_style_pad_all(dialog, 16, 0);
  lv_obj_clear_flag(dialog, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(dialog, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(dialog, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_flag(dialog, LV_OBJ_FLAG_EVENT_BUBBLE);

  char titleBuf[64];
  snprintf(titleBuf, sizeof(titleBuf), "%s - Meteo", weather_city.c_str());
  lv_obj_t *title = lv_label_create(dialog);
  lv_label_set_text(title, titleBuf);
  lv_obj_set_style_text_color(title, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

  int slotW = 95;
  int slotY = 38;
  const char* slots[] = {"Matin", "Midi", "Soir"};
  int temps[3] = {weather_today_morning_temp, weather_today_noon_temp, weather_today_evening_temp};
  int codes[3] = {weather_today_morning_code, weather_today_noon_code, weather_today_evening_code};
  if (temps[0] == 0 && temps[1] == 0 && temps[2] == 0) {
    temps[0] = temps[1] = temps[2] = (int)weather_temp;
    codes[0] = codes[1] = codes[2] = weather_code;
  }
  for (int i = 0; i < 3; i++) {
    lv_obj_t *col = lv_obj_create(dialog);
    lv_obj_set_size(col, slotW - 4, 132);
    lv_obj_set_pos(col, 16 + i * (slotW + 2), slotY);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);
    lv_obj_t *lbl = lv_label_create(col);
    lv_label_set_text(lbl, slots[i]);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xd1d5db), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 2);
    lv_obj_t *img = lv_img_create(col);
    lv_img_set_src(img, weather_getIconFromCode(codes[i]));
    lv_img_set_zoom(img, 320);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 6);
    char tbuf[12];
    snprintf(tbuf, sizeof(tbuf), "%d°", temps[i]);
    lv_obj_t *tlabel = lv_label_create(col);
    lv_label_set_text(tlabel, tbuf);
    lv_obj_set_style_text_color(tlabel, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(tlabel, &lv_font_montserrat_16, 0);
    lv_obj_align(tlabel, LV_ALIGN_BOTTOM_MID, 0, 0);
  }

  int phaseY = slotY + 132 + 18;
  time_t now = time(NULL);
  double dayInCycle = 0.0;
  if (now >= 947116440) {
    double daysSince = (double)(now - 947116440) / 86400.0;
    dayInCycle = daysSince - 29.530588 * (int)(daysSince / 29.530588);
  }
  int phaseIndex = 0;
  if (dayInCycle < 1.8f) phaseIndex = 0;
  else if (dayInCycle < 7.4f) phaseIndex = 1;
  else if (dayInCycle < 9.2f) phaseIndex = 2;
  else if (dayInCycle < 14.7f) phaseIndex = 3;
  else if (dayInCycle < 16.5f) phaseIndex = 4;
  else if (dayInCycle < 22.1f) phaseIndex = 5;
  else if (dayInCycle < 23.9f) phaseIndex = 6;
  else phaseIndex = 7;
  const char* phaseNames[] = {"Nouvelle lune", "Croissant", "Premier quartier", "Gibbeuse croiss.", "Pleine lune", "Gibbeuse decr.", "Dernier quartier", "Croissant"};

  lv_obj_t *phaseRow = lv_obj_create(dialog);
  lv_obj_set_size(phaseRow, 368, 56);
  lv_obj_set_pos(phaseRow, 16, phaseY);
  lv_obj_set_style_bg_opa(phaseRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(phaseRow, 0, 0);
  lv_obj_clear_flag(phaseRow, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_clear_flag(phaseRow, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(phaseRow, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *moonImg = lv_img_create(phaseRow);
  lv_img_set_src(moonImg, &icon_moon);
  lv_img_set_zoom(moonImg, 320);
  lv_obj_align(moonImg, LV_ALIGN_LEFT_MID, 0, 0);
  if (phaseIndex != 4) {
    lv_obj_set_style_image_recolor(moonImg, lv_color_hex(0x6b7280), 0);
    lv_obj_set_style_image_recolor_opa(moonImg, LV_OPA_50, 0);
  }
  lv_obj_t *phaseLabel = lv_label_create(phaseRow);
  lv_label_set_text(phaseLabel, phaseNames[phaseIndex]);
  lv_obj_set_style_text_color(phaseLabel, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(phaseLabel, &lv_font_montserrat_16, 0);
  lv_obj_align(phaseLabel, LV_ALIGN_LEFT_MID, 72, 0);

  lv_obj_t *btn_close = lv_btn_create(dialog);
  lv_obj_set_size(btn_close, 184, 40);
  lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, -14);
  lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x374151), 0);
  lv_obj_set_style_radius(btn_close, 10, 0);
  lv_obj_add_event_cb(btn_close, weather_close_popup, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label_close = lv_label_create(btn_close);
  lv_label_set_text(label_close, "Fermer");
  lv_obj_set_style_text_font(label_close, &lv_font_montserrat_16, 0);
  lv_obj_center(label_close);

  weather_autoclose_timer = lv_timer_create(weather_autoclose_cb, 10000, NULL);
  lv_timer_set_repeat_count(weather_autoclose_timer, 1);
}

void createMainScreen() {
  screenMain = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screenMain, lv_color_hex(COLOR_BG), 0);
  lv_obj_set_scrollbar_mode(screenMain, LV_SCROLLBAR_MODE_OFF);
  
  // ============================================
  // HEADER
  // ============================================
  lv_obj_t *header = lv_obj_create(screenMain);
  lv_obj_set_size(header, 480, 50);
  lv_obj_set_pos(header, 0, 0);
  lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_HEADER), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);
  lv_obj_set_style_pad_all(header, 10, 0);
  lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
  
  // Date (V15.0: orange si Enphase, blanc si MQTT)
  label_date = lv_label_create(header);
  lv_label_set_text(label_date, "Jeudi 30/10/25");
  lv_obj_set_style_text_font(label_date, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_date, lv_color_hex(activeScreenType == 1 ? 0xf59e0b : 0xffffff), 0);
  lv_obj_align(label_date, LV_ALIGN_LEFT_MID, 0, 0);
  
  // Heure (centrée)
  label_time = lv_label_create(header);
  lv_label_set_text(label_time, "23:03");
  lv_obj_set_style_text_font(label_time, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_time, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
  
  // Icônes header (gauche → droite : WiFi, MQTT, Shelly, Enphase, Réglages) — zoom 205, espacement 30 px
  led_wifi = lv_img_create(header);
  lv_img_set_src(led_wifi, &wifi_barre_oblique);
  lv_img_set_zoom(led_wifi, 205);
  lv_obj_align(led_wifi, LV_ALIGN_RIGHT_MID, -135, 0);
  
  led_mqtt = lv_img_create(header);
  lv_img_set_src(led_mqtt, &mqtt_png_gris);
  lv_img_set_zoom(led_mqtt, 205);
  lv_obj_align(led_mqtt, LV_ALIGN_RIGHT_MID, -105, 0);
  
  led_shelly = lv_img_create(header);
  lv_img_set_src(led_shelly, &Shelly32_gris);
  lv_img_set_zoom(led_shelly, 190);  // un peu plus petit que WiFi/MQTT (205) pour homogénéité visuelle
  lv_obj_align(led_shelly, LV_ALIGN_RIGHT_MID, -75, 0);
  
  led_enphase = lv_img_create(header);
  lv_img_set_src(led_enphase, &Enphase_logo_gris);
  lv_img_set_zoom(led_enphase, 190);  // idem
  lv_obj_align(led_enphase, LV_ALIGN_RIGHT_MID, -45, 0);
  
  led_settings = lv_img_create(header);
  lv_img_set_src(led_settings, &roue_dentee);
  lv_img_set_zoom(led_settings, 205);
  lv_obj_align(led_settings, LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_add_flag(led_settings, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(led_settings, settings_open_clicked, LV_EVENT_CLICKED, NULL);
  
  // ============================================
  // COLONNE GAUCHE (CENTRÉE)
  // ============================================
  int main_y = 60;
  int main_height = 320;
  
  lv_obj_t *card_left = lv_obj_create(screenMain);
  lv_obj_set_size(card_left, 225, main_height);
  lv_obj_set_pos(card_left, 10, main_y);
  lv_obj_set_style_bg_color(card_left, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_left, 1, 0);
  lv_obj_set_style_border_color(card_left, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_left, LV_OPA_40, 0);
  lv_obj_set_style_radius(card_left, 12, 0);
  lv_obj_set_style_pad_all(card_left, 15, 0);
  lv_obj_clear_flag(card_left, LV_OBJ_FLAG_SCROLLABLE);
  
  // Production (centrée) — remonté pour laisser place à la zone flux
  lv_obj_t *label_prod_title = lv_label_create(card_left);
  lv_label_set_text(label_prod_title, "PRODUCTION SOLAIRE");
  lv_obj_set_style_text_color(label_prod_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_prod_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_prod_title, 0, 8);
  
  lv_obj_t *img_prod_icon = lv_img_create(card_left);
  lv_img_set_src(img_prod_icon, &panneaux_solaires);
  lv_img_set_zoom(img_prod_icon, 300);  /* ~38 px pour aligner avec police 38 */
  lv_obj_set_pos(img_prod_icon, 0, 33);
  
  label_prod_value = lv_label_create(card_left);
  lv_label_set_text(label_prod_value, "-26");
  lv_obj_set_style_text_color(label_prod_value, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_prod_value, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_prod_value, 60, 33);
  
  label_prod_unit = lv_label_create(card_left);
  lv_label_set_text(label_prod_unit, "W");
  lv_obj_set_style_text_color(label_prod_unit, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_prod_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_prod_unit, 145, 46);
  
  // Consommation (centrée) — remonté 15 px
  lv_obj_t *label_conso_title = lv_label_create(card_left);
  lv_label_set_text(label_conso_title, "CONSO MAISON");
  lv_obj_set_style_text_color(label_conso_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_conso_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_conso_title, 0, 80);
  
  lv_obj_t *img_conso_icon = lv_img_create(card_left);
  lv_img_set_src(img_conso_icon, &reseau_electrique);
  lv_img_set_zoom(img_conso_icon, 300);  /* ~38 px pour aligner avec police 38 */
  lv_obj_set_pos(img_conso_icon, 0, 105);
  
  label_conso_value = lv_label_create(card_left);
  lv_label_set_text(label_conso_value, "660");
  lv_obj_set_style_text_color(label_conso_value, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_conso_value, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_conso_value, 60, 105);
  
  label_conso_unit = lv_label_create(card_left);
  lv_label_set_text(label_conso_unit, "W");
  lv_obj_set_style_text_color(label_conso_unit, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_conso_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_conso_unit, 145, 118);
  
  // CONSO JOUR (centrée) — remonté pour zone flux 80 px en bas
  lv_obj_t *label_conso_jour_title = lv_label_create(card_left);
  lv_label_set_text(label_conso_jour_title, "CONSO JOUR");
  lv_obj_set_style_text_color(label_conso_jour_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_conso_jour_title, &lv_font_montserrat_16, 0);
  lv_obj_align(label_conso_jour_title, LV_ALIGN_TOP_LEFT, 0, 173);
  
  label_conso_jour_value = lv_label_create(card_left);
  lv_label_set_text(label_conso_jour_value, "0.0 kWh");
  lv_obj_set_style_text_color(label_conso_jour_value, lv_color_hex(0xa78bfa), 0);  // Violet
  lv_obj_set_style_text_font(label_conso_jour_value, &lv_font_montserrat_16, 0);
  lv_obj_align(label_conso_jour_value, LV_ALIGN_TOP_RIGHT, 0, 173);
  
  // ============================================
  // ZONE FLUX PV → MAISON → RÉSEAU (en bas de la carte gauche)
  // ============================================
  // Y 210, hauteur 80 → remplit jusqu’au bas de la carte (210+80=290, pas d’espace)
  zone_flow_left = lv_obj_create(card_left);
  lv_obj_set_size(zone_flow_left, 195, 80);
  lv_obj_set_pos(zone_flow_left, 0, 210);
  lv_obj_set_style_bg_color(zone_flow_left, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(zone_flow_left, 0, 0);
  lv_obj_set_style_pad_all(zone_flow_left, 0, 0);
  lv_obj_clear_flag(zone_flow_left, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(zone_flow_left, LV_OBJ_FLAG_OVERFLOW_VISIBLE);  /* labels au-dessus des flèches pas coupés */
  
  // Positions X (adaptées largeur 195) : 5, 46, 88, 129, 171
  #define FLOW_ZOOM 256
  #define FLOW_Y -10
  
  img_flow_pv = lv_img_create(zone_flow_left);
  lv_img_set_src(img_flow_pv, &panneaux_solaires);
  lv_img_set_zoom(img_flow_pv, FLOW_ZOOM);
  lv_obj_align(img_flow_pv, LV_ALIGN_LEFT_MID, 5, FLOW_Y);
  
  img_arrow_pv_house = lv_img_create(zone_flow_left);
  lv_img_set_src(img_arrow_pv_house, &icoflechedroiteverte);
  lv_img_set_zoom(img_arrow_pv_house, FLOW_ZOOM);
  lv_obj_align(img_arrow_pv_house, LV_ALIGN_LEFT_MID, 46, FLOW_Y);
  lv_obj_add_flag(img_arrow_pv_house, LV_OBJ_FLAG_HIDDEN);
  
  img_flow_maison = lv_img_create(zone_flow_left);
  lv_img_set_src(img_flow_maison, &maison);
  lv_img_set_zoom(img_flow_maison, FLOW_ZOOM);
  lv_obj_align(img_flow_maison, LV_ALIGN_LEFT_MID, 88, FLOW_Y);
  
  img_arrow_house_grid = lv_img_create(zone_flow_left);
  lv_img_set_src(img_arrow_house_grid, &icoflechedroiteverte);
  lv_img_set_zoom(img_arrow_house_grid, FLOW_ZOOM);
  lv_obj_align(img_arrow_house_grid, LV_ALIGN_LEFT_MID, 129, FLOW_Y);
  lv_obj_add_flag(img_arrow_house_grid, LV_OBJ_FLAG_HIDDEN);
  
  img_flow_reseau = lv_img_create(zone_flow_left);
  lv_img_set_src(img_flow_reseau, &reseau_electrique);
  lv_img_set_zoom(img_flow_reseau, FLOW_ZOOM);
  lv_obj_align(img_flow_reseau, LV_ALIGN_LEFT_MID, 171, FLOW_Y);
  
  label_flow_pv_val = lv_label_create(zone_flow_left);
  lv_label_set_text(label_flow_pv_val, "0 W");
  lv_obj_set_style_text_color(label_flow_pv_val, lv_color_hex(COLOR_GRAY), 0);
  lv_obj_set_style_text_font(label_flow_pv_val, &lv_font_montserrat_16, 0);
  lv_obj_align_to(label_flow_pv_val, img_arrow_pv_house, LV_ALIGN_OUT_BOTTOM_MID, -10, 2);
  lv_obj_add_flag(label_flow_pv_val, LV_OBJ_FLAG_HIDDEN);
  
  label_flow_grid_val = lv_label_create(zone_flow_left);
  lv_label_set_text(label_flow_grid_val, "0 W");
  lv_obj_set_style_text_color(label_flow_grid_val, lv_color_hex(COLOR_GRAY), 0);
  lv_obj_set_style_text_font(label_flow_grid_val, &lv_font_montserrat_16, 0);
  lv_obj_align_to(label_flow_grid_val, img_arrow_house_grid, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
  lv_obj_add_flag(label_flow_grid_val, LV_OBJ_FLAG_HIDDEN);
  
  // ============================================
  // COLONNE DROITE
  // ============================================
  lv_obj_t *card_right = lv_obj_create(screenMain);
  lv_obj_set_size(card_right, 225, main_height);
  lv_obj_set_pos(card_right, 245, main_y);
  lv_obj_set_style_bg_color(card_right, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_right, 1, 0);
  lv_obj_set_style_border_color(card_right, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_right, LV_OPA_40, 0);
  lv_obj_set_style_radius(card_right, 12, 0);
  lv_obj_set_style_pad_all(card_right, 15, 0);
  lv_obj_clear_flag(card_right, LV_OBJ_FLAG_SCROLLABLE);
  
  // Routeur (ROUTAGE)
  lv_obj_t *label_router_title = lv_label_create(card_right);
  lv_label_set_text(label_router_title, "ROUTAGE");
  lv_obj_set_style_text_color(label_router_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_router_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_router_title, 60, 10);
  
  // Logo M'Sun PV
  lv_obj_t *label_logo = lv_label_create(card_right);
  lv_label_set_text(label_logo, "M'Sun\nPV");
  lv_obj_set_style_text_color(label_logo, lv_color_hex(0xa3e635), 0);
  lv_obj_set_style_text_font(label_logo, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_align(label_logo, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(label_logo, 10, 35);
  
  label_router_value = lv_label_create(card_right);
  lv_label_set_text(label_router_value, "0");
  lv_obj_set_style_text_color(label_router_value, lv_color_hex(COLOR_ROUTER), 0);
  lv_obj_set_style_text_font(label_router_value, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_router_value, 80, 50);
  
  label_router_unit = lv_label_create(card_right);
  lv_label_set_text(label_router_unit, "W");
  lv_obj_set_style_text_color(label_router_unit, lv_color_hex(COLOR_ROUTER), 0);
  lv_obj_set_style_text_font(label_router_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_router_unit, 165, 63);
  
  // ============================================
  // CUMULUS - AVEC BADGE STATUS (V3.0)
  // ============================================
  // Container cliquable pour ouvrir popup
  lv_obj_t *cumulus_header = lv_obj_create(card_right);
  lv_obj_set_size(cumulus_header, 190, 28);
  lv_obj_set_pos(cumulus_header, 5, 105);
  lv_obj_set_style_bg_opa(cumulus_header, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cumulus_header, 0, 0);
  lv_obj_set_style_pad_all(cumulus_header, 0, 0);
  lv_obj_add_flag(cumulus_header, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(cumulus_header, msunpv_open_popup, LV_EVENT_CLICKED, NULL);
  
  // Titre CUMULUS (gauche)
  lv_obj_t *label_thermo_title = lv_label_create(cumulus_header);
  lv_label_set_text(label_thermo_title, "CUMULUS");
  lv_obj_set_style_text_color(label_thermo_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_thermo_title, &lv_font_montserrat_16, 0);
  lv_obj_align(label_thermo_title, LV_ALIGN_LEFT_MID, 0, 0);
  
  // Badge statut (droite) - V3.1 - Décalé de 28px à gauche
  label_msunpv_status = lv_label_create(cumulus_header);
  lv_label_set_text(label_msunpv_status, "AUTO");
  lv_obj_set_style_text_color(label_msunpv_status, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_msunpv_status, &lv_font_montserrat_16, 0);
  lv_obj_set_style_bg_color(label_msunpv_status, lv_color_hex(COLOR_MSUNPV_AUTO), 0);
  lv_obj_set_style_bg_opa(label_msunpv_status, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(label_msunpv_status, 6, 0);
  lv_obj_set_style_pad_hor(label_msunpv_status, 8, 0);
  lv_obj_set_style_pad_ver(label_msunpv_status, 2, 0);
  lv_obj_align(label_msunpv_status, LV_ALIGN_RIGHT_MID, -28, 0);
  
  // Container horizontal : Barre + Infos
  lv_obj_t *cumulus_container = lv_obj_create(card_right);
  lv_obj_set_size(cumulus_container, 190, 100);
  lv_obj_set_pos(cumulus_container, 5, 135);
  lv_obj_set_style_bg_opa(cumulus_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(cumulus_container, 0, 0);
  lv_obj_set_style_pad_all(cumulus_container, 0, 0);
  lv_obj_clear_flag(cumulus_container, LV_OBJ_FLAG_SCROLLABLE);
  
  // Zone gauche cumulus : icône 60×83, recolor selon température (gris ou phase)
  lv_obj_t *thermo_bar = lv_obj_create(cumulus_container);
  lv_obj_set_size(thermo_bar, 60, 100);
  lv_obj_set_pos(thermo_bar, 0, 0);
  lv_obj_set_style_bg_opa(thermo_bar, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(thermo_bar, 0, 0);
  lv_obj_clear_flag(thermo_bar, LV_OBJ_FLAG_SCROLLABLE);
  
  img_cumulus_right = lv_img_create(thermo_bar);
  lv_img_set_src(img_cumulus_right, &Chauffeeaucartedroite);
  lv_img_set_zoom(img_cumulus_right, 256);
  lv_obj_align(img_cumulus_right, LV_ALIGN_BOTTOM_MID, 0, 0);
  
  // COLONNE DROITE : Température + LED
  lv_obj_t *info_column = lv_obj_create(cumulus_container);
  lv_obj_set_size(info_column, 120, 100);
  lv_obj_set_pos(info_column, 75, 0);
  lv_obj_set_style_bg_opa(info_column, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(info_column, 0, 0);
  lv_obj_set_style_pad_all(info_column, 0, 0);
  lv_obj_clear_flag(info_column, LV_OBJ_FLAG_SCROLLABLE);
  
  label_water_temp_value = lv_label_create(info_column);
  lv_label_set_text(label_water_temp_value, "48°C");
  lv_obj_set_style_text_color(label_water_temp_value, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_text_font(label_water_temp_value, &lv_font_montserrat_26, 0);
  lv_obj_align(label_water_temp_value, LV_ALIGN_TOP_MID, 0, 15);
  
  /* LED cumulus = icône cartes_et_drapeaux (40×40 → 35×35), recolor 0x2c2928 si pas OK */
  obj_led_indicator = lv_img_create(info_column);
  lv_img_set_src(obj_led_indicator, &cartes_et_drapeaux);
  lv_img_set_zoom(obj_led_indicator, 224);  /* 256 * 35/40 */
  lv_obj_align(obj_led_indicator, LV_ALIGN_TOP_MID, 0, 55);
  lv_obj_set_style_image_recolor(obj_led_indicator, lv_color_hex(0x2a2625), 0);
  lv_obj_set_style_image_recolor_opa(obj_led_indicator, LV_OPA_70, 0);
  
  // ============================================
  // MÉTÉO 4 JOURS (V12.4)
  // ============================================
  weather_forecast_container = lv_obj_create(card_right);
  lv_obj_set_size(weather_forecast_container, 190, 60);
  lv_obj_set_pos(weather_forecast_container, 5, 240);
  lv_obj_set_style_bg_opa(weather_forecast_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(weather_forecast_container, 0, 0);
  lv_obj_set_style_pad_all(weather_forecast_container, 0, 0);
  lv_obj_clear_flag(weather_forecast_container, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(weather_forecast_container, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(weather_forecast_container, weather_open_popup, LV_EVENT_CLICKED, NULL);
  
  // Créer 4 colonnes (47px chacune)
  for(int i = 0; i < 4; i++) {
    // Container colonne
    lv_obj_t *col = lv_obj_create(weather_forecast_container);
    lv_obj_set_size(col, 47, 60);
    lv_obj_set_pos(col, i * 47, 0);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(col, weather_open_popup, LV_EVENT_CLICKED, NULL);
    
    // Label jour (TOP_MID, offset Y=2)
    weather_day_label[i] = lv_label_create(col);
    lv_label_set_text(weather_day_label[i], "-");
    lv_obj_set_style_text_color(weather_day_label[i], lv_color_hex(0xd1d5db), 0);
    lv_obj_set_style_text_font(weather_day_label[i], &lv_font_montserrat_16, 0);
    lv_obj_align(weather_day_label[i], LV_ALIGN_TOP_MID, 0, 2);
    
    // Image icône (CENTER, zoom 128 pour 32×32)
    weather_icon_img[i] = lv_img_create(col);
    lv_img_set_src(weather_icon_img[i], &icon_na);
    lv_img_set_zoom(weather_icon_img[i], 128);
    lv_obj_align(weather_icon_img[i], LV_ALIGN_CENTER, 0, 0);
    
    // Label température (BOTTOM_MID, offset Y=-2)
    weather_temp_label[i] = lv_label_create(col);
    lv_label_set_text(weather_temp_label[i], "--°");
    lv_obj_set_style_text_color(weather_temp_label[i], lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(weather_temp_label[i], &lv_font_montserrat_16, 0);
    lv_obj_align(weather_temp_label[i], LV_ALIGN_BOTTOM_MID, 0, -2);
  }
  
  // ============================================
  // BARRE TEMPÉRATURES (BAS)
  // ============================================
  lv_obj_t *weather_container = lv_obj_create(screenMain);
  lv_obj_set_size(weather_container, 460, 50);
  lv_obj_set_pos(weather_container, 10, 400);
  lv_obj_set_style_bg_color(weather_container, lv_color_hex(COLOR_WEATHER), 0);
  lv_obj_set_style_border_width(weather_container, 0, 0);
  lv_obj_set_style_radius(weather_container, 12, 0);
  lv_obj_set_style_pad_all(weather_container, 10, 0);
  
  // Température extérieure (gauche)
  label_temp_ext = lv_label_create(weather_container);
  lv_label_set_text(label_temp_ext, "--  --°C");
  lv_obj_set_style_text_font(label_temp_ext, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_temp_ext, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_temp_ext, LV_ALIGN_LEFT_MID, 0, 0);
  
  // Icône météo (centre)
  weather_icon = lv_img_create(weather_container);
  lv_img_set_zoom(weather_icon, 256);
  lv_obj_align(weather_icon, LV_ALIGN_CENTER, 0, 0);
  
  // Température salon (droite)
  label_temp_salon = lv_label_create(weather_container);
  lv_label_set_text(label_temp_salon, "SALON  --°C");
  lv_obj_set_style_text_font(label_temp_salon, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_temp_salon, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_temp_salon, LV_ALIGN_RIGHT_MID, 0, 0);
}

// V15.0 - Écran Enphase (cartes réduites + footer météo 6 jours)
void createEnphaseScreen() {
  screenEnphase = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screenEnphase, lv_color_hex(COLOR_BG), 0);
  lv_obj_set_scrollbar_mode(screenEnphase, LV_SCROLLBAR_MODE_OFF);
  
  // ============================================
  // HEADER (identique MQTT - tous les logos)
  // ============================================
  lv_obj_t *header = lv_obj_create(screenEnphase);
  lv_obj_set_size(header, 480, 50);
  lv_obj_set_pos(header, 0, 0);
  lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_HEADER), 0);
  lv_obj_set_style_border_width(header, 0, 0);
  lv_obj_set_style_radius(header, 0, 0);
  lv_obj_set_style_pad_all(header, 10, 0);
  lv_obj_set_scrollbar_mode(header, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
  
  // Date (orange pour Enphase)
  label_ep_date = lv_label_create(header);
  lv_label_set_text(label_ep_date, "Jeu. 30/10/25");
  lv_obj_set_style_text_font(label_ep_date, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_ep_date, lv_color_hex(0xf59e0b), 0);
  lv_obj_align(label_ep_date, LV_ALIGN_LEFT_MID, 0, 0);
  
  // Heure (centrée)
  label_ep_time = lv_label_create(header);
  lv_label_set_text(label_ep_time, "23:03");
  lv_obj_set_style_text_font(label_ep_time, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(label_ep_time, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_ep_time, LV_ALIGN_CENTER, 0, 0);
  
  // Icônes header (WiFi, MQTT, Shelly, Enphase, Réglages)
  led_ep_wifi = lv_img_create(header);
  lv_img_set_src(led_ep_wifi, &wifi_barre_oblique);
  lv_img_set_zoom(led_ep_wifi, 205);
  lv_obj_align(led_ep_wifi, LV_ALIGN_RIGHT_MID, -135, 0);
  
  led_ep_mqtt = lv_img_create(header);
  lv_img_set_src(led_ep_mqtt, &mqtt_png_gris);
  lv_img_set_zoom(led_ep_mqtt, 205);
  lv_obj_align(led_ep_mqtt, LV_ALIGN_RIGHT_MID, -105, 0);
  
  led_ep_shelly = lv_img_create(header);
  lv_img_set_src(led_ep_shelly, &Shelly32_gris);
  lv_img_set_zoom(led_ep_shelly, 190);
  lv_obj_align(led_ep_shelly, LV_ALIGN_RIGHT_MID, -75, 0);
  
  led_ep_enphase = lv_img_create(header);
  lv_img_set_src(led_ep_enphase, &Enphase_logo_gris);
  lv_img_set_zoom(led_ep_enphase, 190);
  lv_obj_align(led_ep_enphase, LV_ALIGN_RIGHT_MID, -45, 0);
  
  led_ep_settings = lv_img_create(header);
  lv_img_set_src(led_ep_settings, &roue_dentee);
  lv_img_set_zoom(led_ep_settings, 205);
  lv_obj_align(led_ep_settings, LV_ALIGN_RIGHT_MID, -15, 0);
  lv_obj_add_flag(led_ep_settings, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(led_ep_settings, settings_open_clicked, LV_EVENT_CLICKED, NULL);
  
  // ============================================
  // CARTE GAUCHE (Prod, Conso — base alignée sur CONSO JOUR de la carte droite)
  // ============================================
  int main_y = 60;
  int main_height = 210;  // Limite = base des kWh CONSO JOUR (142 + 38 + 2*15 pad)
  
  lv_obj_t *card_left = lv_obj_create(screenEnphase);
  lv_obj_set_size(card_left, 225, main_height);
  lv_obj_set_pos(card_left, 10, main_y);
  lv_obj_set_style_bg_color(card_left, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_left, 1, 0);
  lv_obj_set_style_border_color(card_left, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_left, LV_OPA_40, 0);
  lv_obj_set_style_radius(card_left, 12, 0);
  lv_obj_set_style_pad_all(card_left, 15, 0);
  lv_obj_clear_flag(card_left, LV_OBJ_FLAG_SCROLLABLE);
  
  // PRODUCTION SOLAIRE — même grille verticale que carte droite (titre 8, ligne 40, unité 52, titre2 110, ligne2 142, unité 154)
  lv_obj_t *label_prod_title = lv_label_create(card_left);
  lv_label_set_text(label_prod_title, "PRODUCTION SOLAIRE");
  lv_obj_set_style_text_color(label_prod_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_prod_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_prod_title, 0, 8);
  
  lv_obj_t *img_prod_icon = lv_img_create(card_left);
  lv_img_set_src(img_prod_icon, &panneaux_solaires);
  lv_img_set_zoom(img_prod_icon, 300);
  lv_obj_set_pos(img_prod_icon, 0, 40);
  
  label_ep_prod = lv_label_create(card_left);
  lv_label_set_text(label_ep_prod, "0");
  lv_obj_set_style_text_color(label_ep_prod, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_ep_prod, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_ep_prod, 60, 40);
  
  lv_obj_t *label_prod_unit = lv_label_create(card_left);
  lv_label_set_text(label_prod_unit, "W");
  lv_obj_set_style_text_color(label_prod_unit, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_prod_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_prod_unit, 145, 52);
  
  // CONSO MAISON — aligné sur carte droite (titre 110, ligne 142, unité 154)
  lv_obj_t *label_conso_title = lv_label_create(card_left);
  lv_label_set_text(label_conso_title, "CONSO MAISON");
  lv_obj_set_style_text_color(label_conso_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_conso_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_conso_title, 0, 110);
  
  lv_obj_t *img_conso_icon = lv_img_create(card_left);
  lv_img_set_src(img_conso_icon, &reseau_electrique);
  lv_img_set_zoom(img_conso_icon, 300);
  lv_obj_set_pos(img_conso_icon, 0, 142);
  
  label_ep_conso = lv_label_create(card_left);
  lv_label_set_text(label_ep_conso, "0");
  lv_obj_set_style_text_color(label_ep_conso, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_ep_conso, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_ep_conso, 60, 142);
  
  lv_obj_t *label_conso_unit = lv_label_create(card_left);
  lv_label_set_text(label_conso_unit, "W");
  lv_obj_set_style_text_color(label_conso_unit, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_conso_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_conso_unit, 145, 154);
  
  // ============================================
  // CARTE DROITE (Prod jour, Conso jour)
  // ============================================
  lv_obj_t *card_right = lv_obj_create(screenEnphase);
  lv_obj_set_size(card_right, 225, main_height);
  lv_obj_set_pos(card_right, 245, main_y);
  lv_obj_set_style_bg_color(card_right, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_right, 1, 0);
  lv_obj_set_style_border_color(card_right, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_right, LV_OPA_40, 0);
  lv_obj_set_style_radius(card_right, 12, 0);
  lv_obj_set_style_pad_all(card_right, 15, 0);
  lv_obj_clear_flag(card_right, LV_OBJ_FLAG_SCROLLABLE);
  
  // PRODUCTION JOUR (kWh) — valeur en 38, unité "kWh" en 20 (titre/unité modifiables pour M'SunPV → ROUTAGE / %)
  label_ep_prod_jour_title = lv_label_create(card_right);
  lv_label_set_text(label_ep_prod_jour_title, "PRODUCTION JOUR");
  lv_obj_set_style_text_color(label_ep_prod_jour_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_ep_prod_jour_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_ep_prod_jour_title, 0, 8);
  
  label_ep_prod_jour = lv_label_create(card_right);
  lv_label_set_text(label_ep_prod_jour, "0.0");
  lv_obj_set_style_text_color(label_ep_prod_jour, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_ep_prod_jour, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_ep_prod_jour, 0, 40);
  
  label_ep_prod_jour_unit = lv_label_create(card_right);
  lv_label_set_text(label_ep_prod_jour_unit, "kWh");
  lv_obj_set_style_text_color(label_ep_prod_jour_unit, lv_color_hex(COLOR_PROD), 0);
  lv_obj_set_style_text_font(label_ep_prod_jour_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_ep_prod_jour_unit, 145, 52);
  
  // CONSO JOUR (kWh) — idem
  lv_obj_t *label_conso_jour_title = lv_label_create(card_right);
  lv_label_set_text(label_conso_jour_title, "CONSO JOUR");
  lv_obj_set_style_text_color(label_conso_jour_title, lv_color_hex(0xd1d5db), 0);
  lv_obj_set_style_text_font(label_conso_jour_title, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(label_conso_jour_title, 0, 110);
  
  label_ep_conso_jour = lv_label_create(card_right);
  lv_label_set_text(label_ep_conso_jour, "0.0");
  lv_obj_set_style_text_color(label_ep_conso_jour, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_ep_conso_jour, &lv_font_montserrat_38, 0);
  lv_obj_set_pos(label_ep_conso_jour, 0, 142);
  
  lv_obj_t *label_conso_jour_unit = lv_label_create(card_right);
  lv_label_set_text(label_conso_jour_unit, "kWh");
  lv_obj_set_style_text_color(label_conso_jour_unit, lv_color_hex(COLOR_CONSO), 0);
  lv_obj_set_style_text_font(label_conso_jour_unit, &lv_font_montserrat_20, 0);
  lv_obj_set_pos(label_conso_jour_unit, 145, 154);
  
  // ============================================
  // CARTE HORIZONTALE (Flux réseau 2/3 + réserve 1/3 droite) — 10 px au-dessus et en dessous
  // ============================================
  int gap_flux = 10;
  int card_horiz_y = main_y + main_height + gap_flux;
  int card_horiz_h = 390 - card_horiz_y - gap_flux;  // footer_y=390, 10 px sous la carte
  int card_horiz_w = 460;
  
  lv_obj_t *card_horiz = lv_obj_create(screenEnphase);
  lv_obj_set_size(card_horiz, card_horiz_w, card_horiz_h);
  lv_obj_set_pos(card_horiz, 10, card_horiz_y);
  lv_obj_set_style_bg_color(card_horiz, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_horiz, 1, 0);
  lv_obj_set_style_border_color(card_horiz, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_horiz, LV_OPA_40, 0);
  lv_obj_set_style_radius(card_horiz, 12, 0);
  lv_obj_set_style_pad_all(card_horiz, 15, 0);
  lv_obj_clear_flag(card_horiz, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(card_horiz, LV_OBJ_FLAG_OVERFLOW_VISIBLE);  // ombre du badge visible
  
  // Zone flux PV → Maison → Réseau (2/3 largeur à gauche)
  int inner_w = card_horiz_w - 30;  // - 2*15 pad
  int zone_flow_w = (inner_w * 2) / 3;
  int zone_flow_h = card_horiz_h - 30;
  
  zone_flow_left_ep = lv_obj_create(card_horiz);
  lv_obj_set_size(zone_flow_left_ep, zone_flow_w, zone_flow_h);
  lv_obj_set_pos(zone_flow_left_ep, 0, 0);
  lv_obj_set_style_bg_color(zone_flow_left_ep, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(zone_flow_left_ep, 0, 0);
  lv_obj_set_style_pad_all(zone_flow_left_ep, 0, 0);
  lv_obj_set_scrollbar_mode(zone_flow_left_ep, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(zone_flow_left_ep, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(zone_flow_left_ep, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
  
  #define FLOW_ZOOM_EP 333   // 130% (256 = 100%), valeurs en Montserrat 20 (pas 12)
  #define FLOW_Y_EP -10
  // Répartition 5 éléments sur zone_flow_w : espacement proportionnel
  int flow_step = (zone_flow_w - 20) / 5;
  int flow_x0 = 10;
  
  img_flow_pv_ep = lv_img_create(zone_flow_left_ep);
  lv_img_set_src(img_flow_pv_ep, &panneaux_solaires);
  lv_img_set_zoom(img_flow_pv_ep, FLOW_ZOOM_EP);
  lv_obj_align(img_flow_pv_ep, LV_ALIGN_LEFT_MID, flow_x0, FLOW_Y_EP);
  
  img_arrow_pv_house_ep = lv_img_create(zone_flow_left_ep);
  lv_img_set_src(img_arrow_pv_house_ep, &icoflechedroiteverte);
  lv_img_set_zoom(img_arrow_pv_house_ep, FLOW_ZOOM_EP);
  lv_obj_align(img_arrow_pv_house_ep, LV_ALIGN_LEFT_MID, flow_x0 + flow_step, FLOW_Y_EP);
  lv_obj_add_flag(img_arrow_pv_house_ep, LV_OBJ_FLAG_HIDDEN);
  
  img_flow_maison_ep = lv_img_create(zone_flow_left_ep);
  lv_img_set_src(img_flow_maison_ep, &maison);
  lv_img_set_zoom(img_flow_maison_ep, FLOW_ZOOM_EP);
  lv_obj_align(img_flow_maison_ep, LV_ALIGN_LEFT_MID, flow_x0 + 2 * flow_step, FLOW_Y_EP);
  
  img_arrow_house_grid_ep = lv_img_create(zone_flow_left_ep);
  lv_img_set_src(img_arrow_house_grid_ep, &icoflechedroiteverte);
  lv_img_set_zoom(img_arrow_house_grid_ep, FLOW_ZOOM_EP);
  lv_obj_align(img_arrow_house_grid_ep, LV_ALIGN_LEFT_MID, flow_x0 + 3 * flow_step, FLOW_Y_EP);
  lv_obj_add_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
  
  img_flow_reseau_ep = lv_img_create(zone_flow_left_ep);
  lv_img_set_src(img_flow_reseau_ep, &reseau_electrique);
  lv_img_set_zoom(img_flow_reseau_ep, FLOW_ZOOM_EP);
  lv_obj_align(img_flow_reseau_ep, LV_ALIGN_LEFT_MID, flow_x0 + 4 * flow_step, FLOW_Y_EP);
  
  label_flow_pv_val_ep = lv_label_create(zone_flow_left_ep);
  lv_label_set_text(label_flow_pv_val_ep, "0 W");
  lv_obj_set_style_text_color(label_flow_pv_val_ep, lv_color_hex(COLOR_GRAY), 0);
  lv_obj_set_style_text_font(label_flow_pv_val_ep, &lv_font_montserrat_14, 0);  // était 16 → 14
  lv_obj_align_to(label_flow_pv_val_ep, img_arrow_pv_house_ep, LV_ALIGN_OUT_BOTTOM_MID, -10, 2);
  lv_obj_add_flag(label_flow_pv_val_ep, LV_OBJ_FLAG_HIDDEN);
  
  label_flow_grid_val_ep = lv_label_create(zone_flow_left_ep);
  lv_label_set_text(label_flow_grid_val_ep, "0 W");
  lv_obj_set_style_text_color(label_flow_grid_val_ep, lv_color_hex(COLOR_GRAY), 0);
  lv_obj_set_style_text_font(label_flow_grid_val_ep, &lv_font_montserrat_14, 0);  // était 16 → 14
  lv_obj_align_to(label_flow_grid_val_ep, img_arrow_house_grid_ep, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
  lv_obj_add_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
  
  // 1/3 droite : badge état réseau (Import / Auto / Export)
  int right_zone_x = zone_flow_w + 10;
  int right_zone_w = inner_w - zone_flow_w - 20;
  obj_flow_state_ep = lv_obj_create(card_horiz);
  lv_obj_set_size(obj_flow_state_ep, 120, 44);
  lv_obj_set_pos(obj_flow_state_ep, right_zone_x + (right_zone_w - 120) / 2, (zone_flow_h - 44) / 2);
  lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0x16a34a), 0);  // Auto par défaut
  lv_obj_set_style_radius(obj_flow_state_ep, 22, 0);  // pilule
  lv_obj_set_style_border_width(obj_flow_state_ep, 0, 0);
  lv_obj_set_style_pad_all(obj_flow_state_ep, 8, 0);
  lv_obj_set_style_shadow_width(obj_flow_state_ep, 18, 0);
  lv_obj_set_style_shadow_ofs_x(obj_flow_state_ep, 0, 0);
  lv_obj_set_style_shadow_ofs_y(obj_flow_state_ep, 6, 0);
  lv_obj_set_style_shadow_color(obj_flow_state_ep, lv_color_hex(0x1f2937), 0);
  lv_obj_set_style_shadow_opa(obj_flow_state_ep, LV_OPA_60, 0);
  lv_obj_clear_flag(obj_flow_state_ep, LV_OBJ_FLAG_SCROLLABLE);
  label_flow_state_ep = lv_label_create(obj_flow_state_ep);
  lv_label_set_text(label_flow_state_ep, "Auto");
  lv_obj_set_style_text_color(label_flow_state_ep, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_flow_state_ep, &lv_font_montserrat_16, 0);
  lv_obj_align(label_flow_state_ep, LV_ALIGN_CENTER, 0, 0);
  
  // ============================================
  // FOOTER MÉTÉO (hauteur réduite de moitié, pas de scroll)
  // ============================================
  int footer_y = 390;
  int footer_h = 90;  // Réduit de moitié (était 190)
  
  lv_obj_t *footer = lv_obj_create(screenEnphase);
  lv_obj_set_size(footer, 460, footer_h);
  lv_obj_set_pos(footer, 10, footer_y);
  lv_obj_set_style_bg_color(footer, lv_color_hex(COLOR_WEATHER), 0);
  lv_obj_set_style_border_width(footer, 1, 0);  // comme les autres cartes (1 px)
  lv_obj_set_style_border_color(footer, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(footer, LV_OPA_40, 0);
  lv_obj_set_style_radius(footer, 12, 0);
  lv_obj_set_style_pad_all(footer, 10, 0);
  lv_obj_set_scrollbar_mode(footer, LV_SCROLLBAR_MODE_OFF);
  lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);
  
  // Gauche : Ville, icône, température
  label_ep_weather_city = lv_label_create(footer);
  lv_label_set_text(label_ep_weather_city, "--");
  lv_obj_set_style_text_color(label_ep_weather_city, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_ep_weather_city, &lv_font_montserrat_16, 0);
  lv_obj_align(label_ep_weather_city, LV_ALIGN_LEFT_MID, 0, -25);
  
  img_ep_weather_icon = lv_img_create(footer);
  lv_img_set_src(img_ep_weather_icon, &icon_sun);
  lv_img_set_zoom(img_ep_weather_icon, 358);  // 140% (256 = 100%)
  lv_obj_align(img_ep_weather_icon, LV_ALIGN_LEFT_MID, 48, 15);  // 15 px sous le centre
  
  label_ep_weather_temp = lv_label_create(footer);
  lv_label_set_text(label_ep_weather_temp, "--°C");
  lv_obj_set_style_text_color(label_ep_weather_temp, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_ep_weather_temp, &lv_font_montserrat_20, 0);
  lv_obj_align(label_ep_weather_temp, LV_ALIGN_LEFT_MID, 0, 25);
  
  // Droite : 5 jours J+1 à J+5 (même largeur totale qu'avant pour 6 : 330 px)
  int total_forecast_w = 330;  // 6*55 avant
  int col_w = total_forecast_w / 5;  // 66 px par colonne
  for (int i = 0; i < 5; i++) {
    lv_obj_t *col = lv_obj_create(footer);
    lv_obj_set_size(col, col_w, footer_h - 20);
    lv_obj_set_pos(col, 115 + i * col_w, 0);
    lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(col, 0, 0);
    lv_obj_set_style_pad_all(col, 0, 0);
    lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(col, LV_OBJ_FLAG_SCROLLABLE);
    
    label_ep_weather_day[i] = lv_label_create(col);
    lv_label_set_text(label_ep_weather_day[i], "-");
    lv_obj_set_style_text_color(label_ep_weather_day[i], lv_color_hex(0xd1d5db), 0);
    lv_obj_set_style_text_font(label_ep_weather_day[i], &lv_font_montserrat_16, 0);
    lv_obj_align(label_ep_weather_day[i], LV_ALIGN_TOP_MID, 0, 0);
    
    img_ep_weather_icon_day[i] = lv_img_create(col);
    lv_img_set_src(img_ep_weather_icon_day[i], &icon_na);
    lv_img_set_zoom(img_ep_weather_icon_day[i], 200);
    lv_obj_align(img_ep_weather_icon_day[i], LV_ALIGN_CENTER, 0, -5);  // descendu 3 px (était -8)
    
    label_ep_weather_temp_day[i] = lv_label_create(col);
    lv_label_set_text(label_ep_weather_temp_day[i], "--°");
    lv_obj_set_style_text_color(label_ep_weather_temp_day[i], lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label_ep_weather_temp_day[i], &lv_font_montserrat_16, 0);
    lv_obj_align(label_ep_weather_temp_day[i], LV_ALIGN_BOTTOM_MID, 0, -2);
  }
}

void createSettingsScreen() {
  screenSettings = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screenSettings, lv_color_hex(COLOR_BG), 0);
  lv_obj_set_size(screenSettings, 480, 480);
  lv_obj_set_scrollbar_mode(screenSettings, LV_SCROLLBAR_MODE_OFF);
  lv_obj_t *tv = lv_tabview_create(screenSettings);
  lv_obj_set_size(tv, 480, 430);
  lv_obj_align(tv, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(tv, lv_color_hex(COLOR_BG), 0);
  lv_obj_set_style_bg_color(lv_tabview_get_content(tv), lv_color_hex(COLOR_BG), 0);
  lv_tabview_set_tab_bar_size(tv, 44);
  lv_obj_t *tab_bar = lv_tabview_get_tab_bar(tv);
  if (tab_bar) {
    lv_obj_set_style_bg_color(tab_bar, lv_color_hex(COLOR_HEADER), 0);
    lv_obj_set_style_text_color(tab_bar, lv_color_hex(0xd1d5db), 0);
    lv_obj_set_style_text_color(tab_bar, lv_color_hex(0xfbbf24), LV_PART_ITEMS | LV_STATE_CHECKED);
  }
  lv_obj_t *tab_wifi = lv_tabview_add_tab(tv, "WiFi");
  lv_obj_t *tab_infos = lv_tabview_add_tab(tv, "Infos");
  lv_obj_t *tab_meteo = lv_tabview_add_tab(tv, "Meteo");
  lv_obj_t *tab_maint = lv_tabview_add_tab(tv, "Maint.");
  lv_obj_t *tab_logs = lv_tabview_add_tab(tv, "Logs");
  
  // ============================================
  // ONGLET WIFI - CONFIGURATION COMPLÈTE
  // ============================================
  lv_obj_set_style_pad_all(tab_wifi, 10, 0);
  lv_obj_set_scrollbar_mode(tab_wifi, LV_SCROLLBAR_MODE_ON);
  
  // Carte statut WiFi (Etat, Signal, IP) — design amélioré
  lv_obj_t *card_wifi_status = lv_obj_create(tab_wifi);
  lv_obj_set_size(card_wifi_status, 440, 58);
  lv_obj_set_pos(card_wifi_status, 0, 0);
  lv_obj_set_style_bg_color(card_wifi_status, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_wifi_status, 1, 0);
  lv_obj_set_style_border_color(card_wifi_status, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_wifi_status, LV_OPA_30, 0);
  lv_obj_set_style_radius(card_wifi_status, 10, 0);
  lv_obj_set_style_pad_all(card_wifi_status, 12, 0);
  lv_obj_clear_flag(card_wifi_status, LV_OBJ_FLAG_SCROLLABLE);
  
  label_settings_wifi = lv_label_create(card_wifi_status);
  lv_label_set_text(label_settings_wifi, "Etat: --\nSignal: --\nIP: --");
  lv_obj_set_style_text_font(label_settings_wifi, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label_settings_wifi, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_settings_wifi, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_width(label_settings_wifi, 400);
  
  // Bouton Scanner
  lv_obj_t *btn_scan = lv_btn_create(tab_wifi);
  lv_obj_set_size(btn_scan, 200, 40);
  lv_obj_align(btn_scan, LV_ALIGN_TOP_LEFT, 0, 70);
  lv_obj_set_style_bg_color(btn_scan, lv_color_hex(0x3b82f6), 0);
  lv_obj_add_event_cb(btn_scan, (lv_event_cb_t)wifi_scan_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_scan = lv_label_create(btn_scan);
  lv_label_set_text(lbl_scan, "Scan reseaux");
  lv_obj_set_style_text_font(lbl_scan, &lv_font_montserrat_16, 0);
  lv_obj_center(lbl_scan);
  
  // Status scan
  lv_obj_t *lbl_scan_status = lv_label_create(tab_wifi);
  lv_label_set_text(lbl_scan_status, "Pret");
  lv_obj_set_style_text_font(lbl_scan_status, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_scan_status, lv_color_hex(0x9ca3af), 0);
  lv_obj_align(lbl_scan_status, LV_ALIGN_TOP_RIGHT, -20, 72);
  
  // Dropdown réseaux
  lv_obj_t *dd_wifi = lv_dropdown_create(tab_wifi);
  lv_obj_set_size(dd_wifi, 400, 40);
  lv_obj_align(dd_wifi, LV_ALIGN_TOP_LEFT, 0, 122);
  lv_obj_set_style_text_font(lv_dropdown_get_list(dd_wifi), &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_font(dd_wifi, &lv_font_montserrat_16, 0);
  lv_dropdown_set_text(dd_wifi, "Selectionnez un reseau");
  lv_obj_add_event_cb(dd_wifi, (lv_event_cb_t)wifi_network_selected, LV_EVENT_VALUE_CHANGED, NULL);
  
  // Champ SSID
  lv_obj_t *lbl_ssid = lv_label_create(tab_wifi);
  lv_label_set_text(lbl_ssid, "SSID:");
  lv_obj_set_style_text_font(lbl_ssid, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_ssid, lv_color_hex(0xd1d5db), 0);
  lv_obj_align(lbl_ssid, LV_ALIGN_TOP_LEFT, 0, 172);
  
  lv_obj_t *ta_ssid = lv_textarea_create(tab_wifi);
  lv_obj_set_size(ta_ssid, 400, 40);
  lv_obj_align(ta_ssid, LV_ALIGN_TOP_LEFT, 0, 192);
  lv_textarea_set_placeholder_text(ta_ssid, "Saisir ou selectionner");
  lv_obj_set_style_text_font(ta_ssid, &lv_font_montserrat_16, 0);
  lv_textarea_set_max_length(ta_ssid, 32);
  
  // Champ Mot de passe
  lv_obj_t *lbl_pwd = lv_label_create(tab_wifi);
  lv_label_set_text(lbl_pwd, "Mot de passe:");
  lv_obj_set_style_text_font(lbl_pwd, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_pwd, lv_color_hex(0xd1d5db), 0);
  lv_obj_align(lbl_pwd, LV_ALIGN_TOP_LEFT, 0, 242);
  
  lv_obj_t *ta_pwd = lv_textarea_create(tab_wifi);
  lv_obj_set_size(ta_pwd, 400, 40);
  lv_obj_align(ta_pwd, LV_ALIGN_TOP_LEFT, 0, 262);
  lv_textarea_set_placeholder_text(ta_pwd, "(optionnel)");
  lv_obj_set_style_text_font(ta_pwd, &lv_font_montserrat_16, 0);
  lv_textarea_set_max_length(ta_pwd, 64);
  
  // Bouton Connexion
  lv_obj_t *btn_connect = lv_btn_create(tab_wifi);
  lv_obj_set_size(btn_connect, 190, 44);
  lv_obj_align(btn_connect, LV_ALIGN_TOP_LEFT, 0, 317);
  lv_obj_set_style_bg_color(btn_connect, lv_color_hex(0x22c55e), 0);
  lv_obj_add_event_cb(btn_connect, (lv_event_cb_t)wifi_connect_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_connect = lv_label_create(btn_connect);
  lv_label_set_text(lbl_connect, "Connexion");
  lv_obj_set_style_text_font(lbl_connect, &lv_font_montserrat_16, 0);
  lv_obj_center(lbl_connect);
  
  // Bouton Effacer
  lv_obj_t *btn_clear = lv_btn_create(tab_wifi);
  lv_obj_set_size(btn_clear, 190, 44);
  lv_obj_align(btn_clear, LV_ALIGN_TOP_RIGHT, -10, 317);
  lv_obj_set_style_bg_color(btn_clear, lv_color_hex(0x374151), 0);
  lv_obj_add_event_cb(btn_clear, (lv_event_cb_t)wifi_clear_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lbl_clear = lv_label_create(btn_clear);
  lv_label_set_text(lbl_clear, "Effacer");
  lv_obj_set_style_text_font(lbl_clear, &lv_font_montserrat_16, 0);
  lv_obj_center(lbl_clear);
  
  // Label Status (message dynamique)
  lv_obj_t *lbl_msg = lv_label_create(tab_wifi);
  lv_label_set_text(lbl_msg, "Pret");
  lv_obj_set_style_text_font(lbl_msg, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lbl_msg, lv_color_hex(0x9ca3af), 0);
  lv_obj_align(lbl_msg, LV_ALIGN_BOTTOM_LEFT, 0, -10);
  lv_obj_set_width(lbl_msg, 420);
  lv_label_set_long_mode(lbl_msg, LV_LABEL_LONG_WRAP);
  
  // Stocker les pointeurs pour accès depuis callbacks
  extern lv_obj_t *wifi_dd_networks;
  extern lv_obj_t *wifi_ta_ssid;
  extern lv_obj_t *wifi_ta_pwd;
  extern lv_obj_t *wifi_lbl_status_msg;
  wifi_dd_networks = dd_wifi;
  wifi_ta_ssid = ta_ssid;
  wifi_ta_pwd = ta_pwd;
  wifi_lbl_status_msg = lbl_msg;
  
  // ============================================
  // ONGLET INFOS — Appareils (noms colorés)
  // ============================================
  lv_obj_set_style_pad_all(tab_infos, 10, 0);
  lv_obj_set_scrollbar_mode(tab_infos, LV_SCROLLBAR_MODE_ON);
  
  // Carte "Connexions" avec lignes colorées
  lv_obj_t *card_infos = lv_obj_create(tab_infos);
  lv_obj_set_size(card_infos, 440, 240);
  lv_obj_set_pos(card_infos, 0, 0);
  lv_obj_set_style_bg_color(card_infos, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_infos, 1, 0);
  lv_obj_set_style_border_color(card_infos, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_infos, LV_OPA_30, 0);
  lv_obj_set_style_radius(card_infos, 10, 0);
  lv_obj_set_style_pad_all(card_infos, 14, 0);
  lv_obj_clear_flag(card_infos, LV_OBJ_FLAG_SCROLLABLE);
  
  // IP appareil (blanc comme MQTT)
  lv_obj_t *lbl_ip = lv_label_create(card_infos);
  lv_label_set_text(lbl_ip, "IP appareil:");
  lv_obj_set_style_text_color(lbl_ip, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(lbl_ip, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_ip, 0, 0);
  label_infos_ip_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_ip_val, "--");
  lv_obj_set_style_text_color(label_infos_ip_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_ip_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_ip_val, LV_ALIGN_TOP_RIGHT, 0, 0);
  
  // Shelly 1 (bleu)
  lv_obj_t *lbl_s1 = lv_label_create(card_infos);
  lv_label_set_text(lbl_s1, "Shelly 1:");
  lv_obj_set_style_text_color(lbl_s1, lv_color_hex(0x3b82f6), 0);
  lv_obj_set_style_text_font(lbl_s1, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_s1, 0, 38);
  label_infos_shelly1_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_shelly1_val, "--");
  lv_obj_set_style_text_color(label_infos_shelly1_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_shelly1_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_shelly1_val, LV_ALIGN_TOP_RIGHT, 0, 38);
  
  // Shelly 2 (bleu)
  lv_obj_t *lbl_s2 = lv_label_create(card_infos);
  lv_label_set_text(lbl_s2, "Shelly 2:");
  lv_obj_set_style_text_color(lbl_s2, lv_color_hex(0x3b82f6), 0);
  lv_obj_set_style_text_font(lbl_s2, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_s2, 0, 76);
  label_infos_shelly2_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_shelly2_val, "--");
  lv_obj_set_style_text_color(label_infos_shelly2_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_shelly2_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_shelly2_val, LV_ALIGN_TOP_RIGHT, 0, 76);
  
  // Enphase (orange = 0xf59e0b comme la date Enphase)
  lv_obj_t *lbl_ep = lv_label_create(card_infos);
  lv_label_set_text(lbl_ep, "Enphase:");
  lv_obj_set_style_text_color(lbl_ep, lv_color_hex(0xf59e0b), 0);
  lv_obj_set_style_text_font(lbl_ep, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_ep, 0, 114);
  label_infos_enphase_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_enphase_val, "--");
  lv_obj_set_style_text_color(label_infos_enphase_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_enphase_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_enphase_val, LV_ALIGN_TOP_RIGHT, 0, 114);
  
  // M'SunPV (vert)
  lv_obj_t *lbl_ms = lv_label_create(card_infos);
  lv_label_set_text(lbl_ms, "M'SunPV:");
  lv_obj_set_style_text_color(lbl_ms, lv_color_hex(0x22c55e), 0);
  lv_obj_set_style_text_font(lbl_ms, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_ms, 0, 152);
  label_infos_msunpv_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_msunpv_val, "--");
  lv_obj_set_style_text_color(label_infos_msunpv_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_msunpv_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_msunpv_val, LV_ALIGN_TOP_RIGHT, 0, 152);
  
  // MQTT (blanc comme IP)
  lv_obj_t *lbl_mq = lv_label_create(card_infos);
  lv_label_set_text(lbl_mq, "MQTT:");
  lv_obj_set_style_text_color(lbl_mq, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(lbl_mq, &lv_font_montserrat_16, 0);
  lv_obj_set_pos(lbl_mq, 0, 190);
  label_infos_mqtt_val = lv_label_create(card_infos);
  lv_label_set_text(label_infos_mqtt_val, "--");
  lv_obj_set_style_text_color(label_infos_mqtt_val, lv_color_hex(0xffffff), 0);
  lv_obj_set_style_text_font(label_infos_mqtt_val, &lv_font_montserrat_16, 0);
  lv_obj_align(label_infos_mqtt_val, LV_ALIGN_TOP_RIGHT, 0, 190);
  
  lv_obj_set_style_pad_all(tab_meteo, 10, 0);
  lv_obj_set_scrollbar_mode(tab_meteo, LV_SCROLLBAR_MODE_ON);
  lv_obj_t *card_meteo = lv_obj_create(tab_meteo);
  lv_obj_set_size(card_meteo, 440, 100);
  lv_obj_set_pos(card_meteo, 0, 0);
  lv_obj_set_style_bg_color(card_meteo, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_meteo, 1, 0);
  lv_obj_set_style_border_color(card_meteo, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_meteo, LV_OPA_30, 0);
  lv_obj_set_style_radius(card_meteo, 10, 0);
  lv_obj_set_style_pad_all(card_meteo, 14, 0);
  lv_obj_clear_flag(card_meteo, LV_OBJ_FLAG_SCROLLABLE);
  label_settings_meteo = lv_label_create(card_meteo);
  lv_label_set_text(label_settings_meteo, "");
  lv_obj_set_style_text_font(label_settings_meteo, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(label_settings_meteo, lv_color_hex(0xffffff), 0);
  lv_obj_align(label_settings_meteo, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_width(label_settings_meteo, 400);
  
  lv_obj_set_style_pad_all(tab_maint, 12, 0);
  lv_obj_t *btn_restart = lv_btn_create(tab_maint);
  lv_obj_set_size(btn_restart, 420, 48);
  lv_obj_align(btn_restart, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_style_bg_color(btn_restart, lv_color_hex(0x3b82f6), 0);
  lv_obj_set_style_radius(btn_restart, 10, 0);
  lv_obj_add_event_cb(btn_restart, settings_restart_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lr = lv_label_create(btn_restart);
  lv_label_set_text(lr, "Redemarrer");
  lv_obj_set_style_text_font(lr, &lv_font_montserrat_16, 0);
  lv_obj_center(lr);
  
  lv_obj_t *btn_flip = lv_btn_create(tab_maint);
  lv_obj_set_size(btn_flip, 420, 48);
  lv_obj_align(btn_flip, LV_ALIGN_TOP_LEFT, 0, 58);
  lv_obj_set_style_bg_color(btn_flip, lv_color_hex(0x374151), 0);
  lv_obj_set_style_radius(btn_flip, 10, 0);
  lv_obj_add_event_cb(btn_flip, settings_flip_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lf = lv_label_create(btn_flip);
  lv_label_set_text(lf, "Retourner ecran");
  lv_obj_set_style_text_font(lf, &lv_font_montserrat_16, 0);
  lv_obj_center(lf);
  
  lv_obj_t *btn_reset = lv_btn_create(tab_maint);
  lv_obj_set_size(btn_reset, 420, 48);
  lv_obj_align(btn_reset, LV_ALIGN_TOP_LEFT, 0, 116);
  lv_obj_set_style_bg_color(btn_reset, lv_color_hex(0xef4444), 0);
  lv_obj_set_style_radius(btn_reset, 10, 0);
  lv_obj_add_event_cb(btn_reset, settings_reset_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lres = lv_label_create(btn_reset);
  lv_label_set_text(lres, "Reset (tout effacer)");
  lv_obj_set_style_text_font(lres, &lv_font_montserrat_16, 0);
  lv_obj_center(lres);
  
  lv_obj_set_style_pad_all(tab_logs, 10, 0);
  lv_obj_set_scrollbar_mode(tab_logs, LV_SCROLLBAR_MODE_ON);
  lv_obj_t *card_logs = lv_obj_create(tab_logs);
  lv_obj_set_size(card_logs, 440, 320);
  lv_obj_set_pos(card_logs, 0, 0);
  lv_obj_set_style_bg_color(card_logs, lv_color_hex(COLOR_CARD), 0);
  lv_obj_set_style_border_width(card_logs, 1, 0);
  lv_obj_set_style_border_color(card_logs, lv_color_hex(0xfbbf24), 0);
  lv_obj_set_style_border_opa(card_logs, LV_OPA_30, 0);
  lv_obj_set_style_radius(card_logs, 10, 0);
  lv_obj_set_style_pad_all(card_logs, 12, 0);
  lv_obj_set_scrollbar_mode(card_logs, LV_SCROLLBAR_MODE_AUTO);
  label_settings_logs = lv_label_create(card_logs);
  lv_label_set_text(label_settings_logs, "");
  lv_obj_set_style_text_font(label_settings_logs, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(label_settings_logs, lv_color_hex(0x9ca3af), 0);
  lv_obj_align(label_settings_logs, LV_ALIGN_TOP_LEFT, 0, 0);
  lv_obj_set_width(label_settings_logs, 400);
  lv_label_set_long_mode(label_settings_logs, LV_LABEL_LONG_WRAP);
  
  lv_obj_t *btn_back = lv_btn_create(screenSettings);
  lv_obj_set_size(btn_back, 120, 40);
  lv_obj_align(btn_back, LV_ALIGN_BOTTOM_MID, 0, -8);
  lv_obj_set_style_bg_color(btn_back, lv_color_hex(COLOR_HEADER), 0);
  lv_obj_add_event_cb(btn_back, settings_back_clicked, LV_EVENT_CLICKED, NULL);
  lv_obj_t *lb = lv_label_create(btn_back);
  lv_label_set_text(lb, "Retour");
  lv_obj_set_style_text_font(lb, &lv_font_montserrat_16, 0);
  lv_obj_center(lb);
}

void updateSettingsUI() {
  if (!label_settings_wifi || !label_settings_meteo || !label_settings_logs) return;
  extern String ipAddress;
  extern long rssi;
  extern bool wifiAPMode;
  if (WiFi.status() == WL_CONNECTED) {
    ipAddress = WiFi.localIP().toString();
    rssi = (long)WiFi.RSSI();
  }
  extern String config_shelly1_ip;
  extern String config_shelly2_ip;
  extern String config_enphase_ip;
  extern String config_msunpv_ip;
  extern String config_mqtt_ip;
  extern int config_mqtt_port;
  char buf[120];
  
  // WiFi tab : Etat, Signal, IP (contenu correct, plus d'inscriptions illisibles)
  const char* etat = wifiAPMode ? "Mode AP" : (WiFi.status() == WL_CONNECTED ? "Connecte" : "Deconnecte");
  snprintf(buf, sizeof(buf), "Etat: %s\nSignal: %ld dBm\nIP: %s",
           etat, rssi, ipAddress.length() ? ipAddress.c_str() : "--");
  lv_label_set_text(label_settings_wifi, buf);
  
  // Onglet Infos : valeurs par ligne (noms déjà colorés dans createSettingsScreen)
  if (label_infos_ip_val) lv_label_set_text(label_infos_ip_val, ipAddress.length() ? ipAddress.c_str() : "--");
  if (label_infos_shelly1_val) lv_label_set_text(label_infos_shelly1_val, config_shelly1_ip.length() ? config_shelly1_ip.c_str() : "non configure");
  if (label_infos_shelly2_val) lv_label_set_text(label_infos_shelly2_val, config_shelly2_ip.length() ? config_shelly2_ip.c_str() : "non configure");
  if (label_infos_enphase_val) lv_label_set_text(label_infos_enphase_val, config_enphase_ip.length() ? config_enphase_ip.c_str() : "non configure");
  if (label_infos_msunpv_val) lv_label_set_text(label_infos_msunpv_val, config_msunpv_ip.length() ? config_msunpv_ip.c_str() : "non configure");
  if (label_infos_mqtt_val) {
    snprintf(buf, sizeof(buf), "%s:%d", config_mqtt_ip.length() ? config_mqtt_ip.c_str() : "--", config_mqtt_port);
    lv_label_set_text(label_infos_mqtt_val, buf);
  }
  
  snprintf(buf, sizeof(buf), "Ville: %s\nConfig: portail web", weather_city.length() ? weather_city.c_str() : "non configure");
  lv_label_set_text(label_settings_meteo, buf);
  char logbuf[512];
  settings_get_log_text(logbuf, (int)sizeof(logbuf));
  lv_label_set_text(label_settings_logs, logbuf[0] ? logbuf : "(aucun log)");
}

// Variables WiFi configuration (onglet WiFi)
lv_obj_t *wifi_dd_networks = NULL;
lv_obj_t *wifi_ta_ssid = NULL;
lv_obj_t *wifi_ta_pwd = NULL;
lv_obj_t *wifi_lbl_status_msg = NULL;
static int wifi_networks_count = 0;
static String wifi_networks_str = "";  // "SSID1\nSSID2\nSSID3..."
static int32_t wifi_rssi_values[30] = {0};  // max 30 réseaux

// Callbacks WiFi configuration
void wifi_scan_clicked(lv_event_t *e);
void wifi_network_selected(lv_event_t *e);
void wifi_connect_clicked(lv_event_t *e);
void wifi_clear_clicked(lv_event_t *e);
// static bool ledLockedGreen = false;
// static bool firstRun = true;

void updateMainUI() {
  char buffer[64];
  
  // V10.0 - Démarrage : LED ROUGE jusqu'aux données MQTT + 30s timeout
  extern bool ledStartupLock;
  extern unsigned long startupTime;
  extern bool mqttDataReceived;
  if (ledStartupLock) {
    // Attendre : données MQTT reçues ET minimum 30s écoulées
    if (mqttDataReceived && (millis() - startupTime) > 30000) {
      ledStartupLock = false;
      Serial.println("[V10.0] Déverrouillage LED (données MQTT + timeout)");
    }
    // OU : timeout de 60s même sans données MQTT
    else if ((millis() - startupTime) > 60000) {
      ledStartupLock = false;
      Serial.println("[V10.0] Déverrouillage LED (timeout 60s)");
    }
  }
  
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  // V14.0 - Formats de date configurables
  extern int dateFormatIndex;
  const char* daysFull[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
  const char* daysShort[] = {"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
  const char* months[] = {"Jan.", "Fev.", "Mar.", "Avr.", "Mai", "Juin", "Juil.", "Aout", "Sep.", "Oct.", "Nov.", "Dec."};
  
  switch(dateFormatIndex) {
    case 0: // Dimanche 28/12/25 (format original)
      sprintf(buffer, "%s %02d/%02d/%02d", 
              daysFull[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year % 100);
      break;
    case 1: // Dim. 28 Déc. 2025 (format demandé par l'utilisateur)
      sprintf(buffer, "%s %d %s %d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday,
              months[timeinfo->tm_mon],
              timeinfo->tm_year + 1900);
      break;
    case 2: // 28/12/2025 (compact)
      sprintf(buffer, "%02d/%02d/%04d", 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year + 1900);
      break;
    case 3: // Dim. 28/12/2025 (abrégé + année complète)
      sprintf(buffer, "%s %02d/%02d/%04d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday, 
              timeinfo->tm_mon + 1, 
              timeinfo->tm_year + 1900);
      break;
    default:
      sprintf(buffer, "%s %d %s %d", 
              daysShort[timeinfo->tm_wday], 
              timeinfo->tm_mday,
              months[timeinfo->tm_mon],
              timeinfo->tm_year + 1900);
      break;
  }
  lv_label_set_text(label_date, buffer);
  // V15.0 - Couleur date : orange pour Enphase, blanc pour MQTT
  lv_obj_set_style_text_color(label_date, lv_color_hex(activeScreenType == 1 ? 0xf59e0b : 0xffffff), 0);
  
  sprintf(buffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  lv_label_set_text(label_time, buffer);
  
  // Production (valeur + unité séparées)
  sprintf(buffer, "%.0f", solarProd);
  lv_label_set_text(label_prod_value, buffer);
  
  // Consommation
  sprintf(buffer, "%.0f", homeConso);
  lv_label_set_text(label_conso_value, buffer);
  
  // Routeur
  sprintf(buffer, "%.0f", routerPower);
  lv_label_set_text(label_router_value, buffer);
  
  // Conso Jour
  sprintf(buffer, "%.1f kWh", consoJour);
  lv_label_set_text(label_conso_jour_value, buffer);
  
  // Température cumulus
  sprintf(buffer, "%.0f°C", waterTemp);
  lv_label_set_text(label_water_temp_value, buffer);
  
  // Cumulus : recolor de l’icône (gris < 18°C, sinon phase 1–5). Fond reste transparent.
  #define CUMULUS_GRAY 0x374151
  /* Phases : 45-58°C = même couleur que Production solaire (COLOR_PROD), opacité un peu plus élevée */
  static const uint32_t phase_color[] = { 0x1e40af, 0x3b82f6, 0xf59e0b, (uint32_t)COLOR_PROD, 0xef4444 };
  uint32_t cumulus_recolor = (waterTemp < 18.f) ? (uint32_t)CUMULUS_GRAY
    : (waterTemp >= 58.f) ? phase_color[4]
    : (waterTemp >= 45.f) ? phase_color[3]
    : (waterTemp >= 36.f) ? phase_color[2]
    : (waterTemp >= 27.f) ? phase_color[1]
    : phase_color[0];
  if (img_cumulus_right) {
    lv_obj_set_style_image_recolor(img_cumulus_right, lv_color_hex(cumulus_recolor), 0);
    lv_obj_set_style_image_recolor_opa(img_cumulus_right, (waterTemp >= 36.f) ? LV_OPA_80 : LV_OPA_70, 0);
  }
  /* Couleur du label = phase actuelle */
  uint32_t label_color = (waterTemp >= 58.f) ? 0xef4444 : (waterTemp >= 45.f) ? (uint32_t)COLOR_PROD : (waterTemp >= 36.f) ? 0xf59e0b : (waterTemp >= 27.f) ? 0x3b82f6 : (waterTemp >= 18.f) ? 0x1e40af : CUMULUS_GRAY;
  lv_obj_set_style_text_color(label_water_temp_value, lv_color_hex(label_color), 0);
  
  // LED - LOGIQUE STICKY GREEN
  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  
  // Réinitialisation à 23h00 (LED redevient rouge et peut re-changer)
  if (hour == 23 && minute == 0) {
    ledLockedGreen = false;
  }
  
  // V10.0 - Pendant startup, forcer LED rouge
  if (!ledStartupLock) {
    // Si pas encore verrouillée, vérifier les conditions
    if (!ledLockedGreen) {
      // VERTE si : (cumulus > 55) ET (routage = 0)
      if ((waterTemp > 55) && (routerPower == 0)) {
        ledLockedGreen = true;
      }
    }
  } else {
    // Pendant startup : forcer rouge
    ledLockedGreen = false;
  }
  
  // Appliquer la couleur
  /* OK = couleurs d’origine ; pas OK = recolor 0x2a2625 (entre LED et fond carte), opa 70 % */
  if (ledLockedGreen) {
    lv_obj_set_style_image_recolor_opa(obj_led_indicator, LV_OPA_TRANSP, 0);
  } else {
    lv_obj_set_style_image_recolor(obj_led_indicator, lv_color_hex(0x2a2625), 0);
    lv_obj_set_style_image_recolor_opa(obj_led_indicator, LV_OPA_70, 0);
  }
  
  // Températures
  snprintf(buffer, sizeof(buffer), "%s  %.0f°C", weather_city.c_str(), tempExt);
  lv_label_set_text(label_temp_ext, buffer);
  
  sprintf(buffer, "Salon  %.0f°C", tempSalon);
  lv_label_set_text(label_temp_salon, buffer);
  
  // Mise à jour LED status
  extern bool wifiConnected;
  extern bool mqttConnected;
  extern bool enphase_connected;
  extern bool shelly1_connected;
  extern bool shelly2_connected;
  extern String config_shelly1_ip;
  extern String config_shelly2_ip;
  
  if (wifiConnected) {
    lv_img_set_src(led_wifi, &wifi_cercle_vert);
  } else {
    lv_img_set_src(led_wifi, &wifi_barre_oblique);
  }
  
  if (mqttConnected) {
    lv_img_set_src(led_mqtt, &mqtt_png);
  } else {
    lv_img_set_src(led_mqtt, &mqtt_png_gris);
  }
  
  if (led_shelly) {
    bool shelly_ok = (config_shelly1_ip.length() == 0 || shelly1_connected) && (config_shelly2_ip.length() == 0 || shelly2_connected);
    lv_img_set_src(led_shelly, shelly_ok ? &Shelly32 : &Shelly32_gris);
  }
  
  if (led_enphase) {
    lv_img_set_src(led_enphase, enphase_connected ? &Enphase_logo : &Enphase_logo_gris);
  }
  
  // ============================================
  // MISE À JOUR CARTE FLUX PV → MAISON → RÉSEAU
  // ============================================
  if (zone_flow_left && img_arrow_pv_house && img_arrow_house_grid && label_flow_pv_val && label_flow_grid_val && img_flow_reseau) {
    float prod_w = solarProd;
    float conso_w = homeConso;  /* signe: négatif = export, positif = import */
    bool pv_arrow_visible = (prod_w > (float)FLOW_THRESHOLD_PV_W);
    bool cumulus_active = (routerPower > 0.f);

    sprintf(buffer, "%d W", (int)(prod_w + 0.5f));
    if (pv_arrow_visible) {
      lv_img_set_src(img_arrow_pv_house, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_pv_house, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(label_flow_pv_val, buffer);
      lv_obj_remove_flag(label_flow_pv_val, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(img_arrow_pv_house, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(label_flow_pv_val, LV_OBJ_FLAG_HIDDEN);
    }

    if (cumulus_active) {
      lv_img_set_src(img_flow_reseau, &chauffe_eau);
      lv_img_set_src(img_arrow_house_grid, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_house_grid, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(label_flow_grid_val, LV_OBJ_FLAG_HIDDEN);
      sprintf(buffer, "%d W", (int)(routerPower + 0.5f));
      lv_label_set_text(label_flow_grid_val, buffer);
    } else {
      lv_img_set_src(img_flow_reseau, &reseau_electrique);
      if (conso_w < 0.f) {
        /* Export: flèche maison → réseau, verte, valeur en magnitude */
        lv_img_set_src(img_arrow_house_grid, &icoflechedroiteverte);
        lv_obj_remove_flag(img_arrow_house_grid, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(label_flow_grid_val, LV_OBJ_FLAG_HIDDEN);
        sprintf(buffer, "%d W", (int)(-conso_w + 0.5f));
        lv_label_set_text(label_flow_grid_val, buffer);
      } else if (conso_w > 0.f) {
        /* Import: flèche réseau → maison, orange, valeur en magnitude */
        lv_img_set_src(img_arrow_house_grid, &icoflechegaucheorange);
        lv_obj_remove_flag(img_arrow_house_grid, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(label_flow_grid_val, LV_OBJ_FLAG_HIDDEN);
        sprintf(buffer, "%d W", (int)(conso_w + 0.5f));
        lv_label_set_text(label_flow_grid_val, buffer);
      } else {
        lv_obj_add_flag(img_arrow_house_grid, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(label_flow_grid_val, LV_OBJ_FLAG_HIDDEN);
      }
    }
  }
  
  // ============================================
  // MISE À JOUR BADGE M'SunPV (V3.0)
  // ============================================
  lv_label_set_text(label_msunpv_status, msunpv_status.c_str());
  
  uint32_t badgeColor;
  if (msunpv_status == "AUTO") {
    badgeColor = COLOR_MSUNPV_AUTO;  // Vert
  } else if (msunpv_status == "MANU") {
    badgeColor = COLOR_MSUNPV_MANU;  // Bleu
  } else {
    badgeColor = COLOR_MSUNPV_OFF;   // Blanc
  }
  lv_obj_set_style_bg_color(label_msunpv_status, lv_color_hex(badgeColor), 0);
  
  // Texte noir si OFF (blanc)
  if (msunpv_status == "OFF") {
    lv_obj_set_style_text_color(label_msunpv_status, lv_color_hex(0x000000), 0);
  } else {
    lv_obj_set_style_text_color(label_msunpv_status, lv_color_hex(0xffffff), 0);
  }
}

// V15.0 - Mise à jour écran Enphase (données Envoy + météo footer)
void updateEnphaseUI() {
  char buffer[64];
  extern int dateFormatIndex;
  const char* daysFull[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
  const char* daysShort[] = {"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
  const char* months[] = {"Jan.", "Fev.", "Mar.", "Avr.", "Mai", "Juin", "Juil.", "Aout", "Sep.", "Oct.", "Nov.", "Dec."};
  
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  // Date : même format configurable que l'écran principal (dateFormatIndex)
  switch(dateFormatIndex) {
    case 0:
      sprintf(buffer, "%s %02d/%02d/%02d",
              daysFull[timeinfo->tm_wday], timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year % 100);
      break;
    case 1:
      sprintf(buffer, "%s %d %s %d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, months[timeinfo->tm_mon], timeinfo->tm_year + 1900);
      break;
    case 2:
      sprintf(buffer, "%02d/%02d/%04d",
              timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
      break;
    case 3:
      sprintf(buffer, "%s %02d/%02d/%04d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
      break;
    default:
      sprintf(buffer, "%s %d %s %d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, months[timeinfo->tm_mon], timeinfo->tm_year + 1900);
      break;
  }
  lv_label_set_text(label_ep_date, buffer);
  lv_obj_set_style_text_color(label_ep_date, lv_color_hex(0xf59e0b), 0);
  
  if (label_ep_prod_jour_title) lv_label_set_text(label_ep_prod_jour_title, "PRODUCTION JOUR");
  if (label_ep_prod_jour_unit) lv_label_set_text(label_ep_prod_jour_unit, "kWh");
  
  sprintf(buffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  lv_label_set_text(label_ep_time, buffer);
  
  // Production live (W) - enphase_pact_prod
  sprintf(buffer, "%.0f", enphase_pact_prod);
  lv_label_set_text(label_ep_prod, buffer);
  lv_obj_set_style_text_color(label_ep_prod, lv_color_hex(COLOR_PROD), 0);
  
  // Conso maison live (W) - enphase_pact_grid (ENEDIS - négatif = export)
  sprintf(buffer, "%.0f", enphase_pact_grid);
  lv_label_set_text(label_ep_conso, buffer);
  lv_obj_set_style_text_color(label_ep_conso, lv_color_hex(COLOR_CONSO), 0);
  
  // Production jour (kWh) - enphase_energy_produced en Wh
  sprintf(buffer, "%.1f", enphase_energy_produced / 1000.0f);
  lv_label_set_text(label_ep_prod_jour, buffer);
  
  // Conso jour (kWh) - enphase_energy_imported en Wh (uniquement importé réseau, comme page web Enphase)
  sprintf(buffer, "%.1f", enphase_energy_imported / 1000.0f);
  lv_label_set_text(label_ep_conso_jour, buffer);
  
  // Zone flux PV → Maison → Réseau (données Enphase)
  if (zone_flow_left_ep && img_arrow_pv_house_ep && img_arrow_house_grid_ep && label_flow_pv_val_ep && label_flow_grid_val_ep && img_flow_reseau_ep) {
    float prod_w = enphase_pact_prod;
    float grid_w = enphase_pact_grid;  // ENEDIS au lieu de conso
    bool pv_arrow_visible = (prod_w > (float)FLOW_THRESHOLD_PV_W);
    
    sprintf(buffer, "%d W", (int)(prod_w + 0.5f));
    if (pv_arrow_visible) {
      lv_img_set_src(img_arrow_pv_house_ep, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_pv_house_ep, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(label_flow_pv_val_ep, buffer);
      lv_obj_remove_flag(label_flow_pv_val_ep, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(img_arrow_pv_house_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(label_flow_pv_val_ep, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (grid_w < 0.f) {
      lv_img_set_src(img_flow_reseau_ep, &reseau_electrique);
      lv_img_set_src(img_arrow_house_grid_ep, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
      sprintf(buffer, "%d W", (int)(-grid_w + 0.5f));
      lv_label_set_text(label_flow_grid_val_ep, buffer);
    } else if (grid_w > 0.f) {
      lv_img_set_src(img_flow_reseau_ep, &reseau_electrique);
      lv_img_set_src(img_arrow_house_grid_ep, &icoflechegaucheorange);
      lv_obj_remove_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
      sprintf(buffer, "%d W", (int)(grid_w + 0.5f));
      lv_label_set_text(label_flow_grid_val_ep, buffer);
    } else {
      lv_img_set_src(img_flow_reseau_ep, &reseau_electrique);
      lv_obj_add_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
    }
  }
  
  // Badge état réseau (Import / Auto / Export) — tiers droit flux
  if (obj_flow_state_ep && label_flow_state_ep) {
    float grid_w = enphase_pact_grid;  // ENEDIS au lieu de conso
    #define FLOW_STATE_THRESHOLD_W 5.f
    if (grid_w > FLOW_STATE_THRESHOLD_W) {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0xd97706), 0);   // Import (ambre)
      lv_label_set_text(label_flow_state_ep, "Import");
    } else if (grid_w < -FLOW_STATE_THRESHOLD_W) {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0x0d9488), 0);   // Export (bleu-vert)
      lv_label_set_text(label_flow_state_ep, "Export");
    } else {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0x16a34a), 0);   // Auto (vert)
      lv_label_set_text(label_flow_state_ep, "Auto");
    }
  }
  
  // Footer météo - Ville, icône, temp
  lv_label_set_text(label_ep_weather_city, weather_city.c_str());
  lv_img_set_src(img_ep_weather_icon, weather_getIconFromCode(weather_code));
  sprintf(buffer, "%.0f°C", weather_temp);
  lv_label_set_text(label_ep_weather_temp, buffer);
  
  // 5 jours prévision J+1 à J+5 — libellé = Lun, Mar, Mer, Jeu, Ven, Sam, Dim (3 lettres)
  const char* daysShort3[] = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
  struct tm midnight = *timeinfo;
  midnight.tm_hour = 0;
  midnight.tm_min = 0;
  midnight.tm_sec = 0;
  time_t midnight_ts = mktime(&midnight);
  for (int i = 0; i < 5; i++) {
    time_t day_ts = midnight_ts + (time_t)(1 + i) * 86400;
    struct tm *day_tm = localtime(&day_ts);
    if (day_tm) {
      lv_label_set_text(label_ep_weather_day[i], daysShort3[day_tm->tm_wday]);
    } else {
      lv_label_set_text(label_ep_weather_day[i], "-");
    }
    int idx = i + 1;  // forecast index 0 = aujourd'hui, 1 = J+1, ...
    if (idx < 6 && weather_forecast_codes[idx] > 0) {
      lv_img_set_src(img_ep_weather_icon_day[i], weather_getIconFromCode(weather_forecast_codes[idx]));
    }
    if (idx < 6) {
      sprintf(buffer, "%d°", weather_forecast_temps[idx]);
      lv_label_set_text(label_ep_weather_temp_day[i], buffer);
    } else {
      lv_label_set_text(label_ep_weather_temp_day[i], "--°");
    }
  }
  
  // Couleurs TEMPO si activé : ville = jour, J+1 = demain (vert si en attente après minuit)
  if (tempo_enabled) {
    uint32_t colorToday = 0xffffff;
    if (tempo_today_color == "Bleu") colorToday = 0x3b82f6;
    else if (tempo_today_color == "Blanc") colorToday = 0x94a3b8;
    else if (tempo_today_color == "Rouge") colorToday = 0xef4444;
    lv_obj_set_style_text_color(label_ep_weather_city, lv_color_hex(colorToday), 0);
    uint32_t colorTomorrow = 0xd1d5db;
    if (tempo_tomorrow_pending) colorTomorrow = 0x22c55e;
    else if (tempo_tomorrow_color == "Bleu") colorTomorrow = 0x3b82f6;
    else if (tempo_tomorrow_color == "Blanc") colorTomorrow = 0x94a3b8;
    else if (tempo_tomorrow_color == "Rouge") colorTomorrow = 0xef4444;
    lv_obj_set_style_text_color(label_ep_weather_day[0], lv_color_hex(colorTomorrow), 0);
  } else {
    lv_obj_set_style_text_color(label_ep_weather_city, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_color(label_ep_weather_day[0], lv_color_hex(0xd1d5db), 0);
  }
  
  // LEDs header
  extern bool wifiConnected;
  extern bool mqttConnected;
  extern bool shelly1_connected;
  extern bool shelly2_connected;
  extern String config_shelly1_ip;
  extern String config_shelly2_ip;
  
  lv_img_set_src(led_ep_wifi, wifiConnected ? &wifi_cercle_vert : &wifi_barre_oblique);
  lv_img_set_src(led_ep_mqtt, mqttConnected ? &mqtt_png : &mqtt_png_gris);
  bool shelly_ok = (config_shelly1_ip.length() == 0 || shelly1_connected) && (config_shelly2_ip.length() == 0 || shelly2_connected);
  lv_img_set_src(led_ep_shelly, shelly_ok ? &Shelly32 : &Shelly32_gris);
  lv_img_set_src(led_ep_enphase, enphase_connected ? &Enphase_logo : &Enphase_logo_gris);
}

// M'SunPV - Mise à jour écran (même layout qu'Enphase, date verte, données M'SunPV)
void updateMSunPVUI() {
  char buffer[64];
  extern int dateFormatIndex;
  const char* daysFull[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
  const char* daysShort[] = {"Dim.", "Lun.", "Mar.", "Mer.", "Jeu.", "Ven.", "Sam."};
  const char* months[] = {"Jan.", "Fev.", "Mar.", "Avr.", "Mai", "Juin", "Juil.", "Aout", "Sep.", "Oct.", "Nov.", "Dec."};
  
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  switch(dateFormatIndex) {
    case 0:
      sprintf(buffer, "%s %02d/%02d/%02d",
              daysFull[timeinfo->tm_wday], timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year % 100);
      break;
    case 1:
      sprintf(buffer, "%s %d %s %d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, months[timeinfo->tm_mon], timeinfo->tm_year + 1900);
      break;
    case 2:
      sprintf(buffer, "%02d/%02d/%04d",
              timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
      break;
    case 3:
      sprintf(buffer, "%s %02d/%02d/%04d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);
      break;
    default:
      sprintf(buffer, "%s %d %s %d",
              daysShort[timeinfo->tm_wday], timeinfo->tm_mday, months[timeinfo->tm_mon], timeinfo->tm_year + 1900);
      break;
  }
  lv_label_set_text(label_ep_date, buffer);
  lv_obj_set_style_text_color(label_ep_date, lv_color_hex(0x22c55e), 0);  // Vert M'SunPV
  
  if (label_ep_prod_jour_title) lv_label_set_text(label_ep_prod_jour_title, "ROUTAGE");
  if (label_ep_prod_jour_unit) lv_label_set_text(label_ep_prod_jour_unit, "%");
  
  sprintf(buffer, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  lv_label_set_text(label_ep_time, buffer);
  
  // Carte gauche: Prod PV (W), Conso maison (W)
  sprintf(buffer, "%.0f", msunpv_powPV);
  lv_label_set_text(label_ep_prod, buffer);
  lv_obj_set_style_text_color(label_ep_prod, lv_color_hex(COLOR_PROD), 0);
  
  sprintf(buffer, "%.0f", msunpv_powReso);
  lv_label_set_text(label_ep_conso, buffer);
  lv_obj_set_style_text_color(label_ep_conso, lv_color_hex(COLOR_CONSO), 0);
  
  // Carte droite: ROUTAGE (OUTBAL %), CONSO JOUR (kWh)
  sprintf(buffer, "%d", msunpv_outBal);
  lv_label_set_text(label_ep_prod_jour, buffer);
  lv_obj_set_style_text_color(label_ep_prod_jour, lv_color_hex(COLOR_PROD), 0);
  
  sprintf(buffer, "%.1f", msunpv_enConso / 1000.0f);
  lv_label_set_text(label_ep_conso_jour, buffer);
  
  // Zone flux: gauche = Prod PV (W), droite = Conso maison (W)
  if (zone_flow_left_ep && img_arrow_pv_house_ep && img_arrow_house_grid_ep && label_flow_pv_val_ep && label_flow_grid_val_ep && img_flow_reseau_ep) {
    float prod_w = msunpv_powPV;
    float conso_w = msunpv_powReso;
    bool pv_arrow_visible = (prod_w > (float)FLOW_THRESHOLD_PV_W);
    
    sprintf(buffer, "%d W", (int)(prod_w + 0.5f));
    if (pv_arrow_visible) {
      lv_img_set_src(img_arrow_pv_house_ep, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_pv_house_ep, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(label_flow_pv_val_ep, buffer);
      lv_obj_remove_flag(label_flow_pv_val_ep, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(img_arrow_pv_house_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(label_flow_pv_val_ep, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Flèche Maison ↔ Réseau : Import = réseau → maison (flèche gauche orange), Export = maison → réseau (flèche droite verte)
    lv_img_set_src(img_flow_reseau_ep, &reseau_electrique);
    #define FLOW_MSUNPV_THRESHOLD_W 5.f
    if (conso_w > prod_w + FLOW_MSUNPV_THRESHOLD_W) {
      // Import : flux du réseau vers la maison → flèche gauche (orange)
      lv_img_set_src(img_arrow_house_grid_ep, &icoflechegaucheorange);
      lv_obj_remove_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
      sprintf(buffer, "%d W", (int)(conso_w - prod_w + 0.5f));
      lv_label_set_text(label_flow_grid_val_ep, buffer);
    } else if (conso_w < prod_w - FLOW_MSUNPV_THRESHOLD_W) {
      // Export : flux de la maison vers le réseau → flèche droite (verte)
      lv_img_set_src(img_arrow_house_grid_ep, &icoflechedroiteverte);
      lv_obj_remove_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
      sprintf(buffer, "%d W", (int)(prod_w - conso_w + 0.5f));
      lv_label_set_text(label_flow_grid_val_ep, buffer);
    } else {
      // Auto : conso ≈ prod, pas de flux net
      lv_obj_add_flag(img_arrow_house_grid_ep, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(label_flow_grid_val_ep, LV_OBJ_FLAG_HIDDEN);
    }
  }
  
  // Badge: Conso maison vs Prod PV → Import / Auto / Export
  if (obj_flow_state_ep && label_flow_state_ep) {
    float prod_w = msunpv_powPV;
    float conso_w = msunpv_powReso;
    #define FLOW_STATE_THRESHOLD_W 5.f
    if (conso_w > prod_w + FLOW_STATE_THRESHOLD_W) {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0xd97706), 0);
      lv_label_set_text(label_flow_state_ep, "Import");
    } else if (conso_w < prod_w - FLOW_STATE_THRESHOLD_W) {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0x0d9488), 0);
      lv_label_set_text(label_flow_state_ep, "Export");
    } else {
      lv_obj_set_style_bg_color(obj_flow_state_ep, lv_color_hex(0x16a34a), 0);
      lv_label_set_text(label_flow_state_ep, "Auto");
    }
  }
  
  // Footer météo (identique Enphase)
  lv_label_set_text(label_ep_weather_city, weather_city.c_str());
  lv_img_set_src(img_ep_weather_icon, weather_getIconFromCode(weather_code));
  sprintf(buffer, "%.0f°C", weather_temp);
  lv_label_set_text(label_ep_weather_temp, buffer);
  
  const char* daysShort3[] = {"Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"};
  struct tm midnight = *timeinfo;
  midnight.tm_hour = 0;
  midnight.tm_min = 0;
  midnight.tm_sec = 0;
  time_t midnight_ts = mktime(&midnight);
  for (int i = 0; i < 5; i++) {
    time_t day_ts = midnight_ts + (time_t)(1 + i) * 86400;
    struct tm *day_tm = localtime(&day_ts);
    if (day_tm) {
      lv_label_set_text(label_ep_weather_day[i], daysShort3[day_tm->tm_wday]);
    } else {
      lv_label_set_text(label_ep_weather_day[i], "-");
    }
    int idx = i + 1;
    if (idx < 6 && weather_forecast_codes[idx] > 0) {
      lv_img_set_src(img_ep_weather_icon_day[i], weather_getIconFromCode(weather_forecast_codes[idx]));
    }
    if (idx < 6) {
      sprintf(buffer, "%d°", weather_forecast_temps[idx]);
      lv_label_set_text(label_ep_weather_temp_day[i], buffer);
    } else {
      lv_label_set_text(label_ep_weather_temp_day[i], "--°");
    }
  }
  
  if (tempo_enabled) {
    uint32_t colorToday = 0xffffff;
    if (tempo_today_color == "Bleu") colorToday = 0x3b82f6;
    else if (tempo_today_color == "Blanc") colorToday = 0x94a3b8;
    else if (tempo_today_color == "Rouge") colorToday = 0xef4444;
    lv_obj_set_style_text_color(label_ep_weather_city, lv_color_hex(colorToday), 0);
    uint32_t colorTomorrow = 0xd1d5db;
    if (tempo_tomorrow_pending) colorTomorrow = 0x22c55e;
    else if (tempo_tomorrow_color == "Bleu") colorTomorrow = 0x3b82f6;
    else if (tempo_tomorrow_color == "Blanc") colorTomorrow = 0x94a3b8;
    else if (tempo_tomorrow_color == "Rouge") colorTomorrow = 0xef4444;
    lv_obj_set_style_text_color(label_ep_weather_day[0], lv_color_hex(colorTomorrow), 0);
  } else {
    lv_obj_set_style_text_color(label_ep_weather_city, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_color(label_ep_weather_day[0], lv_color_hex(0xd1d5db), 0);
  }
  
  extern bool wifiConnected;
  extern bool mqttConnected;
  extern bool shelly1_connected;
  extern bool shelly2_connected;
  extern String config_shelly1_ip;
  extern String config_shelly2_ip;
  
  lv_img_set_src(led_ep_wifi, wifiConnected ? &wifi_cercle_vert : &wifi_barre_oblique);
  lv_img_set_src(led_ep_mqtt, mqttConnected ? &mqtt_png : &mqtt_png_gris);
  bool shelly_ok = (config_shelly1_ip.length() == 0 || shelly1_connected) && (config_shelly2_ip.length() == 0 || shelly2_connected);
  lv_img_set_src(led_ep_shelly, shelly_ok ? &Shelly32 : &Shelly32_gris);
  lv_img_set_src(led_ep_enphase, enphase_connected ? &Enphase_logo : &Enphase_logo_gris);
}

#endif
