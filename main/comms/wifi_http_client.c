#include "wifi_http_client.h"
#include "json_parser.h"      // To pass the data once downloaded
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "WIFI_HTTP";
static const char *TARGET_URL = "http://192.168.4.1/GET_DU_STATUS";
// NEW: Global flag to track Wi-Fi state
volatile bool is_wifi_connected = false;


// --- 1. Wi-Fi Event Handler ---
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) 
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        is_wifi_connected = false; // NEW: Block HTTP requests
        ESP_LOGW(TAG, "Wi-Fi disconnected. Retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Connected! IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        is_wifi_connected = true; // NEW: Allow HTTP requests!
    }
}


// --- 2. HTTP Event Handler (Collects the JSON chunks) ---
esp_err_t http_event_handler(esp_http_client_event_t *evt) 
{
    static char *json_buffer = NULL;
    static int json_len = 0;

    switch(evt->event_id) {
        case HTTP_EVENT_ON_CONNECTED:
            // Allocate memory for the incoming JSON when connection opens
            json_buffer = calloc(1, 4096); // 4KB buffer (adjust if JSON is larger)
            json_len = 0;
            break;
            
        case HTTP_EVENT_ON_DATA:
            // Append incoming data chunks to our buffer
            if (json_buffer != NULL) {
                memcpy(json_buffer + json_len, evt->data, evt->data_len);
                json_len += evt->data_len;
            }
            break;
            
        case HTTP_EVENT_ON_FINISH:
            // Data is fully downloaded. Pass it to the parser!
            if (json_buffer != NULL) {
                // Null-terminate the string safely
                json_buffer[json_len] = '\0'; 
                
                // Call the parser (defined in json_parser.c)
                // Passing target "DU-01" and Display "1" as an example
                parse_live_data(json_buffer); 
                
                free(json_buffer);
                json_buffer = NULL;
            }
            break;
            
        case HTTP_EVENT_DISCONNECTED:
            // Clean up memory if connection drops unexpectedly
            if (json_buffer != NULL) {
                free(json_buffer);
                json_buffer = NULL;
            }
            break;
            
        default:
            break;
    }
    return ESP_OK;
}

// --- 3. The Background Fetching Task ---
// static void http_get_task(void *pvParameters) 
static void http_get_task(void *pvParameters) 
{
    ESP_LOGI(TAG, "Starting HTTP GET Monitor Task...");

    esp_http_client_config_t config = {
        .url = TARGET_URL,
        .event_handler = http_event_handler,
        .timeout_ms = 1500, 
    };

    while (1) {
        // NEW: Do not attempt to connect if Wi-Fi is down!
        if (!is_wifi_connected) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_err_t err = esp_http_client_perform(client);
        
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --- 4. Public Initialization APIs ---
void wifi_network_init(const char* ssid, const char* password) 
{
    // NVS is required by the Wi-Fi driver to store calibration data
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .threshold = { .authmode = WIFI_AUTH_WPA2_PSK },
        },
    };
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void start_live_data_monitor(void) 
{
    // Spin up the background task with a generous stack size for HTTP processing
    xTaskCreate(http_get_task, "http_get_task", 8192, NULL, 5, NULL);
}