#pragma once
#include <stdbool.h>

bool parse_live_data(const char* raw_json);
void send_pump_control(void);
