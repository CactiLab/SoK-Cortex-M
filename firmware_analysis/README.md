
# Environment Setup

- [Ghidra](https://github.com/NationalSecurityAgency/ghidra/releases) 10.2.2
- Python version > 3.6
- Replace the `ARMTHUMBinstructions.sinc` file in `<your_ghidra_path>/Ghidra/Processors/ARM/data/languages/` with the one in `code/` folder
- A MySQL database server to import the firmware database in `database/cortexm_firmware_merged_fw.sql`
- Firmware dataset: [Link](https://drive.google.com/file/d/1uWVKqKGIP1HCtJI6HcY344SV8IgiraXM/view?usp=sharing)
- Firmware disassembly results: [Link](https://drive.google.com/file/d/1EHPaiSU4ySEfFBE_tv0xxvT-UPH1wZwQ/view?usp=sharing)

# File Description

Files marked with `*` are not required in replicating results.

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
│   └── queries.sql                      /* SQL queries for analysis results */
└── README.md
```

# Usage

## Get disassembly from firmware

All the firmware disassembly results are in the link above. To replicate the results, follow the steps:

1. Download the full firmware dataset and install Ghidra properly. Import the firmware database to a local MySQL server.
2. Use the tool in [FirmXRay](https://github.com/OSUSecLab/FirmXRay) to analyze the dataset and get the base address of each firmware image. Update the `base_addr` column in the database with results.
3. Update the path variables and SQL info at the beginning of the `get_disass.py` script.
4. Run the disassembler script with `python get_disass.py`. This may take some time.

## Get analysis results from the disassembly

1. Download the full firmware disassembly results or get them from the above steps. Install Ghidra properly. Import the firmware database to a local MySQL server.
2. Update the path variable for results and the SQL info at the beginning of the `match_reg.py` script.
3. Run the analyzing script with `python match_reg.py`.
4. Update the SQL info at the beginning of the `merge_linked.py` script and run it with `python merge_linked.py`. This step is to merge the results of the linked firmware images (e.g., the bootloader and application images for the same device) to prevent duplicates.
5. Use the example queries in `queries.sql` to query the analysis results.
