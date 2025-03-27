# Nucleo F446RE Development Board

**Link to the Google Drive Document:**  
[Google Drive Link](https://uccsoffice365-my.sharepoint.com/:w:/g/personal/apadiyal_uccs_edu/EbtBpDt2p7dPn8wQoQ-r1awBHwUQqn1_jXAvR8i2_dfkiA?e=0oHe4r)

This folder contains tests and documentation for the Nucleo F446RE development board.

---

## Purpose
- Demonstrate Cortex-M features specific to the Nucleo F446RE board.
- Organize test suites and relevant files for this board.

---

## Getting Started
- Load the appropriate default project files into **STM32CubeIDE**.

---

## MPU Configuration Overview

### **Configuration Steps and Initial Assignments**
1. **Disabling the MPU**  
   The MPU (Memory Protection Unit) is initially disabled to allow configuration changes.  

2. **Configuring Memory Regions**  
   Region 0 is selected and configured to cover the start of SRAM. The attributes include access permissions, memory type, and region size to ensure safe and controlled memory access.  

3. **Enabling MPU and Default Memory Mapping**  
   After configuration, the MPU is enabled, and the system retains a default memory map for regions not explicitly defined.  

4. **Enabling Memory Fault Handling**  
   The system is configured to trigger faults when any illegal or unauthorized memory access occurs, aiding in debugging and protection.  

5. **Applying Memory Barriers**  
   Data and instruction synchronization barriers ensure the MPU configuration takes effect correctly across the system.

### **Peripherals and Functionalities Table**
The table below summarizes the key peripherals used in this project and their respective functionalities.

| **Peripheral** | **Description** | **Function Name** |
|--------------|---------------------------------------------|-----------------------------|
| **GPIO (General-Purpose Input/Output)** | Controls the LED (LD2) and configures the button (B1) as an input with an interrupt. | `MX_GPIO_Init()` |
| **USART2 (UART)** | Handles serial communication, transmits "Cycles taken" messages. | `MX_USART2_UART_Init()` |
| **EXTI (External Interrupt)** | Detects button press and triggers an interrupt to toggle the LED. | `HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)` |
| **System Clock** | Configures the system clock using the internal HSI oscillator with PLL. | `SystemClock_Config()` |
| **Error Handling** | Handles system errors by disabling interrupts and entering an infinite loop. | `Error_Handler()` |
| **DWT (Data Watchpoint and Trace Unit)** | Enables and configures the cycle counter for measuring execution time. | `DWT_EnableCycleCounter()` |
| **Delay Function** | Implements a simple dummy delay loop for profiling. | `delay_function()` |
| **Cycle Counter Measurement** | Measures CPU cycles taken by a function and sends the result over UART. | `main()` |
| **UART (USART2)** | Transmits `"Hello, UART!"` message five times over UART2 with a 1-second delay. | `HAL_UART_Transmit()` |
| **Delay Function** | Introduces a 1-second delay between UART transmissions to prevent flooding. | `HAL_Delay(1000)` |
| **Basic Peripheral Initialization** | Initializes system clock, GPIO, and UART without additional runtime behavior. | `main()` (idle loop) |
| **ChaCha20 Encryption** | Encrypts a plaintext message using ChaCha20. | `crypto_chacha20_x()` |
| **UART Transmission of Encrypted Data** | Converts encrypted data to hex format and sends it via UART. | `HAL_UART_Transmit()` |
| **UART Helper Function** | Simplifies sending strings over UART. | `UART_SendString()` |
| **MPU Configuration** | Configures the **Memory Protection Unit (MPU)** to define access permissions for memory regions. | `MPU_Config()` |
| **Privilege Mode Switching** | Switches execution from **privileged mode** to **unprivileged mode** and back to test MPU settings. | `test_p_up_rw()` |
| **MPU Testing Function** | Calls MPU test functions and toggles LED based on memory access success/failure. | `test_mpu()` |
| **Supervisor Call (SVC) Execution** | Executes an **SVC (Supervisor Call) instruction**, typically used to request privileged services from the OS or firmware. | `test_svc()` |

---

## Console Setup

### **UART Console Configuration**
- The UART interface is used for serial communication between the development board and an external device (like a PC).  
- Key settings include the serial port, baud rate, data size, parity, and stop bits to ensure correct data transmission.

---

## Data Watch Point and Trace (`DWT`)

### **Purpose:**  
This hardware feature is used to measure the number of clock cycles required to execute specific functions, providing insights into performance and efficiency.

### **How It Works:**  
- The cycle counter is enabled and reset to measure the time taken by a function from start to finish.  
- The cycle count is then sent via UART to an external terminal or device for analysis.  
- This data is critical for profiling, debugging, and optimizing the code.

---

## ChaCha20 Encryption (`Monocipher`)

### **Overview:**  
ChaCha20 is a secure and efficient algorithm used to encrypt and decrypt data. It ensures confidentiality during data storage and transmission by transforming plaintext into unreadable ciphertext.

### **Main Components:**
- **Secret Key (`key[]`):** A 256-bit key that only authorized users possess. It is used to encrypt and decrypt the data securely.  
- **Nonce (`nonce[]`):** A unique value that ensures encryption uniqueness, even when the same data is encrypted multiple times.  
- **Plaintext (`plaintext[]`):** The original readable data that needs encryption.  
- **Ciphertext (`ciphertext[]`):** The encrypted version of the plaintext, which is unreadable without the key and nonce.  
- **Encryption and Decryption Functions:** The encryption process scrambles the plaintext into ciphertext, and the decryption process reverses it using the same key and nonce.  
- **Message Length:** Ensures that the correct amount of data is processed without errors or truncation.

---

## Memory Region Configurations (`F1` to F3)

### **Purpose:**  
Each directory (`F1` to `F6`) demonstrates the MPU's ability to control memory access by varying base addresses, sizes, and permission settings.

### **Region Settings Overview:**
- **Base Addresses:** Assigned to specific memory regions like SRAM or Flash.  
- **Region Sizes:** Defines the size of each memory region, typically 32 KB for SRAM and 1 MB for Flash memory.  
- **Access Permissions:** Ranges from no access to full access, depending on the security level needed.

### **Fault Handling:**  
Attempts to access restricted regions trigger memory faults, which are handled using exception handlers to prevent system crashes.

---

## NVIC and GPIO Interrupts

### **Purpose:**  
The NVIC (Nested Vector Interrupt Controller) manages interrupts with different priority levels, ensuring real-time responsiveness.

### **Functionality:**  
- External interrupts (like button presses) are configured through GPIO pins.  
- The NVIC prioritizes and handles these interrupts efficiently.  
- LEDs or other indicators are used to provide feedback on interrupt handling.

---

## SVC Handler Implementation

### **Overview**
The **Supervisor Call (SVC) handler** is implemented to allow controlled execution of privileged operations. The handler reads the **SVC number** from the triggering instruction and executes the corresponding function. This enables secure system calls for handling **hardware interactions** and **power management** in embedded applications.

### **SVC Test Cases Implemented**

The following table outlines two test cases implemented using the **SVC handler**:

| **SVC Number** | **Operation**                  | **Description** |
|--------------|--------------------------------|------------------|
| **SVC #0**  | Toggle LED (PA5)              | Toggles the built-in LED on the STM32 board. |
| **SVC #6**  | Enter Sleep Mode              | Puts the microcontroller into low-power sleep mode until woken by an interrupt. |

These test cases demonstrate how **SVC calls** can be used to manage system behavior in a structured and secure way.


