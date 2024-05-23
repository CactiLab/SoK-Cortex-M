# SoK: Where’s the “up”?! A Comprehensive (bottom-up) Study on the Security of Arm Cortex-M Systems

The results of this project were published in the paper entitled [SoK: Where’s the “up”?! A Comprehensive (bottom-up) Study on the Security of Arm Cortex-M Systems](https://cactilab.github.io/assets/pdf/tan2024sok.pdf) in the USENIX WOOT Conference on Offensive Technologies 2024. If you want to cite our paper in your work, please use the following BibTeX entry.

```
@inproceedings{tan24sok,
  author = {Tan, Xi and Ma, Zheyuan and Pinto, Sandro and Guan, Le and Zhang, Ning and Xu, Jun and Lin, Zhiqiang and Hu, Hongxin and Zhao, Ziming},
  title = {{SoK: Where's the ``up''?! A Comprehensive (bottom-up) Study on the Security of Arm Cortex-M Systems}},
  booktitle = {USENIX WOOT Conference on Offensive Technologies},
  year = {2024}
}
```

This repository contains the following supplementary materials:

1. The Cortex-M firmware analysis tool and firmware database (in [firmware_analysis](https://github.com/CactiLab/code-SoK-Cortex-M/tree/main/firmware_analysis) folder)
2. Cortex-M hardware feature test suits (in [hw_feature_test_suites](https://github.com/CactiLab/code-SoK-Cortex-M/tree/main/hw_feature_test_suites) folder). The example code suite includes the following components:
   - 2.1. Helper and configuration functions. These functions provide a C language interface to configure system and peripheral registers. The current version include:
     - a. exception handlers that retrieve detailed fault information;
     - b. functions to enable and disable data and instruction cache;
     - c. stack dump function that prints out regular stack and exception stack contents;
     - d. helper functions to configure and control (i) MPU; (ii) SAU; (iii) MPC; (iv) DWT; (v) MTB.
   - 2.2. Test cases to demonstrate how to use hardware features. The current version include the following test cases:
     - a. change `xPSR`;
     - b. legal and illegal memory accesses;
     - c. `TT` instruction examples;
     - d. stacking and unstacking with and without TrustZone;
     - e. instruction tracing with MTB;
     - f. performance measurement and data access monitor with DWT;
     - g. state transition tests and performance measurement.
3. A document explaining the background of Cortex-M architecture (in [3_background.pdf](https://github.com/CactiLab/code-SoK-Cortex-M/blob/main/3_background.pdf)).
4. An interactive figure showcasing the relationships between Cortex-M limitations, issues, and mitigation (download [4_relations_interactive_fig.html](https://github.com/CactiLab/code-SoK-Cortex-M/blob/main/4_relations_interactive_fig.html))
5. A figure demonstrates the paper and CVE growing trends from year 2014 to 2023 (in [5_paper_and_cve_trend.pdf](https://github.com/CactiLab/code-SoK-Cortex-M/blob/main/5_paper_and_cve_trend.pdf)).
6. [The Google Sheet link](https://docs.google.com/spreadsheets/d/1xWaPkYhWN8H_5XIA9zWZErhj2j2HMoxlJKs5AMwcE5c/edit?usp=sharing) to a collection of Cortex-M-related CVEs.
