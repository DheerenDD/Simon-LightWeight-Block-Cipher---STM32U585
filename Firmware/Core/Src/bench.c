#include "bench.h"
#include "simon.h"
#include "aes_hw.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;

/* ---------- UART print helper ---------- */
static void up(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

/* ---------- DWT cycle counter ---------- */
int bench_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;   /* enable trace */
#ifdef DWT_LAR_KEY
    DWT->LAR = 0xC5ACCE55;                              /* unlock (if present) */
#endif
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;              /* start counter */

    /* Verify it actually ticks (the classic M33 gotcha). */
    uint32_t t0 = DWT->CYCCNT;
    for (volatile int i = 0; i < 10; i++) { }
    uint32_t t1 = DWT->CYCCNT;
    return (t1 != t0) ? 0 : -1;
}

static inline uint32_t cyc_now(void) { return DWT->CYCCNT; }

/* ---------- measurement config ---------- */
#define MAX_BLOCKS   64
#define N_TRIALS     200      /* repeats per measurement; we keep the min */

static const uint32_t g_block_counts[] = {1,2,4,8,16,32,64};
#define N_SWEEP (sizeof(g_block_counts)/sizeof(g_block_counts[0]))

/* Buffers sized for the largest sweep point.
 * Simon block = 8 bytes (2 words); AES block = 16 bytes (4 words). */
static uint32_t simon_in [MAX_BLOCKS * 2];
static uint32_t simon_out[MAX_BLOCKS * 2];
static uint32_t aes_in   [MAX_BLOCKS * 4];
static uint32_t aes_out  [MAX_BLOCKS * 4];

/* ---------- Simon multi-block, ECB and CBC ---------- */
static void simon_ecb(const uint32_t rk[SIMON_ROUNDS],
                      const uint32_t *in, uint32_t *out, uint32_t nblk)
{
    for (uint32_t b = 0; b < nblk; b++)
        simon_encrypt(rk, &in[b*2], &out[b*2]);
}

static void simon_cbc(const uint32_t rk[SIMON_ROUNDS],
                      const uint32_t *in, uint32_t *out, uint32_t nblk,
                      const uint32_t iv[2])
{
    uint32_t prev0 = iv[0], prev1 = iv[1];
    for (uint32_t b = 0; b < nblk; b++) {
        uint32_t blk[2];
        blk[0] = in[b*2]   ^ prev0;
        blk[1] = in[b*2+1] ^ prev1;
        simon_encrypt(rk, blk, &out[b*2]);
        prev0 = out[b*2];
        prev1 = out[b*2+1];
    }
}

/* measurement overhead (empty timed region) to subtract */
static uint32_t measure_overhead(void)
{
    uint32_t best = 0xFFFFFFFFu;
    for (int t = 0; t < N_TRIALS; t++) {
        uint32_t s = cyc_now();
        uint32_t e = cyc_now();
        uint32_t d = e - s;
        if (d < best) best = d;
    }
    return best;
}

/* ---------- report a row ---------- */
static void report_row(const char *tag, uint32_t nblk, uint32_t bytes,
                       uint32_t cycles, uint32_t sysclk_hz)
{
    uint64_t ns = (uint64_t)cycles * 1000000000ull / sysclk_hz;
    uint32_t cpb_x1000 = (uint32_t)((uint64_t)cycles * 1000ull / bytes);

    char buf[160];
    snprintf(buf, sizeof buf,
        "  %-10s blk=%2lu  bytes=%4lu  cyc=%7lu  t=%6lu ns  c/B=%lu.%03lu\r\n",
        tag,
        (unsigned long)nblk, (unsigned long)bytes,
        (unsigned long)cycles, (unsigned long)ns,
        (unsigned long)(cpb_x1000/1000), (unsigned long)(cpb_x1000%1000));
    up(buf);
}

/* ---------- main sweep ---------- */
void bench_run(uint32_t sysclk_hz)
{
    char buf[96];

    up("\r\n=== DWT benchmark (min cycles over ");
    snprintf(buf, sizeof buf, "%d trials, fclk=%lu Hz) ===\r\n",
             N_TRIALS, (unsigned long)sysclk_hz);
    up(buf);

    uint32_t ovh = measure_overhead();
    snprintf(buf, sizeof buf, "  (timing overhead floor: %lu cyc, subtracted)\r\n",
             (unsigned long)ovh);
    up(buf);

    /* fill input buffers with non-trivial data */
    for (int i = 0; i < MAX_BLOCKS*2; i++) simon_in[i] = 0x6E696C63u ^ (uint32_t)(i*0x9E3779B9u);
    for (int i = 0; i < MAX_BLOCKS*4; i++) aes_in[i]   = 0x00112233u ^ (uint32_t)(i*0x9E3779B9u);

    /* keys / IVs */
    const uint32_t skey[4]  = {0x03020100u,0x0b0a0908u,0x13121110u,0x1b1a1918u};
    const uint32_t aeskey[4]= {0x00010203u,0x04050607u,0x08090a0bu,0x0c0d0e0fu};
    const uint32_t aesiv[4] = {0x00010203u,0x04050607u,0x08090a0bu,0x0c0d0e0fu};
    const uint32_t iv2[2]   = {0x11111111u,0x22222222u};
    uint32_t srk[SIMON_ROUNDS];
    simon_key_schedule(skey, srk);

    /* ---- Simon ECB ---- */
    up("\r\n[Simon64/128 ECB]\r\n");
    for (uint32_t s = 0; s < N_SWEEP; s++) {
        uint32_t nblk = g_block_counts[s];
        simon_ecb(srk, simon_in, simon_out, nblk);   /* warm-up */
        uint32_t best = 0xFFFFFFFFu;
        for (int t = 0; t < N_TRIALS; t++) {
            uint32_t c0 = cyc_now();
            simon_ecb(srk, simon_in, simon_out, nblk);
            uint32_t d = cyc_now() - c0;
            if (d < best) best = d;
        }
        best -= ovh;
        report_row("ECB", nblk, nblk*8, best, sysclk_hz);
    }

    /* ---- Simon CBC ---- */
    up("\r\n[Simon64/128 CBC]\r\n");
    for (uint32_t s = 0; s < N_SWEEP; s++) {
        uint32_t nblk = g_block_counts[s];
        simon_cbc(srk, simon_in, simon_out, nblk, iv2);  /* warm-up */
        uint32_t best = 0xFFFFFFFFu;
        for (int t = 0; t < N_TRIALS; t++) {
            uint32_t c0 = cyc_now();
            simon_cbc(srk, simon_in, simon_out, nblk, iv2);
            uint32_t d = cyc_now() - c0;
            if (d < best) best = d;
        }
        best -= ovh;
        report_row("CBC", nblk, nblk*8, best, sysclk_hz);
    }

    /* ---- HW AES ECB ---- */
    up("\r\n[HW AES-128 ECB]\r\n");
    aes_hw_init_ecb(aeskey);
    for (uint32_t s = 0; s < N_SWEEP; s++) {
        uint32_t nblk = g_block_counts[s];
        aes_hw_encrypt_ecb(aes_in, aes_out, nblk);   /* warm-up */
        uint32_t best = 0xFFFFFFFFu;
        for (int t = 0; t < N_TRIALS; t++) {
            uint32_t c0 = cyc_now();
            aes_hw_encrypt_ecb(aes_in, aes_out, nblk);
            uint32_t d = cyc_now() - c0;
            if (d < best) best = d;
        }
        best -= ovh;
        report_row("AES-ECB", nblk, nblk*16, best, sysclk_hz);
    }

    /* ---- HW AES CBC ---- */
    up("\r\n[HW AES-128 CBC]\r\n");
    aes_hw_init_cbc(aeskey, aesiv);
    for (uint32_t s = 0; s < N_SWEEP; s++) {
        uint32_t nblk = g_block_counts[s];
        aes_hw_encrypt_cbc(aes_in, aes_out, nblk);   /* warm-up */
        uint32_t best = 0xFFFFFFFFu;
        for (int t = 0; t < N_TRIALS; t++) {
            uint32_t c0 = cyc_now();
            aes_hw_encrypt_cbc(aes_in, aes_out, nblk);
            uint32_t d = cyc_now() - c0;
            if (d < best) best = d;
        }
        best -= ovh;
        report_row("AES-CBC", nblk, nblk*16, best, sysclk_hz);
    }

    up("\r\n=== end benchmark ===\r\n");
}
