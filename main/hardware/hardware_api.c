#include "hardware_api.h"
#include "waveshare_rgb_lcd_port.h"
#include "esp_log.h"

static const char *HW_TAG = "Hardware_API"; 

void hardware_init_all(void) 
{
    ESP_LOGI(HW_TAG, "Initializing RGB Display, Touch, and LVGL Base...");
    waveshare_esp32_s3_rgb_lcd_init();

    ESP_LOGI(HW_TAG, "Turning on Display Backlight...");
    wavesahre_rgb_lcd_bl_on(); 
}