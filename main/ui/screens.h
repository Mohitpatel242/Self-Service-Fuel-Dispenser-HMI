#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN_SCREEN = 1,
    SCREEN_ID_MODE_SELECT_SCREEN = 2,
    SCREEN_ID_NUMPAD_SCREEN = 3,
    SCREEN_ID_CONFIRM_SCREEN = 4,
    SCREEN_ID_PAYMENT_SCREEN = 5,
    SCREEN_ID_QR_SCREEN = 6,
    SCREEN_ID_CARD_SCREEN = 7,
    SCREEN_ID_NOZZLE_PIKUP_SCREEN = 8,
    SCREEN_ID_LIVE_COUNTING_SCREEN = 9,
    _SCREEN_ID_LAST = 9
};

typedef struct _objects_t {
    lv_obj_t *main_screen;
    lv_obj_t *mode_select_screen;
    lv_obj_t *numpad_screen;
    lv_obj_t *confirm_screen;
    lv_obj_t *payment_screen;
    lv_obj_t *qr_screen;
    lv_obj_t *card_screen;
    lv_obj_t *nozzle_pikup_screen;
    lv_obj_t *live_counting_screen;
    lv_obj_t *header_panel;
    lv_obj_t *system_config_con;
    lv_obj_t *dispenser_num;
    lv_obj_t *dispenser_serial_num;
    lv_obj_t *header_label;
    lv_obj_t *system_time_con;
    lv_obj_t *date_label;
    lv_obj_t *time_label;
    lv_obj_t *main_container;
    lv_obj_t *nozzle_con;
    lv_obj_t *product_panel;
    lv_obj_t *product_label;
    lv_obj_t *nozzle_panel;
    lv_obj_t *nozzel_label;
    lv_obj_t *nozzle_btn;
    lv_obj_t *btn_label;
    lv_obj_t *rate_con;
    lv_obj_t *title_rate;
    lv_obj_t *val_rate;
    lv_obj_t *density_con;
    lv_obj_t *title_density;
    lv_obj_t *val_density;
    lv_obj_t *header_panel_1;
    lv_obj_t *header_label_1;
    lv_obj_t *mode_select_screen_con;
    lv_obj_t *active_product_panel;
    lv_obj_t *lbl_active_product;
    lv_obj_t *active_nozzle_panel;
    lv_obj_t *lbl_active_nozzle;
    lv_obj_t *mode_select_screen_con_label;
    lv_obj_t *by_amount;
    lv_obj_t *by_amount_btn_label;
    lv_obj_t *by_volume;
    lv_obj_t *by_volume_btn_label;
    lv_obj_t *back_btn;
    lv_obj_t *back_btn_label;
    lv_obj_t *header_panel_numpad_screen;
    lv_obj_t *header_label_numpad_screen;
    lv_obj_t *numpad_screen_keyboard;
    lv_obj_t *numpad_screen_con;
    lv_obj_t *numpad_screen_con_2;
    lv_obj_t *active_product_panel_numpad;
    lv_obj_t *lbl_active_product_numpad;
    lv_obj_t *active_nozzle_panel_numpad;
    lv_obj_t *lbl_active_nozzle_numpad;
    lv_obj_t *numpad_screen_rate_con;
    lv_obj_t *lbl_active_rate;
    lv_obj_t *numpad_screen_density_con;
    lv_obj_t *lbl_active_density;
    lv_obj_t *ta_input;
    lv_obj_t *lbl_unit;
    lv_obj_t *numpad_back_btn;
    lv_obj_t *numpad_back_btn_label;
    lv_obj_t *numpad_confirm_btn;
    lv_obj_t *numpad_confirm_btn_label;
    lv_obj_t *header_panel_2;
    lv_obj_t *system_config_con_1;
    lv_obj_t *dispenser_num_1;
    lv_obj_t *dispenser_serial_num_1;
    lv_obj_t *header_label_2;
    lv_obj_t *system_time_con_1;
    lv_obj_t *date_label_1;
    lv_obj_t *time_label_1;
    lv_obj_t *confirm_screen_con_1;
    lv_obj_t *confirm_screen_pdetails_con;
    lv_obj_t *confirm_screen_active_product_panel;
    lv_obj_t *confirm_screen_lbl_active_product;
    lv_obj_t *active_nozzle_panel_confirm_screen;
    lv_obj_t *confirm_screen_lbl_active_nozzle;
    lv_obj_t *confirm_screen_rate_con;
    lv_obj_t *confirm_screen_lbl_active_rate;
    lv_obj_t *confirm_screen_density_con;
    lv_obj_t *confirm_screen_lbl_active_density;
    lv_obj_t *confirm_screen_t_amount_con;
    lv_obj_t *obj0;
    lv_obj_t *lbl_conf_amount;
    lv_obj_t *confirm_screen_t_volume_con;
    lv_obj_t *obj1;
    lv_obj_t *lbl_conf_volume;
    lv_obj_t *confirm_screen_back_btn;
    lv_obj_t *confirm_screen_back_btn_label;
    lv_obj_t *confirm_screen_confirm_btn;
    lv_obj_t *confirm_screen_confirm_btn_label;
    lv_obj_t *header_panel_3;
    lv_obj_t *system_config_con_2;
    lv_obj_t *dispenser_num_2;
    lv_obj_t *dispenser_serial_num_2;
    lv_obj_t *header_label_3;
    lv_obj_t *system_time_con_2;
    lv_obj_t *date_label_2;
    lv_obj_t *time_label_2;
    lv_obj_t *payment_screen_con;
    lv_obj_t *payment_screen_methods_con;
    lv_obj_t *payment_screen_qr_panel;
    lv_obj_t *obj2;
    lv_obj_t *payment_screen_card_panel;
    lv_obj_t *obj3;
    lv_obj_t *payment_screen_back_btn;
    lv_obj_t *payment_screen_back_btn_label;
    lv_obj_t *payment_screen_confirm_btn;
    lv_obj_t *payment_screen_confirm_btn_label;
    lv_obj_t *payment_screen_label;
    lv_obj_t *header_panel_4;
    lv_obj_t *system_config_con_3;
    lv_obj_t *dispenser_num_3;
    lv_obj_t *dispenser_serial_num_3;
    lv_obj_t *header_label_4;
    lv_obj_t *system_time_con_3;
    lv_obj_t *date_label_3;
    lv_obj_t *time_label_3;
    lv_obj_t *qr_screen_con;
    lv_obj_t *qr_screen_panel;
    lv_obj_t *qr_container;
    lv_obj_t *qr_label;
    lv_obj_t *qr_screen_back_btn;
    lv_obj_t *qr_screen_back_btn_label;
    lv_obj_t *qr_screen_completed_btn;
    lv_obj_t *qr_screen_completed_btn_label;
    lv_obj_t *payment_screen_label_1;
    lv_obj_t *header_panel_5;
    lv_obj_t *system_config_con_4;
    lv_obj_t *dispenser_num_4;
    lv_obj_t *dispenser_serial_num_4;
    lv_obj_t *header_label_5;
    lv_obj_t *system_time_con_4;
    lv_obj_t *date_label_4;
    lv_obj_t *time_label_4;
    lv_obj_t *card_screen_con;
    lv_obj_t *card_screen_panel;
    lv_obj_t *card_label;
    lv_obj_t *card_screen_back_btn;
    lv_obj_t *card_screen_back_btn_label;
    lv_obj_t *card_screen_completed_btn;
    lv_obj_t *card_screen_completed_btn_label;
    lv_obj_t *card_screen_label;
    lv_obj_t *header_panel_7;
    lv_obj_t *system_config_con_6;
    lv_obj_t *dispenser_num_6;
    lv_obj_t *dispenser_serial_num_6;
    lv_obj_t *header_label_7;
    lv_obj_t *system_time_con_6;
    lv_obj_t *date_label_6;
    lv_obj_t *time_label_6;
    lv_obj_t *live_counting_screen_con_1;
    lv_obj_t *card_screen_label_3;
    lv_obj_t *card_screen_label_2;
    lv_obj_t *nozzle_pikup_active_nozzle_panel;
    lv_obj_t *nozzle_pikup_active_nozzle_label;
    lv_obj_t *header_panel_6;
    lv_obj_t *system_config_con_5;
    lv_obj_t *dispenser_num_5;
    lv_obj_t *dispenser_serial_num_5;
    lv_obj_t *header_label_6;
    lv_obj_t *system_time_con_5;
    lv_obj_t *date_label_5;
    lv_obj_t *time_label_5;
    lv_obj_t *live_counting_screen_con;
    lv_obj_t *card_screen_label_1;
    lv_obj_t *live_counting_screen_amount_panel;
    lv_obj_t *live_counting_screen_amount_label;
    lv_obj_t *live_counting_screen_volume_panel;
    lv_obj_t *live_counting_screen_volume_label;
    lv_obj_t *live_counting_screen_rs_label;
    lv_obj_t *live_counting_screen_liter_label;
    lv_obj_t *confirm_screen_t_amount_con_1;
    lv_obj_t *obj4;
    lv_obj_t *live_counting_s_t_amount_lbl;
    lv_obj_t *confirm_screen_t_volume_con_1;
    lv_obj_t *obj5;
    lv_obj_t *live_counting_s_t_volume_lbl;
} objects_t;

extern objects_t objects;

void create_screen_main_screen();
void tick_screen_main_screen();

void create_screen_mode_select_screen();
void tick_screen_mode_select_screen();

void create_screen_numpad_screen();
void tick_screen_numpad_screen();

void create_screen_confirm_screen();
void tick_screen_confirm_screen();

void create_screen_payment_screen();
void tick_screen_payment_screen();

void create_screen_qr_screen();
void tick_screen_qr_screen();

void create_screen_card_screen();
void tick_screen_card_screen();

void create_screen_nozzle_pikup_screen();
void tick_screen_nozzle_pikup_screen();

void create_screen_live_counting_screen();
void tick_screen_live_counting_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

// Color themes

enum Themes {
    THEME_ID_DEFAULT,
};
enum Colors {
    COLOR_ID_PANEL_COLOR,
    COLOR_ID_PANEL_GRAD_COLOR,
    COLOR_ID_HEADER_COLOR,
    COLOR_ID_BUTTON_COLOR,
    COLOR_ID_BUTTON_GRAD_COLOR,
    COLOR_ID_NOZZLE_PANEL_COLOR,
    COLOR_ID_WHITE_TEXT,
};
void change_color_theme(uint32_t themeIndex);
extern uint32_t theme_colors[1][7];
extern uint32_t active_theme_index;

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/