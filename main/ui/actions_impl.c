/**
 * @file actions_impl.c
 * @brief This bridges EEZ Studio UI events to your C backend.
 * EEZ Studio generates the headers in actions.h, we implement them here.
 */

#include "actions.h"
#include "lvgl.h"
#include "../ui/ui.h" // EEZ objects
#include "esp_log.h"
#include "../backend/data_model.h"
#include <stdlib.h> // Required for atof()
#include "../frontend/ui_manager.h" // Include this to access our new function

// Triggered by the BACK button in EEZ Studio


void action_go_back(lv_event_t * e) 
{
    // Find out which screen the user is currently looking at
    lv_obj_t * current_screen = lv_scr_act();

    // If they are on Mode Select, take them back to the Main Screen
    if (current_screen == objects.mode_select_screen) {
        lv_scr_load(objects.main_screen); // Use whatever name you gave your home screen
    }
    // If they are on the Numpad Screen (Future), take them back to Mode Select
    else if (current_screen == objects.numpad_screen) {
        lv_scr_load(objects.mode_select_screen);
    }
    // If they are on the Numpad Screen (Future), take them back to Mode Select
    else if (current_screen == objects.numpad_screen) {
        lv_scr_load(objects.mode_select_screen);
    }
    // If they are on the Confirm Screen, take them back to the Numpad
    else if (current_screen == objects.confirm_screen) {
        transition_to_numpad(get_transaction_mode());
    }
    // If they are on the Payment Screen, take them back to the Confirm Screen
    else if (current_screen == objects.payment_screen) {
        transition_to_confirm();
    }
    // If they are on the QR Screen, take them back to the Payment Screen
    else if (current_screen == objects.qr_screen) {
        lv_scr_load(objects.payment_screen);
    }
    // If they are on the Card Screen, take them back to the Payment Screen
    else if (current_screen == objects.card_screen) {
        lv_scr_load(objects.payment_screen);
    }
}


// Triggered by the "BY AMOUNT" button
void action_mode_amount(lv_event_t * e) 
{
    set_transaction_mode(MODE_AMOUNT); // Save to backend
    transition_to_numpad(MODE_AMOUNT); // Let ui_manager handle the graphics
    // transition_to_confirm(MODE_AMOUNT);
}

// Triggered by the "BY VOLUME" button
void action_mode_volume(lv_event_t * e) 
{
    set_transaction_mode(MODE_VOLUME); // Save to backend
    transition_to_numpad(MODE_VOLUME); // Let ui_manager handle the graphics
    // transition_to_confirm(MODE_VOLUME);
}


// Triggered by the LVGL Keyboard "Checkmark/Ready" button
void action_numpad_ready(lv_event_t * e) 
{
    // 1. Get the raw text string from the LVGL Text area
    const char * input_str = lv_textarea_get_text(objects.ta_input);
    
    // 2. Convert the string to a floating point number (e.g., "500" -> 500.0)
    float entered_value = atof(input_str);

    // Safety check: Don't let them proceed if they typed 0
    if (entered_value <= 0.0f) {
        ESP_LOGW("UI", "Invalid amount entered.");
        // Optional: Show an error message on screen here
        return;
    }

    // 3. Save the final value to the Backend
    set_transaction_value(entered_value);
    
    // 4. Log the transaction details so far!
    NozzleData * active_nozzle = get_active_transaction_nozzle();
    DispenseMode mode = get_transaction_mode();
    
    ESP_LOGI("TRANSACTION", "=====================================");
    ESP_LOGI("TRANSACTION", " FUEL: %s (Nozzle %d)", active_nozzle->product_name, active_nozzle->id);
    ESP_LOGI("TRANSACTION", " MODE: %s", (mode == MODE_AMOUNT) ? "By Amount" : "By Volume");
    ESP_LOGI("TRANSACTION", " VALUE: %.2f", entered_value);
    ESP_LOGI("TRANSACTION", "=====================================");


    // Trigger the transition function we just wrote!
    transition_to_confirm(); 
}


// 3. Triggered by the "PROCEED TO PAY" button on the Confirm Screen
void action_confirm_pay(lv_event_t * e) 
{
    ESP_LOGI("TRANSACTION", "User Confirmed Details. Moving to Payment...");
    
    // In the next step, we will load the Payment Method Screen here!
    lv_scr_load(objects.payment_screen); 
}


// 1. Triggered by the new "CONFIRM" button on the Numpad Screen
void action_numpad_confirm(lv_event_t * e) 
{
    // Get text from the Text area
    const char * input_str = lv_textarea_get_text(objects.ta_input);
    float entered_value = atof(input_str);

    // Safety Check: Reject 0 or empty inputs
    if (entered_value <= 0.0f) {
        ESP_LOGW("UI", "Invalid amount entered. Ignoring.");
        return; // Do nothing if invalid
    }

    // Save the value to the Backend Model
    set_transaction_value(entered_value);

    // Trigger the transition function we just wrote!
    transition_to_confirm(); 
}


// --- Payment Selection Visuals ---
void action_select_qr(lv_event_t * e) 
{
    set_payment_method(PAYMENT_QR);
    
    // Add a green border to QR, remove border from Card
    lv_obj_set_style_border_color(objects.payment_screen_qr_panel, lv_color_hex(0x31b957), LV_PART_MAIN);
    lv_obj_set_style_border_width(objects.payment_screen_qr_panel, 4, LV_PART_MAIN);
    
    lv_obj_set_style_border_width(objects.payment_screen_card_panel, 0, LV_PART_MAIN);
}


void action_select_card(lv_event_t * e) 
{
    set_payment_method(PAYMENT_CARD);
    
    // Add a green border to Card, remove border from QR
    lv_obj_set_style_border_color(objects.payment_screen_card_panel, lv_color_hex(0x31b957), LV_PART_MAIN);
    lv_obj_set_style_border_width(objects.payment_screen_card_panel, 4, LV_PART_MAIN);
    
    lv_obj_set_style_border_width(objects.payment_screen_qr_panel, 0, LV_PART_MAIN);
}


// // --- Navigation ---
// void action_payment_back(lv_event_t * e) 
// {
//     // Go back to the Confirm Summary screen
//     transition_to_confirm();
// }

void action_payment_method_confirm(lv_event_t * e) 
{
    PaymentMethod method = get_payment_method();
    
    if (method == PAYMENT_NONE) {
        ESP_LOGW("UI", "Please select a payment method first!");
        return;
    }
    
    if (method == PAYMENT_QR) {
        transition_to_qr_screen(); // We will write this next!
    } else if (method == PAYMENT_CARD) {
        lv_scr_load(objects.card_screen); // Just load the placeholder
    }
}

void action_payment_completed(lv_event_t * e) 
{
    ESP_LOGI("ACTIONS", "PICK-UP NOZZLE");
    
    // In the next step, we will load the Payment Method Screen here!
    transition_to_nozzle_pikup();
}