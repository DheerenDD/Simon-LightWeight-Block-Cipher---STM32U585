# Firmware

STM32CubeIDE project for the B-U585I-IOT02A.

## Add your project files here

Copy your CubeIDE project into this folder, preserving structure:

- `Core/Src/` and `Core/Inc/` — including your hand-written modules:
  - `simon.c` / `simon.h` — Simon64/128 core
  - `aes_hw.c` / `aes_hw.h` — HAL_CRYP wrapper (ECB + CBC)
  - `aes_selftest.c` / `aes_selftest.h` — known-answer tests
  - `bench.c` / `bench.h` — DWT timing harness
  - `selftest_run.c` / `selftest_run.h` — boot-time verification + report
- `*.ioc` — CubeMX configuration
- `.project`, `.cproject` — CubeIDE project files
- `Drivers/` — HAL drivers

The `Debug/` build folder is gitignored.

## Configuration summary

- SYSCLK 160 MHz, Power Regulator Voltage Scale 1
- AES peripheral enabled, polling mode
- USART1 on PA9/PA10 (ST-Link VCP), 115200 8N1
- TrustZone disabled
- Instruction cache enabled
