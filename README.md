# Simon64/128 (software) vs AES-128 (hardware) on STM32U585 Cortex-M33

A cycle-accurate benchmark comparing the **Simon64/128 lightweight block cipher in software** against the **STM32U585's on-chip hardware AES-128 accelerator**, on the B-U585I-IOT02A Discovery board (Arm Cortex-M33 @ 160 MHz).

**Central question:** on a modern microcontroller that ships with a crypto accelerator, does a *lightweight* cipher in software actually beat the *standard* cipher in dedicated hardware?

**Answer:** no — hardware AES-128 is roughly **6× more efficient per byte** than software Simon64/128 (~35 vs ~211 cycles/byte). "Lightweight" describes silicon footprint and gate count, not software execution speed.

---

## Key results

| Configuration | Implementation | Asymptotic efficiency |
|---|---|---|
| Simon64/128 ECB | Software (C) | ~211 cycles/byte |
| Simon64/128 CBC | Software (C) | ~215 cycles/byte |
| AES-128 ECB | Hardware accelerator | ~35 cycles/byte |
| AES-128 CBC | Hardware accelerator | ~35 cycles/byte |

- Hardware AES wins by ~6× per byte, in both ECB and CBC modes.
- Cipher block chaining adds a negligible cost on both ciphers (<2% on Simon, <0.2% on hardware AES at bulk sizes).
- All four configurations are verified against official test vectors (NSA Simon, FIPS-197 AES-ECB, NIST SP800-38A AES-CBC) **before** any timing is recorded.

![Cycles per byte vs data size](docs/figures/fig1_cpb.png)

---

## Why this is interesting

Prior benchmarking of Simon (El Hanine et al., 2025) covered only Cortex-M3 and Cortex-M4 devices (STM32 F1/F4/L4) and compared lightweight ciphers only against each other, in software. This project:

1. **Extends the comparison to the Cortex-M33** (STM32U585) — a newer, security-oriented core never previously benchmarked for Simon.
2. **Reframes the question as software-lightweight vs hardware-standard** on a single device — the choice a firmware engineer actually faces on accelerator-equipped hardware.

On the M33, this Simon implementation reaches ~211 cycles/byte, substantially better than the earlier-generation figures (480–765 cycles/byte), thanks to the improved pipeline, enabled I-cache, and an optimized implementation.

---

## Methodology (in brief)

- **Timing:** Cortex-M33 DWT cycle counter (`DWT->CYCCNT`), 1 tick = 1 core cycle at 160 MHz (6.25 ns resolution).
- **Statistic:** minimum over 200 trials (the floor is the true cost; noise only adds cycles).
- **Discipline:** warm-up runs, measurement-overhead subtraction (14-cycle floor), block sweep {1, 2, 4, 8, 16, 32, 64}.
- **Fairness:** normalized to **cycles per byte** (Simon = 8-byte blocks, AES = 16-byte blocks); AES driven in **polling mode** for a clean per-operation comparison.
- **Correctness gate:** every cipher passes its official known-answer test on-target before timing.

See [`docs/Simon_vs_AES_Report.pdf`](docs/Simon_vs_AES_Report.pdf) for the full write-up, including the justification of each comparison and threats-to-validity analysis.

---

## Repository layout

```
firmware/     STM32CubeIDE project (Simon core, AES wrapper, timing harness, self-tests)
analysis/     Python script + measured data to regenerate the figures
docs/         The 20-page report (PDF) and generated figures
images/       Serial-capture screenshot of the on-target run
```

---

## Building and running the firmware

**Requirements:** STM32CubeIDE, B-U585I-IOT02A Discovery board.

1. Open `firmware/simon_vs_aes.ioc` in STM32CubeIDE (or import the `firmware/` project directly).
2. Key config: SYSCLK 160 MHz (VOS range 1), AES peripheral enabled, USART1 on PA9/PA10 (ST-Link VCP) at 115200 8N1, TrustZone disabled.
3. Build and flash.
4. Open a serial terminal on the ST-Link Virtual COM Port at **115200 8N1**, press RESET.

The device runs the known-answer tests and the full benchmark sweep at startup and prints the results over UART. Expected output is in [`analysis/raw_serial_output.txt`](analysis/raw_serial_output.txt).

---

## Reproducing the figures

```bash
cd analysis
pip install matplotlib numpy
python plot_results.py     # reads benchmark_data.csv, writes figures to ../docs/figures/
```

---

## What "lightweight" really means

Simon's round function uses only AND, XOR, and rotation — no S-boxes, no modular addition. That makes it tiny in **hardware gates**, which is its design goal. But as a *software loop* over 44 rounds it still costs many instructions per byte. AES, though algorithmically heavier, runs here on **fixed-function silicon** that processes a block in a small fixed number of cycles. The comparison measures software arithmetic against dedicated hardware — and the hardware wins.

The practical takeaway: on a microcontroller with an AES accelerator, prefer the hardware accelerator for performance- and energy-sensitive encryption. Software lightweight ciphers retain their niche only where no accelerator is present, where a specific algorithm is mandated, or where minimal code footprint is the binding constraint.

---

## Authors

Ankitha Mule · Dheerendranath Dadige
M.Eng. Information Technology, SRH Hochschule Heidelberg

## Reference

M. El Hanine, A. El-Yahyaoui, R. Es-Sadaoui, "Design and assessment of lightweight cryptographic algorithms on ESP32 and STM32 for IoD security," *Egyptian Informatics Journal*, vol. 32, 100818, 2025.

## License

Released under the MIT License — see [LICENSE](LICENSE).
