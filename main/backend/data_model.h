/**
 * @file data_model.h
 * @brief Thread-safe structural storage engine for gas station multi-dispensers.
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>

#define MAX_DISPENSERS 5
#define MAX_DISPLAYS_PER_DU 3
#define MAX_NOZZLES_PER_DISP 4

// --- Core Data Structures ---

typedef struct {
    int nozzle_id;
    char fuel_type[16];
    float rate;
    float density;
    uint32_t color_hex; // Default to white if not set   
} NozzleNode;

typedef struct {
    int display_id;
    char status[32];
    int nozzle_count;
    NozzleNode nozzles[MAX_NOZZLES_PER_DISP];
} DisplayNode;

typedef struct {
    int dispenser_id;
    char serial_number[32];
    int display_count;
    DisplayNode displays[MAX_DISPLAYS_PER_DU];
} DispenserNode;

typedef struct {
    int dispenser_count;
    DispenserNode dispensers[MAX_DISPENSERS];
} StationRegistry;

// --- Enums ---

typedef enum {
    MODE_NONE,
    MODE_AMOUNT,
    MODE_VOLUME
} DispenseMode;

typedef enum {
    PAYMENT_NONE,
    PAYMENT_QR,
    PAYMENT_CARD
} PaymentMethod;

// --- System Thread Safety Controls ---
void station_model_init(void);
bool lock_station_model(void);
void unlock_station_model(void);

// --- Deep Insertion Data Update API (Used by JSON Parser) ---
int register_or_get_dispenser(int dispenser_id, const char* serial_number, int display_count);
bool update_display_node(int du_idx, int display_id, const char* status);
bool update_nozzle_node(int du_idx, int display_id, int nozzle_id, const char* fuel_type, float rate, float density);

// --- System Debug API ---
void print_station_model_registry(void);
int station_model_get_total_dispensers(void);




//=====================================================================================================================================================

// --- Active Context Management (Index-Based) ---
void set_active_dispenser_index(int index);
void set_active_display_index(int index);
void set_active_nozzle_index(int index);

// --- Safe O(1) Pointer-to-Index Resolvers ---
int get_dispenser_index_from_ptr(DispenserNode* du_ptr);
int get_display_index_from_ptr(DisplayNode* disp_ptr);
int get_nozzle_index_from_ptr(NozzleNode* noz_ptr);

//=====================================================================================================================================================


StationRegistry* get_active_Station(void);


//--- Contextual Dynamic Panel Getters (Used by UI) ---
int get_current_dispenser_count(void);
DispenserNode* get_dispenser(int index);

// --- Active Context Management ---
void set_active_dispenser_context(int dispenser_index);
DispenserNode* get_active_dispenser(void);



//--- Contextual Dynamic Panel Getters (Used by UI) ---
int get_current_display_count(void);
DisplayNode* get_display(int index);

// --- Active Context Management ---
// void set_active_display_context( int display_index);
DisplayNode* get_active_display(void);



// --- Contextual Dynamic Panel Getters (Used by UI) ---
int get_current_nozzle_count(void);
NozzleNode* get_nozzle(int index);

// --- Active Transaction Flow APIs ---
// void set_active_transaction_nozzle(int nozzle_id);
NozzleNode* get_active_nozzle(void);


//=====================================================================================================================================================


void set_transaction_mode(DispenseMode mode);
DispenseMode get_transaction_mode(void);

void set_transaction_value(float value);
float get_transaction_value(void);

void set_payment_method(PaymentMethod method);
PaymentMethod get_payment_method(void);