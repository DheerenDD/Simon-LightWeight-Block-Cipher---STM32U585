# Integrating the benchmark modules into main.c

The files under `Core/Src` and `Core/Inc` are drop-in modules. They are wired
into the CubeMX-generated `main.c` with two small insertions, both inside the
`USER CODE` regions so they survive project regeneration.

## 1. Include (in `/* USER CODE BEGIN Includes */`)

```c
#include "selftest_run.h"
#include "bench.h"
```

## 2. Run (in `/* USER CODE BEGIN 2 */`, after the MX_*_Init() calls)

```c
run_selftests();          /* verify Simon + AES against official vectors */
bench_dwt_init();         /* enable the DWT cycle counter */
bench_run(160000000u);    /* full benchmark sweep @ 160 MHz */
```

That is all. On boot the device prints the known-answer-test results and the
full benchmark sweep over USART1.

## Handles used

The modules reference two CubeMX-generated handles via `extern`:

- `CRYP_HandleTypeDef hcryp;`  (AES peripheral)
- `UART_HandleTypeDef huart1;` (USART1, ST-Link VCP)

If your generated handle names differ (e.g. per-peripheral file generation
places them in `aes.c`/`usart.c` instead of `main.c`), the `extern`
declarations still resolve as long as the names match. Adjust the includes if
you generated per-peripheral files:

- with per-peripheral files: `#include "aes.h"`, `#include "usart.h"`
- single-file layout (default): `#include "main.h"` covers both (already used)

## Expected output

```
=== Simon64/128 vs HW AES :: self-tests ===

[Simon64/128]
  CT  = 44c8fc20 b9dfa07a  (vector PASS)
  round-trip: PASS

[HW AES-128 ECB]
  CT  = 69c4e0d8 6a7b0430 d8cdb780 70b4c55a  (KAT PASS)

[HW AES-128 CBC]
  CT  = 7649abac 8119b246 cee98e9b 12e9197d  (KAT PASS)

=== end self-tests ===

=== DWT benchmark (min cycles over 200 trials, fclk=160000000 Hz) ===
  ... (full sweep for Simon ECB/CBC and AES ECB/CBC) ...
=== end benchmark ===
```
