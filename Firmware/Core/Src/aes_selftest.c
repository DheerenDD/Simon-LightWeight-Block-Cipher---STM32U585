#include "aes_hw.h"
#include "aes_selftest.h"
#include <string.h>

/* On-target known-answer test for the AES hardware ECB path.
 * FIPS-197 Appendix B / NIST AES-128 ECB vector:
 *   key = 000102030405060708090a0b0c0d0e0f
 *   pt  = 00112233445566778899aabbccddeeff
 *   ct  = 69c4e0d86a7b0430d8cdb78070b4c55a
 * Returns 0 = PASS (enc matches + round-trip), nonzero = FAIL. */
int aes_hw_selftest(uint32_t got_ct[4])
{
    const uint32_t key[4] = {0x00010203u,0x04050607u,0x08090a0bu,0x0c0d0e0fu};
    const uint32_t pt[4]  = {0x00112233u,0x44556677u,0x8899aabbu,0xccddeeffu};
    const uint32_t exp[4] = {0x69c4e0d8u,0x6a7b0430u,0xd8cdb780u,0x70b4c55au};

    uint32_t ct[4], back[4];

    if (aes_hw_init_ecb(key) != 0)            return 1;
    if (aes_hw_encrypt_ecb(pt, ct, 1) != 0)   return 2;
    if (aes_hw_decrypt_ecb(ct, back, 1) != 0) return 3;

    if (got_ct) memcpy(got_ct, ct, sizeof ct);

    int enc_ok = (memcmp(ct, exp, sizeof exp) == 0);
    int rt_ok  = (memcmp(back, pt, sizeof pt) == 0);

    return (enc_ok && rt_ok) ? 0 : 4;
}

/* On-target known-answer test for the AES hardware CBC path.
 * NIST SP800-38A AES-128-CBC vector (first block):
 *   key = 2b7e151628aed2a6abf7158809cf4f3c
 *   iv  = 000102030405060708090a0b0c0d0e0f
 *   pt  = 6bc1bee22e409f96e93d7e117393172a
 *   ct  = 7649abac8119b246cee98e9b12e9197d
 * Returns 0 = PASS, nonzero = FAIL. */
int aes_hw_selftest_cbc(uint32_t got_ct[4])
{
    const uint32_t key[4] = {0x2b7e1516u,0x28aed2a6u,0xabf71588u,0x09cf4f3cu};
    const uint32_t iv[4]  = {0x00010203u,0x04050607u,0x08090a0bu,0x0c0d0e0fu};
    const uint32_t pt[4]  = {0x6bc1bee2u,0x2e409f96u,0xe93d7e11u,0x7393172au};
    const uint32_t exp[4] = {0x7649abacu,0x8119b246u,0xcee98e9bu,0x12e9197du};

    uint32_t ct[4], back[4];

    if (aes_hw_init_cbc(key, iv) != 0)        return 1;
    if (aes_hw_encrypt_cbc(pt, ct, 1) != 0)   return 2;

    /* reload IV before decrypt: CBC decrypt needs the original IV */
    if (aes_hw_init_cbc(key, iv) != 0)        return 3;
    if (aes_hw_decrypt_cbc(ct, back, 1) != 0) return 4;

    if (got_ct) memcpy(got_ct, ct, sizeof ct);

    int enc_ok = (memcmp(ct, exp, sizeof exp) == 0);
    int rt_ok  = (memcmp(back, pt, sizeof pt) == 0);

    return (enc_ok && rt_ok) ? 0 : 5;
}
