#include "ui_manager.h"
#include "../backend/data_model.h"
#include "../hardware/hardware_api.h" 

#include <stdio.h>

#include <time.h>             // For the live clock
#include "../backend/system_config.h" 

#include "lvgl.h"
#include "extra/libs/qrcode/lv_qrcode.h"  
#include "ui_widgets.h"
#include "esp_log.h"
#include "actions.h"
#include "images.h"


static const char *TAG = "UI_WIDGETS";


// --- Global Pointers for the Header ---
static lv_obj_t * sys_header_panel;
static lv_obj_t * date_label;
static lv_obj_t * time_label;


//Make label color brighter by a factor (0.0 to 1.0)
unsigned int get_shine(unsigned int color, float factor) {
    unsigned int r = (color >> 16) & 0xFF;
    unsigned int g = (color >> 8)  & 0xFF;
    unsigned int b = color         & 0xFF;
    
    r += (255 - r) * factor;
    g += (255 - g) * factor;
    b += (255 - b) * factor;

    return (r << 16) | (g << 8) | b;
}


//=====================================================================================================================================================

// void on_dispenser_select_clicked(lv_event_t * e) 
// {
//     DispenserNode * selected_data = (DispenserNode *)lv_event_get_user_data(e);
    
//     // Save state to backend
//     set_active_dispenser_context(selected_data->dispenser_id);
    
    
//     ESP_LOGW(TAG, "selected dispenser id = %d", selected_data->dispenser_id );

//     // Prepare and load the new screen
//     transition_to_display_select_screen(); 
// }



// void on_display_select_clicked(lv_event_t * e){

//     DisplayNode * selected_data = (DisplayNode *)lv_event_get_user_data(e);
    
//     // Save state to backend
//     set_active_display_context(selected_data->display_id);
    
//     // Prepare and load the new screen
//     transition_to_nozzle_select_screen(); 
// }

// void on_nozzle_select_clicked(lv_event_t * e) 
// {
//     NozzleNode * selected_data = (NozzleNode *)lv_event_get_user_data(e);
    
//     // Save state to backend
//     set_active_transaction_nozzle(selected_data->nozzle_id);

//     // Prepare and load the new screen
//     transition_to_mode_select(); 
// }



void on_dispenser_select_clicked(lv_event_t * e) 
{

    // 1. Retrieve the node pointer
    DispenserNode * clicked_du = (DispenserNode *)lv_event_get_user_data(e);
    
    // 2. Ask the backend to safely resolve the exact index
    int safe_index = get_dispenser_index_from_ptr(clicked_du);
    
    if (safe_index != -1) {
        set_active_dispenser_index(safe_index);
        transition_to_display_select_screen();
    }

    // // 1. Recover the exact array index
    // int selected_idx = (int)(intptr_t)lv_event_get_user_data(e);
    
    // ESP_LOGW(TAG, "selected_idx = %d", selected_idx );

    // // 2. Set the index context
    // set_active_dispenser_index(selected_idx);
    
    // // 3. Move to the next hierarchy level
    // transition_to_display_select_screen();

}


void on_display_select_clicked(lv_event_t * e) 
{

    DisplayNode * clicked_disp = (DisplayNode *)lv_event_get_user_data(e);
    
    int safe_index = get_display_index_from_ptr(clicked_disp);
    
    if (safe_index != -1) {
        set_active_display_index(safe_index);
        transition_to_nozzle_select_screen();
    }

    // int selected_idx = (int)(intptr_t)lv_event_get_user_data(e);
    // set_active_display_index(selected_idx);
    // transition_to_nozzle_select_screen();
}


void on_nozzle_select_clicked(lv_event_t * e) 
{
    NozzleNode * clicked_noz = (NozzleNode *)lv_event_get_user_data(e);
    
    int safe_index = get_nozzle_index_from_ptr(clicked_noz);
    
    if (safe_index != -1) {
        set_active_nozzle_index(safe_index);
        transition_to_mode_select(); 
    }
    
    // int selected_idx = (int)(intptr_t)lv_event_get_user_data(e);
    // set_active_nozzle_index(selected_idx);
    // transition_to_mode_select(); 
}


//=====================================================================================================================================================


void live_clock_timer_cb(lv_timer_t * timer)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    // Format Date: DD/MM/YYYY
    char date_str[16];
    strftime(date_str, sizeof(date_str), "%d/%m/%Y", &timeinfo);
    
    // Format Time: HH:MM:SS
    char time_str[16];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
    
    // Update the EEZ labels
    if (date_label != NULL) lv_label_set_text(date_label, date_str);
    if (time_label != NULL) lv_label_set_text(time_label, time_str);
}


//===============================================================================================================================================================================================================================================



void create_dispenser_widget(lv_obj_t * parent_obj, DispenserNode* data)
{
    // -------------------------------------------------------------------------
    // 1. MAIN DISPENSER CONTAINER
    // -------------------------------------------------------------------------

    {
        // dispenser_widget_con
        lv_obj_t *dispenser_widget_con = lv_obj_create(parent_obj);
        // objects.display_widget_con = obj;
        lv_obj_set_pos(dispenser_widget_con, LV_PCT(0), LV_PCT(0));
        lv_obj_set_size(dispenser_widget_con, LV_PCT(24), LV_PCT(90));
        lv_obj_set_style_pad_left(dispenser_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(dispenser_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(dispenser_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(dispenser_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(dispenser_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(dispenser_widget_con, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_set_scrollbar_mode(dispenser_widget_con, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(dispenser_widget_con, lv_color_hex(0x555878), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(dispenser_widget_con, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(dispenser_widget_con, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(dispenser_widget_con, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(dispenser_widget_con, lv_color_hex(0x232323), LV_PART_MAIN | LV_STATE_DEFAULT);
        {
            {
                // dispenser_heading_panel
                lv_obj_t *dispenser_heading_panel = lv_obj_create(dispenser_widget_con);
                // objects.dispenser_heading_panel = obj;
                lv_obj_set_pos(dispenser_heading_panel, 19, 20);
                lv_obj_set_size(dispenser_heading_panel, LV_PCT(80), LV_PCT(15));
                lv_obj_clear_flag(dispenser_heading_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                lv_obj_set_scrollbar_mode(dispenser_heading_panel, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_layout(dispenser_heading_panel, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(dispenser_heading_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(dispenser_heading_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_row_dsc_array(dispenser_heading_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_column_dsc_array(dispenser_heading_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                lv_obj_set_style_bg_color(dispenser_heading_panel, lv_color_hex(theme_colors[active_theme_index][5]), LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // dispenser_heading_label
                        lv_obj_t *dispenser_heading_label = lv_label_create(dispenser_heading_panel);
                        // objects.heading_label = obj;
                        lv_obj_set_pos(dispenser_heading_label, 19, 23);
                        lv_obj_set_size(dispenser_heading_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(dispenser_heading_label, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(dispenser_heading_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_heading_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(dispenser_heading_label, "DISPENSER");
                    }
                }
            }
            {
                // info_con_1
                lv_obj_t *dispenser_info_con_1 = lv_obj_create(dispenser_widget_con);
                // objects.info_con_1 = obj;
                lv_obj_set_pos(dispenser_info_con_1, 10, 87);
                lv_obj_set_size(dispenser_info_con_1, LV_PCT(90), LV_PCT(15));
                lv_obj_set_style_pad_left(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(dispenser_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(dispenser_info_con_1, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(dispenser_info_con_1, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(dispenser_info_con_1, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(dispenser_info_con_1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(dispenser_info_con_1, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                
                {
                    {
                        // info_con_1_lable_1
                        lv_obj_t *dispenser_info_con_1_lable_1 = lv_label_create(dispenser_info_con_1);
                        // objects.info_con_1_lable_1 = obj;
                        lv_obj_set_pos(dispenser_info_con_1_lable_1, 0, 0);
                        lv_obj_set_size(dispenser_info_con_1_lable_1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(dispenser_info_con_1_lable_1, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(dispenser_info_con_1_lable_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_info_con_1_lable_1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(dispenser_info_con_1_lable_1, "DISPENSER ID");
                    }
                    {
                        // info_con_1_lable_2
                        lv_obj_t *dispenser_info_con_1_lable_2 = lv_label_create(dispenser_info_con_1);
                        // objects.info_con_1_lable_2 = obj;
                        lv_obj_set_pos(dispenser_info_con_1_lable_2, 53, 1);
                        lv_obj_set_size(dispenser_info_con_1_lable_2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(dispenser_info_con_1_lable_2, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(dispenser_info_con_1_lable_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_info_con_1_lable_2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(dispenser_info_con_1_lable_2, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(info_con_1_lable_2, "103.45");

                        char id_buf[16];
                        snprintf(id_buf, sizeof(id_buf), "%d", data->dispenser_id);
                        lv_label_set_text(dispenser_info_con_1_lable_2, id_buf);
                    }
                }
            }
            {
                // dispenser_info_con_2
                lv_obj_t *dispenser_info_con_2 = lv_obj_create(dispenser_widget_con);
                // objects.info_con_2 = obj;
                lv_obj_set_pos(dispenser_info_con_2, 9, 151);
                lv_obj_set_size(dispenser_info_con_2, LV_PCT(90), LV_PCT(15));
                lv_obj_set_style_pad_left(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(dispenser_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(dispenser_info_con_2, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(dispenser_info_con_2, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(dispenser_info_con_2, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(dispenser_info_con_2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(dispenser_info_con_2, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // dispenser_info_con_2_lable_1
                        lv_obj_t *dispenser_info_con_2_lable_1 = lv_label_create(dispenser_info_con_2);
                        // objects.dispenser_info_con_2_lable_1 = obj;
                        lv_obj_set_pos(dispenser_info_con_2_lable_1, 0, 0);
                        lv_obj_set_size(dispenser_info_con_2_lable_1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(dispenser_info_con_2_lable_1, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(dispenser_info_con_2_lable_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_info_con_2_lable_1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(dispenser_info_con_2_lable_1, "SERIAL NUMBER");
                    }
                    {
                        // dispenser_info_con_2_lable_2
                        lv_obj_t *dispenser_info_con_2_lable_2 = lv_label_create(dispenser_info_con_2);
                        // objects.dispenser_info_con_2_lable_2 = obj;
                        lv_obj_set_pos(dispenser_info_con_2_lable_2, 53, 1);
                        lv_obj_set_size(dispenser_info_con_2_lable_2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(dispenser_info_con_2_lable_2, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(dispenser_info_con_2_lable_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_info_con_2_lable_2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(dispenser_info_con_2_lable_2, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(dispenser_info_con_2_lable_2, "103.45");
                    
                        char id_buf[32];
                        snprintf(id_buf, sizeof(id_buf), "%s", data->serial_number);
                        lv_label_set_text(dispenser_info_con_2_lable_2, id_buf);
                    }
                }
            }
            {
                // dispenser_select_btn
                lv_obj_t *dispenser_select_btn = lv_btn_create(dispenser_widget_con);
                // objects.select_btn = obj;
                lv_obj_set_pos(dispenser_select_btn, 29, 230);
                lv_obj_set_size(dispenser_select_btn, LV_PCT(70), LV_PCT(15));
                lv_obj_set_style_bg_color(dispenser_select_btn, lv_color_hex(theme_colors[active_theme_index][3]), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(dispenser_select_btn, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_dir(dispenser_select_btn, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_color(dispenser_select_btn, lv_color_hex(theme_colors[active_theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
                
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // btn_label_1
                        lv_obj_t *dispenser_btn_label = lv_label_create(dispenser_select_btn);
                        // objects.btn_label_1 = obj;
                        lv_obj_set_pos(dispenser_btn_label, 0, 0);
                        lv_obj_set_size(dispenser_btn_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_align(dispenser_btn_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(dispenser_btn_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(dispenser_btn_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(dispenser_btn_label, "SELECT");
                        
                        lv_obj_add_event_cb(dispenser_select_btn, on_dispenser_select_clicked, LV_EVENT_CLICKED, (void*)data);
                    
                    }
                }
            }
        }
    }
}


//===============================================================================================================================================================================================================================================


void create_display_widget(lv_obj_t * parent_obj, DisplayNode* data)
{
    // -------------------------------------------------------------------------
    // 1. MAIN DISPENSER CONTAINER
    // -------------------------------------------------------------------------

    {
        // display_widget_con
        lv_obj_t *display_widget_con = lv_obj_create(parent_obj);
        // objects.display_widget_con = obj;
        lv_obj_set_pos(display_widget_con, LV_PCT(0), LV_PCT(0));
        lv_obj_set_size(display_widget_con, LV_PCT(24), LV_PCT(90));
        lv_obj_set_style_pad_left(display_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(display_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(display_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(display_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(display_widget_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(display_widget_con, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
        lv_obj_set_scrollbar_mode(display_widget_con, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(display_widget_con, lv_color_hex(0x555878), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(display_widget_con, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(display_widget_con, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(display_widget_con, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(display_widget_con, lv_color_hex(0x232323), LV_PART_MAIN | LV_STATE_DEFAULT);
        {
            {
                // heading_panel
                lv_obj_t *display_heading_panel = lv_obj_create(display_widget_con);
                // objects.display_heading_panel = obj;
                lv_obj_set_pos(display_heading_panel, 19, 20);
                lv_obj_set_size(display_heading_panel, LV_PCT(80), LV_PCT(15));
                lv_obj_clear_flag(display_heading_panel, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                lv_obj_set_scrollbar_mode(display_heading_panel, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_layout(display_heading_panel, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(display_heading_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(display_heading_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_row_dsc_array(display_heading_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_column_dsc_array(display_heading_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                lv_obj_set_style_bg_color(display_heading_panel, lv_color_hex(theme_colors[active_theme_index][5]), LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // display_heading_label
                        lv_obj_t *display_heading_label = lv_label_create(display_heading_panel);
                        // objects.heading_label = obj;
                        lv_obj_set_pos(display_heading_label, 19, 23);
                        lv_obj_set_size(display_heading_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(display_heading_label, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(display_heading_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_heading_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(display_heading_label, "DISPLAY");
                    }
                }
            }
            {
                // info_con_1
                lv_obj_t *display_info_con_1 = lv_obj_create(display_widget_con);
                // objects.info_con_1 = obj;
                lv_obj_set_pos(display_info_con_1, 10, 87);
                lv_obj_set_size(display_info_con_1, LV_PCT(90), LV_PCT(15));
                lv_obj_set_style_pad_left(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(display_info_con_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(display_info_con_1, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(display_info_con_1, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(display_info_con_1, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(display_info_con_1, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(display_info_con_1, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // display_info_con_1_lable_1
                        lv_obj_t *display_info_con_1_lable_1 = lv_label_create(display_info_con_1);
                        // objects.display_info_con_1_lable_1 = obj;
                        lv_obj_set_pos(display_info_con_1_lable_1, 0, 0);
                        lv_obj_set_size(display_info_con_1_lable_1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(display_info_con_1_lable_1, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(display_info_con_1_lable_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_info_con_1_lable_1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(display_info_con_1_lable_1, "DISPLAY ID");
                    }
                    {
                        // display_info_con_1_lable_2
                        lv_obj_t *display_info_con_1_lable_2 = lv_label_create(display_info_con_1);
                        // objects.display_info_con_1_lable_2 = obj;
                        lv_obj_set_pos(display_info_con_1_lable_2, 53, 1);
                        lv_obj_set_size(display_info_con_1_lable_2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(display_info_con_1_lable_2, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(display_info_con_1_lable_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_info_con_1_lable_2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(display_info_con_1_lable_2, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(display_info_con_1_lable_2, "103.45");

                        char id_buf[16];
                        snprintf(id_buf, sizeof(id_buf), "%d", data->display_id);
                        lv_label_set_text(display_info_con_1_lable_2, id_buf);
                    }
                }
            }
            {
                // display_info_con_2
                lv_obj_t *display_info_con_2 = lv_obj_create(display_widget_con);
                // objects.info_con_2 = obj;
                lv_obj_set_pos(display_info_con_2, 9, 151);
                lv_obj_set_size(display_info_con_2, LV_PCT(90), LV_PCT(15));
                lv_obj_set_style_pad_left(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(display_info_con_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(display_info_con_2, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(display_info_con_2, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(display_info_con_2, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(display_info_con_2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(display_info_con_2, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // display_info_con_2_lable_1
                        lv_obj_t *display_info_con_2_lable_1 = lv_label_create(display_info_con_2);
                        // objects.display_info_con_2_lable_1 = obj;
                        lv_obj_set_pos(display_info_con_2_lable_1, 0, 0);
                        lv_obj_set_size(display_info_con_2_lable_1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(display_info_con_2_lable_1, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(display_info_con_2_lable_1, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_info_con_2_lable_1, &lv_font_montserrat_16, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(display_info_con_2_lable_1, "STATUS");
                    }
                    {
                        // display_info_con_2_lable_2
                        lv_obj_t *display_info_con_2_lable_2 = lv_label_create(display_info_con_2);
                        // objects.display_info_con_2_lable_2 = obj;
                        lv_obj_set_pos(display_info_con_2_lable_2, 53, 1);
                        lv_obj_set_size(display_info_con_2_lable_2, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_add_flag(display_info_con_2_lable_2, LV_OBJ_FLAG_CLICKABLE);
                        lv_obj_set_style_text_color(display_info_con_2_lable_2, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_info_con_2_lable_2, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(display_info_con_2_lable_2, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(display_info_con_2_lable_2, "103.45");
                    
                        char id_buf[32];
                        snprintf(id_buf, sizeof(id_buf), "%s", data->status);
                        lv_label_set_text(display_info_con_2_lable_2, id_buf);
                    }
                }
            }
            {
                // display_select_btn
                lv_obj_t *display_select_btn = lv_btn_create(display_widget_con);
                // objects.select_btn = obj;
                lv_obj_set_pos(display_select_btn, 29, 230);
                lv_obj_set_size(display_select_btn, LV_PCT(70), LV_PCT(15));
                lv_obj_set_style_bg_color(display_select_btn, lv_color_hex(theme_colors[active_theme_index][3]), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(display_select_btn, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_dir(display_select_btn, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_color(display_select_btn, lv_color_hex(theme_colors[active_theme_index][4]), LV_PART_MAIN | LV_STATE_DEFAULT);
                
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // display_btn_label_1
                        lv_obj_t *display_btn_label_1 = lv_label_create(display_select_btn);
                        // objects.btn_label_1 = obj;
                        lv_obj_set_pos(display_btn_label_1, 0, 0);
                        lv_obj_set_size(display_btn_label_1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_align(display_btn_label_1, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(display_btn_label_1, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(display_btn_label_1, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(display_btn_label_1, "SELECT");
                        
                        lv_obj_add_event_cb(display_select_btn, on_display_select_clicked, LV_EVENT_CLICKED, (void*)data);
                        
                    }
                }
            }
        }
    }
}


//===============================================================================================================================================================================================================================================


void create_nozzle_widget(lv_obj_t * parent_obj, NozzleNode * data)
{
    // -------------------------------------------------------------------------
    // 1. MAIN NOZZLE CONTAINER
    // -------------------------------------------------------------------------

    {
        // nozzle_con
        lv_obj_t * nozzle_con = lv_obj_create(parent_obj);
        // lv_obj_set_pos(nozzle_con, LV_PCT(0), LV_PCT(0));
        lv_obj_set_size(nozzle_con, LV_PCT(24), LV_PCT(90));
        lv_obj_set_style_pad_left(nozzle_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(nozzle_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(nozzle_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(nozzle_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(nozzle_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_clear_flag(nozzle_con, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
        lv_obj_set_scrollbar_mode(nozzle_con, LV_SCROLLBAR_MODE_OFF);
        lv_obj_set_style_bg_color(nozzle_con, lv_color_hex(theme_colors[active_theme_index][0]), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(nozzle_con, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_radius(nozzle_con, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_dir(nozzle_con, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_grad_color(nozzle_con, lv_color_hex(theme_colors[active_theme_index][1]), LV_PART_MAIN | LV_STATE_DEFAULT);
      {
            {
                // product_panel
                lv_obj_t * product_panel = lv_obj_create(nozzle_con);
                // objects.product_panel = product_panel;
                lv_obj_set_pos(product_panel, 10, 9);
                lv_obj_set_size(product_panel, LV_PCT(90), LV_PCT(15));
                lv_obj_clear_flag(product_panel, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                lv_obj_set_scrollbar_mode(product_panel, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_bg_color(product_panel, lv_color_hex(data->color_hex), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(product_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(product_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(product_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_dir(product_panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_color(product_panel, lv_color_lighten(lv_color_hex(get_shine(data->color_hex, 0.5f)), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(product_panel, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // Product_label
                        lv_obj_t * product_label = lv_label_create(product_panel);
                        // objects.product_label = product_label;
                        lv_obj_set_pos(product_label, 0, 0);
                        lv_obj_set_size(product_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_font(product_label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(product_label, "PETROL");
                        lv_label_set_text(product_label, data->fuel_type); // Inject Struct Data

                    }
                }
            }
            {
                // nozzle_panel
                lv_obj_t * nozzle_panel = lv_obj_create(nozzle_con);
                // objects.nozzle_panel = nozzle_panel;
                lv_obj_set_pos(nozzle_panel, 29, 64);
                lv_obj_set_size(nozzle_panel, LV_PCT(70), LV_PCT(15));
                lv_obj_clear_flag(nozzle_panel, LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
                lv_obj_set_scrollbar_mode(nozzle_panel, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_bg_color(nozzle_panel, lv_color_hex(theme_colors[active_theme_index][5]), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(nozzle_panel, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(nozzle_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(nozzle_panel, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_row_dsc_array(nozzle_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                {
                    static lv_coord_t dsc[] = {0, LV_GRID_TEMPLATE_LAST};
                    lv_obj_set_style_grid_column_dsc_array(nozzle_panel, dsc, LV_PART_MAIN | LV_STATE_DEFAULT);
                }
                // lv_obj_set_style_bg_grad_dir(nozzle_panel, LV_GRAD_DIR_HOR, LV_PART_MAIN | LV_STATE_DEFAULT);
                // lv_obj_set_style_bg_grad_color(nozzle_panel, lv_color_darken(lv_color_hex(0xe4ab65), 0), LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // nozzel_label
                        lv_obj_t * nozzel_label = lv_label_create(nozzle_panel);
                        // objects.nozzel_label = obj;
                        lv_obj_set_pos(nozzel_label, 19, 23);
                        lv_obj_set_size(nozzel_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_color(nozzel_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(nozzel_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(nozzel_label, "NOZZEL 1");

                        char id_buf[16];
                        snprintf(id_buf, sizeof(id_buf), "NOZZLE %d", data->nozzle_id);
                        lv_label_set_text(nozzel_label, id_buf);
                    }
                }
            }
            {
                // rate_con
                
                lv_obj_t * rate_con = lv_obj_create(nozzle_con);
                // objects.rate_con = obj;
                lv_obj_set_pos(rate_con, 10, 130);
                lv_obj_set_size(rate_con, LV_PCT(90), LV_PCT(10));
                lv_obj_set_style_pad_left(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(rate_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(rate_con, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(rate_con, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(rate_con, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(rate_con, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // title_rate
                        lv_obj_t * title_rate = lv_label_create(rate_con);
                        objects.title_rate = title_rate;
                        lv_obj_set_pos(title_rate, 0, 0);
                        lv_obj_set_size(title_rate, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_color(title_rate, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(title_rate, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(title_rate, "RATE");
                    }
                    {
                        // val_rate
                        // lv_obj_t * val_rate = lv_label_create(rate_con);
                        lv_obj_t * val_rate = lv_label_create(rate_con);
                        // objects.val_rate = val_rate;
                        lv_obj_set_pos(val_rate, 53, 1);
                        lv_obj_set_size(val_rate, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_color(val_rate, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(val_rate, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(val_rate, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(val_rate, "103.45");
                        char rate_buf[16];
                        snprintf(rate_buf, sizeof(rate_buf), "%.2f", data->rate);
                        lv_label_set_text(val_rate, rate_buf);

                    }
                }
            }
            {
                // density_con
                // lv_obj_t *obj = lv_obj_create(parent_obj);
                lv_obj_t * density_con = lv_obj_create(nozzle_con);
                lv_obj_set_pos(density_con, 10, 159);
                lv_obj_set_size(density_con, LV_PCT(90), LV_PCT(10));
                lv_obj_set_style_pad_left(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(density_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(density_con, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(density_con, LV_FLEX_FLOW_ROW, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(density_con, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(density_con, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    // lv_obj_t *parent_obj = obj;
                    {
                        // title_density
                        lv_obj_t * title_density = lv_label_create(density_con);
                        objects.title_density = title_density;
                        lv_obj_set_pos(title_density, 0, 0);
                        lv_obj_set_size(title_density, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_color(title_density, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(title_density, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(title_density, "DENSITY");
                    }
                    {
                        // val_density
                        lv_obj_t *val_density = lv_label_create(density_con);
                        // objects.val_density = val_density;
                        lv_obj_set_pos(val_density, 53, 1);
                        lv_obj_set_size(val_density, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_color(val_density, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(val_density, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_align(val_density, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                        // lv_label_set_text_static(val_density, "0.785");
                        char density_buf[16];
                        snprintf(density_buf, sizeof(density_buf), "%.3f", data->density);
                        lv_label_set_text(val_density, density_buf);
                    }
                }
            }
            {
                // nozzle_btn
                lv_obj_t * nozzle_btn = lv_btn_create(nozzle_con);
                // objects.nozzle_btn = nozzle_btn;
                lv_obj_set_pos(nozzle_btn, 29, 230);
                lv_obj_set_size(nozzle_btn, LV_PCT(70), LV_PCT(15));
                lv_obj_set_style_bg_color(nozzle_btn, lv_color_hex(0x31b957), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(nozzle_btn, 120, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_dir(nozzle_btn, LV_GRAD_DIR_VER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_grad_color(nozzle_btn, lv_color_darken(lv_color_hex(0x31b957), 89), LV_PART_MAIN | LV_STATE_DEFAULT);
                
                {
                    // lv_obj_t *parent_obj = nozzle_btn;
                    {
                        // btn_label
                        lv_obj_t * btn_label = lv_label_create(nozzle_btn);
                        // objects.btn_label = btn_label;
                        lv_obj_set_pos(btn_label, 0, 0);
                        lv_obj_set_size(btn_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_align(btn_label, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(btn_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text_static(btn_label, "SELECT");
                        
                        lv_obj_add_event_cb(nozzle_btn, on_nozzle_select_clicked, LV_EVENT_CLICKED, (void*)data);
                    
                    }
                }
            }
        }
    }
}




//======================================================================================================================================================================================================================


//======================================================================================================================================================================================================================



void init_system_header(void) 
{
 
    // header_panel

    SystemConfig * sys_data = get_system_config();
    // lv_obj_t * current_screen = lv_scr_act();/


    sys_header_panel = lv_obj_create(lv_layer_top());

    lv_obj_set_pos(sys_header_panel, 0, 0);
    lv_obj_set_size(sys_header_panel, LV_PCT(100), LV_PCT(10));
    lv_obj_clear_flag(sys_header_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(sys_header_panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(sys_header_panel, lv_color_hex(theme_colors[active_theme_index][0]), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(sys_header_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_x(sys_header_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_ofs_y(sys_header_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_color(sys_header_panel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(sys_header_panel, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_spread(sys_header_panel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(sys_header_panel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_opa(sys_header_panel, 160, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_align(sys_header_panel, LV_ALIGN_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
    {
        {
            // system_config_con
            lv_obj_t * system_config_con = lv_obj_create(sys_header_panel);
            lv_obj_set_pos(system_config_con, -20, 0);
            lv_obj_set_size(system_config_con, 144, 35);
            lv_obj_set_style_pad_left(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_top(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_right(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_pad_bottom(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_bg_opa(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_radius(system_config_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_scrollbar_mode(system_config_con, LV_SCROLLBAR_MODE_OFF);
            lv_obj_set_style_layout(system_config_con, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(system_config_con, LV_ALIGN_LEFT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_cross_place(system_config_con, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_track_place(system_config_con, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_flow(system_config_con, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_flex_main_place(system_config_con, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
            {
                {
                    // dispenser_num
                    lv_obj_t * dispenser_num = lv_label_create(system_config_con);
                    // objects.dispenser_num = obj;
                    lv_obj_set_pos(dispenser_num, 29, 13);
                    lv_obj_set_size(dispenser_num, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(dispenser_num, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(dispenser_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(dispenser_num, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    // lv_label_set_text_static(dispenser_num, "DU-01");
                    lv_label_set_text(dispenser_num, sys_data->dispenser_id);

                }
                {
                    // dispenser_serial_num
                    lv_obj_t * dispenser_serial_num = lv_label_create(system_config_con);
                    // objects.dispenser_serial_num = obj;
                    lv_obj_set_pos(dispenser_serial_num, 29, 0);
                    lv_obj_set_size(dispenser_serial_num, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                    lv_obj_set_style_text_font(dispenser_serial_num, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_text_color(dispenser_serial_num, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_style_layout(dispenser_serial_num, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                    // lv_label_set_text_static(dispenser_serial_num, "SN  : 998822A");
                    lv_label_set_text(dispenser_serial_num, sys_data->serial_number);
                }
            }
        }
        {
            // header_label
            lv_obj_t * lbl_company = lv_label_create(sys_header_panel);
            // objects.header_label = obj;
            lv_obj_set_pos(lbl_company, 0, 0);
            lv_obj_set_size(lbl_company, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
            lv_obj_clear_flag(lbl_company, LV_OBJ_FLAG_CLICK_FOCUSABLE|LV_OBJ_FLAG_GESTURE_BUBBLE|LV_OBJ_FLAG_PRESS_LOCK|LV_OBJ_FLAG_SCROLLABLE|LV_OBJ_FLAG_SCROLL_CHAIN_HOR|LV_OBJ_FLAG_SCROLL_CHAIN_VER|LV_OBJ_FLAG_SCROLL_ELASTIC|LV_OBJ_FLAG_SCROLL_MOMENTUM|LV_OBJ_FLAG_SCROLL_WITH_ARROW|LV_OBJ_FLAG_SNAPPABLE);
            lv_obj_set_style_text_font(lbl_company, &lv_font_montserrat_20, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_text_color(lbl_company, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_align(lbl_company, LV_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text(lbl_company, sys_data->company_name);

        }

        // if (current_screen == objects.nozzle_select_screen){

            {
                // system_time_con
                lv_obj_t * system_time_con = lv_obj_create(sys_header_panel);
                lv_obj_set_pos(system_time_con, -38, 0);
                lv_obj_set_size(system_time_con, 144, 35);
                lv_obj_set_style_pad_left(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_scrollbar_mode(system_time_con, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_layout(system_time_con, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(system_time_con, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(system_time_con, LV_FLEX_ALIGN_END, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(system_time_con, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(system_time_con, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_align(system_time_con, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // date_label
                        date_label = lv_label_create(system_time_con);
                        // objects.date_label = obj;
                        lv_obj_set_pos(date_label, -5, 11);
                        lv_obj_set_size(date_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(date_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_layout(date_label, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(date_label, "Loading...");
                    }
                    {
                        // time_label
                        time_label = lv_label_create(system_time_con);
                        // objects.time_label = obj;
                        lv_obj_set_pos(time_label, 15, 0);
                        lv_obj_set_size(time_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_layout(time_label, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(time_label, "Loading...");
                    }
                }
            }
            {
                // settings_btn
                lv_obj_t * settings_btn = lv_btn_create(sys_header_panel);
                // objects.settings_btn = obj;
                lv_obj_set_pos(settings_btn, 722, -21);
                lv_obj_set_size(settings_btn, 50, 50);
                lv_obj_add_event_cb(settings_btn, action_config_login, LV_EVENT_CLICKED, (void *)0);
                lv_obj_set_style_bg_color(settings_btn, lv_color_hex(theme_colors[active_theme_index][0]), LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_shadow_width(settings_btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_layout(settings_btn, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(settings_btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(settings_btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(settings_btn, LV_FLEX_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // setting_icon_img
                        lv_obj_t *setting_icon_img = lv_img_create(settings_btn);
                        // objects.obj1 = obj;
                        lv_obj_set_pos(setting_icon_img, -8, -9);
                        lv_obj_set_size(setting_icon_img, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_img_set_src(setting_icon_img, &img_config_icon);
                        lv_img_set_zoom(setting_icon_img, 180);
                        lv_obj_add_event_cb(setting_icon_img, action_config_login, LV_EVENT_CLICKED, (void *)0);
                        lv_obj_set_style_img_recolor(setting_icon_img, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_img_recolor_opa(setting_icon_img, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
                    }
                }
            }

        // }
        /*else{

            {
                // system_time_con
                lv_obj_t * system_time_con = lv_obj_create(sys_header_panel);
                lv_obj_set_pos(system_time_con, 0, 0);
                lv_obj_set_size(system_time_con, 144, 35);
                lv_obj_set_style_pad_left(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_top(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_right(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_pad_bottom(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_bg_opa(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_border_width(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_radius(system_time_con, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_scrollbar_mode(system_time_con, LV_SCROLLBAR_MODE_OFF);
                lv_obj_set_style_layout(system_time_con, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_cross_place(system_time_con, LV_FLEX_ALIGN_START, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_track_place(system_time_con, LV_FLEX_ALIGN_END, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_flow(system_time_con, LV_FLEX_FLOW_COLUMN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_flex_main_place(system_time_con, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_PART_MAIN | LV_STATE_DEFAULT);
                lv_obj_set_style_align(system_time_con, LV_ALIGN_RIGHT_MID, LV_PART_MAIN | LV_STATE_DEFAULT);
                {
                    {
                        // date_label
                        date_label = lv_label_create(system_time_con);
                        // objects.date_label = obj;
                        lv_obj_set_pos(date_label, 29, 13);
                        lv_obj_set_size(date_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(date_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_layout(date_label, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(date_label, "Loading...");
                    }
                    {
                        // time_label
                        time_label = lv_label_create(system_time_con);
                        // objects.time_label = obj;
                        lv_obj_set_pos(time_label, 29, 0);
                        lv_obj_set_size(time_label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
                        lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_obj_set_style_layout(time_label, LV_LAYOUT_FLEX, LV_PART_MAIN | LV_STATE_DEFAULT);
                        lv_label_set_text(time_label, "Loading...");
                    }
                }
            }

        }*/
    }

    lv_timer_create(live_clock_timer_cb, 1000, NULL);
}



//===============================================================================================================================================================================================================================================



// Make this accessible so we can delete it from the refresh function
lv_obj_t * loading_overlay = NULL;

void create_loading_overlay(void)
{
    // Ensure we don't create multiple overlays
    if (loading_overlay != NULL) return;

    // 1. Create a full-screen container
    loading_overlay = lv_obj_create(lv_scr_act()); 
    lv_obj_set_size(loading_overlay, 800, 480);
    lv_obj_set_style_bg_color(loading_overlay, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(loading_overlay, 180, LV_PART_MAIN); // 180 = semi-transparent
    lv_obj_clear_flag(loading_overlay, LV_OBJ_FLAG_SCROLLABLE);

    // 2. Add a Spinner
    lv_obj_t * spinner = lv_spinner_create(loading_overlay, 1000, 60);
    lv_obj_set_size(spinner, 60, 60);
    lv_obj_center(spinner);

    // 3. Add Loading Text
    lv_obj_t * label = lv_label_create(loading_overlay);
    lv_label_set_text(label, "Loading Dispenser Data...");
    lv_obj_align_to(label, spinner, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}


//===============================================================================================================================================================================================================================================

