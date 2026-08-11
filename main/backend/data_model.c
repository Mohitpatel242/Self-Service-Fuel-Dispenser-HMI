/**
 * @file data_model.c
 * @brief Thread-safe storage engine tracking real-time station metrics.
 */

#include "data_model.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "DATA_MODEL";

static StationRegistry registry;
static SemaphoreHandle_t model_mutex = NULL;

// --- Index Context Variables ---
static int active_du_idx = 0;   
static int active_disp_idx = 0; 
static int active_noz_idx = 0;

// static int active_transaction_nozzle_id = -1; 

static DispenseMode active_dispense_mode = MODE_NONE;
static float active_dispense_value = 0.0f;
static PaymentMethod active_payment = PAYMENT_NONE;


bool lock_station_model(void) 
{
    if (model_mutex == NULL) return false;
    return (xSemaphoreTake(model_mutex, pdMS_TO_TICKS(150)) == pdTRUE);
}

void unlock_station_model(void) 
{
    if (model_mutex != NULL) {
        xSemaphoreGive(model_mutex);
    }
}

// --- Thread Safety ---
void station_model_init(void) 
{
    if (model_mutex == NULL) {
        model_mutex = xSemaphoreCreateMutex();
    }
    lock_station_model();
    memset(&registry, 0, sizeof(StationRegistry));
    unlock_station_model();
    ESP_LOGI(TAG, "Thread-safe Station Model Matrix Storage Initialized.");
}


// --- Data Ingestion ---
int register_or_get_dispenser(int dispenser_index, const char* serial_number, int display_count) 
{
    for (int i = 0; i < registry.dispenser_count; i++) {
        if (registry.dispensers[i].dispenser_index == dispenser_index) {
            if (serial_number) {
                strncpy(registry.dispensers[i].serial_number, serial_number, sizeof(registry.dispensers[i].serial_number) - 1);
            }
            if (display_count > 0 && display_count <= MAX_DISPLAYS_PER_DU) {
                registry.dispensers[i].display_count = display_count;
            }
            return i; 
        }
    }

    if (registry.dispenser_count < MAX_DISPENSERS) {
        int new_idx = registry.dispenser_count++;
        registry.dispensers[new_idx].dispenser_index = dispenser_index;
        registry.dispensers[new_idx].display_count = (display_count > 0 && display_count <= MAX_DISPLAYS_PER_DU) ? display_count : 1;
        
        if (serial_number) {
            strncpy(registry.dispensers[new_idx].serial_number, serial_number, sizeof(registry.dispensers[new_idx].serial_number) - 1);
            registry.dispensers[new_idx].serial_number[sizeof(registry.dispensers[new_idx].serial_number) - 1] = '\0';
        }
        ESP_LOGI(TAG, "Registered new Dispenser ID %d at Index [%d]", dispenser_index, new_idx);
        return new_idx;
    }
    return -1;
}



bool update_display_node(int du_idx, int display_id, int display_pos_id, const char* status, const char* running_transaction_amt, const char* running_transaction_qty, const char* trans_running_status ) 
{
    if (du_idx < 0 || du_idx >= registry.dispenser_count) return false;
    int d_idx = display_id - 1;
    if (d_idx < 0 || d_idx >= MAX_DISPLAYS_PER_DU) return false;

    DisplayNode *disp = &registry.dispensers[du_idx].displays[d_idx];
    disp->display_id = display_id;
    disp->display_pos_id = display_pos_id;

    if (status) {
        strncpy(disp->status, status, sizeof(disp->status) - 1);
        disp->status[sizeof(disp->status) - 1] = '\0';
    }
    if (running_transaction_amt) {
        strncpy(disp->running_transaction_amt, running_transaction_amt, sizeof(disp->running_transaction_amt) - 1);
        disp->running_transaction_amt[sizeof(disp->running_transaction_amt) - 1] = '\0';
    }
    if (running_transaction_qty) {
        strncpy(disp->running_transaction_qty, running_transaction_qty, sizeof(disp->running_transaction_qty) - 1);
        disp->running_transaction_qty[sizeof(disp->running_transaction_qty) - 1] = '\0';
    }
    if (trans_running_status) {
        strncpy(disp->trans_running_status, trans_running_status, sizeof(disp->trans_running_status) - 1);
        disp->trans_running_status[sizeof(disp->trans_running_status) - 1] = '\0';
    }
    return true;
}

bool update_nozzle_node(int du_idx, int display_id, int nozzle_id, int nozzle_pos_id, const char* fuel_type, float rate, float density) 
{
    if (du_idx < 0 || du_idx >= registry.dispenser_count) return false;
    int d_idx = display_id - 1;
    if (d_idx < 0 || d_idx >= MAX_DISPLAYS_PER_DU) return false;

    DisplayNode *disp = &registry.dispensers[du_idx].displays[d_idx];
    
    int n_idx = -1;
    for (int i = 0; i < disp->nozzle_count; i++) {
        if (disp->nozzles[i].nozzle_id == nozzle_id) {
            n_idx = i;
            break;
        }
    }

    if (n_idx == -1) {
        if (disp->nozzle_count < MAX_NOZZLES_PER_DISP) {
            n_idx = disp->nozzle_count++;
        } else {
            return false;
        }
    }

    NozzleNode *noz = &disp->nozzles[n_idx];
    noz->nozzle_id = nozzle_id;
    noz->nozzle_pos_id = nozzle_pos_id;
    noz->rate = rate;
    noz->density = density;
    
    // Auto-assign colors based on common fuel types for better UI look
    if (fuel_type) {
        strncpy(noz->fuel_type, fuel_type, sizeof(noz->fuel_type) - 1);
        noz->fuel_type[sizeof(noz->fuel_type) - 1] = '\0';
        
        if (strstr(fuel_type, "PETROL")) noz->color_hex = 0x31b957;
        else if (strstr(fuel_type, "DIESEL")) noz->color_hex = 0x2756d8;
        else if (strstr(fuel_type, "E20")) noz->color_hex = 0xd3e015;
        else noz->color_hex = 0xFFFFFF;
    }
    return true;
}



//=====================================================================================================================================================


// --- Setters ---
void set_active_dispenser_index(int index) { 
    if (lock_station_model()) {
        active_du_idx = index; 
        unlock_station_model();
    }
}
void set_active_display_index(int index) { 
    if (lock_station_model()) {
        active_disp_idx = index; 
        unlock_station_model();
    }
}
void set_active_nozzle_index(int index) { 
    if (lock_station_model()) {
        active_noz_idx = index; 
        unlock_station_model();
    }
}




// --- The Crash-Proof Resolvers ---

int get_dispenser_index_from_ptr(DispenserNode* du_ptr) 
{
    // Check if the pointer sits within the memory bounds of our dispenser array
    if (du_ptr >= registry.dispensers && du_ptr < (registry.dispensers + registry.dispenser_count)) {
        return du_ptr - registry.dispensers; // Pointer math yields the exact index!
    }
    ESP_LOGE("DATA", "Invalid Dispenser Pointer received from UI!");
    return -1; 
}

int get_display_index_from_ptr(DisplayNode* disp_ptr) 
{
    DispenserNode* active_du = get_active_dispenser(); // Gets the currently selected dispenser
    if (active_du != NULL) {
        if (disp_ptr >= active_du->displays && disp_ptr < (active_du->displays + active_du->display_count)) {
            return disp_ptr - active_du->displays;
        }
    }
    ESP_LOGE("DATA", "Invalid Display Pointer received from UI!");
    return -1;
}

int get_nozzle_index_from_ptr(NozzleNode* noz_ptr) 
{
    DisplayNode* active_disp = get_active_display(); // Gets the currently selected display
    if (active_disp != NULL) {
        if (noz_ptr >= active_disp->nozzles && noz_ptr < (active_disp->nozzles + active_disp->nozzle_count)) {
            return noz_ptr - active_disp->nozzles;
        }
    }
    ESP_LOGE("DATA", "Invalid Nozzle Pointer received from UI!");
    return -1;
}


//=====================================================================================================================================================


StationRegistry* get_active_Station(void) 
{
    return &registry;
}

//=====================================================================================================================================================

int get_current_dispenser_count(void) 
{
    int count = 0;
    if (lock_station_model()) {
        StationRegistry *sr = get_active_Station();
        if (sr != NULL) {
            count = sr->dispenser_count;
        }
        unlock_station_model();
    }
    return count;
}

DispenserNode* get_dispenser(int index) 
{
    StationRegistry *result = NULL;
    if (lock_station_model()) {
        StationRegistry *sr = get_active_Station();
        if (sr != NULL && index >= 0 && index < sr->dispenser_count) {
            result = &sr->dispensers[index];
        }
        unlock_station_model();
    }    
    return result;
}    

// --- Active Context & UI Getters ---
void set_active_dispenser_context(int dispenser_index) 
{
    if (lock_station_model()) {
        active_du_idx = dispenser_index;
        unlock_station_model();
    }
}


DispenserNode* get_active_dispenser(void) 
{   
    return &registry.dispensers[0]; // this for only testing of of single dispenser, will be removed later

    if (active_du_idx < 0 || active_du_idx >= registry.dispenser_count) {
        return NULL;
    }

    // Instantly retrieve using the exact memory index
    return &registry.dispensers[active_du_idx];

}


//==================================================================================================================================================


int get_current_display_count(void) 
{
    int count = 0;
    if (lock_station_model()) {

        DispenserNode* dis = get_active_dispenser();

        if (dis != NULL) {
            count = dis->display_count;
        
        }else{

            ESP_LOGE(TAG, " \"DispenserNode *dis = get_active_dispenser();\" is NULL " );
        }         
        unlock_station_model();
    }
    return count;
}

DisplayNode* get_display(int index) 
{
    DisplayNode *result = NULL;
    if (lock_station_model()) {
        DispenserNode *dis = get_active_dispenser();
        if (dis != NULL && index >= 0 && index < dis->display_count) {
            result = &dis->displays[index];
        }
        unlock_station_model();
    }    
    return result;
}    



DisplayNode* get_active_display(void) 
{
    DispenserNode* du = get_active_dispenser();
    
    if (du == NULL || active_disp_idx < 0 || active_disp_idx >= du->display_count) {
        return NULL;
    }
    // Instantly retrieve using the exact memory index
    return &du->displays[active_disp_idx];
}



//=====================================================================================================================================================



int get_current_nozzle_count(void) 
{
    int count = 0;
    if (lock_station_model()) {
        DisplayNode *disp = get_active_display();
        if (disp != NULL) {
            count = disp->nozzle_count;
        }
        unlock_station_model();
    }
    return count;
}

NozzleNode* get_nozzle(int index) 
{
    NozzleNode *result = NULL;
    if (lock_station_model()) {
        DisplayNode *disp = get_active_display();
        if (disp != NULL && index >= 0 && index < disp->nozzle_count) {
            result = &disp->nozzles[index];
        }
        unlock_station_model();
    }    
    return result;
}    


NozzleNode* get_active_nozzle(void) 
{
    DisplayNode* disp = get_active_display();
    
    // Safety boundary check
    if (disp == NULL || active_noz_idx < 0 || active_noz_idx >= disp->nozzle_count) {
        return NULL; 
    }
    
    // CRITICAL FIX: Do not loop. Do not check nozzle_id.
    // Just return the memory slot directly.
    return &disp->nozzles[active_noz_idx];
}



//=====================================================================================================================================================


bool is_nozzle_picked_up(void){
    DisplayNode *disp = get_active_display();
    char *trans_running_status = disp->trans_running_status;
    if (strcmp(trans_running_status, "ENABLE") == 0) {
        return true;
        ESP_LOGI(TAG, "Nozzle has been picked up. Transitioning to Mode Select Screen.");
    }

    return false;
}


void set_transaction_mode(DispenseMode mode) { active_dispense_mode = mode; }
DispenseMode get_transaction_mode(void) { return active_dispense_mode; }

void set_transaction_value(float value) { active_dispense_value = value; }
float get_transaction_value(void) { return active_dispense_value; }

void set_payment_method(PaymentMethod method) { active_payment = method; }
PaymentMethod get_payment_method(void) { return active_payment; }

// --- Debug ---
int station_model_get_total_dispensers(void) { return registry.dispenser_count; }





void print_station_model_registry(void) 
{
    ESP_LOGI(TAG, "=============== ACTIVE REAL-TIME LOCAL DATABASE MAP ===============");
    for (int i = 0; i < registry.dispenser_count; i++) {
        DispenserNode *du = &registry.dispensers[i];
        ESP_LOGI(TAG, "Dispenser Index [%d] -> ID: %d | Serial: %s | Active Screens: %d", 
                 i, du->dispenser_index, du->serial_number, du->display_count);
        
        for (int d = 0; d < du->display_count; d++) {
            DisplayNode *disp = &du->displays[d];
            ESP_LOGI(TAG, "   ├── Screen ID %d Status: [ %s ]", disp->display_id, disp->status);
            
            for (int n = 0; n < disp->nozzle_count; n++) {
                NozzleNode *nz = &disp->nozzles[n];
                ESP_LOGI(TAG, "   │     └── Nozzle Slot %d: %s | Price Rate: %.2f | Density: %.3f", 
                         nz->nozzle_id, nz->fuel_type, nz->rate, nz->density);
            }
        }
    }
    ESP_LOGI(TAG, "====================================================================");
}