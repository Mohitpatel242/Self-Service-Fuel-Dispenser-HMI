// main.c
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware_api.h"
#include "data_model.h"
#include "ui_manager.h"
#include "ui_widgets.h"
#include "system_config.h"
#include "comms/comms_api.h"



void app_main(void)
{
    hardware_init_all();
    load_system_config();


    static StationRegistry registry;
    registry.dispenser_count = 1; // Initialize with one dispenser for testing

    comms_init(get_system_config()->ssid, get_system_config()->pass);

    comms_start_polling();
    
    // Initialize the thread-safe data model
    station_model_init();
    
    set_active_dispenser_context(1); // Set the initial active dispenser context

    // Boot the UI (it will load empty initially)
    start_ui_manager();


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}