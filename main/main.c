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
    
    // Boot network and background JSON polling
    comms_init("Fuel_Dispenser_Network", "123456789");
    comms_start_polling();
    
    // Initialize the thread-safe data model
    station_model_init();
    
    
    // Boot the UI (it will load empty initially)
    start_ui_manager();
    
    // if (lvgl_port_lock(-1)) {
    //     create_loading_overlay();
    //     lvgl_port_unlock();
    // }
    

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}