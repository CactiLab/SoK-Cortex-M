# Introduction

Project: **SoK: All You Ever Wanted to Know About Arm
Cortex-M Security but Were Afraid to Ask**

This project tests all kinds of faults, e.g. memory access violations, on a Cortex-M system considering different setups of privilege levels, TrustZone, MPU, SAU, IDAU, MPC, PPC.

- Board: [Arm MPS2+](https://developer.arm.com/tools-and-software/development-boards/fpga-prototyping-boards/mps2)
- IDE: Keil
- Armv8-M: 
  - image: Cortex-M33 (AN505: Cortex-M33 with IoT kit FPGA for MPS2+)
  - project: IOTKit_CM33_S_NS

# File description

File tree
```c
IOTKit_CM33_S_NS
|--IOTKit_CM33_ns
  |--DebugConfig/   /* debug configurations for compiler */
  |--RTE/           /* startup and partitioning */
  |--...
  |--main_ns.c      /*  main function to call the test_cases */
  |--Secure_Function    /* non-secure-callable functions */
  |--test_case_ns.c /* all test cases are declared in this file */
  |--test_ns.c      /* provide basic functions the test cases need */
  |--test_ns.h      /* all macros are defined in this file */
|--IOTKit_CM33_s
  |--DebugConfig/   /* debug configurations for compiler */
  |--RTE/           /* startup and partitioning */
  |--...
  |--main_s.c       /*  main function to call the test_cases */
  |--Secure_Function    /* non-secure-callable functions */
  |--test_case_s.c  /* all test cases are declared in this file */
  |--test_s.c       /* provide basic function the test cases need, including fault handlers */
  |--test_s.h       /* all macros are defined in this file */
|--...
|--README.md
|--Update_log.md
```

# Usage

For each test case, there are macros in the test/test_s/ns.h file.
To test those cases, you can change the value in test_cases_ns/s.c: test_cases() function.
```c
void test_cases()
{
	test_cases_s(TEST_DWT_S);
}
```

The example code suite includes the following components:
1) Helper and configuration functions. These functions provide a C language interface to configure system and peripheral registers. 
The current version include: 
   - exception handlers that retrieve detailed fault information; 
   - functions to enable and disable data and instruction cache; 
   - stack dump function that prints out regular stack and exception stack contents;
   - helper functions to configure and control iv) MPU; v) SAU; vi) MPC; vii) DWT; viii) MTB.

2) Test cases to demonstrate how to use hardware features.
The current version include the following test cases: 
   - change xPSR; 
   - legal and illegal memory accesses; 
   - TT instruction examples; 
   - hardware stacking and unstacking with (m33) and without TrustZone (m3); 
   - instruction tracing with MTB; 
   - performance measurement and data access monitor with DWT; 
   - state transition tests and performance measurement.

## For Cortex-M33

This section explains the macros and tests.
Projects on Cortex-M33 uses TrustZone, there are two projects including the project in the secure state and the non-secure state.

### ***In the secure state***

**Helper and configuration functions**

| Function name (test_s.c)  | Description                                                 | Addition |
| --------------------------| ----------------------------------------------------------- | -------- |
| stackDump                 | dump exception stack                                        |
| sys_recover               | -
| sys_reset                 | -
| clear_stack               | - 
| printxxErrorMsg           | print out error messages
| XX_Hard_Fault_Handler     | handle exceptions                                           |
| SVC_Handler               | Handle SVC call to enable privileged mode                   |        |
| mpu_mpc_rom_config        | set MPU region 6 to protect MPC (ROM)                       |
| arm_mpu_config_s          | set 6 regions to protect by MPU                             |
| test_config_mpu_s         | configure mpu
| test_config_sau_s         | configure sau
| test_config_mpc_s         | configure mpc
| test_config_ppc_s         | configure ppc
| elapsed_time_init/start/stop/clr/result | dwt time cycle
| dwt_install_watchpoint    | setup dwt watchpoint
| mtb_enable/disable/resume | configure mtb


**Test Cases**

| Macro (test_s.h)        | Case_name (test_cases_s.c) | Description                                                               |
| ----------------------- | -------------------------- | ------------------------------------------------------------------------- |
| TEST_USER_BUTTON_S      | test_user_botton_s()       |  arm express vasatile board provides userpb0 and userpb1 two buttons      |
| TEST_SYSTICK_S          | test_systick_s()           | test the systick interrupt to collect running time (1ms)                  |
| TEST_LCD_S              | test_lcd_s()                 | display string on LCD peripheral                                          |
| TEST_MPU_MPC_ROM_CONFIG_S| test_mpu_mpc_rom_config_s()  | set MPU region 0 to protect MPC (ROM)                                     |
| TEST_MOD_PRIVILEGES     | test_drop_privileges()     | change the privilege                                                      |
|	TEST_SAU                | test_sau()                  | configure SAU at run-time 
|	TEST_MPC                |test_mpc_s()                 | configure MPC at run-time 
|	TEST_PPC                | test_config_ppc_s ()        | configure PPC at run-time 
| TEST_MPU_S              | test_mpu_s()                | access regions which are protected by the MPU                             |
|	TEST_SWITCH_S           | test_switch_s()             | observe the state switches 
|	TEST_MOD_IPSR           | test_ipsr()                 | try to modify the IPSR directly 
|	TEST_UNSTACKING_S       | test_unstacking_s()         | put value EXC_RETURN in LR to trigger the unstacking
|	TEST_DWT_S              | test_dwt_s()                | monitor data access, trigger debug_monitor handler
|	TEST_MTB                | 		test_mtb_s_infinite_loop(); test_mtb_s_stack_overflow(); test_mtb_s_memory_corrup();  | observe the value in memory 0x24000000 
|	TEST_BB_S               | test_bb_s()                 | unintended branch 
|	TEST_SVC_S              | test_svc_s()                | run SVC in handler mode
|	TEST_TT_S               | test_tt_s()                 | observe TT instruction
|	TEST_SG_S               | test_sg_s()                 | insert code after SG instruction in the NSC region
|	TEST_SG_2_NS_S          | test_sg_2_ns_s()            | insert code after SG instruction in the NSC region, BX to non-secure world                                                  


### ***In the non-secure state***

| Macro (test_ns.h) | Test case name (test_cases_ns.c)          | Description  |
| ----------------- | ----------------------------------------- | ------------ |
|  /* those cases will trigger faults */
| TEST_UNSTACKING_NS  | test_unstacking_ns()                    | fake FNC_RETURN
| TEST_ACCESS_NS       | test_access(), test_access_ns()        | read/write to secure region, call NSC function test_access_ns_s()   |
| TEST_ALIAS_MPC_NS   | test_mpc_alias_ns()                     | access MPC alias memory
| TEST_MPC_NS         | test_mpc_ns()                           | /* Only secure state can configure the MPC */
|  /* benign tests */
| TEST_SWITCH_NS      | test_switch_ns()                        | state switches
| TEST_NSC_NS         | test_nsc_ns()                           | call nsc function, observe the stack
| TEST_DWT_NS         | test_dwt_ns()                           |!! SET *(uint32_t*)0xE000EDF0 to value a05f0000 (0xA05F<<16) via the debugger memory view */
| TEST_SCB_NS         | test_scb_ns()                           | Read/Write to SCB
| TEST_CRTL_NS      | test_ctrl(), test_ctrl_ns()                | modify CTRL directly on privileged/non-privileged mode, call NSC function test_ctrl_ns_s()                                                                                  |
| TEST_LP_ACCESS    | test_lp_access_s_ns()                      | drop privilege for both secure and non-secure world, call NSC function in the non-secure world                                                                           |
| TEST_LP_ACCESS    | test_lp_access_ns()                        | drop privilege for non-secure world, call NSC function                                                                                                                   |
| TEST_LP_ACCESS    | test_lp_access()                           | access data with and without non-secure MPU configuration (RW Privilege only), drop privilege for both secure and non-secure world, call NSC function test_lp_access_ns_s() |
| TEST_RUNNING_TIME | test_state_switch_time()                   | measure the time when ns to s and s to ns                                                                                                                                |
| TEST_RUNNING_TIME | test_privilege_switch_time_missed_cycles() | measure the time when np to p with missed cycles (time cycles to branch to svc handler)                                                                                  |
| TEST_RUNNING_TIME | test_privilege_switch_time_added_cycles()  | measure the time when np to p with added time cycles (starting with drop privilege)                                                                                      |
| TEST_RUNNING_TIME | test_ctr_instruction_time();               | measure the time when execute instrution to write to CONTROL register                                                                                                    |
| -                 | Secret_lcd_print()                         | NSC function, run it to check the call routine     


>For more details, please refer to Update_log.md