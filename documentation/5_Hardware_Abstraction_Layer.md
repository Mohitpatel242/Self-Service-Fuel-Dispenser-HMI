# 5. Hardware Abstraction Layer (HAL)

The Hardware Abstraction Layer (HAL) in the `hardware/` directory serves to decouple the main application logic from the specific hardware details. This makes the application more portable and easier to adapt to different display controllers or other peripherals in the future.

## 5.1. `hardware_api.h` / `hardware_api.c`

This is the main interface to the HAL. It provides high-level functions that the rest of the application can use without needing to know about the underlying hardware-specific function calls.

### Key Functions

*   **`hardware_init_all()`**: This is the primary function of the HAL. It is called once at startup from `app_main` to initialize all the necessary hardware components for the application to run. In the current implementation, it performs the following actions:
    1.  Initializes the RGB LCD display and the touch controller by calling `waveshare_esp32_s3_rgb_lcd_init()`. This function, located in a lower-level driver, sets up the SPI communication, configures the display controller, and prepares the LVGL drivers.
    2.  Turns on the display backlight by calling `wavesahre_rgb_lcd_bl_on()`.

## 5.2. Low-Level Drivers

The `hardware` directory also contains the specific drivers for the hardware in use. The key driver in this project is for the display:

*   **`waveshare_rgb_lcd_port.h` / `.c`**: This file contains the specific implementation for the Waveshare ESP32-S3 4.3-inch RGB LCD. It handles the low-level details of:
    *   Sending commands and data to the display controller over SPI.
    *   Initializing the touch input driver (e.g., GT911).
    *   Integrating the display and touch drivers with the LVGL library so that LVGL can render graphics and receive touch input.

By having this separation, the main application in `main.c` only needs to call `hardware_init_all()`, and the `ui_manager` can work with LVGL objects, without either of them needing to know the specific model of the display or touch controller being used.
