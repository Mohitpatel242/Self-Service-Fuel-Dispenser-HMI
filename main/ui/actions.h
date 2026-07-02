#ifndef EEZ_LVGL_UI_EVENTS_H
#define EEZ_LVGL_UI_EVENTS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void action_go_back(lv_event_t * e);
extern void action_numpad_ready(lv_event_t * e);
extern void action_mode_amount(lv_event_t * e);
extern void action_mode_volume(lv_event_t * e);
extern void action_numpad_confirm(lv_event_t * e);
extern void action_confirm_pay(lv_event_t * e);
extern void action_select_qr(lv_event_t * e);
extern void action_select_card(lv_event_t * e);
extern void action_payment_method_confirm(lv_event_t * e);
extern void action_payment_completed(lv_event_t * e);

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_EVENTS_H*/