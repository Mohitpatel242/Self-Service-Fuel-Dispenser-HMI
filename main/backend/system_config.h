#pragma once

#include <stdbool.h>

typedef struct {
    char company_name[64];
    char upi_id[64]; // NEW: For receiving payments
    char target_url[128]; // NEW: For API endpoint
    char ssid[32]; // NEW: For Wi-Fi SSID
    char pass[16]; // NEW: For Wi-Fi Password

} SystemConfig;

void load_system_config(void);
SystemConfig* get_system_config(void);

// --- Security APIs ---
const char* get_admin_pin(void);
bool verify_admin_pin(const char* entered_pin);

// char* get_target_url(void);
