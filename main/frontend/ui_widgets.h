#pragma once

#include "../backend/data_model.h"
#include "../ui/ui.h" // EEZ Studio main header



unsigned int get_shine(unsigned int color, float factor);



void live_clock_timer_cb(lv_timer_t * timer);
void init_system_header(void) ;

void create_loading_overlay(void);


void on_dispenser_select_clicked(lv_event_t * e);
void create_dispenser_widget(lv_obj_t * parent_obj, DispenserNode* data);


void on_display_select_clicked(lv_event_t * e);
void create_display_widget(lv_obj_t * parent_obj, DisplayNode* data);

void on_nozzle_select_clicked(lv_event_t * e) ;
void create_nozzle_widget(lv_obj_t * parent_obj, NozzleNode * data);


