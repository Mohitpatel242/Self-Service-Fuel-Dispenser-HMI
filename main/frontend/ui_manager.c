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


static const char *UI_TAG = "UI_Manager";

// Forward declarations
void transition_to_mode_select(void);
void transition_to_numpad(DispenseMode mode);
void transition_to_confirm(void);



// --- Global Pointers for the Header ---
static lv_obj_t * sys_header_panel;
static lv_obj_t * date_label;
static lv_obj_t * time_label;

// Declare a global pointer so we can delete the old QR code if they generate a new one
static lv_obj_t * active_qrcode = NULL;


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

static void on_nozzle_select_clicked(lv_event_t * e) 
{
    NozzleData * selected_data = (NozzleData *)lv_event_get_user_data(e);
    
    // Save state to backend
    set_active_transaction_nozzle(selected_data->id);

    // Prepare and load the new screen
    transition_to_mode_select(); 
}

static void live_clock_timer_cb(lv_timer_t * timer)
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



static void create_nozzle_widget(lv_obj_t * parent_obj, NozzleData * data)
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
                        lv_label_set_text(product_label, data->product_name); // Inject Struct Data

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
                        snprintf(id_buf, sizeof(id_buf), "NOZZLE %d", data->id);
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
                        
                        lv_obj_add_event_cb(nozzle_btn, on_nozzle_select_clicked, LV_EVENT_CLICKED, data);
                    }
                }
            }
        }
    }
}


static void init_system_header(void) 
{
 
    // header_panel

    SystemConfig * sys_data = get_system_config();


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
    }

    lv_timer_create(live_clock_timer_cb, 1000, NULL);
}




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
    
    // // Layout formatting
    // lv_obj_set_flex_align(objects.main_container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_obj_set_style_pad_all(objects.main_container, 20, LV_PART_MAIN);
    // lv_obj_clear_flag(objects.main_container, LV_OBJ_FLAG_SCROLL_ELASTIC);

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