#ifndef AES_SELFTEST_H
#define AES_SELFTEST_H

#include <stdint.h>

/* Runs the FIPS-197 AES-128 ECB known-answer test on the HW engine.
 * Returns 0 = PASS, nonzero = FAIL. got_ct (4 words) optionally filled. */
int aes_hw_selftest(uint32_t got_ct[4]);

/* Runs the NIST SP800-38A AES-128 CBC known-answer test on the HW engine.
 * Returns 0 = PASS, nonzero = FAIL. got_ct (4 words) optionally filled. */
int aes_hw_selftest_cbc(uint32_t got_ct[4]);

#endif
