#include "simon.h"

/* z-sequence z3 for Simon64/128 (m=4 key words), consumed LSB-first.
 * Derived from the constant 4336092406851450331 and verified against the
 * official NSA test vector. */
static const uint8_t Z3[62] = {
    1,1,0,1,1,0,1,1,1,0,1,0,1,1,0,0,
    0,1,1,0,0,1,0,1,1,1,1,0,0,0,0,0,
    0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,1,
    0,0,1,1,0,1,0,0,0,0,1,1,1,1
};

/* Round-constant base c = 2^n - 4 = 0xFFFFFFFC for n=32. */
#define SIMON_C 0xFFFFFFFCu

void simon_key_schedule(const uint32_t key[SIMON_KEYWORDS],
                        uint32_t rk[SIMON_ROUNDS])
{
    /* First m round keys are the master key words directly. */
    rk[0] = key[0];
    rk[1] = key[1];
    rk[2] = key[2];
    rk[3] = key[3];

    for (int i = SIMON_KEYWORDS; i < SIMON_ROUNDS; i++) {
        uint32_t tmp = ROTR32(rk[i - 1], 3);
        tmp ^= rk[i - 3];                 /* m=4 path mixes in k[i-3] */
        tmp ^= ROTR32(tmp, 1);
        /* rk[i] = c ^ z_bit ^ rk[i-m] ^ tmp   (c = 2^n - 4) */
        rk[i] = SIMON_C ^ (uint32_t)Z3[(i - SIMON_KEYWORDS) % 62]
                ^ rk[i - SIMON_KEYWORDS] ^ tmp;
    }
}

void simon_encrypt(const uint32_t rk[SIMON_ROUNDS],
                   const uint32_t pt[2], uint32_t ct[2])
{
    uint32_t x = pt[0];   /* left  */
    uint32_t y = pt[1];   /* right */

    for (int i = 0; i < SIMON_ROUNDS; i++) {
        uint32_t t = x;
        /* f(x) = (ROTL(x,1) & ROTL(x,8)) ^ ROTL(x,2) */
        x = y ^ (ROTL32(x, 1) & ROTL32(x, 8)) ^ ROTL32(x, 2) ^ rk[i];
        y = t;
    }
    ct[0] = x;
    ct[1] = y;
}

void simon_decrypt(const uint32_t rk[SIMON_ROUNDS],
                   const uint32_t ct[2], uint32_t pt[2])
{
    uint32_t x = ct[0];
    uint32_t y = ct[1];

    for (int i = SIMON_ROUNDS - 1; i >= 0; i--) {
        uint32_t t = y;
        y = x ^ (ROTL32(y, 1) & ROTL32(y, 8)) ^ ROTL32(y, 2) ^ rk[i];
        x = t;
    }
    pt[0] = x;
    pt[1] = y;
}
