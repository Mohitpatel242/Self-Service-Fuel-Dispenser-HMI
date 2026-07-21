file(GLOB_RECURSE project_sources "*.c")

idf_component_register(
                    
                    SRCS ${project_sources}
      

                    INCLUDE_DIRS "." "ui" "hardware" "backend" "frontend" "comms" 
                    REQUIRES 
                        esp_lcd 
                        driver 
                        lvgl 
                        json            
                        esp_wifi        # This fixes the missing esp_wifi.h error
                        esp_event       # Required for Wi-Fi events
                        esp_netif       # Required for TCP/IP networking
                        esp_http_client # You will need this for your web server polling
                        nvs_flash
                       )

idf_component_get_property(lvgl_lib lvgl__lvgl COMPONENT_LIB)
target_compile_options(${lvgl_lib} PRIVATE -Wno-format)