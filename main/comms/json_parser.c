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

                cJSON *running_amt_obj = cJSON_GetObjectItem(display_node, "RUNNING_TRANSACTION_AMT");
                const char* running_amt_txt = (running_amt_obj && running_amt_obj->valuestring) ? running_amt_obj->valuestring : "00.00";

                cJSON *running_qty_obj = cJSON_GetObjectItem(display_node, "RUNNING_TRANSACTION_QTY");
                const char* running_qty_txt = (running_qty_obj && running_qty_obj->valuestring) ? running_qty_obj->valuestring : "00.00";

                cJSON *trans_running_status_obj = cJSON_GetObjectItem(display_node, "TRANS_RUNNING_STATUS");
                const char* trans_running_status_txt = (trans_running_status_obj && trans_running_status_obj->valuestring) ? trans_running_status_obj->valuestring : "DISABLE";

                update_display_node(du_idx, d, status_txt, running_amt_txt, running_qty_txt, trans_running_status_txt);

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



