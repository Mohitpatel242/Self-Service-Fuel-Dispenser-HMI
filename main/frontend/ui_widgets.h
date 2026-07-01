#pragma once

#include "../backend/data_model.h"
#include "../ui/ui.h" // EEZ Studio main header



unsigned int get_shine(unsigned int color, float factor);
void on_nozzle_select_clicked(lv_event_t * e) ;
void live_clock_timer_cb(lv_timer_t * timer);


void create_nozzle_widget(lv_obj_t * parent_obj, NozzleData * data);
void init_system_header(void) ;


