# Nucleo F446RE

This folder contains tests and documentation for the Nucleo F446RE development board.

## Purpose
- To demonstrate Cortex-M features specific to the Nucleo F446RE board.
- To organize test suites and relevant files for this board.

## Getting Started
- Load the appropriate default project files into STM32CubeIDE
# Nucleo F446RE
file= nucleo_f>Core>Src>MPU_Config.c
Function Overview
Function Name: MPU_Config
Purpose: To configure MPU regions, set memory attributes, and enable the MPU with specific protections.
Parameter Assignments
Disable MPU:

MPU is disabled initially.
Region 0 Configuration:

Region Number: 0
Base Address: 0x20000000 (Start of SRAM)
Access Rights: Privileged Read-Only, No User Access
Memory Type: Normal Memory
Region Size: 32 KB
Region Enable: Enabled
Enable MPU and Default Memory Map:

MPU is enabled.
Default memory map is retained for undefined regions.
Fault Handling:

Memory management faults are enabled for invalid memory access.
Memory Barriers:

Data and Instruction Synchronization Barriers are applied to ensure the changes take effect.
*******************************************************************************************************************************************************************************************************************************************************************************
