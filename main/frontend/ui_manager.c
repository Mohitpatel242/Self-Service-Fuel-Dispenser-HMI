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
#include "lvgl.h"
#include "extra/libs/qrcode/lv_qrcode.h"  
#include "wifi_http_client.h"
#include "../backend/system_config.h" 
#include "ui_widgets.h"

static const char *UI_TAG = "UI_Manager";

static ActiveScreenState current_screen_state = SCREEN_BOOTING;
static lv_obj_t *loading_overlay = NULL;


// Forward declarations
void transition_to_display_select_screen(); 
void transition_to_nozzle_select_screen(); 

void transition_to_mode_select(void);
void transition_to_numpad(DispenseMode mode);
void transition_to_confirm(void);


// 2. Perform the automated calculations
static float final_amount = 0.0f;
static float final_volume = 0.0f;



// Declare a global pointer so we can delete the old QR code if they generate a new one
static lv_obj_t * active_qrcode = NULL;


// 2. The Dynamic Generator
void generate_dynamic_dispenser_panels(void) 
{
    // EEZ Studio stores named widgets in the 'objects' struct.
    // Ensure you named your container 'main_container' in the EEZ editor.
    if (objects.dispenser_main_con == NULL) {
        ESP_LOGE(UI_TAG, "Error: dispenser_main_con not found!");
        return;
    }

    lv_obj_clean(objects.dispenser_main_con);

    int count = get_current_dispenser_count();
    for(int i = 0; i < count; i++) {
        DispenserNode* data = get_dispenser(i);
        if(data != NULL) {
            create_dispenser_widget(objects.dispenser_main_con, data);
        }
    }
}


// 2. The Dynamic Generator
static void generate_dynamic_display_panels(void) 
{
    // EEZ Studio stores named widgets in the 'objects' struct.
    // Ensure you named your container 'main_container' in the EEZ editor.
    if (objects.display_main_con == NULL) {
        ESP_LOGE(UI_TAG, "Error: display_main_con not found!");
        return;
    }

    lv_obj_clean(objects.display_main_con);

    int count = get_current_display_count();
    ESP_LOGW(UI_TAG, "Display found = %d", count);
    
    for(int i = 0; i < count; i++) {
        DisplayNode* data = get_display(i);
        if(data != NULL) {
            create_display_widget(objects.display_main_con, data);
        }
    }
}


// 2. The Dynamic Generator
static void generate_dynamic_nozzle_panels(void) 
{
    // EEZ Studio stores named widgets in the 'objects' struct.
    if (objects.nozzle_main_con == NULL) {
        ESP_LOGE(UI_TAG, "Error: main_container not found!");
        return;
    }

    lv_obj_clean(objects.nozzle_main_con);

    int count = get_current_nozzle_count();
    for(int i = 0; i < count; i++) {
        NozzleNode* data = get_nozzle(i);
        if(data != NULL) {
            create_nozzle_widget(objects.nozzle_main_con, data);
        }
    }
}


extern lv_obj_t * loading_overlay; // Reference the global defined in widgets

void refresh_dynamic_panels(void)
{
    // Always lock LVGL when called from an external background task (JSON Parser)
    if (lvgl_port_lock(-1)) {
        
        // Act like a smart router based on the user's current view
        switch (current_screen_state) {
            
            case SCREEN_DISPENSER_SELECT:
                if (objects.dispenser_main_con != NULL) {
                    ESP_LOGI(UI_TAG, "Live update: Redrawing Dispenser Select Screen");
                    generate_dynamic_dispenser_panels();
                    
                    // // 1. If the overlay exists, delete it immediately upon data arrival
                    // if (loading_overlay != NULL) {
                    //     lv_obj_del(loading_overlay);
                    //     loading_overlay = NULL;
                    //     ESP_LOGI("UI", "Data received. Removing loading overlay.");
                    // }
                }
                break;
                
            case SCREEN_DISPLAY_SELECT:
                if (objects.display_main_con != NULL) {
                    ESP_LOGI(UI_TAG, "Live update: Redrawing Display Select Screen");
                    generate_dynamic_display_panels();
                }
                break;
                
            case SCREEN_NOZZLE_SELECT:
                if (objects.nozzle_main_con != NULL) {
                    ESP_LOGI(UI_TAG, "Live update: Redrawing Nozzle Select Screen");
                    generate_dynamic_nozzle_panels();
                }
                break;
                
            case SCREEN_STATIC_FORM:
            case SCREEN_BOOTING:

            case SCREEN_LIVE_COUNTING:
                if (objects.live_counting_screen != NULL) {
                    ESP_LOGI(UI_TAG, "Live update: Redrawing Live Counting Screen");
                }

            default:
                // Do nothing. The user is on a static screen (like a payment QR code)
                // We should not interrupt them or waste CPU redrawing hidden dynamic widgets.
                break;
        }
        
        lvgl_port_unlock();
    }
}



bool is_ui_booting(void) {
    return (current_screen_state == SCREEN_BOOTING);
}


void transition_to_dispenser_select_screen(void)
{
    if (lvgl_port_lock(-1)) {
        // Change state so refresh_dynamic_panels knows to handle background updates
        current_screen_state = SCREEN_DISPENSER_SELECT;
        
        
        // Generate widgets using the freshly arrived JSON data
        generate_dynamic_dispenser_panels();
        
        // Load the actual main screen
        lv_scr_load(objects.main_dispenser_select_screen);
        
        lvgl_port_unlock();
    }
}



void transition_to_display_select_screen(void)
{
    if (lvgl_port_lock(-1)) {
        current_screen_state = SCREEN_DISPLAY_SELECT;
        
        generate_dynamic_display_panels();
        
        lv_scr_load(objects.display_select_screen); 
        
        lvgl_port_unlock();
    }
}




void transition_to_nozzle_select_screen(void)
{
    if (lvgl_port_lock(-1)) {
        current_screen_state = SCREEN_NOZZLE_SELECT;
        
        generate_dynamic_nozzle_panels();
        lv_scr_load(objects.nozzle_select_screen); 
        

        lvgl_port_unlock();
    }
}


void show_loading_overlay(const char *message)
{
    if (lvgl_port_lock(-1)) {
        // Prevent creating duplicate overlays
        if (loading_overlay != NULL) {
            lvgl_port_unlock();
            return;
        }

        // 1. Create a full-screen background container over the active screen
        loading_overlay = lv_obj_create(lv_scr_act());
        lv_obj_set_size(loading_overlay, LV_PCT(100), LV_PCT(100));
        lv_obj_center(loading_overlay);
        
        // 2. Style the background (Solid color or semi-transparent)
        lv_obj_set_style_bg_color(loading_overlay, lv_palette_main(LV_PALETTE_GREY), 0);
        lv_obj_set_style_bg_opa(loading_overlay, LV_OPA_COVER, 0); // Use LV_OPA_70 for semi-transparent
        lv_obj_set_style_border_width(loading_overlay, 0, 0);
        lv_obj_set_style_radius(loading_overlay, 0, 0);

        // 3. Add a loading text label
        lv_obj_t *label = lv_label_create(loading_overlay);
        lv_label_set_text(label, message ? message : "Loading Data...");
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0); // Change to your project's font
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);

        // 4. Optional: Add a spinning loader wheel if your LVGL build has it enabled
        #if LV_USE_SPINNER
        lv_obj_t *spinner = lv_spinner_create(loading_overlay, 1000, 60);
        lv_obj_set_size(spinner, 50, 50);
        lv_obj_align(spinner, LV_ALIGN_CENTER, 0, -30);
        #endif

        lvgl_port_unlock();
    }
}

void hide_loading_overlay(void)
{
    if (lvgl_port_lock(-1)) {
        if (loading_overlay != NULL) {
            lv_obj_del(loading_overlay);
            loading_overlay = NULL; // Reset pointer safely
        }
        lvgl_port_unlock();
    }
}


// NEW FUNCTION: Prepares and loads the Mode Select Screen
void transition_to_mode_select(void) 
{
    if (lvgl_port_lock(-1)) {
        current_screen_state = SCREEN_STATIC_FORM; // Stop dynamic widget generation here

    
    // 1. Get the data the user just selected from the Backend
    NozzleNode* active_data = get_active_nozzle();
    
    if (active_data == NULL || objects.mode_select_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load screen: Data or Screen object is missing.");
        return;
    }

    // 2. Inject the data into the EEZ Studio labels we just created
    if (objects.lbl_active_product) {
        lv_label_set_text(objects.lbl_active_product, active_data->fuel_type);
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
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->nozzle_id);
        lv_label_set_text(objects.lbl_active_nozzle, buf);
    }

    // 3. Perform the actual screen switch!
    lv_scr_load(objects.mode_select_screen);

    lvgl_port_unlock();
    }
}


// NEW FUNCTION: Prepares and loads the Numpad Screen
void transition_to_numpad(DispenseMode mode) 
{
    // 1. Get the ACTIVE data from the Backend!
    NozzleNode * active_data = get_active_nozzle();
    
    if (active_data == NULL || objects.numpad_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load Numpad: Data or Screen is missing.");
        return;
    }

    // 2. Inject all the backend data into the EEZ Labels
    if (objects.lbl_active_product_numpad) {
        lv_label_set_text(objects.lbl_active_product_numpad, active_data->fuel_type);
    }

    if (objects.active_product_panel_numpad) {
        lv_obj_set_style_bg_color(objects.active_product_panel_numpad, lv_color_hex(active_data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(objects.active_product_panel_numpad, lv_color_lighten(lv_color_hex(get_shine(active_data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    
    if (objects.lbl_active_nozzle_numpad) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->nozzle_id);
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
    NozzleNode * active_data = get_active_nozzle();
    DispenseMode mode = get_transaction_mode();
    float entered_value = get_transaction_value();
    
    if (active_data == NULL || objects.confirm_screen == NULL) {
        ESP_LOGE(UI_TAG, "Cannot load Confirm Screen: Data missing.");
        return;
    }

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
        lv_label_set_text(objects.confirm_screen_lbl_active_product, active_data->fuel_type);
    }

    if (objects.confirm_screen_active_product_panel) {
        lv_obj_set_style_bg_color(objects.confirm_screen_active_product_panel, lv_color_hex(active_data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(objects.confirm_screen_active_product_panel, lv_color_lighten(lv_color_hex(get_shine(active_data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    if (objects.confirm_screen_lbl_active_nozzle) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->nozzle_id);
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
    NozzleNode * active_data = get_active_nozzle();
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
        char buf_amt[16];
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



void transition_to_nozzle_pikup(void){

    if (objects.nozzle_pickup_screen == NULL){
        ESP_LOGE(UI_TAG, "Cannot load screen: nozzle pickup screen is missing.");
        return;
    } 

        // 1. Get the data the user just selected from the Backend
    NozzleNode * active_data = get_active_nozzle();
    
    
    if (objects.nozzle_pikup_active_nozzle_label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "NOZZLE %d", active_data->nozzle_id);
        lv_label_set_text(objects.nozzle_pikup_active_nozzle_label, buf);
    }

    // 3. Perform the actual screen switch!
    lv_scr_load(objects.nozzle_pickup_screen);

    xTaskCreate(
        nozzle_monitor_task,    // Task function
        "nozzle_monitor",       // Name of task
        4096,                   // Stack size in bytes
        NULL,                   // Task input parameter
        5,                      // Priority (Adjust based on your project)
        NULL                    // Task handle
    );
}




// 1. Define the background task function
void nozzle_monitor_task(void *pvParameters) {
    while (1) {

        start_live_data_monitor(); // Start monitoring live data in the background

        if (is_nozzle_picked_up()) {
            ESP_LOGW("UI_TAG", "Nozzle picked up. Transitioning screen.");
            
            // It is safest to update UI elements on the main GUI thread
            transition_to_live_counting(); 
            
            // Self-delete the task to free up memory
            vTaskDelete(NULL); 
        }
        // Yields control back to the CPU so other tasks can run
        ESP_LOGW("UI_TAG", "Monitoring Nozzle pickup status.");

        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}



void transition_to_live_counting(void){
    
    if (objects.live_counting_screen == NULL){
        ESP_LOGE(UI_TAG, "Cannot load screen: live counting screen is missing.");
        return;
    } 

    
    // NozzleNode * active_data = get_active_nozzle();
    
    // Final Amount
    if (objects.live_counting_s_t_amount_lbl) {
        char buf_amt[32];
        snprintf(buf_amt, sizeof(buf_amt), " %.2f Rs.", final_amount);
        lv_label_set_text(objects.live_counting_s_t_amount_lbl, buf_amt);
    }
    
    // Final Volume
    if (objects.live_counting_s_t_volume_lbl) {
        char buf_vol[32];
        snprintf(buf_vol, sizeof(buf_vol), "%.3f Liters", final_volume);
        lv_label_set_text(objects.live_counting_s_t_volume_lbl, buf_vol);
    }
    
    xTaskCreate(
        refresh_live_counting_screen,    // Task function
        "live_counting_monitor",       // Name of task
        4096,                   // Stack size in bytes
        NULL,                   // Task input parameter
        5,                      // Priority (Adjust based on your project)
        NULL                    // Task handle
    );

    // 3. Perform the actual screen switch!
    lv_scr_load(objects.live_counting_screen);
    // 
    current_screen_state = SCREEN_LIVE_COUNTING;
}



// 1. Define the background task function
void refresh_live_counting_screen(void *pvParameters) {
    while (1) {
        if (is_nozzle_picked_up()) {
            ESP_LOGW("UI_TAG", "Running Live Counting Screen Refresh Task.");
            
            start_live_data_monitor(); // Start monitoring live data in the background            
            // Self-delete the task to free up memory

            DisplayNode * active_display = get_active_display();
            // Final Amount
            if (objects.live_counting_screen_amount_label) {
                char buf_amt[32];
                snprintf(buf_amt, sizeof(buf_amt), "%s", active_display->running_transaction_amt);
                lv_label_set_text(objects.live_counting_screen_amount_label, buf_amt);
            }
            // Final Volume
            if (objects.live_counting_screen_volume_label) {
                char buf_vol[32];
                snprintf(buf_vol, sizeof(buf_vol), "%s", active_display->running_transaction_qty);
                lv_label_set_text(objects.live_counting_screen_volume_label, buf_vol);
            }

        }
        else {
            
            ESP_LOGW("UI_TAG", "Nozzle is down. Stopping Live Counting Screen Refresh Task.");
            vTaskDelete(NULL); 
        }
        // Yields control back to the CPU so other tasks can run
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}




void transition_to_login_screen(){

        if (lvgl_port_lock(-1)) {
            
        lv_scr_load(objects.login_screen);
                
        current_screen_state = SCREEN_LOGIN;
        
        // Clear out any old text from previous login attempts
        lv_textarea_set_text(objects.pass_input_text_area, ""); 
            
        lvgl_port_unlock();
        
    }
    else{

        ESP_LOGE(UI_TAG, "ERROR while loading login screen");
        
    }


}



void transition_to_config_screen(void)
{
    if (lvgl_port_lock(-1)) {
        current_screen_state = SCREEN_CONFIG;
        lv_scr_load(objects.config_screen);
        lvgl_port_unlock();
    }
}



// 3. The Thread-Safe Startup Task
static void startup_ui_task(void *pvParameter) 
{
    ESP_LOGI(UI_TAG, "Locking Engine and Building EEZ UI...");
    if (lvgl_port_lock(-1)) {
        
        current_screen_state = SCREEN_BOOTING;
        
        ui_init(); // Boot the EEZ Studio generated code

        init_system_header();

        transition_to_display_select_screen();

        // transition_to_dispenser_select_screen();

        // // Load your direct main screen template safely
        // lv_scr_load(objects.main_dispenser_select_screen);
    
        // // generate_dynamic_nozzle_panels(); // Inject our dynamic nozzle structs  
        // generate_dynamic_dispenser_panels(); 
        


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