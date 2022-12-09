
# Environment Setup

- [Ghidra](https://github.com/NationalSecurityAgency/ghidra/releases) 10.2.2
- Replace the `ARMTHUMBinstructions.sinc` file in `<your_ghidra_path>/Ghidra/Processors/ARM/data/languages/` with the one in `code/`
- A mysql database server to import the firmware database in `database/cortexm_firmware_merged_fw.sql`

# File Description
```c
├── code
│   ├── string_search
│   │   ├── search_string.py                /* Search strings in the firmware image */
│   │   ├── strings.txt                     /* All strings to search */
│   │   ├── strings_mbed.txt                /* String searching results for Mebd OS */
│   │   ├── strings_rtos.txt                /* String searching results for RTOS */
│   │   ├── strings_smashing.txt            /* String searching results for stack canary string */
│   │   ├── strings_stack.txt               /* String searching results for stack string */
│   │   └── strings_zephyr.txt              /* String searching results for Zephyr OS */
│   ├── ARMTHUMBinstructions.sinc        /* Ghidra ARM THUMB ISA specification with stack limit register support */
│   ├── build_database.py*               /* Build database from the directory */
│   ├── export_ascii.py                  /* Export ASCII disassembly from a raw binary image */
│   ├── export_elf_ascii.py*             /* Export ASCII disassembly from an ELF image */
│   ├── get_base.sh                      /* Use FirmXRay tool to get base address of the binary */
│   ├── get_disass.py                    /* Execute Ghidra headless analyzer to export disassembly for all the fws in database */
│   ├── get_elf_text.sh*                 /* Export ASCII disassembly from all ELF images in a directory */
│   ├── match_reg.py                     /* Match regular expressions in all disassembly */
│   ├── merge_linked.py                  /* Merge linked firmware in the database */
│   ├── regex_pattern.py                 /* Regular expression patterns (used by match_reg.py) */
│   ├── rename_result.py*                /* Rename the firmware name in the database batchly */
│   └── utilities.py*                    /* Utility functions */
├── database
│   ├── cortexm_firmware_merged_fw.sql   /* Firmware database */
│   ├── fw_build.sql*                    /* SQL to build database */
│   └── queries.sql*                     /* SQL queries for analysis results */
└── README.md
```