# 6. Building and Flashing

This project is built using the Espressif IoT Development Framework (ESP-IDF). To compile and flash the firmware to your ESP32-S3 device, you will need to have the ESP-IDF environment set up correctly.

## Prerequisites

1.  **ESP-IDF Setup:** Ensure you have a working installation of the ESP-IDF. Please follow the official [ESP-IDF Get Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/index.html) for your operating system.
2.  **Hardware:** An ESP32-S3 development board (the target for this project is the Waveshare ESP32-S3 4.3" RGB LCD board).
3.  **USB Cable:** For flashing and monitoring.

## Configuration

Before building, you may want to configure the project. The most common configuration is setting the correct serial port for flashing.

1.  **Open the ESP-IDF Terminal:** Launch an ESP-IDF terminal that has the environment variables set up.
2.  **Navigate to the Project Directory:**
    ```bash
    cd path/to/Self_Service_Fuel_Dispenser_ESP-IDF
    ```
3.  **Launch Menuconfig:**
    ```bash
    idf.py menuconfig
    ```
4.  **Set Target Chip:**
    *   Navigate to `Component config` -> `ESP-IDF Settings` -> `Target chip` and ensure it is set to `ESP32-S3`.
5.  **Save and Exit:** Save your configuration and exit `menuconfig`.

## Building the Project

To compile the application, run the following command in the ESP-IDF terminal from the project's root directory:

```bash
idf.py build
```

This command will compile all the source files and generate the final binary firmware file in the `build/` directory.

## Flashing the Firmware

1.  **Connect the Device:** Connect your ESP32-S3 board to your computer via USB.
2.  **Run the Flash Command:** From the project's root directory, run the following command. This will build the project (if necessary), flash the firmware to the device, and then start a serial monitor.

    ```bash
    idf.py flash monitor
    ```

    *   If you have multiple serial ports, you may need to specify the port explicitly:
        ```bash
        idf.py -p YOUR_PORT_HERE flash monitor
        ```
        (e.g., `idf.py -p COM3 flash monitor` on Windows or `idf.py -p /dev/ttyUSB0 flash monitor` on Linux).

3.  **Reset the Device:** The `flash` command should automatically reset the device. If the application doesn't start, you may need to press the `RST` button on your board.

You should now see the application's log output in the serial monitor, and the UI should appear on the LCD screen.
