# 4. Backend

The backend is the core of the application, responsible for managing data and state. It is designed to be completely independent of the UI, which allows for easier testing and maintenance. The backend consists of two main parts: the Data Model and the System Configuration.

## 4.1. Data Model (`data_model.h`)

The data model defines the structures and APIs for handling all transaction-related data.

### Enums

The data model defines several enums to represent the state of the transaction:

*   **`DispenseMode`**: Represents the user's choice for presetting the transaction.
    *   `MODE_NONE`: Default state.
    *   `MODE_AMOUNT`: The user is presetting a monetary value.
    *   `MODE_VOLUME`: The user is presetting a fuel volume.
*   **`PaymentMethod`**: Represents the user's selected payment method.
    *   `PAYMENT_NONE`: Default state.
    *   `PAYMENT_QR`: QR code payment.
    *   `PAYMENT_CARD`: Credit/Debit card payment.

### Structures

*   **`NozzleData`**: This structure holds all the information related to a single fuel nozzle.
    ```c
    typedef struct {
        uint8_t id;
        char product_name[32];
        float rate;
        float density;
        uint32_t color_hex;
    } NozzleData;
    ```
    *   `id`: A unique identifier for the nozzle.
    *   `product_name`: The name of the fuel (e.g., "PETROL", "DIESEL").
    *   `rate`: The price of the fuel per liter.
    *   `density`: The density of the fuel.
    *   `color_hex`: A hexadecimal color code associated with the product for UI purposes.

### Key Functions

*   `load_nozzle_data()`: Loads the nozzle information from a storage source (in the current implementation, this is hardcoded in `data_model.c` for demonstration).
*   `get_nozzle(int index)`: Retrieves a nozzle by its index.
*   `set_active_transaction_nozzle(int nozzle_id)`: Stores the nozzle selected by the user for the current transaction.
*   `get_active_transaction_nozzle()`: Retrieves the currently active nozzle.
*   `set_transaction_mode(DispenseMode mode)`: Sets whether the transaction is by amount or volume.
*   `get_transaction_mode()`: Gets the current transaction mode.
*   `set_transaction_value(float value)`: Stores the amount or volume entered by the user.
*   `get_transaction_value()`: Retrieves the value entered by the user.
*   `set_payment_method(PaymentMethod method)`: Stores the selected payment method.
*   `get_payment_method()`: Retrieves the current payment method.

## 4.2. System Configuration (`system_config.h`)

This part of the backend manages static configuration data for the dispenser unit itself.

### Structures

*   **`SystemConfig`**: Holds system-wide identifiers and settings.
    ```c
    typedef struct {
        char company_name[64];
        char dispenser_id[16];
        char serial_number[32];
        char upi_id[64];
    } SystemConfig;
    ```
    *   `company_name`: The name of the fuel station or company.
    *   `dispenser_id`: A unique ID for the dispenser unit (e.g., "DU-01").
    *   `serial_number`: The serial number of the hardware.
    *   `upi_id`: The UPI ID to which QR code payments should be directed.

### Key Functions

*   `load_system_config()`: Loads the system configuration. Similar to the nozzle data, this is currently hardcoded in `system_config.c`.
*   `get_system_config()`: Retrieves a pointer to the loaded system configuration data.
