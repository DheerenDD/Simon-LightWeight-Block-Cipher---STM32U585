#include "aes_hw.h"
#include "main.h"   /* HAL types + CubeMX handles (hcryp defined in main.c) */

/* hcryp is defined in CubeMX-generated code (MX_AES_Init). */
extern CRYP_HandleTypeDef hcryp;

/* Storage for the active key/IV (HAL keeps pointers; keep them valid). */
static uint32_t s_key[4];
static uint32_t s_iv[4];

/* HAL timeout for blocking calls (ms). Generous; we measure cycles, not this. */
#define AES_HW_TIMEOUT 100u

/* ============================ ECB ============================ */

int aes_hw_init_ecb(const uint32_t key[4])
{
    for (int i = 0; i < 4; i++) {
        s_key[i] = key[i];
    }

    hcryp.Init.DataType    = CRYP_NO_SWAP;     /* words used as-is */
    hcryp.Init.KeySize     = CRYP_KEYSIZE_128B;
    hcryp.Init.pKey        = s_key;
    hcryp.Init.Algorithm   = CRYP_AES_ECB;
    hcryp.Init.pInitVect   = NULL;             /* ECB: no IV */

    if (HAL_CRYP_Init(&hcryp) != HAL_OK) {
        return -1;
    }
    return 0;
}

int aes_hw_encrypt_ecb(const uint32_t *in, uint32_t *out, uint32_t nblocks)
{
    uint32_t words = nblocks * 4u;             /* Size is in WORDS */
    if (HAL_CRYP_Encrypt(&hcryp, (uint32_t *)in, (uint16_t)words,
                         out, AES_HW_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int aes_hw_decrypt_ecb(const uint32_t *in, uint32_t *out, uint32_t nblocks)
{
    uint32_t words = nblocks * 4u;
    if (HAL_CRYP_Decrypt(&hcryp, (uint32_t *)in, (uint16_t)words,
                         out, AES_HW_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

/* ============================ CBC ============================ */

int aes_hw_init_cbc(const uint32_t key[4], const uint32_t iv[4])
{
    for (int i = 0; i < 4; i++) {
        s_key[i] = key[i];
        s_iv[i]  = iv[i];
    }

    hcryp.Init.DataType    = CRYP_NO_SWAP;     /* match the ECB/KAT convention */
    hcryp.Init.KeySize     = CRYP_KEYSIZE_128B;
    hcryp.Init.pKey        = s_key;
    hcryp.Init.Algorithm   = CRYP_AES_CBC;
    hcryp.Init.pInitVect   = s_iv;             /* CBC: 128-bit IV required */

    if (HAL_CRYP_Init(&hcryp) != HAL_OK) {
        return -1;
    }
    return 0;
}

int aes_hw_encrypt_cbc(const uint32_t *in, uint32_t *out, uint32_t nblocks)
{
    uint32_t words = nblocks * 4u;
    if (HAL_CRYP_Encrypt(&hcryp, (uint32_t *)in, (uint16_t)words,
                         out, AES_HW_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}

int aes_hw_decrypt_cbc(const uint32_t *in, uint32_t *out, uint32_t nblocks)
{
    uint32_t words = nblocks * 4u;
    if (HAL_CRYP_Decrypt(&hcryp, (uint32_t *)in, (uint16_t)words,
                         out, AES_HW_TIMEOUT) != HAL_OK) {
        return -1;
    }
    return 0;
}
