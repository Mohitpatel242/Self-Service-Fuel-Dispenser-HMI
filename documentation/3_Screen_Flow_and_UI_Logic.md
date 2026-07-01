# 3. Screen Flow & UI Logic

The user interface is designed to be a step-by-step process, guiding the user from selecting a fuel type to making a payment. The `frontend/ui_manager.c` module orchestrates this flow, preparing the data for each screen before it is displayed.

## 3.1. Application Initialization and Main Screen

1.  **Entry Point:** The application starts with `app_main()` in `main/main.c`.
2.  **UI Manager Start:** `app_main()` calls `start_ui_manager()`.
3.  **UI Initialization:** `start_ui_manager()` creates a dedicated task (`startup_ui_task`) that initializes the LVGL UI by calling `ui_init()` (from `ui/ui.c`).
4.  **Header:** A persistent header is created at the top of the screen using `init_system_header()`. This header is always visible and displays:
    *   Dispenser ID and Serial Number
    *   Company Name
    *   Live Date and Time
5.  **Main Screen (Nozzle Selection):**
    *   The `generate_dynamic_panels()` function is called.
    *   It retrieves the list of available fuel nozzles from the backend (`get_nozzle()`).
    *   For each nozzle, it dynamically creates a widget on the `main_screen` that displays:
        *   Product Name (e.g., "PETROL")
        *   Nozzle ID (e.g., "NOZZLE 1")
        *   Rate (Price per liter)
        *   Density
        *   A "SELECT" button.

**User Action:** The user selects a fuel type by pressing the "SELECT" button on one of the nozzle widgets. This triggers the `on_nozzle_select_clicked` event.

## 3.2. Mode Selection Screen

1.  **Transition:** `on_nozzle_select_clicked` saves the chosen nozzle to the backend and calls `transition_to_mode_select()`.
2.  **Data Population:** This function updates the `mode_select_screen` with the details of the selected nozzle (Product Name, Nozzle ID).
3.  **Display:** The `mode_select_screen` is loaded, presenting the user with two choices:
    *   **"BY AMOUNT":** To preset a specific monetary value.
    *   **"BY VOLUME":** To preset a specific fuel volume.
    *   A "BACK" button to return to the Main Screen.

**User Action:** The user presses either "BY AMOUNT" (`action_mode_amount`) or "BY VOLUME" (`action_mode_volume`).

## 3.3. Numpad (Value Input) Screen

1.  **Transition:** The action function (`action_mode_amount` or `action_mode_volume`) saves the selected mode to the backend and calls `transition_to_numpad()`.
2.  **Data Population:** This function prepares the `numpad_screen`:
    *   It displays the selected nozzle's details (Product, Nozzle ID, Rate, Density).
    *   It sets the input unit label to "Rs." for Amount mode or "Liters" for Volume mode.
    *   It clears any previous input from the text area.
3.  **Display:** The `numpad_screen` is loaded, showing:
    *   An input text area.
    *   An on-screen LVGL keyboard for entering numbers.
    *   A "CONFIRM" button.
    *   A "BACK" button.

**User Action:** The user types the desired value and presses the "CONFIRM" button (`action_numpad_confirm`).

## 3.4. Confirmation Screen

1.  **Transition:** `action_numpad_confirm` saves the entered value to the backend and calls `transition_to_confirm()`.
2.  **Calculation & Data Population:** This is a critical step. `transition_to_confirm()`:
    *   Retrieves the nozzle data, dispense mode, and entered value from the backend.
    *   Calculates the corresponding value (if the user entered an amount, it calculates the volume; if they entered a volume, it calculates the amount).
    *   Populates the `confirm_screen` with a full transaction summary: Product, Nozzle, Rate, Density, Final Amount, and Final Volume.
3.  **Display:** The `confirm_screen` is loaded, showing the complete summary and asking the user to confirm. It has two buttons:
    *   **"PROCEED TO PAY"**
    *   **"BACK"**

**User Action:** The user verifies the details and presses "PROCEED TO PAY" (`action_confirm_pay`).

## 3.5. Payment Method Screen

1.  **Transition:** `action_confirm_pay` loads the `payment_screen`.
2.  **Display:** This screen gives the user a choice of payment methods:
    *   A selectable "Pay by QR" panel (`action_select_qr`).
    *   A selectable "Pay by Card" panel (`action_select_card`).
    *   A "CONFIRM" button.
    *   A "BACK" button.

**User Action:** The user selects a method (which highlights their choice) and presses "CONFIRM" (`action_payment_method_confirm`).

## 3.6. Payment Execution Screens

### 3.6.1. QR Code Screen

1.  **Transition:** If the user chose QR, `action_payment_method_confirm` calls `transition_to_qr_screen()`.
2.  **QR Code Generation:** This function:
    *   Constructs a standard UPI (Unified Payments Interface) string containing the payment details (UPI ID from config, company name, final amount).
    *   Uses the `lv_qrcode` library to generate and display a QR code image from this string.
    *   Displays the final amount to be paid.
3.  **Display:** The `qr_screen` is shown with the QR code. It contains a "COMPLETED" and "BACK" button.

### 3.6.2. Card Screen

If the user chose Card, the `card_screen` is loaded. In the current implementation, this is a placeholder screen.

## 3.7. Dispensing and Completion

The UI flow, as defined in `ui_manager.c` and `actions_impl.c`, concludes after initiating the payment step. There are no explicit screens for "Dispensing in Progress" or "Transaction Complete".

It is presumed that after the payment is confirmed (e.g., after the "COMPLETED" button on the QR screen is pressed), the application logic would:
1.  Verify the payment status through a backend process (not detailed in the UI code).
2.  Authorize the physical dispenser hardware to begin pumping fuel.
3.  Once dispensing is complete, the UI likely returns to the `main_screen` to be ready for the next customer.
