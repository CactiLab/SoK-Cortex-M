**Nucleo F446RE**

This folder contains tests and documentation for the Nucleo F446RE
development board.

**Purpose**

-   To demonstrate Cortex-M features specific to the Nucleo F446RE
    board.

-   To organize test suites and relevant files for this board.

**Getting Started**

-   Load the appropriate default project files into STM32CubeIDE

**Nucleo F446RE**

**Nucleo_f\>Core\>Src\>MPU_Config.c**

**Configuration Steps and Initial Assignments**

1.  **Disable the MPU:**

    -   Disables the MPU to allow modifications.

MPU-\>CTRL &= \~MPU_CTRL_ENABLE_Msk;

2.  **Configure Region 0:**

    -   **Region Number:** Set to 0 (Region 0 selected).

MPU-\>RNR = 0;

-   **Base Address:** Set to the start of SRAM (0x20000000).

MPU-\>RBAR = 0x20000000;

-   **Region Attributes and Size:**

    -   **Access Rights:** Privileged Read-Only, No User Access.

    -   **Memory Type:** Normal Memory.

    -   **Region Size:** 32 KB (0b011).

    -   **Region Enable:** Enabled.

MPU-\>RASR = (0b01 \<\< MPU_RASR_AP_Pos) \| // Privileged read-only

(0b000 \<\< MPU_RASR_TEX_Pos) \| // Normal memory

(0b011 \<\< MPU_RASR_SIZE_Pos) \| // 32 KB size

(1 \<\< MPU_RASR_ENABLE_Pos); // Enable region

3.  **Enable MPU and Set Default Memory Map:**

    -   Enables the MPU and retains the default memory mapping for
        undefined regions.

MPU-\>CTRL \|= MPU_CTRL_ENABLE_Msk \| MPU_CTRL_PRIVDEFENA_Msk;

4.  **Enable Memory Management Faults:**

    -   Configures the system to generate a fault for invalid memory
        access.

SCB-\>SHCSR \|= SCB_SHCSR_MEMFAULTENA_Msk;

5.  **Memory Barriers:**

    -   Ensures the configuration takes effect using data and
        instruction synchronization barriers.

DSB();

ISB();

///////////////////////*nucleo_UART_console_connection 1/21/2025*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

Open Console\>Command shell console

Note To Self: Default settings (do not open multiple console windows
without disconnecting the board)

Serial port:COM5

Baud Rate:115200

Data Size: 8

Parity: None

Stop Bits: 1

*** Data Watch Point and Trace 1/22/2024 ***

// Data Watch Point and Trace is a hardware feature we  
are measuring the number of clock cycles it takes to execute 
a specific function (in this case, delay_function()).

//A clock cycle is the basic unit of time for a processor. 
Each instruction and operation on the microcontroller 
takes a certain number of 
clock cycles to complete.
By counting the number of clock cycles between the start and end of a function, 
we can measure how long that function takes to execute in terms of CPU time. 
This is useful for profiling, debugging, and optimizing code

// UART (Universal Asynchronous Receiver-Transmitter) is 
being used for serial communication 
to send the measured clock cycle count (cycles_taken) 
to a terminal or another device (e.g., PC).

// Function used for enabling the registers for clock cycle calculation 

void DWT_EnableCycleCounter(void) 
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable DWT
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;           // Enable Cycle Counter
    DWT->CYCCNT = 0;                               // Reset Cycle Counter
}
///////////////////////*nucleo_monocipher  1/27/2025*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
Main Variables in the Monocipher 
This code demonstrates the implementation of ChaCha20 encryption using the Monocipher API. At its core, the code encrypts and decrypts data securely by transforming it into an unreadable format that only authorized users (those with the correct key) can reverse.

Think of it like a digital "lock and key" system, where sensitive information (like messages, files, or passwords) is locked up (encrypted) so that unauthorized users can't access it. Here's a simplified explanation:

Encryption Setup:
The program starts by initializing ChaCha20, a secure and fast algorithm designed to scramble data using a combination of a secret key, initial value (nonce), and a message.

ChaCha20 Algorithm:
The ChaCha20 algorithm works by taking the input message and applying multiple rounds of mathematical transformations. These transformations ensure that the output (the encrypted message) looks completely random and cannot be easily guessed or decoded.

Encryption Process:
The program encrypts the original message using the ChaCha20 algorithm. The result is encrypted data (ciphertext) that would appear meaningless to anyone without the secret key and nonce.

Decryption Process:
The encrypted data can be passed through the algorithm again (with the same key and nonce) to "unscramble" it back into its original form. This is how authorized users retrieve the original data safely.
Implementation Using ChaCha20 Encryption
key[] (Secret Key):

Purpose: This is the primary "lock" or "password" used for encrypting and decrypting the data.
Explanation: The key is a 256-bit array (usually 32 bytes), randomly generated or securely provided, that only authorized users should know. Without the correct key, decryption of the encrypted data is virtually impossible.
nonce[] (Initialization Vector/Nonce):

Purpose: A unique random value used during encryption to ensure that the same message, when encrypted multiple times, will produce different ciphertexts each time.
Explanation: The nonce ensures data uniqueness and prevents replay attacks. Even with the same key, using different nonces guarantees that the encryption process will create different outputs for identical inputs.
plaintext[] (Input Data or Message):

Purpose: This variable contains the original message or data that needs to be encrypted.
Explanation: Before encryption, this is readable text or data. Once passed through the ChaCha20 algorithm, it is transformed into ciphertext (encrypted text). Examples of plaintext include messages, file contents, or any sensitive information.
ciphertext[] (Encrypted Output):

Purpose: Stores the result of encrypting the plaintext using the ChaCha20 algorithm.
Explanation: The ciphertext is a scrambled, unreadable form of the original message. Only users with the correct key and nonce can decrypt it. The ciphertext protects sensitive data during storage or transmission.
monocipher_chacha20_encrypt() or similar function:

Purpose: This function performs the actual encryption, transforming the plaintext into ciphertext.
Explanation: It takes in the key, nonce, and plaintext and outputs the encrypted message. Internally, it executes multiple rounds of the ChaCha20 algorithm to ensure data security.
monocipher_chacha20_decrypt() (or a decryption equivalent):

Purpose: This function reverses the encryption process, recovering the original message from the ciphertext.
Explanation: With the same key and nonce, the encrypted data is unscrambled back to its original form (plaintext).
message_length (Length of Data):

Purpose: Tracks how much data is being encrypted or decrypted.
Explanation: The algorithm needs to know the size of the message to encrypt/decrypt the correct portion of memory. Incorrect lengths could lead to errors or partial encryption.
output_buffer[] (Temporary Storage):

Purpose: Temporarily holds encrypted or decrypted data during processing.
Explanation: Encryption and decryption may require intermediate storage as ChaCha20 operates on fixed-size blocks of data.
These variables, combined, implement a secure encryption pipeline where the key and nonce provide security, and the plaintext and ciphertext ensure data is protected during communication or storage.




///////////////////////*nucleo_f to f6 series  1/30/2025*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


In the MPU Configuration across the directories F1 to F6, we experimented with different base addresses, sizes, and permission sets to demonstrate the full capabilities of the ARM Memory Protection Unit (MPU). Here’s how each region was configured:

Region Configurations and Access Permissions
Directory	Region	Base Address	Size	Permissions (AP[2:0])	Description
F1	Region 0	0x20000000	32 KB	000 (No access)	No access for both privileged and unprivileged modes
F2	Region 1	0x20004000	32 KB	001 (Privileged R/W)	Privileged read/write, unprivileged no access
F3	Region 1	0x20004000	32 KB	010 (Privileged R/W, Unprivileged Read)	Unprivileged read-only, privileged read/write
F4	Region 0	0x20000000	32 KB	011 (Full access)	Full read/write access for both privileged and unprivileged modes
F5	Region 1	0x20004000	32 KB	101 (Privileged Read-Only)	Privileged read-only, unprivileged no access
F6	Region 0	0x20000000	32 KB	011 (Full access)	Full access for general-purpose memory
F6	Region 1	0x20004000	32 KB	001 (Privileged R/W)	Privileged access only (critical data protection)
F6	Region 2	0x08000000	1 MB (Flash)	110 (Read-only)	Read-only access for both privileged and unprivileged modes
How Regions Were Configured:
1. Base Addresses:
Each region was assigned a base address aligned to the size of the region. For example:

Region 0: 0x20000000 (start of SRAM)
Region 1: 0x20004000 (next section of SRAM)
Region 2: 0x08000000 (start of Flash memory)
2. Sizes:
The size of the regions was configured using the MPU_RASR_SIZE_Pos field. Sizes included:

32 KB for Region 0 and Region 1 (SRAM)
1 MB for Region 2 (Flash)
Example configuration:


MPU->RNR = 0;                        // Select Region 0
MPU->RBAR = 0x20000000;              // Base address for Region 0
MPU->RASR = (0b011 << MPU_RASR_AP_Pos) |  // Full access (privileged/unprivileged)
            (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB region size
            (0b1 << MPU_RASR_ENABLE_Pos); // Enable Region 0
3. Access Permissions:
Access permissions were configured using the MPU_RASR_AP_Pos field according to the AP[2:0] settings:

Region 0 (AP[2:0] = 011): Full access for testing unrestricted memory operations.
Region 1 (AP[2:0] = 001, 010, or 101 in different directories): Used for restricted access testing.
Region 2 (AP[2:0] = 110): Read-only configuration to prevent accidental writes to Flash.
What Has Been Implemented:
Disable MPU Before Configuration


MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;
Configure Regions
Each region is configured with its base address, size, and access permissions using RBAR (Region Base Address Register) and RASR (Region Attribute and Size Register).

Example for Region 1:


MPU->RNR = 1;                        // Select Region 1
MPU->RBAR = 0x20004000;              // Base address
MPU->RASR = (0b001 << MPU_RASR_AP_Pos) |  // Privileged R/W, unprivileged no access
            (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB size
            (0b1 << MPU_RASR_ENABLE_Pos); // Enable region
Enable MPU and Apply Memory Barriers
Once all regions are configured, the MPU is enabled with the default memory map for undefined regions:


MPU->CTRL |= MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;
__DSB();  // Data synchronization barrier
__ISB();  // Instruction synchronization barrier


Access Permissions from the Table
AP[2:0]	Privileged Access	Unprivileged Access	Notes
000	No access	No access	Any access generates a permission fault
001	Read/Write	No access	Privileged access only
010	Read/Write	Read-only	Any unprivileged write generates a permission fault
011	Read/Write	Read/Write	Full access
100	Unpredictable	Unpredictable	Reserved (not used)
101	Read-only	No access	Privileged read-only
110	Read-only	Read-only	Privileged and unprivileged read-only
111	Read-only	Read-only	Privileged and unprivileged read-only
 

 General Structure of Each Directory
MPU Configuration (MPU_Config.c): Sets up different MPU regions with the desired access permissions based on the directory.
Main Program (main.c): Implements tests to validate memory access:
Reading and writing to each region.
Toggling LEDs or triggering MemManage faults based on outcomes.
Example configuration in F2 (AP[2:0] = 001):


MPU->RNR = 1;                        // Select Region 1
MPU->RBAR = 0x20004000;              // Base address for the region
MPU->RASR = (0b001 << MPU_RASR_AP_Pos) |  // Privileged read/write, unprivileged no access
            (0x8 << MPU_RASR_SIZE_Pos) |  // 32 KB region size
            (0b1 << MPU_RASR_ENABLE_Pos); // Enable region
Main Focus of Each Directory
Demonstrate the effect of different access levels:
What happens when privileged and unprivileged modes try to access protected memory regions?
Handle faults appropriately using MemManage_Handler():
Provide feedback (using LEDs) when access violations occur.
Testing Example in main.c
For each directory, the main test logic follows this pattern:

Write or read in privileged mode.
Switch to unprivileged mode and attempt the same operation.
Observe if the access succeeds or triggers a fault.
Example:


// Switch to unprivileged mode
__set_CONTROL(__get_CONTROL() | 0x01);
__ISB();  // Synchronize the instruction pipeline

// Attempt to write to Region 1 (should trigger a fault)
*sram_region1 = 0xCAFEBABE;
Summary of Key Implementations
F1 to F6 progressively demonstrate the range of MPU access permissions.
They provide a practical understanding of how to protect critical memory regions using the MPU.



///////////////////////*nucleo_Interrupt_Handler  1/29/2025*\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

In the given STM32 HAL-based code, the NVIC (Nested Vector Interrupt Controller) isn't explicitly configured, but we can see its role in managing interrupts, especially when the button is configured to trigger an interrupt using B1_Pin.It allows external and internal interrupts to be processed with different priority levels, and higher-priority interrupts can preempt lower-priority ones.
NVIC is designed to minimize latency and enhance real-time responsiveness, crucial for embedded systems.Enabling NVIC for the interrupt source: When HAL_GPIO_Init() configures the button as an interrupt source, it automatically:

Configures the EXTI (External Interrupt Controller).
Enables the NVIC entry for the corresponding EXTI line.
In the provided code, the line that specifies which button to trigger the interrupt is within the GPIO initialization function, specifically this part:

HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
is a function call in STM32’s HAL library to control a GPIO pin and is commonly used for controlling LEDs, relays, or other digital outputs. Let’s break it down:

Explanation of Parameters
HAL_GPIO_WritePin(): This is a function provided by the STM32 HAL library to set the state of a GPIO pin (either high or low).

LD2_GPIO_Port: Refers to the GPIO port associated with LD2, which is the built-in LED on many STM32 development boards (such as Nucleo boards). This could refer to ports like GPIOA, GPIOB, etc.

LD2_Pin: Refers to the specific pin on the port where the LED (or device) is connected. For example, Pin 5 of GPIOA.

GPIO_PIN_RESET: This parameter specifies the logic level to write to the pin:

GPIO_PIN_RESET means set the pin to low (0V).
This turns off the LED because many LEDs are active high (they light up when the pin is high and turn off when the pin is low).
What is Happening in the Code
The microcontroller sends a low signal to the pin connected to the LD2 LED.
This causes the LED to turn off.