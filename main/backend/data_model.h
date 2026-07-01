#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_NOZZLES 24

typedef struct {
    uint8_t id;
    char product_name[32]; 
    float rate;
    float density;
    uint32_t color_hex;
} NozzleData;

void load_nozzle_data(void);
int get_current_nozzle_count(void);
NozzleData* get_nozzle(int index);

// New APIs for tracking the active user session
void set_active_transaction_nozzle(int nozzle_id);
NozzleData* get_active_transaction_nozzle(void);


// 1. Add this Enum to define the dispensing modes
typedef enum {
    MODE_NONE,
    MODE_AMOUNT,
    MODE_VOLUME
} DispenseMode;

// 2. Add these function definitions to the bottom
void set_transaction_mode(DispenseMode mode);
DispenseMode get_transaction_mode(void);

void set_transaction_value(float value);
float get_transaction_value(void);


// 1. The Enum Definition
typedef enum {
    PAYMENT_NONE,
    PAYMENT_QR,
    PAYMENT_CARD
} PaymentMethod;

// 2. Just the prototypes (Ending in semicolons!)
void set_payment_method(PaymentMethod method);
PaymentMethod get_payment_method(void);