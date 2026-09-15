#pragma once

#include "../backend/data_model.h"


// 1. Define the possible dynamic screens
typedef enum {
    SCREEN_BOOTING,
    SCREEN_DISPENSER_SELECT,
    SCREEN_DISPLAY_SELECT,
    SCREEN_NOZZLE_SELECT,
    SCREEN_STATIC_FORM, // Use this for Numpad, QR, etc., where dynamic widgets aren't drawn
    SCREEN_LOGIN,
    SCREEN_CONFIG, 
    SCREEN_LIVE_COUNTING
} ActiveScreenState;


void start_ui_manager(void);


// At the top of main/comms/json_parser.c
void transition_to_dispenser_select_screen(void);
// Add a way to read the screen state, or just expose a check function
// bool is_ui_booting(void); 


void transition_to_display_select_screen(); 
void transition_to_nozzle_select_screen(); 


void transition_to_login_screen(); 
void transition_to_config_screen(void);


void transition_to_mode_select(void);
void transition_to_numpad(DispenseMode mode);
void transition_to_confirm(void);
void transition_to_qr_screen(void);
void transition_to_nozzle_pikup(void);
void transition_to_thank_you_screen(void);

void refresh_dynamic_panels(void);
void transition_to_live_counting(void);


// void show_loading_overlay(const char *message);
// void hide_loading_overlay(void);
bool is_ui_booting(void);


void nozzle_monitor_task(void *pvParameters);

void refresh_live_counting_screen(void *pvParameters);
