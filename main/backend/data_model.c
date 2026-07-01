#include "data_model.h"
#include <stdio.h>
#include <stdint.h>


static NozzleData machine_nozzles[MAX_NOZZLES];
static int current_nozzle_count = 0; 

void load_nozzle_data(void) 
{
    current_nozzle_count = 4; 

    machine_nozzles[0].id = 1;
    snprintf(machine_nozzles[0].product_name, 32, "PETROL");
    machine_nozzles[0].rate = 103.45;
    machine_nozzles[0].density = 0.785;
    machine_nozzles[0].color_hex = 0x31b957;

    machine_nozzles[1].id = 2;
    snprintf(machine_nozzles[1].product_name, 32, "DIESEL");
    machine_nozzles[1].rate = 92.10;
    machine_nozzles[1].density = 0.832;
    machine_nozzles[1].color_hex = 0x2756d8;

    machine_nozzles[2].id = 3;
    snprintf(machine_nozzles[2].product_name, 32, "E20");
    machine_nozzles[2].rate = 85.60;
    machine_nozzles[2].density = 0.510;
    machine_nozzles[2].color_hex = 0xd3e015;

    machine_nozzles[3].id = 4;
    snprintf(machine_nozzles[3].product_name, 32, "E35");
    machine_nozzles[3].rate = 75.20;
    machine_nozzles[3].density = 0.480;
    machine_nozzles[3].color_hex = 0xebba36;

}

int get_current_nozzle_count(void) { return current_nozzle_count; }

NozzleData* get_nozzle(int index) {
    if (index < 0 || index >= current_nozzle_count) return NULL;
    return &machine_nozzles[index];
}

// ... existing code ...

// This variable remembers which nozzle the user selected for the current flow
static uint8_t active_transaction_nozzle_id = -1; 

void set_active_transaction_nozzle(int nozzle_id) 
{
    active_transaction_nozzle_id = nozzle_id;
}

NozzleData* get_active_transaction_nozzle(void) 
{
    for(int i = 0; i < current_nozzle_count; i++) {
        if(machine_nozzles[i].id == active_transaction_nozzle_id) {
            return &machine_nozzles[i];
        }
    }
    return NULL; // No active transaction
}

// ... existing variables ...
static DispenseMode active_dispense_mode = MODE_NONE;
static float active_dispense_value = 0.0f;

void set_transaction_mode(DispenseMode mode) {
    active_dispense_mode = mode;
}

DispenseMode get_transaction_mode(void) {
    return active_dispense_mode;
}

void set_transaction_value(float value) {
    active_dispense_value = value;
}

float get_transaction_value(void) {
    return active_dispense_value;
}


// 1. Create the storage variable
static PaymentMethod active_payment = PAYMENT_NONE;

// 2. Write the actual logic (Using curly braces {})
void set_payment_method(PaymentMethod method) 
{ 
    active_payment = method; 
}

PaymentMethod get_payment_method(void) 
{ 
    return active_payment; 
}