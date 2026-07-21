#pragma once

// Initializes the ESP32 Wi-Fi station and connects to the target network
void wifi_network_init(const char* ssid, const char* password);

// Starts the background FreeRTOS task to fetch live data from 192.168.4.1
void start_live_data_monitor(void);