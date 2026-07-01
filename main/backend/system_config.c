#include "system_config.h"
#include <string.h>

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