#include "selftest_run.h"
#include "simon.h"
#include "aes_selftest.h"
#include "main.h"      /* HAL types + CubeMX handles (huart1 defined in main.c) */
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;

/* Blocking UART print helper (no printf retarget needed). */
static void up(const char *s)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

static void up_line(const char *label, const uint32_t *w, int n, const char *tag)
{
    char buf[96];
    int p = snprintf(buf, sizeof buf, "%s", label);
    for (int i = 0; i < n; i++)
        p += snprintf(buf + p, sizeof buf - p, "%08lx ", (unsigned long)w[i]);
    snprintf(buf + p, sizeof buf - p, " %s\r\n", tag ? tag : "");
    up(buf);
}

void run_selftests(void)
{
    up("\r\n=== Simon64/128 vs HW AES :: self-tests ===\r\n");

    /* ---- Simon64/128 against NSA test vector ---- */
    const uint32_t skey[4] = {0x03020100u,0x0b0a0908u,0x13121110u,0x1b1a1918u};
    const uint32_t spt[2]  = {0x656b696cu,0x20646e75u};
    const uint32_t sexp[2] = {0x44c8fc20u,0xb9dfa07au};
    uint32_t srk[SIMON_ROUNDS], sct[2], sback[2];

    simon_key_schedule(skey, srk);
    simon_encrypt(srk, spt, sct);
    simon_decrypt(srk, sct, sback);

    int simon_enc_ok = (sct[0]==sexp[0] && sct[1]==sexp[1]);
    int simon_rt_ok  = (sback[0]==spt[0] && sback[1]==spt[1]);

    up("\r\n[Simon64/128]\r\n");
    up_line("  CT  = ", sct, 2, simon_enc_ok ? "(vector PASS)" : "(vector FAIL)");
    up(simon_rt_ok ? "  round-trip: PASS\r\n" : "  round-trip: FAIL\r\n");

    /* ---- HW AES-128 ECB against FIPS-197 KAT ---- */
    uint32_t aes_ct[4];
    int aes_rc = aes_hw_selftest(aes_ct);

    up("\r\n[HW AES-128 ECB]\r\n");
    up_line("  CT  = ", aes_ct, 4, (aes_rc==0) ? "(KAT PASS)" : "(KAT FAIL)");
    if (aes_rc != 0) {
        char b[48];
        snprintf(b, sizeof b, "  selftest rc=%d (see aes_hw.c codes)\r\n", aes_rc);
        up(b);
    }

    /* ---- HW AES-128 CBC against NIST SP800-38A KAT ---- */
    uint32_t aes_cbc_ct[4];
    int aes_cbc_rc = aes_hw_selftest_cbc(aes_cbc_ct);

    up("\r\n[HW AES-128 CBC]\r\n");
    up_line("  CT  = ", aes_cbc_ct, 4, (aes_cbc_rc==0) ? "(KAT PASS)" : "(KAT FAIL)");
    if (aes_cbc_rc != 0) {
        char b[48];
        snprintf(b, sizeof b, "  selftest rc=%d\r\n", aes_cbc_rc);
        up(b);
    }

    up("\r\n=== end self-tests ===\r\n");
}
