#include "system_config.h"
#include <string.h>
#include <stdbool.h>

static SystemConfig current_system;

void load_system_config(void) 
{
    // Later, you will read these from NVS flash or an RS485 API fetch. 
    // For now, we mock them based on your design.
    strcpy(current_system.company_name, "CZAR METRIC SYSTEM PVT. LTD.");
    strcpy(current_system.dispenser_id, "DU-01");
    strcpy(current_system.serial_number, "SN: 998822A");

    strcpy(current_system.upi_id, "9265150419@upi"); // Replace with your actual UPI ID
}

SystemConfig* get_system_config(void) 
{
    return &current_system;
}


// Store this securely. Eventually, you can load this from ESP32 NVS flash.
static char current_admin_pin[16] = "123456"; 

const char* get_admin_pin(void) 
{
    return current_admin_pin;
}

bool verify_admin_pin(const char* entered_pin) 
{
    if (entered_pin == NULL) return false;
    
    // Compare the strings. Returns true if they match exactly.
    return (strcmp(entered_pin, current_admin_pin) == 0);
}