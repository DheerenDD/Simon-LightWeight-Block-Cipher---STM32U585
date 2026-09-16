#ifndef BENCH_H
#define BENCH_H

#include <stdint.h>

/* Enable the DWT cycle counter. Call once at startup. Returns 0 on success. */
int bench_dwt_init(void);

/* Run the full benchmark sweep and print results over UART. */
void bench_run(uint32_t sysclk_hz);

#endif
