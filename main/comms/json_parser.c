// #include "json_parser.h"
// #include "cJSON.h"
// #include "data_model.h"
// #include "esp_log.h"
// #include <string.h>
// #include <stdio.h>
// #include <ui_manager.h>

// static const char* TAG = "JSON_PARSER";



// // Bridge to the UI layer to safely update the screen dynamically
// extern void refresh_dynamic_panels(void);
// // At the top of main/comms/json_parser.c
// // extern void transition_to_dispenser_select_screen(void);
// // // Add a way to read the screen state, or just expose a check function
// // extern bool is_ui_booting(void); 



// bool parse_live_data(const char* raw_json) 
// {
//     if (raw_json == NULL) return false;

//     cJSON *root = cJSON_Parse(raw_json);
//     if (root == NULL) {
//         ESP_LOGE(TAG, "cJSON structural validation failure.");
//         return false;
//     }

//     cJSON *count_obj = cJSON_GetObjectItem(root, "DISPENSER_COUNT");
//     if (!count_obj) count_obj = cJSON_GetObjectItem(root, "DISPENSRE_COUNT"); // Handle potential typo
    
//     int total_dispensers = cJSON_IsNumber(count_obj) ? count_obj->valueint : 1;

//     // LOCK MUTEX: Ensure UI thread stops reading while database updates
//     if (!lock_station_model()) {
//         cJSON_Delete(root);
//         return false;
//     }

//     for (int i = 1; i <= total_dispensers; i++) {
//         char disp_key[32];
//         snprintf(disp_key, sizeof(disp_key), "DISPENSER_%d", i);
        
//         cJSON *dispenser_obj = cJSON_GetObjectItem(root, disp_key);
//         if (!dispenser_obj) {
//             snprintf(disp_key, sizeof(disp_key), "DISPENSRE_%d", i);
//             dispenser_obj = cJSON_GetObjectItem(root, disp_key);
//         }
//         if (!dispenser_obj) continue;

//         cJSON *disp_id_obj = cJSON_GetObjectItem(dispenser_obj, "DISPENSER_ID");
//         cJSON *disp_sn_obj = cJSON_GetObjectItem(dispenser_obj, "DISPENSER_SERIAL_NUMBER");
//         cJSON *disp_cnt_obj = cJSON_GetObjectItem(dispenser_obj, "DISPLAY_COUNT");

//         if (!disp_id_obj) continue;
        
//         int parsed_id = disp_id_obj->valueint;
//         int sub_display_count = cJSON_IsNumber(disp_cnt_obj) ? disp_cnt_obj->valueint : 1;
//         char sn_str[32] = "N/A";

//         if (disp_sn_obj) {
//             if (cJSON_IsNumber(disp_sn_obj)) {
//                 snprintf(sn_str, sizeof(sn_str), "%d", disp_sn_obj->valueint);
//             } else if (disp_sn_obj->valuestring) {
//                 strncpy(sn_str, disp_sn_obj->valuestring, sizeof(sn_str)-1);
//             }
//         }

//         // 1. Ingestion: Register or look up the core dispenser map entry
//         int du_idx = register_or_get_dispenser(parsed_id, sn_str, sub_display_count);
//         if (du_idx == -1) continue;

//         cJSON *du_status = cJSON_GetObjectItem(dispenser_obj, "DU_STATUS");
//         if (!du_status) continue;

//         for (int d = 1; d <= sub_display_count; d++) {
//             char disp_node_key[32];
//             snprintf(disp_node_key, sizeof(disp_node_key), "DISPLAY_%d", d);
            
//             cJSON *display_node = cJSON_GetObjectItem(du_status, disp_node_key);
//             if (!display_node) continue;

//             cJSON *status_obj = cJSON_GetObjectItem(display_node, "STATUS");
//             const char* status_txt = (status_obj && status_obj->valuestring) ? status_obj->valuestring : "IDLE";

//             // 2. Ingestion: Push screens details directly into sub-struct parameters
//             update_display_node(du_idx, d, status_txt);

//             for (int n = 1; n <= 4; n++) {
//                 char noz_key[16];
//                 snprintf(noz_key, sizeof(noz_key), "NOZZLE_%d", n);
                
//                 cJSON *noz_obj = cJSON_GetObjectItem(display_node, noz_key);
//                 if (!noz_obj) continue;

//                 cJSON *fuel_type = cJSON_GetObjectItem(noz_obj, "FUEL_TYPE");
//                 cJSON *rate = cJSON_GetObjectItem(noz_obj, "RATE");
//                 cJSON *density = cJSON_GetObjectItem(noz_obj, "DENSITY");

//                 if (fuel_type && rate) {
//                     // 3. Ingestion: Store nested nozzle records deep inside arrays
//                     update_nozzle_node(du_idx, d, n, 
//                                        fuel_type->valuestring, 
//                                        (float)rate->valuedouble, 
//                                        density ? (float)density->valuedouble : 0.0f);
//                 }
//             }
//         }
//     }

    
//     // UNLOCK MUTEX: Allow UI tasks to safely read the newly parsed data
//     unlock_station_model();
    
//     // Free JSON parsing memory
//     cJSON_Delete(root);

//     // Print snapshot to verify backend logic
//     print_station_model_registry();

    
//     refresh_dynamic_panels();
    
//     hide_loading_overlay(); 
//     // 4. TRIGGER UI UPDATE: Safely instruct LVGL to redraw based on new data
//         // Check if the user is still staring at the loading screen

        
//     if (is_ui_booting()) {
//         // This is our very first packet! Switch from loading to the main menu.
//         transition_to_dispenser_select_screen();
//     } else {
//         // The UI is already running; just update the active screen in the background.
//         refresh_dynamic_panels();
//     }
    
//     return true;
// }







#include "json_parser.h"
#include "cJSON.h"
#include "data_model.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h> 
#include <ui_manager.h>

static const char* TAG = "JSON_PARSER";

// Bridge to the UI layer to safely update the screen dynamically
extern void refresh_dynamic_panels(void);
extern void transition_to_display_select_screen(void); // CHANGED: Bypasses dispenser selection screen
extern bool is_ui_booting(void); 

bool parse_live_data(const char* raw_json) 
{
    if (raw_json == NULL) return false;

    cJSON *root = cJSON_Parse(raw_json);
    if (root == NULL) {
        ESP_LOGE(TAG, "cJSON structural validation failure.");
        return false;
    }

    cJSON *dispenser_obj = cJSON_GetObjectItem(root, "DISPENSER");
    if (!dispenser_obj) {
        ESP_LOGE(TAG, "Missing standard DISPENSER object in testing payload.");
        cJSON_Delete(root);
        return false;
    }

    // LOCK MUTEX: Ensure UI thread stops reading while database updates
    if (!lock_station_model()) {
        cJSON_Delete(root);
        return false;
    }

    cJSON *disp_id_obj = cJSON_GetObjectItem(dispenser_obj, "DISPENSER_ID");
    cJSON *disp_sn_obj = cJSON_GetObjectItem(dispenser_obj, "DISPENSER_SERIAL_NUMBER");
    cJSON *disp_cnt_obj = cJSON_GetObjectItem(dispenser_obj, "DISPLAY_COUNT");
    
    int parsed_id = (disp_id_obj) ? disp_id_obj->valueint : 1;
    int sub_display_count = (disp_cnt_obj) ? disp_cnt_obj->valueint : 1;
    char sn_str[32] = "N/A";

    if (disp_sn_obj) {
        if (cJSON_IsNumber(disp_sn_obj)) {
            snprintf(sn_str, sizeof(sn_str), "%d", disp_sn_obj->valueint);
        } else if (disp_sn_obj->valuestring) {
            strncpy(sn_str, disp_sn_obj->valuestring, sizeof(sn_str)-1);
        }
    }

    // Ingestion: Register the single dispenser entry (maps safely to database index 0)
    int du_idx = register_or_get_dispenser(parsed_id, sn_str, sub_display_count);
    
    if (du_idx != -1) {
        cJSON *du_status = cJSON_GetObjectItem(dispenser_obj, "DU_STATUS");
        if (du_status) {
            for (int d = 1; d <= sub_display_count; d++) {
                // FIXED: Increased size to 32 bytes to completely stop format truncation errors
                char disp_node_key[32]; 
                snprintf(disp_node_key, sizeof(disp_node_key), "DISPLAY_%d", d);
                
                cJSON *display_node = cJSON_GetObjectItem(du_status, disp_node_key);
                if (!display_node) continue;

                cJSON *status_obj = cJSON_GetObjectItem(display_node, "STATUS");
                const char* status_txt = (status_obj && status_obj->valuestring) ? status_obj->valuestring : "IDLE";

                update_display_node(du_idx, d, status_txt);

                cJSON *noz_cnt_obj = cJSON_GetObjectItem(display_node, "NOZZLE_COUNT");
                int loop_nozzle_count = (noz_cnt_obj) ? noz_cnt_obj->valueint : 4;

                for (int n = 1; n <= loop_nozzle_count; n++) {
                    // FIXED: Increased size to 32 bytes to completely stop format truncation errors
                    char noz_key[32]; 
                    snprintf(noz_key, sizeof(noz_key), "NOZZLE_%d", n);
                    
                    cJSON *noz_obj = cJSON_GetObjectItem(display_node, noz_key);
                    if (!noz_obj) continue;

                    cJSON *fuel_type = cJSON_GetObjectItem(noz_obj, "FUEL_TYPE");
                    cJSON *rate_obj = cJSON_GetObjectItem(noz_obj, "RATE");
                    cJSON *density_obj = cJSON_GetObjectItem(noz_obj, "DENSITY");

                    if (fuel_type && fuel_type->valuestring) {
                        float parsed_rate = 0.0f;
                        float parsed_density = 0.0f;

                        if (rate_obj) {
                            parsed_rate = (rate_obj->valuestring) ? (float)atof(rate_obj->valuestring) : (float)rate_obj->valuedouble;
                        }
                        if (density_obj) {
                            parsed_density = (density_obj->valuestring) ? (float)atof(density_obj->valuestring) : (float)density_obj->valuedouble;
                        }

                        update_nozzle_node(du_idx, d, n, 
                                           fuel_type->valuestring, 
                                           parsed_rate, 
                                           parsed_density);
                    }
                }
            }
        }
    }

    // UNLOCK MUTEX: Allow UI tasks to safely read data
    unlock_station_model();
    cJSON_Delete(root);

    // Print snapshot to verify backend memory tables
    print_station_model_registry();

    // Hide any showing loader animations
    hide_loading_overlay(); 
        
    // 4. TRIGGER UI UPDATE: Bypass dispenser selection screen if booting up
    if (is_ui_booting()) {
        // Force context explicitly to Index 0 before shifting views
        set_active_dispenser_index(0); 
        
        // This is our very first packet! Transition straight into the display select layout screen
        transition_to_display_select_screen();
    } else {
        // The UI is already running; just update the active screen panels in the background.
        refresh_dynamic_panels();
    }
    
    return true;
}
