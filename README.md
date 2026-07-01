# Self-Service Fuel Dispenser ESP-IDF - Documentation

This document provides a detailed overview of the Self-Service Fuel Dispenser project, built using the ESP-IDF framework and the LVGL library for the user interface.

## Table of Contents

1.  [Project Overview](./documentation/1_Project_Overview.md)
2.  [System Architecture](./documentation/2_System_Architecture.md)
3.  [Screen Flow & UI Logic](./documentation/3_Screen_Flow_and_UI_Logic.md)
4.  [Backend](./documentation/4_Backend.md)
5.  [Hardware Abstraction Layer](./documentation/5_Hardware_Abstraction_Layer.md)
6.  [Building and Flashing](./documentation/6_Building_and_Flashing.md)

### Hardware Connection

The connection between ESP Board and the LCD is as follows:

```
       ESP Board                           RGB  Panel
+-----------------------+              +-------------------+
|                   GND +--------------+GND                |
|                       |              |                   |
|                   3V3 +--------------+VCC                |
|                       |              |                   |
|                   PCLK+--------------+PCLK               |
|                       |              |                   |
|             DATA[15:0]+--------------+DATA[15:0]         |
|                       |              |                   |
|                  HSYNC+--------------+HSYNC              |
|                       |              |                   |
|                  VSYNC+--------------+VSYNC              |
|                       |              |                   |
|                     DE+--------------+DE                 |
|                       |              |                   |
|               BK_LIGHT+--------------+BLK                |
+-----------------------+              |                   |
                               3V3-----+DISP_EN            |
                                       |                   |
                                       +-------------------+
```

* The LCD parameters and GPIO number used by this example can be changed in [example_rgb_avoid_tearing.c](main/example_rgb_avoid_tearing.c). Especially, please pay attention to the **vendor specific initialization**, it can be different between manufacturers and should consult the LCD supplier for initialization sequence code.
* The LVGL parameters can be changed not only through `menuconfig` but also directly in `lvgl_conf.h`

### Configure the Project

Run `idf.py menuconfig` and navigate to `Example Configuration` menu.

### Build and Flash

Run `idf.py set-target esp32s3` to select the target chip.

Run `idf.py -p PORT build flash monitor` to build, flash and monitor the project. A fancy animation will show up on the LCD as expected.

The first time you run `idf.py` for the example will cost extra time as the build system needs to address the component dependencies and downloads the missing components from registry into `managed_components` folder.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

## Troubleshooting

For any technical queries, please open an [issue](https://github.com/espressif/esp-iot-solution/issues) on GitHub. We will get back to you soon.
