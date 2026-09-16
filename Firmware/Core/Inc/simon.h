#ifndef SIMON_H
#define SIMON_H

#include <stdint.h>

/* Simon64/128 : block = 64 bits (two 32-bit words), key = 128 bits, 44 rounds */
#define SIMON_ROUNDS   44
#define SIMON_KEYWORDS 4      /* m = 4 for the 128-bit key variant */

/* Inline circular rotates on 32-bit words. Map to single ARM ror/barrel-shift. */
#define ROTL32(x, r) (((x) << (r)) | ((x) >> (32 - (r))))
#define ROTR32(x, r) (((x) >> (r)) | ((x) << (32 - (r))))

/* Expand the 128-bit master key (4 words, k[0] is least-significant word)
 * into SIMON_ROUNDS round keys. */
void simon_key_schedule(const uint32_t key[SIMON_KEYWORDS],
                        uint32_t round_keys[SIMON_ROUNDS]);

/* Encrypt one 64-bit block. ct may alias pt.
 * Convention: word[0] = left/upper half, word[1] = right/lower half. */
void simon_encrypt(const uint32_t round_keys[SIMON_ROUNDS],
                   const uint32_t pt[2], uint32_t ct[2]);

void simon_decrypt(const uint32_t round_keys[SIMON_ROUNDS],
                   const uint32_t ct[2], uint32_t pt[2]);

#endif /* SIMON_H */
