/**
 * @file ui_manager.c
 * @brief The Controller. Takes data from the Backend and renders it on the EEZ View.
 */

#include "ui_manager.h"
#include "../backend/data_model.h"
#include "../hardware/hardware_api.h" 
#include "../hardware/waveshare_rgb_lcd_port.h"
#include "../ui/ui.h" // EEZ Studio main header
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include "screens.h"
#include "styles.h"

#include <time.h>             // For the live clock
#include "../backend/system_config.h" // For the company name

#include "lvgl.h"
#include "extra/libs/qrcode/lv_qrcode.h"  // <-- ADD THIS LINE

#include "ui_widgets.h"

static const char *UI_TAG = "UI_Manager";

// Forward declarations
void transition_to_mode_select(void);
void transition_to_numpad(DispenseMode mode);
void transition_to_confirm(void);



// Declare a global pointer so we can delete the old QR code if they generate a new one
static lv_obj_t * active_qrcode = NULL;



// 2. The Dynamic Generator
static void generate_dynamic_panels(void) 
{
    // EEZ Studio stores named widgets in the 'objects' struct.
    // Ensure you named your container 'main_container' in the EEZ editor.
    if (objects.main_container == NULL) {
        ESP_LOGE(UI_TAG, "Error: main_container not found!");
        return;
    }

    lv_obj_clean(objects.main_container);

    int count = get_current_nozzle_count();
    for(int i = 0; i < count; i++) {
        NozzleData* data = get_nozzle(i);
        if(data != NULL) {
            create_nozzle_widget(objects.main_container, data);
        }
    }
}



// NEW FUNCTION: Prepares and loads the Mode Select Screen
void transition_to_mode_select(void) 
{
    // 1. Get the data the user just selected from the Backend
    NozzleData * active_data = get_active_transaction_nozzle();
    
    if (active_data == NULL || objects.mode_select_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load screen: Data or Screen object is missing.");
        return;
    }

    // 2. Inject the data into the EEZ Studio labels we just created
    if (objects.lbl_active_product) {
        lv_label_set_text(objects.lbl_active_product, active_data->product_name);
    }

    if (objects.active_product_panel) {
        lv_obj_set_style_bg_color(objects.active_product_panel, lv_color_hex(active_data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(objects.active_product_panel, lv_color_lighten(lv_color_hex(get_shine(active_data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    if (objects.back_btn_label) {
        lv_obj_clear_flag(objects.back_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }
   
    if (objects.by_amount_btn_label) {
        lv_obj_clear_flag(objects.by_amount_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }
    
    if (objects.by_volume_btn_label) {
        lv_obj_clear_flag(objects.by_volume_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }
    
    if (objects.lbl_active_nozzle) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->id);
        lv_label_set_text(objects.lbl_active_nozzle, buf);
    }

    // 3. Perform the actual screen switch!
    lv_scr_load(objects.mode_select_screen);
}


// NEW FUNCTION: Prepares and loads the Numpad Screen
void transition_to_numpad(DispenseMode mode) 
{
    // 1. Get the ACTIVE data from the Backend!
    NozzleData * active_data = get_active_transaction_nozzle();
    
    if (active_data == NULL || objects.numpad_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load Numpad: Data or Screen is missing.");
        return;
    }

    // 2. Inject all the backend data into the EEZ Labels
    if (objects.lbl_active_product_numpad) {
        lv_label_set_text(objects.lbl_active_product_numpad, active_data->product_name);
    }

    if (objects.active_product_panel_numpad) {
        lv_obj_set_style_bg_color(objects.active_product_panel_numpad, lv_color_hex(active_data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(objects.active_product_panel_numpad, lv_color_lighten(lv_color_hex(get_shine(active_data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    if (objects.lbl_active_nozzle_numpad) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->id);
        lv_label_set_text(objects.lbl_active_nozzle_numpad, buf);
    }
    if (objects.lbl_active_rate) {
        char buf[32];
        snprintf(buf, sizeof(buf), "RATE:  %.2f", active_data->rate);
        lv_label_set_text(objects.lbl_active_rate, buf);
    }
    if (objects.lbl_active_density) {
        char buf[32];
        snprintf(buf, sizeof(buf), "DENSITY:  %.3f", active_data->density);
        lv_label_set_text(objects.lbl_active_density, buf);
    }

    // 3. Set up the specific Mode parameters (Rs. vs Liters)
    if (mode == MODE_AMOUNT && objects.lbl_unit) {
        lv_label_set_text(objects.lbl_unit, "Rs.");
    } else if (mode == MODE_VOLUME && objects.lbl_unit) {
        lv_label_set_text(objects.lbl_unit, "Liters");
    }

    // 4. THE BUG FIX: Explicitly tell the keyboard to type into the text area!
    if (objects.numpad_screen_keyboard && objects.ta_input) {
        lv_textarea_set_text(objects.ta_input, ""); // Clear old text
        lv_keyboard_set_textarea(objects.numpad_screen_keyboard, objects.ta_input); // Link them!
    }


    if (objects.numpad_back_btn_label) {
        lv_obj_clear_flag(objects.numpad_back_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }

    if (objects.numpad_confirm_btn_label) {
        lv_obj_clear_flag(objects.numpad_confirm_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }

    // 5. Perform the actual screen switch!
    lv_scr_load(objects.numpad_screen);
}


// NEW FUNCTION: Calculates totals and loads the Confirmation Screen
void transition_to_confirm(void) 
{
    // 1. Fetch all current state data from the Backend
    NozzleData * active_data = get_active_transaction_nozzle();
    DispenseMode mode = get_transaction_mode();
    float entered_value = get_transaction_value();
    
    if (active_data == NULL || objects.confirm_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load Confirm Screen: Data missing.");
        return;
    }

    // 2. Perform the automated calculations
    float final_amount = 0.0f;
    float final_volume = 0.0f;

    if (mode == MODE_AMOUNT) {
        final_amount = entered_value;
        final_volume = entered_value / active_data->rate; // Volume = Amount / Rate
    } 
    else if (mode == MODE_VOLUME) {
        final_volume = entered_value;
        final_amount = entered_value * active_data->rate; // Amount = Volume * Rate
    }

        // 2. Inject all the backend data into the EEZ Labels
    if (objects.confirm_screen_lbl_active_product) {
        lv_label_set_text(objects.confirm_screen_lbl_active_product, active_data->product_name);
    }

    if (objects.confirm_screen_active_product_panel) {
        lv_obj_set_style_bg_color(objects.confirm_screen_active_product_panel, lv_color_hex(active_data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(objects.confirm_screen_active_product_panel, lv_color_lighten(lv_color_hex(get_shine(active_data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (objects.confirm_screen_lbl_active_nozzle) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->id);
        lv_label_set_text(objects.confirm_screen_lbl_active_nozzle, buf);
    }
    if (objects.confirm_screen_lbl_active_rate) {
        char buf[32];
        snprintf(buf, sizeof(buf), "RATE:  %.2f", active_data->rate);
        lv_label_set_text(objects.confirm_screen_lbl_active_rate, buf);
    }
    if (objects.confirm_screen_lbl_active_density) {
        char buf[32];
        snprintf(buf, sizeof(buf), "DENSITY:  %.3f", active_data->density);
        lv_label_set_text(objects.confirm_screen_lbl_active_density, buf);
    }
    // Final Amount
    if (objects.lbl_conf_amount) {
        char buf_amt[32];
        snprintf(buf_amt, sizeof(buf_amt), " %.2f Rs.", final_amount);
        lv_label_set_text(objects.lbl_conf_amount, buf_amt);
    }
    
    // Final Volume
    if (objects.lbl_conf_volume) {
        char buf_vol[32];
        snprintf(buf_vol, sizeof(buf_vol), "%.3f Liters", final_volume);
        lv_label_set_text(objects.lbl_conf_volume, buf_vol);
    }
    

    if (objects.confirm_screen_back_btn_label) {
        lv_obj_clear_flag(objects.confirm_screen_back_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }

    if (objects.confirm_screen_confirm_btn_label) {
        lv_obj_clear_flag(objects.confirm_screen_confirm_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }
    // 4. Switch the screen!
    lv_scr_load(objects.confirm_screen);
}



void transition_to_qr_screen(void) 
{
    if (objects.qr_screen == NULL || objects.qr_container == NULL) return;

    // 1. Get the Data
    NozzleData * active_data = get_active_transaction_nozzle();
    SystemConfig * sys_data = get_system_config();
    DispenseMode mode = get_transaction_mode();
    float entered_value = get_transaction_value();
    
    // 2. Calculate Final Amount
    float final_amount = 0.0f;
    if (mode == MODE_AMOUNT) {
        final_amount = entered_value;
    } else if (mode == MODE_VOLUME) {
        final_amount = entered_value * active_data->rate;
    }

    // 3. Format the standard UPI Payment String
    // Format: upi://pay?pa=[UPI_ID]&pn=[NAME]&am=[AMOUNT]&cu=INR
    char upi_string[256];
    snprintf(upi_string, sizeof(upi_string), "upi://pay?pa=%s&pn=%s&am=%.2f&cu=INR", 
             sys_data->upi_id, sys_data->company_name, final_amount);
             
    ESP_LOGI("PAYMENT", "Generated UPI String: %s", upi_string);

    // 4. Clean up any old QR code before making a new one
    if (active_qrcode != NULL) {
        lv_obj_del(active_qrcode);
        active_qrcode = NULL;
    }

    // 5. Generate the LVGL QR Code inside your EEZ Studio container
    // Syntax: lv_qrcode_create(parent, size, dark_color, light_color)
    active_qrcode = lv_qrcode_create(objects.qr_container, 200, lv_color_hex(0x000000), lv_color_hex(0xffffff));
    lv_qrcode_update(active_qrcode, upi_string, strlen(upi_string));

        // Final Amount
    if (objects.qr_label) {
        char buf_amt[32];
        snprintf(buf_amt, sizeof(buf_amt), " %.2f Rs.", final_amount);
        lv_label_set_text(objects.qr_label, buf_amt);
    }

    if (objects.qr_screen_back_btn_label) {
        lv_obj_clear_flag(objects.payment_screen_back_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }

    if (objects.qr_screen_completed_btn_label) {
        lv_obj_clear_flag(objects.payment_screen_confirm_btn_label, LV_OBJ_FLAG_CLICKABLE);
    }
    

    // Center it in the container
    lv_obj_center(active_qrcode);

    // 6. Switch the screen!
    lv_scr_load(objects.qr_screen);
}




// 3. The Thread-Safe Startup Task
static void startup_ui_task(void *pvParameter) 
{
    ESP_LOGI(UI_TAG, "Locking Engine and Building EEZ UI...");
    if (lvgl_port_lock(-1)) {
        
        ui_init(); // Boot the EEZ Studio generated code

        init_system_header();

        generate_dynamic_panels(); // Inject our dynamic nozzle structs        
        lvgl_port_unlock(); 
    }
    ESP_LOGI(UI_TAG, "EEZ UI Build Complete. Freeing startup memory.");
    vTaskDelete(NULL); 
}
void start_ui_manager(void) 
{
    // Huge 8192 stack size prevents string formatting crashes
    xTaskCreate(startup_ui_task, "Startup_Task", 8192, NULL, 5, NULL);
}