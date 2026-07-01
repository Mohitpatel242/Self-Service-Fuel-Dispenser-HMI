#pragma once

typedef struct {
    char company_name[64];
    char dispenser_id[16];
    char serial_number[32];
    char upi_id[64]; // NEW: For receiving payments
} SystemConfig;

void load_system_config(void);
SystemConfig* get_system_config(void);