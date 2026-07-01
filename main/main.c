#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "hardware_api.h"
#include "data_model.h"
#include "ui_manager.h"
#include "system_config.h"

void app_main(void)
{
    hardware_init_all();
    load_system_config();
    load_nozzle_data();
    start_ui_manager();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}