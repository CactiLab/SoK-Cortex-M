## update log 08/16/2021

- Register a non-secure function that can be called from the secure state
- Four steps
1. in the secure state function, define the pointer and callback function
```c
/*----------------------------------------------------------------------------
  NonSecure callback functions
 *----------------------------------------------------------------------------*/
extern NonSecure_fpParam pfNonSecure_test_add_ns;
NonSecure_fpParam pfNonSecure_test_add_ns = (NonSecure_fpParam)NULL;

/*----------------------------------------------------------------------------
  Secure function for NonSecure callbacks exported to NonSecure application
 *----------------------------------------------------------------------------*/
int32_t Secure_test_add_ns_callback(NonSecure_fpParam callback) __attribute__((cmse_nonsecure_entry));
int32_t Secure_test_add_ns_callback(NonSecure_fpParam callback)
{
	pfNonSecure_test_add_ns = callback;
	return 0;
}
```
2. Declare the function in the NSC file (Secure_Functions.h)
```c
extern int32_t Secure_test_add_ns_callback(NonSecure_funcptr);
```
3. Declare the function in the non-secure state
```c
/*----------------------------------------------------------------------------
  NonSecure functions used for callbacks
 *----------------------------------------------------------------------------*/
int32_t test_add_ns(uint32_t a, uint32_t b)
{
  uint32_t c;
  c = a + b;
  return c;
}
```

4. register this non-secure function in the non-secure state function
```c
  /* register NonSecure callbacks in Secure application */
  Secure_test_add_ns_callback(test_add_ns);
```


## update log 07/21/2021
- DebugMon_Handler, DebugMon_Handler_Main
- test_dwt_s, test_dwt_ns
- While using the debugger, it uses the halting debug with the DCB->DHCSR debug key enabled. set *(uint32_t*)0xE000EDF0=(0xA05F<<16) to disable the halting debug, then dwt will work. 0xA05F<<16 = a05f0000
> The value of BCD->DHCSR can only be changed via **debugger** or **IDE**

> Load the image to the board can also trigger the Debug monitor handler
- load two images to the mps2+ board
```c
// change \MB\HBI0263C\AN505\images.txt
[IMAGES]
TOTALIMAGES: 2 ;Number of Images (Max: 32)
IMAGE0ADDRESS: 0x10000000 ; Secure address start
IMAGE0FILE: \SOFTWARE\IOT_s.axf ; Secure image
IMAGE1ADDRESS: 0x00200000 ; Non-secure address start
IMAGE1FILE: \SOFTWARE\IOT_ns.axf ; Non-secure imag
```

---
- CONTROL register, bit[3] and bit[2] are not banked. MRS, MSR.

|bit[3]|bit[2]|bit[1]|bit[0]|
|-|-|-|-|
|SFPA|FPCA|SPSEL|nPRIV|
|1: Floating-point registers belongs to secure|1: Floating-point context active|0: MSP, 1: PSP| 0: P, 1: UP|

- FPSCR: Floating-point Status Control Register p335. 
- VMSR: move to floating-point System Register from Arm Core register and updates FPSCR
- VMRS: move to core register from floating-point Special Register
- BIC: Logical AND NOT, or bit clear

```asm
; with 
0x1000275C B083      SUB      sp,sp,#0xc
0x1000275E 9002      STR      r0,[sp,#8]
0x10002760 9101      STR      r1,[sp,#4]
0x10002762 9802      LDR      r0,[sp,#8]
0x10002764 9901      LDR      r1,[sp,#4]
0x10002766 4408      ADD      r0,r0,r1
0x10002768 9000      STR      r0,[sp,#0]
0x1000276A 9800      LDR      r0,[sp,#0]
0x1000276C B003      ADD      sp,sp,#0xc

; check whether the floating-point register is belong to secure state or not.
0x1000276E F3EF8C14  MRS      r12,CONTROL
0x10002772 F01C0F08  TST      r12,#8
0x10002776 D027      BEQ      0x100027c8
; clear the floating-point register, which are already pushed on to the stack 
0x10002778 EE00EA10  VMOV     s0,lr
0x1000277C EE00EA90  VMOV     s1,lr
0x10002780 EE01EA10  VMOV     s2,lr
0x10002784 EE01EA90  VMOV     s3,lr
0x10002788 EE02EA10  VMOV     s4,lr
0x1000278C EE02EA90  VMOV     s5,lr
0x10002790 EE03EA10  VMOV     s6,lr
0x10002794 EE03EA90  VMOV     s7,lr
0x10002798 EE04EA10  VMOV     s8,lr
0x1000279C EE04EA90  VMOV     s9,lr
0x100027A0 EE05EA10  VMOV     s10,lr
0x100027A4 EE05EA90  VMOV     s11,lr
0x100027A8 EE06EA10  VMOV     s12,lr
0x100027AC EE06EA90  VMOV     s13,lr
0x100027B0 EE07EA10  VMOV     s14,lr
0x100027B4 EE07EA90  VMOV     s15,lr
0x100027B8 EEF1CA10  VMRS     r12,FPSCR
0x100027BC F02C0C9F  BIC      r12,r12,#0x9f
0x100027C0 F02C4C70  BIC      r12,r12,#0xf0000000
0x100027C4 EEE1CA10  VMSR     FPSCR,r12
0x100027C8 4671      MOV      r1,lr
0x100027CA 4672      MOV      r2,lr
0x100027CC 4673      MOV      r3,lr
0x100027CE 46F4      MOV      r12,lr
; APSR flags
0x100027D0 F38E8C00  MSR      APSR_nzcvqg,lr
0x100027D4 4774      BXNS     lr
0x100027D6 BF00      NOP

----
0x100027D8 B083      SUB      sp,sp,#0xc
0x100027DA 9002      STR      r0,[sp,#8]
0x100027DC 9101      STR      r1,[sp,#4]
0x100027DE 9802      LDR      r0,[sp,#8]
0x100027E0 9901      LDR      r1,[sp,#4]
0x100027E2 4408      ADD      r0,r0,r1
0x100027E4 9000      STR      r0,[sp,#0]
0x100027E6 9800      LDR      r0,[sp,#0]
0x100027E8 B003      ADD      sp,sp,#0xc
0x100027EA 4770      BX       lr
```

## update log 07/02/2021

- mtb, dwt [link](https://interrupt.memfault.com/blog/cortex-m-watchpoints).

## update log 06/30/2021, 07/01/2021

- test_sg_s, test_sg_ns, insert code in the NSC region and execute SG
  - NSC region: 0x10000000-0x101FFFFF
  - EPSR.T (p77)
  - Interworking branches (C1.4.7): BX, BLX, POP (multi-registers), LDR (immediate/literal/register). bit[0] of an interworking branch instruction sets EPSR.T. If EPSR.T is cleared to 0, an INVSTATE UsageFault or HardFault is generated when next instruction executes.
  - `BX 0x0x100057D1` has the bit[0] = 1, then after BX instruction, EPSR.T changes to 1. Or else, `BX 0x0x100057D0` changes EPSR.T to 0.

### MPU settings
- same MPU configuration in secure and non-secure state
   
| MPU# |SAU#| Address               | Memory Map           | AP                                   |
| ---- |--- |--------------------- | -------------------- | ------------------------------------ |
| S/NS-0    |1| 0x00200000-0x003FFFFF | NS SRAM1 for code     | Normal, Non-Shareable, RO            |
| S/NS-1    |0| 0x10000000-0x101FFFFF | S SRAM1 for code, NSC | Normal, Non-Shareable, RO            |
| S-2    |-| 0x38000000-0x381FFFFF | NS SRAM2 for data    | Normal, Non-Shareable, RW, XN            |
| NS-2    |-| 0x28200000-0x283FFFFF | NS SRAM3 for data    | Normal, Non-Shareable, RW, XN            |
| S/NS-3    |3| 0x40000000-0x4000FFFF | NS CMSDK APB         | Device-nGnRnE, Non-Shareable, RW, XN |
| S/NS-4    |3| 0x40010000-0x400013FF | NS GPIO              | Device-nGnRnE, Non-Shareable, RW, XN |
| S/NS-5    |3| 0x40014000-0x40017FFF | NS Default AHB Slave | Device-nGnRnE, Non-Shareable, RO, XN |
| S/NS-6    |?| 0xF0000000-0xF0000FFF | System ROM           | Normal, Non-Shareable, RO            |

<!-- | test cases               | without MPU           | MPU AP | with MPU                 |
| ------------------------ | --------------------- | ------ | ------------------------ |
| Reading reserved region  | preciser bus error    | RO     | preciser bus error       |
| Writing to flash         | no error              | RW     | DACCVIOL MemManage Fault |
| Reading undefined region | precise bus error     | -      | preciser bus error       |
| Executiong SRAM          | instruction bus error | RW, XN | IACCVIOL memmanage fault | -->

- IREGION: IDAU; S: target address is secure or not

|Ins|IREGION|IRVALID|S|NSRW|NSR|RW|R|SRVALID|MRVALID|SREGION MERGION|
|-|-------|-------|-|----|---|--|-|-------|-------|---------------|
||bit[31:24]|bit[23]|bit[22]|bit[21]|bit[20]|bit[19]|bit[18]|bit[17]|bit[16]|bits[15:8]bits[7:0]|
|**Secure state**|
|`TTT?(0x10100000)` in S-P|01|1|1|0|0|0|1|1|1|00000000 00000001|
|`TTAT?(0x10100000)` in S-P|01|1|1|0|0|1|1|1|0|00000000 00000000|
|`TTT?(0x00300000)` in S-P|00|1|0|0|1|0|1|1|1|00000001 00000000|
|`TTAT?(0x00300000)` in S-P|00|1|0|1|1|1|1|1|0|00000001 00000000|
|Drop privilege|
|`TTT?(0x10100000)` in S-UP|01|1|1|0|0|0|0|1|0|00000000 00000000|
|`TTAT?(0x10100000)` in S-UP|01|1|1|0|0|1|1|1|0|00000000 00000000|
|`TTT?(0x00300000)` in S-UP|00|1|0|0|0|0|0|1|0|00000001 00000000|
|`TTAT?(0x00300000)` in S-UP|00|1|0|1|1|1|1|1|0|00000001 00000000|
|**Non-secure state**|
|`TTT?(0x10100000)` in NS-P|00|0|0|0|0|0|1|0|1|00000000 00000001|
|`TTT?(0x00300000)` in NS-P|00|0|0|0|0|0|1|0|1|00000000 00000000|
|-Drop Privilege-|
|`TTT?(0x10100000)` in NS-UP|00|0|0|0|0|0|0|0|0|00000000 00000000|
|`TTT?(0x00300000)` in NS-UP|00|0|0|0|0|0|0|0|0|00000000 00000000|

  - -Secure-
  - `TTT?(0x10100000)` in S-P, `TT_RESP: 0x01C70001 --> 001110001110000000000000001`
  - `TTAT?(0x10100000)` in S-P, `TT_RESP: 0x01CE0000 --> 01110011100000000000000000`
  - `TTT?(0x00300000)` in S-P, `TT_RESP: 0x00970100 --> 000100101110000000100000000`
  - `TTAT?(0x00300000)` in S-P, `TT_RESP: 0x00BE0100 --> 00101111100000000100000000`
  - -Drop privilege-
  - `TTT?(0x10100000)` in S-UP, `TT_RESP: 0x01C20000 --> 01110000100000000000000000`
  - `TTAT?(0x10100000)` in S-UP, `TT_RESP: 0x01CE0000 --> 1110011100000000000000000`
  - `TTT?(0x00300000)` in S-UP, `TT_RESP: 0x00820100 --> 00100000100000000100000000`
  - `TTAT?(0x00300000)` in S-UP, `TT_RESP: 0x00BE0100 --> 0101111100000000100000000`
  - -Non-secure-
  - `TTT?(0x10100000)` in NS-P, `TT_RESP: 0x00050001 --> 1010000000000000001`
  - `TTT?(0x00300000)` in NS-P, `TT_RESP: 0x00050001 --> 1010000000000000000`
  - -Drop privilege-
  - `TTT?(0x10100000)` in NS-UP, `TT_RESP: 0x00000000 --> 0`
  - `TTT?(0x00300000)` in NS-UP, `TT_RESP: 0x00000000 --> 0`

## update log 06/30/2021
- sys_reset
- sys_recover [link](https://interrupt.memfault.com/blog/cortex-m-fault-debug#fn:15)

## update log 06/28/2021
- test_b_s: normal branches
- test_b_switch_ns: unintended switch
- test_bb_s: unintended branches

|-        |-      |-        |-      |
|---------|--------|--------|----------------------------|
|0x100056C8| E97FE97F|  SG|   |
|0x100056CC| F7FCBD68|  B    |    test_add_s (0x100021A0)|

## update log 06/25/2021. 06/30/2021

> Test Target (Alternate Domain, Unprivileged). Test Target (TT) queries the Security state and access permissions of a memory location.

> Test Target Unprivileged (TTT) queries the Security state and access permissions of a memory location for an unprivileged access to that location.

> Test Target Alternate Domain (TTA) and Test Target Alternate Domain Unprivileged (TTAT) query the Security state and access permissions of a memory location for a Non-secure access to that location. **These instructions are only valid when executing in Secure state, and are UNDEFINED if used from Non-secure state**.

- CMSIS provides functions: `cmse_TTA?T?(void *__p)`;
  - `TT r0, r0`: test target in r0 and store the payload back to r0
- test_tt_s
  - Configure region 0 (0x00000000 - 0x007FFFFF): NS CMSDK APB (Device-nGnRnE, Non-Shareable, RW, Any Privilege Level, XN)
  - Configure region 1 (0x30000000 - 0x307FFFFF): S SRAM2&3 (Normal, Non-Shareable, RW, XN, Any Privilege Level)
  - `TTA?T?(0x28200000)` in S-P, `TT_RESP: 0x02BE0200 --> 10101111100000001000000000`
  - `TTT?(0x28200000)` in S-UP, `TT_RESP: 0x02820200 --> 10100000100000001000000000`
  - `TTAT?(0x28200000)` in S-UP, `TT_RESP: 0x02BE0200`
  - -- after drop privileged --
  - `TTA?T?(0x18200000)` in S-P, `TT_RESP: 0x01CC0000 --> 1110011000000000000000000`
  - `TTT?(0x18200000)` in S-UP, `TT_RESP: 0x01C00000 --> 1110000000000000000000000`
  - `TTAT?(0x18200000)` in S-UP, `TT_RESP: 0x01CC0000 --> 1110011000000000000000000`
  
|Ins|IREGION|IRVALID|S|NSRW|NSR|RW|R|SRVALID|MRVALID|SREGION MERGION|
|-|-------|-------|-|----|---|--|-|-------|-------|---------------|
||bit[31:24]|bit[23]|bit[22]|bit[21]|bit[20]|bit[19]|bit[18]|bit[17]|bit[16]|bits[15:8]bits[7:0]|
|TTA?T?(NS) in S-P|10|1|0|1|1|1|1|1|0|00000010 00000000|
|TTT?(NS) in S-UP|10|1|0|0|0|0|0|1|0|00000010 00000000|
|TTAT?(NS) in S-UP|10|1|0|1|1|1|1|1|0|00000010 00000000|
|TTA?T?(S) in S-P|01|1|1|0|0|1|1|0|0|00000000 00000000|
|TTT?(S) in S-UP|01|1|1|0|0|0|0|0|0|00000000 00000000|
|TTAT?(S) in S-UP|01|1|1|0|0|1|1|0|0|00000000 00000000|

- test_tt_ns (compile non-secure project with -mcmse flag)
  - `TTA?T?(0x10100000)` in NS-P, `TT_RESP: 0x000C0000 --> 11000000000000000000`

|Ins|IREGION|IRVALID|S|NSRW|NSR|RW|R|SRVALID|MRVALID|SREGION MERGION|
|-|-------|-------|-|----|---|--|-|-------|-------|---------------|
|TTAT?| Hardfault|
|TTT?(NS/S) in NS-P|0|0|0|0|1|1|0|0|00000000|00000000|
|TTT?(NS) in NS-UP|0|0|0|0|0|0|0|0|00000000|00000000|

- fix test_b_ns(), add test_b_s(): 24 types of branch transition, and there are more.

## update log 06/24/2021

- Code clean
- SwitchStackPointer(): change the MSP to PSP
- test_svc_s: after SVC instruction, the processor runs in handler mode, execute SVC in handler mode will trigger HardFault (Unidentified fault)
- test_sg_s/test_sg_ns: run SG instruction in NS will be ignored.

## update log 02/15/2021

Insert instructions after SG (insert instructions in the NSC region)

In test_case_s.c: modify_ins_after_sg() copies instruction to the address after SG

In test_case_ns.c: call nsc function which will branch to the SG instruction


## update log 12/21/2020

- Refer to document: `mps2 and mps2plus fpga prototyping`
  - The board provides the following LEDs and switches:
    - Two LEDs and two push buttons that connect directly to the FPGA:
      - The board labels the LEDs as USERLED0 and USERLED1.
      - The board labels the push buttons as PB0 and PB1.
    - Eight LEDs and an 8-way DIP switch that connect to the MCC.
  
- test_user_buttons_s()
  - Refer to `Cortex-M33_IoT_Kit_2.0/software/selftest/apleds.c` to check the button test

- Refer to [link](https://docs.zephyrproject.org/latest/boards/arm/mps2_an521/doc/index.html)
  - IOTKIT_SECURE_GPIO1: 50101000
  - IOTKIT_GPIO1: 40101000 GPIO1[5]->userbutton0 GPIO[6]->userbotton1


## update log 12/20/2020

- test_mpc_alias_ns(), test_mpc_alias_s(), print_mpc_alias
  - Table `alias_region_tbl` fixed at address `0x282000a0`
  - In case that data will be clear after state switch, print out all data we collect before.

1. MPC change the region to secure, will block all data in this region to show
2. SAU change to non-secure, then this region can show the data.
3. Non-secure can not access MPC peripheral, which will trigger the secure fault: attribution unit volation.
4. After MPC configure the non-secure region, the data is still in this region but blocked. Then after SAU release the non-secure region, the data becomes acceable again. After all MPC and SAU configurations, process starts to run. At this stage, if the process changes the state from secure to non-secure, the data will still be remained. This would has a potential data leakage information.
5. `cmse_nonsecure_call` attribute

## update log 12/10/2020
- keep on test_switch_ns()

## update log 12/09/2020

- test_switch_s(), test_switch_ns()
- TODOs, complete the tests

## update log 12/02/2020

- Code clean, macros
- Boot from SD Card
  ```log
  [IMAGES]
  TOTALIMAGES: 2                     ;Number of Images (Max: 32)

  IMAGE0ADDRESS: 0x00000000          ;Please select the required executable program
  IMAGE0FILE: \SOFTWARE\s.axf 	   ; - secure project

  IMAGE1ADDRESS: 0x00200000          ;Please select the required executable program
  IMAGE1FILE: \SOFTWARE\ns.axf       ; - non-secure project
  ```

## update log 11/30/2020

(refer to corelink_sse200_subsystem_reference_manual: Appx-A-162)
refer to [link](https://developer.arm.com/documentation/101104/0200/signal-descriptions/security-component-interfaces/apb-peripheral-protection-controller-interface)

## update log 11/28/2020

- test_mpc(), test_config_mpc_s()
  - Configure three MPCs with MPC region number, index, and block settings
  - MPCSSRAM1, MPCSSRAM2, MPCSSRAM3
  
  | Address                 | Size | Region name | Security |
  | ----------------------- | ---- | ----------- | -------- |
  | 0x5008_3000 0x5008_3FFF | 4KB  | SRAM0MPC    | SP       |
  | 0x5008_4000 0x5008_4FFF | 4KB  | SRAM1MPC    | SP       |
  | 0x5008_5000 0x5008_5FFF | 4KB  | SRAM2MPC    | SP       |
  | 0x5008_6000 0x5008_6FFF | 4KB  | SRAM3MPC    | SP       |
> p3-97

  | Offset | Name       | Access | Reset | Description                                                                   |
  | ------ | ---------- | ------ | ----- | ----------------------------------------------------------------------------- |
  | 0x000  | CTRL       | RW     | 0x0   |
  | 0x010  | BLK_MAX    | RO     | -     | Maximum value of block-based index register                                   |
  | 0x014  | BLK_CFG    | RO     | -     | Bit[3:0] Block size: 0: 32 Bytes 1: 64 Bytes... Block size = 1 << (BLK_CFG+5) |
  | 0x018  | BLK_IDX    | RW     | 0x0   | Index value for accessing block-based lookup table.                           |
  | 0x01C  | BLK_LUT[n] | RW     | -     | Access to block based lookup configuration space pointed to by BLK_IDX.       |
  |        |            |        |       | Bit[31:0]: each bit indicate one block:                                       |
  |        |            |        |       | If BLK_IDX is 0, bit[0] is block #0, bit[31] is block #31.                    |
  |        |            |        |       | If BLK_IDX is 1, bit[0] is block #32, bit[31] is block #63                    |
  |        |            |        |       | For each configuration bit, 0 indicates Secure, 1 indicates Nonsecure.        |

- Look Up Table (LUT) examples (p3-99)
- Other information about MPC
  - 4 Secure MPC, 4 Non-secure MPC (refer to corelink_sse200_subsystem_reference_manual: p3-77)


  - Ensuring the cache handles memory modifications (p3-87)
  - Security Privilege Control Block (refer to corelink_sse200_subsystem_reference_manual: p3-101)
    - Secure Privilege Control registers
      - AHBNSPPC0: The Non-secure Access AHB Slave Peripheral Protection Controller Register allows software to configure if each AHB peripheral that it controls from an AHB PPC is Secure access only or is Nonsecure access only
        -  Each field defines the Secure or Non-secure access setting for an associated peripheral, as follows:
            -  1: Allow Non-secure access only.
            -  2: Allow Secure access only
        - SSE-200 does not have an AHB slave interface that needs security configuration support of the PPC. This register is reserved and RAZ/WI. (p3-113)
      - AHBNSPPCEXP0/1/2/3: The Expansion Non-secure Access AHB Slave Peripheral Protection Controller registers 0, 1, 2 and 3 allow software to configure each AHB peripheral that it controls from each AHB PPC that resides in the expansion subsystem outside the SSE-200.
      - APBNSPPC0/1, APBNSPPCEXP0/1/2/3
      - AHBSPPPC0: Secure Unprivileged Access AHB Slave Peripheral Protection Controller register allows software to configure if each AHB peripheral that it controls from an AHB PPC is Secure privileged, Secure Access, or if it is allowed Secure Unprivileged access as well. Each field defines this for an associated peripheral, by the following settings:
        - 1: Allow Secure unprivileged and privileged access.
        - 2: Allow Secure privileged access only
        - SSE-200 does not have an AHB slave interface that needs security configuration support of the PPC. This register is reserved and RAZ/WI.
      - AHBSPPPCEXP0/1/2/3
      - APBSPPPC0/1, APBSPPPCEXP0/1/2/3. 
        - Each field defines the Secure unprivileged access settings for an associated peripheral
          - 1: allow secure unprivileged access
          - 2: disallow secure unprivileged access
      - Secure Unprivileged Access APB slave Peripheral Protection Control 
    - Or Non-secure Privilege Control Block (p3-116): Non-secure Unprivileged Access AHB Slave Peripheral Protection Controll (APBNSPPP)


## update log 11/27/2020

> IDAU Configuration, refer to Example IoT Subsystem design for V2M-MPS2+, the security attributes should be implemented by the Vendor. 

 | Address                 | IDAU | Region Name     | Description       | Peripheral                   |
 | ----------------------- | ---- | --------------- | ----------------- | ---------------------------- |
 | 0x0000_0000 0x003F_FFFF | NS   | Code Memory     | ZBT SRAM (SSRAM1) | MPC to SSRAM1 (0x5800_7000)  |
 | 0x0040_0000 0x007F_FFFF | NS   | Code Memory     | SSRAM1 alias      |
 | 0x0080_0000 0x0DFF_FFFF | NS   | Code Memory     |
 | 0x1000_0000 0x103F_FFFF | S    | Code Memory     | ZBT SRAM (SSRAM1) |
 | 0x1040_0000 0x107F_FFFF | S    | Code Memory     | SSRAM1 alias      |
 | 0x2000_0000 0x20FF_FFFF | NS   | Internal SRAM   |
 | 0x2800_0000 0x281F_FFFF | NS   | Expansion 0     | ZBT SRAM (SSRAM2) | MPC to SSRAM2 (0x5800_8000)  |
 | 0x2820_0000 0x283F_FFFF | NS   | Expansion 0     | ZBT SRAM (SSRAM3) | MPC to SSRAM3 (0x5800_9000 ) |
 | 0x3000_0000 0x30FF_FFFF | S    | Internal SRAM   |
 | 0x3800_0000 0x381F_FFFF | NS   | Expansion 0     | ZBT SRAM (SSRAM2) |
 | 0x3820_0000 0x383F_FFFF | NS   | Expansion 0     | ZBT SRAM (SSRAM3) |
 | 0x4000_0000 0x4000_FFFF | NS   | Base Peripheral |
 | 0x4001_0000 0x4001_FFFF | NS   | Private CPU     |
 | 0x4002_0000 0x4002_FFFF | NS   | System Control  |
 | ...                     | NS   |
 | 0x4010_0000 0x4FFF_FFFF | NS   | Expansion 1     |
 | 0x5000_0000 0x5000_FFFF | S    | Base Peripheral |
 | 0x5001_0000 0x5001_FFFF | S    | Private CPU     |
 | 0x5002_0000 0x5002_FFFF | S    | System Control  |
 | ...                     | S    |
 | 0x8000_999 0x80FF_FFFF  | NS   | AHB Master      | PSRAM             |

> By default setting of this example project, 4 SAU regions are configured, all other addresses are secure (partition_IOTKit_CM33.h: TZ_SAU_Setup()).

| SAU ID | Address                 | Att | IDAU | SAU    | Final       |
| ------ | ----------------------- |-----| ---- | ------ | ----------- |
| 0      | 0x1000_0000 0x101F_FFFF |SSRAM1| S    | S, NSC | S, NSC Code |
| 1      | 0x0020_0000 0x003F_FFFF |SSRAM1| NS   | NS     | NS Code     |
| 2      | 0x2820_0000 0x283F_FFFF |SSRAM3| NS   | NS     | NS Data     |
| 3      | 0x4000_0000 0x403F_FFFF || NS   | NS     | NS          |
| -      | -                       || NS/S | S      | S           |

> 2 MPC resions are configured (startup_IOTKit_CM33.c: SystemInit()). 

| -------- | Address                 | SAU | IDAU | MPC                   | Final   |
| -------- | ----------------------- | --- | ---- | --------------------- | ------- |
|          | 0x0020_0000 0x003F_FFFF | NS  | NS   | MPSSSRAM1: 0xFFFF0000 | NS Code |
|          | 0x2820_0000 0x283F_FFFF | NS  | NS   | MPSSSRAM3: 0xFFFFFFFF | NS Data |

>  From the default setting we could know that there are three valid cases for the combination of SAU and IDAU.

| IDAU | SAU    | Final       |
| ---- | ------ | ----------- |
| S    | S, NSC | S, NSC Code |
| NS   | NS     | NS          |
| NS/S | S      | S           |


- Configure the SAU region 4, 5, 6, 7 to test

| SAU ID | Address                 | IDAU | SAU    | Final  | Addition    |
| ------ | ----------------------- | ---- | ------ | ------ | ----------- |
| 4      | 0x1020_0000 0x103F_FFFF | S    | NS     | NS     | How to know the final?? Non-secure to test the region   |
| 5      | 0x0000_0000 0x001F_FFFF | NS   | S, NSC | S, NSC | No error??? |

- test_config_sau_s()
- test_config_mpc_s()
- test_config_ppc_s()

- The IDAU security value does not define privileged or unprivileged accessibility. That is defined by the MPC, PPC, or register blocks mapped to each area.
- The IDAU NSC values are defined by registers in the Secure Privilege Control registers
## update log 11/24/2020
- test_dwt_s()
  - DWT is privilege access only, non-privilege access will trigger data bus error (secure and non-secure).
  
- test_running_time() with interrupt disabled, cache enable or disable.
  - test_state_switch_time();
	- test_privilege_switch_time_missed_cycles(); // uncomment #define MISSED_CYCLES and this line
	- test_privilege_switch_time_added_cycles();
	- test_ctr_instruction_time();
  - ran 25\*500 times to get the final result 

- The image we use IOTKit_CM33 does not implement the caches. All processor and core peripherals are declared at IOTKit_CM33_FP.h file.
  ```c
  /* --------  Configuration of the IOTKIT_IoT Processor and Core Peripherals  ------- */
  #define __CM33_REV            0x0001U       /* Core revision r0p1 */
  #define __SAUREGION_PRESENT       1U        /* SAU regions are present */
  #define __MPU_PRESENT             1U        /* MPU present */
  #define __VTOR_PRESENT            1U        /* VTOR present */
  #define __NVIC_PRIO_BITS          3U        /* Number of Bits used for Priority Levels */
  #define __Vendor_SysTickConfig    0U        /* Set to 1 if different SysTick Config is used */
  #define __FPU_PRESENT             1U        /* FPU present */
  #define __DSP_PRESENT             1U        /* DSP extension present */
  ```

  For other images or other devices, if they implement the cache, the header file will provide several interfaces to diable or enable either instruction cache or data cache.
  Refer to [link](https://www.keil.com/pack/doc/cmsis/Core/html/group__cache__functions__m7.html)

  ```c
  SCB_EnableICache(), SCB_DisableICache(), SCB_EnableDCache(), SCB_DisableDCache(), ...
  ```
    - check_cache_ns()
      - refer to ARMv8-M Architecture Reference Manual D1.2.18: CTR register in SCB tells us whether this device implement cache or not. D1.2.12: Cache type field indicates the type of cache: 1) 0b00 No cache; 2) 0b001 Instruction cache only; 3) Data cache only; 4) 0b011: Sperate instruction and data caches; 5) Unified cache 

## update log 11/23/2020

SystemInit calls the TZ_SAU_Setup to initializr the SAU and to define non-secure interrupts (NVIC_INIT_ITNS).

- test_sau() in test_case_s.c
  - Board_Config_TZ(), test_config_sau_s()
  - refer to [link](https://github.com/ARM-software/CMSIS-Zone/tree/master/Examples/LPC55S69)

## update log 11/18/2020

- test_running_time()
  - add new test to check the time cycle for NP->P
  - A problem here is seems non-privilegede mode can not access DWT which triggered bus error.
  - Two ways to check the time cycle which are not precise
    - Time cycle starts when jump into SVC handler. It actually missed several cycles. The result is 54.
    - Starts from dropping privilege (MSR CONTROL, 1). It actually added the time cycle to run the drop_privilege instruction. The result is 77.
    - Even though with missed time cycles, the time cost of context switch for non-privilege to privilege is 42% higher than TrustZone.

| non-secure->secure | non-privilege->privilege (missed cycles) | non-privilege->privilege (added cycles) |
| ------------------ | ---------------------------------------- | --------------------------------------- |
| 38                 | 54                                       | 77                                      |

- SVC_Handler, SVC_Handler_Main()
  - refer to [link](https://www.keil.com/support/docs/4063.htm)

## update log 11/17/2020

- test_lp_access_s_ns(), test_lp_access_ns_s()

  - access data with low privilege and high privilege.
    - both secure and non-secure world are low privilege
    - only non-secure world is low privilege
  - recall back to 11/12/2020 tests, MPU will block calling NSC function (TODO: why?)

- test_running_time()
  - setup the DWT (Data Watchpoint and Trace) to count running cycles without interrupt the cpu refer to Cortex-M33 processor ARMv8-M IoT Kit FVP User Guide.
  - the implementation please refer to [link](https://www.embedded-computing.com/articles/measuring-code-execution-time-on-arm-cortex-m-mcus)
  - test context switch time from non-secure to secure world
  - **ATTENTION**: a fixed address table to store the time values.

## update log 11/12/2020

- test_sg_ns(), test_sg_ns_s()
  - A special region stores all SG instructions and NSC functions, this region is non-secure executable only. Non-secure wolrd can not read or write to this region.
- test_lp_access()
  - Tried to drop priviledge to read or write to the privilege r/w only region.
  - Secure MPU can not control the data access in the non-secure world while non-secure MPU can control. Low priviledged mode can not call NSC function. It will trigger hardfault but the fault value is 0 that is can not identify the fault type.

## update log 11/11/2020

- update README.md

## update log 11/04/2020

- codde clean
- test_scb_ns_s(), test_ctrl_ns_s()

- test_nvic_s()
  - test cortex-m exceptions and nvic, refer to [link](https://interrupt.memfault.com/blog/arm-cortex-m-exceptions-and-nvic)

## update log 10/28/2020

- test_scb_ns()
  - priviliege mode (both secure and non-secure) can read/write to scb
  - non-privilege non-secure mode can not read/write scb: data bus error
- test_ctrl_ns()

  - same as test_scb_ns()

- test_sg()

  - Try to call/jump to an SG instruction that is not in the NSC

    - **Background**: NSC function is anotated by arm compiler attributes `__attribute__((cmse_nonsecure_entry))`. When non-secure program calls NSC function, it triggers SG instrcution to handle this call and execution mode changes to secure thread.

      - The routione to call the NSC is:

      ```armasm
      ; call NSC, this function is still in non-secure world
                  BL $$Ven$TT$L$$NSC_function

      ; $$Ven$TT$L$$NSC_function
                  MOVW r12, #0x44a1
                  MOVT r12, #0x1000
                  BX r12

      ; enter, secure world
      0x100044a0  SG
                  B NSC_function

      ; exit
                  MRS r12, CONTROL
                  TST r12, #8 ; check #8 ???
                  BEQ 0x10000504
      0x10000504  MOV r1, lr
                  MOV r2, lr
                  MOV r3, lr
                  MOV r12, lr
                  MSR APSR_nzcvqg, lr
                  BXNS lr
      ```

      - **Test**: use _SG_ instrcution directly and BL a secure function or secure address.
      - **Result**: BL to a non-secure address, then trigger secure fault: invalid entry point.

- test_s_b()
  - save the non-secure function address to r12, `B/BL/BX/BLX r12`
  - same result with 10/27/2020
- test_ns_b()
  - the address is changed to a non-secure address ???

## update log 10/27/2020

### General tests

- test_b() in non-secure project will call test_s_b() in secure project
  - B/BX/BL/BLX from the secure world to the non-secure world directly.
    1. Target for B/BL must be a label (imm), Target for BX/BLX/BXNS/BLXNS must be a register
    2. In the test_s_b() function, `LR` stores the next instruction after returning from the secure world.
  - test 1: set the value of R0 to the address of the non-secure function.
    - When run, observing from disassembly code, the address was changed to another value in the secure world.
      0x0020088C (address of test_add_ns) -> 0x10001e04(??)
  - test 2: `BX/BLX/BXNS/BLXNS LR`
    - `BX/BLX LR` will branch to the non-secure world, then trigger secure fault: invalid transition.
      There may be a register monitor the status.
    - `BXNS/BLXNS LR` branch to the non-secure world without error, but BLXNS will change the LR to 0xFEFFFFFF (not the EXC_RETURN).

## update log 10/26/2020

### General tests

- test_s_access()
  - Access secure region from non-secure region, triggers secure fault
- test_access_ns_s()
  - Access non-secure data section from secure world, no errors
    - write to non-secure data region from non-secure world
    - read the data region from secure world

### SecureFault_Handler

Without MPU, any invalid access from non-secure to secure will trigger secure fault.

## update log 10/21/2020

- test_lcd_s()

  - Since Retargetting to the UART has the priority issue, print out to the LCD can be another solution.
  - lcd_print()

- Changed all printf function to lcd_print
  - Set the LCD font size as 6\*8 can show more messages.
  - 0.5 is one line spacing

## update log 10/20/2020

### General tests

- test_unstacking_s() - EXC_RETURN has three valid values when without FPU: 0xFFFFFFF1, 0xFFFFFFF9, 0xFFFFFFFD
  Three valid values with FPU: 0xFFFFFFE1, 0xFFFFFFE9, 0xFFFFFFED. - Put special EXC_RETURN value directly in the lr, then run `BX LR`. - Result: did not trigger the unstacking, pc changed to [lr] - 1, then ran instuctions locatted at 0xffffffe0, then triggered IACCVIOL MemManage_Fault, ` - Instruction fetch from a location that does not permit execution.` - IPSR indicates whether the processor is in thread mode or handler mode. I guess, if it is in the thread mode, `BX LR` has nothing todo with unstacking. But IPSR is RO, so malicious LR can not trigger unstacking.

### Revise test_undef_ins()

1. Execute an undefined instruction.
   - Call function from RAM without definition

### SysTick_Handler

- Refer to [link](https://arm-software.github.io/CMSIS_5/Core/html/group__SysTick__gr.html)
- test_systick(): test the systick interrupt to count the running time (ms)
- Back to the test_unstacking_s(), when systick interrupt happens, the processor is in the secure handler mode, lr = 0xffffffe9, then `BX LR` will trigger the unstacking. To observe it, we can set a breakpoint at SysTick_Handler().

## update log 10/19/2020

### Retargeting I/O to UART

[TODO] The interrupt for UART (IRQ[33]) has a lower priority than HardFault_Handler. So the I/O retargeting to UART can not work when a hard fault happens. Another possible solution is to use the UART without the interrupt. Refer to [link](https://www.keil.com/pack/doc/compiler/RetargetIO/html/_retarget__examples__u_a_r_t.html)

## update log 10/16/2020

### MPU tests

- The default MPU CMSIS-Core provides is the Secure-MPU (defined at core-m33.h)

  ```c
    #define SCS_BASE            (0xE000E000UL)                             /*!< System Control Space Base Address */
    ...
    #define MPU_BASE          (SCS_BASE +  0x0D90UL)                     /*!< Memory Protection Unit */
    #define MPU               ((MPU_Type       *)     MPU_BASE         ) /*!< Memory Protection Unit */
  ```

  > 0xE000E000 - 0xE000EFFF: Secure SCS

- Configured 7 regions. Refer to Connect ARMv8-M Memory Protection Unit User Guide V2.0.
  - Try to write to RO region, trigger MemMangeFault: ` - Load or store at a location that does not permit the operation.`
  - test_mpu_s()

### General tests

1. Execute an undefined instruction.
   - Compile error:
     `test_case_s.c(24): error: invalid instruction...`
   - test_undef()
2. Modify IPSR directly to enter handler mode.
   - IPSR (Interrupt Program Status Register) provides status information on whether an application thread or exception handler is executing on the processor. It contains the exception number. The register is part of the XPSR. XPSR is the term that is used to describe the combination of the APSR, EPSR, and IPSR into a single 32-bit Program Status Register. **Can only be accessed by MRS and MSR**.
   - IPSR is read-only, which can not be modified by `MSR` instruction. The processor ignores writes to the IPSR bits. APSR is RW. EPSR is RO, PSR is RW. Refer to [link](https://developer.arm.com/documentation/100235/0004/the-cortex-m33-processor/programmer-s-model/core-registers?lang=en)
   - test_ipsr()
3. How `isb` instruction works.
   - Based on MPU configuration tests, ignore the `isb` instruction.
   - With the observation of test_isb() without `isb` instruction to configure the MPU, it also triggers MemMangeFault immediately when trying to write to RO address after MPU configuration.
   - The document says "An Instruction Synchronization Barrier (ISB) ensures the updated configuration used by any subsequent instructions." It is hard to see the difference between isb enable and isb disable at IDE debugging level.
   - test_isb()

## update log 10/15/2020

1. Code cleaning

   - Moved all test cases to test_case_s.c file, formated codes
   - Completed other Fault_Handler c functions

2. I/O retargeting:

   - refer to [link](https://www.keil.com/support/man/docs/uv4/uv4_db_dbg_serialwin.htm)
   - I/O retargeting via UART

     - refer to [link](https://www.keil.com/pack/doc/compiler/RetargetIO/html/_retarget__examples__u_a_r_t.html)
     - Missing serial adaptor (two days later)

   - Retargeting via ITM
     - To see the output from the debugging view, can use printf() via the ITM. This IOTKit image does not implement ITM, ETM, CTI, DAP, or TPIU. Refer to **Cortex-M33 processor ARMv8-M IoT Kit FVP User Guide**.

## update log 10/14/2020

1. Created hard_fault_handler, shown specific error messages for testing.

   - UsageFault
   - BusFault
   - MemoryManagementFault

   Refer to

   - [ARM hard fualt handler](https://github.com/ferenc-nemeth/arm-hard-fault-handler)
   - [developing a generic hadr fault handler for arm cortex-m3/m4](https://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/)

2. Created test.h and test.c, setting up test cases
   - Test case MPU_MPC_ROM_CONFIG
     - Configure MPU region 0 to protect the MPC (SSRAM1-ROM)
     - refer to: ARMv8 technique document
   - Dropping privilege (CONTROL register is configurable)
     - Non-privileged mode run in secure world will trigger the Bus error.

**Tips: disable compiler optimization to debug.**

---

## Important header file CMSIS-Core provided to us

---

## Important source code Analysis

**\_\_ARM_FEATURE_CMSE attributes and -mcmse**

-mcmse. Enables the generation of code for the Secure state of the Armv8-M Security Extension. This option is required when creating a Secure image. [Link](https://developer.arm.com/documentation/dui0774/j/compiler-command-line-options/-mcmse)

**MPC (memory protection controller)**

IOTKit_CM33_s/RTE/Device/IOTKit_CM33_FP/startup_IOTKit_CM33.c: SystemInit()

```C
IOTKIT_MPCSSRAM1->CTRL    &= ~(1UL << 8U);            /* clear auto increment */
IOTKIT_MPCSSRAM1->BLK_IDX  = 0;                       /* write LUT index */
IOTKIT_MPCSSRAM1->BLK_LUT  = 0xFFFF0000UL;
```

The TrustZone-M requirement for boot up is that execution starts from a Secure memory space, and optionally automatically executes Non-secure firmware after Secure world initialization. At boot, the SRAM is Secure only. Software must change or restore the settings in the MPC to release memory for Non-secure world use. (refer to Corelink sse200 subsystem)

Each SRAM block is implemented within an SRAM element. Each MPC APB configuration interface is mapped to the following base addresses:

- 0x5008_3000 for SRAM Bank 0.
- 0x5008_4000 for SRAM Bank 1.
- 0x5008_5000 for SRAM Bank 2
- 0x5008_6000 for SRAM Bank 3.

Important settings:

- The _cfg_init_value_ of each MPC is tied to 0 so that at boot, the SRAM is Secure only. Software must change or restore the settings in the MPC to release memory for Non-secure world use.
- The _BLK_SIZE_ configuration of each MPC, which defines the MPC block size is defined by the top-level parameter _SRAM_MPC_BLK_SIZE_. This is set at a default value of 3 to select 256 byte blocks.
- The _GATE_PRESENT_ configuration parameter of each MPC is set to 0 to disable the MPC gating feature.

This board uses FPGA SSRAM11 to emulate ROM or code memory, SSRAM1 and SSRAM2 to emulate SRAM or data memory.

**SAU**

IOTKit_CM33_s/RTE/Device/IOTKit_CM33_FP/partition_IOTKit_CM33.h: TZ_SAU_Setup()

```c
#if defined (__SAUREGION_PRESENT) && (__SAUREGION_PRESENT == 1U)

  #if defined (SAU_INIT_REGION0) && (SAU_INIT_REGION0 == 1U)
    SAU_INIT_REGION(0);
  #endif
  ...
#endif
```

Initialize SAU regions to non-secure or secure + non-secure callable.

**IDAU, SAU, and MPC**

IDAU provides the fixed physical memory partition of secure and non-secure space. IDAU is outside of the processor, SAU is software programmable inside the processor. The secure of the region can be configured by either IDAU and SAU. But SAU can not change secure to non-secure. Only secure world can change the SAU.

MPC decides whether the memory maps to secure or non-secure space. It controlls up to 64 blocks with 8 bytes value. Each bit control one block, one block is 128KB, 1 is non-secure, 0 is secure.

### Testing

1. Non-secure can not modify MPC
   - Add **-mcmse** to the non-secure compilation, when non-secure trys to comfigure the MPC, it triggers secure fault.
2. Non-privileged mode can modify MPC
   - Non-previleged mode can modify the value of the MPC, when we use MPU to protect the peripheral region related to MPC, then non-previleged mdoe is not able to change the MPC. It triggers memory fualt.

## Environment setup

For this test, we use Cortex-M33 IoT Kit Imgae on MPS2+. Refer to [link](http://www.keil.com/appnotes/files/apnt_300.pdf)

<!-- **ATTENTION**

From the pack, CMSIS-Core, Keil provided to us already implemented some APIs to help the user to configure the MPU or CONTROL, but with the perspective that learns it deeply, we will not use those libraries.  -->

## Old tests

### General Test Cases
1. Execute an undefined instruction.
2. Modify IPSR directly to enter handler mode.
3. The effect of ISB instruction. If ISB is not used, when will changes take places?
4. What if malicious ISR changes CONTROL[0]?
5. Malicious kernel puts EXEC_RET in LR and triggers unstacking.
6. Any attempts to access secure regions from the non-secure code, or a mismatch between the code that is executed and the security state of the system results in a HardFault exception
7. Access non-secure data sections from the secure state program.
8. B/BX/BL/BLX from secure program to non-secure program directly.

### Test Cases (Non-privileged and thread model code; No MPU; Either normal or secure world)
1. Read and write to sensitive regions, e.g. SCB.
2. Read and write to system registers, e.g. CONTROL.
3. Use privileged instructions.
4. Call a function that is supposed to be privileged but no MPU protection.

### Test Cases (Privileged Code and thread model code; No MPU; Either normal or secure world)
1. Read and write to sensitive regions, e.g. SCB.
2. Read and write to system registers, e.g. CONTROL.
3. Use privileged instructions.
4. Call a function that is supposed to be non-privileged but no MPU protection.

### Test Cases (Either privileged or non-privileged; No MPU; Either normal or secure world)
1. Try to a) read from and b) write to an address where no physical memory of any type is there.
2. Try to write to a physically non-writable region, e.g. Flash or ROM.
3. Execute code from a) SRAM or b) DRAM.

### Test Cases (Either privileged or non-privileged; No MPU; Cross-world)
1. Call a secure world function directly from normal world.
2. Call a normal world function directly from secure world.
3. Change the code after the SG instruction in the Non-Secure Callable (NSC) to jump to normal world instead of secure world.
4. Call/Jump to an SG instruction that is not in the NSC.

### Memory PRotection Controller
1. No need to use priviledged instruction to change MPC. Whether you can change it based on MPU settings. Test: In Thread Priviledged, write to MPC. In Thread Non-Priviledge, write to MPC.
2. If Non-secure Priviledged mode, can write to MPC. We tried, it generates Secure Fault. It does not look like it is caused by PPC settings.
3. We tried set the region of MPC registers as RW Privileged, then try to change it in Unpriviledged mode, it generates MemFault.
