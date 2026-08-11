#include "comms_api.h"
#include "wifi_http_client.h"

void comms_init(const char* ssid, const char* password) 
{
    wifi_network_init(ssid, password);
}

void comms_start_polling(void) 
{
    start_live_data_monitor();
}
