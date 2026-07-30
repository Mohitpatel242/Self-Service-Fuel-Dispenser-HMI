// #pragma once
// #include <stdbool.h>

// void comms_init(const char* ssid, const char* password);
// void comms_start_polling(void);

#pragma once
void comms_init(const char* ssid, const char* password);
void comms_start_polling(void);

void comms_send_pump_control(void);