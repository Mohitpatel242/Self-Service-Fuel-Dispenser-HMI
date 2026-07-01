# 1. Project Overview

The Self-Service Fuel Dispenser is an embedded system designed to provide a modern, user-friendly interface for fuel dispensing operations. It runs on an ESP32-S3 microcontroller and utilizes the ESP-IDF framework for its core functionalities.

The user interface is built with the open-source LVGL (Light and Versatile Graphics Library), enabling a rich and responsive touch-based experience. The UI design and layout have been facilitated by EEZ Studio, which generates the necessary C code for screens and widgets.

## Key Features

*   **Touch-based UI:** A modern and intuitive touch screen interface for seamless user interaction.
*   **Multiple Dispensing Modes:** Supports presetting the fueling amount either by a specific monetary value (e.g., $50) or by a specific volume (e.g., 20 liters).
*   **Flexible Payment Options:** Offers multiple payment methods, including QR code-based payments and traditional card payments.
*   **Clear Transaction Summary:** Provides a clear confirmation screen for users to review their selection before proceeding to payment and fueling.
*   **Modular Software Architecture:** The code is organized into distinct layers (Hardware, Backend, Frontend, UI), making it easier to maintain, debug, and extend.
