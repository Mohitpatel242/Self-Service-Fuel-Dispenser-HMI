#pragma once

#include "../backend/data_model.h"


void start_ui_manager(void);

void transition_to_mode_select(void);
void transition_to_numpad(DispenseMode mode);
void transition_to_confirm(void);
void transition_to_qr_screen(void);
