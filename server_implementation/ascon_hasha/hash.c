#include "api.h"
#include "ascon.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "permutations.h"
#include "printstate.h"

#if !ASCON_INLINE_MODE
#undef forceinline
#define forceinline
#endif

forceinline void ascon_hashinit(state_t* s) {
  /* initialize */
#if ASCON_HASH_OUTLEN == 32 && ASCON_HASH_ROUNDS == 12
  s->x0 = ASCON_HASH_IV0;
  s->x1 = ASCON_HASH_IV1;
  s->x2 = ASCON_HASH_IV2;
  s->x3 = ASCON_HASH_IV3;
  s->x4 = ASCON_HASH_IV4;
#elif ASCON_HASH_OUTLEN == 32 && ASCON_HASH_ROUNDS == 8
  s->x0 = ASCON_HASHA_IV0;
  s->x1 = ASCON_HASHA_IV1;
  s->x2 = ASCON_HASHA_IV2;
  s->x3 = ASCON_HASHA_IV3;
  s->x4 = ASCON_HASHA_IV4;
#elif ASCON_HASH_OUTLEN == 0 && ASCON_HASH_ROUNDS == 12
  s->x0 = ASCON_XOF_IV0;
  s->x1 = ASCON_XOF_IV1;
  s->x2 = ASCON_XOF_IV2;
  s->x3 = ASCON_XOF_IV3;
  s->x4 = ASCON_XOF_IV4;
#elif ASCON_HASH_OUTLEN == 0 && ASCON_HASH_ROUNDS == 8
  s->x0 = ASCON_XOFA_IV0;
  s->x1 = ASCON_XOFA_IV1;
  s->x2 = ASCON_XOFA_IV2;
  s->x3 = ASCON_XOFA_IV3;
  s->x4 = ASCON_XOFA_IV4;
#endif
  printstate("initialization", s);
}

forceinline void ascon_absorb(state_t* s, const uint8_t* in, uint64_t inlen) {
  /* absorb full plaintext blocks */
  while (inlen >= ASCON_HASH_RATE) {
    s->x0 = XOR(s->x0, LOAD(in, 8));
    P(s, ASCON_HASH_ROUNDS);
    in += ASCON_HASH_RATE;
    inlen -= ASCON_HASH_RATE;
  }
  /* absorb final plaintext block */
  if (inlen) s->x0 = XOR(s->x0, LOAD(in, inlen));
  s->x0 = XOR(s->x0, PAD(inlen));
  P(s, 12);
  printstate("absorb plaintext", s);
}

forceinline void ascon_squeeze(state_t* s, uint8_t* out, uint64_t outlen) {
  /* squeeze full output blocks */
  while (outlen > ASCON_HASH_RATE) {
    STORE(out, s->x0, 8);
    P(s, ASCON_HASH_ROUNDS);
    out += ASCON_HASH_RATE;
    outlen -= ASCON_HASH_RATE;
  }
  /* squeeze final output block */
  STORE(out, s->x0, outlen);
  printstate("squeeze output", s);
}

int crypto_hash(unsigned char* out, const unsigned char* in,
                unsigned long long inlen) {
  state_t s;
  ascon_hashinit(&s);
  ascon_absorb(&s, in, inlen);
  ascon_squeeze(&s, out, CRYPTO_BYTES);
  return 0;
}

void asconhasha_hmac_init(struct asconhasha_ctx *ctx,const uint8_t *key, uint32_t keylen)
{
	uint32_t i;
	uint8_t sum[32];

	if (keylen > 64) {
		crypto_hash(sum, key, keylen);
		keylen = 32;
		key = sum;
	}

	memset(ctx->ipad, 0x36, 64);
	memset(ctx->opad, 0x5C, 64);

	for (i = 0; i < keylen; i++) {
		ctx->ipad[i] ^= key[i];
		ctx->opad[i] ^= key[i];
	}

	ascon_hashinit(&ctx->s);
	ascon_absorb(&ctx->s, ctx->ipad, 64);

}

void asconhasha_hmac_update(struct asconhasha_ctx *ctx, const uint8_t *input, uint32_t ilen)
{
	ascon_absorb(&ctx->s, input, ilen);
}

void asconhasha_hmac_final(struct asconhasha_ctx *ctx, uint8_t* out, uint64_t outlen)
{
	uint8_t tmpbuf[32];

	ascon_squeeze(&ctx->s, tmpbuf,CRYPTO_BYTES);
	ascon_hashinit(&ctx->s);
	ascon_absorb(&ctx->s,(uint8_t *)&ctx->opad, 64);
	ascon_absorb(&ctx->s, tmpbuf, 32);
	ascon_squeeze(&ctx->s, out, outlen);
}

int crypto_hmac(unsigned char* out, const unsigned char* in,
                unsigned long long inlen, const char *key) {
    struct asconhasha_ctx  *local_ctx = (struct asconhasha_ctx*) malloc(sizeof(struct asconhasha_ctx));
    asconhasha_hmac_init(local_ctx, key, 32);
    asconhasha_hmac_update(local_ctx, in, inlen);
    asconhasha_hmac_final(local_ctx, out, 32);

    free(local_ctx);

    return 0;
}

int main()
{
  uint8_t testVector_au8[] = {
      0x03, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x42, 0x30, 0x40, 0x80, 0x01, 0x03, 0x81, 0x01, 
      0x01, 0x82, 0x04, 0x56, 0x34, 0x12, 0x65, 0x83, 0x01, 0x01, 0x84, 0x02, 0x5A, 0x23, 0x85, 0x02, 
      0x64, 0x46, 0x86, 0x02, 0x00, 0xF0, 0x87, 0x04, 0xFF, 0xFF, 0x01, 0xFE, 0x88, 0x02, 0x10, 0xFE, 
      0x89, 0x01, 0x40, 0x8A, 0x01, 0x01, 0x8B, 0x07, 0x01, 0x23, 0x45, 0x60, 0x12, 0x34, 0x56, 0x8C, 
      0x02, 0xFF, 0xFF, 0xAD, 0x06, 0x80, 0x01, 0x01, 0x81, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 
    };
  uint8_t key_au8[32] = { 0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02, 
                                    0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02}; 
  uint8_t hash_au8[32] = {0};
  
  crypto_hmac(hash_au8,testVector_au8,sizeof(testVector_au8), key_au8);
  
  for (uint8_t i=0; i<32; i++)
  {
    printf("%x",hash_au8[i]);
  }
  printf("\n");
  return 0;
}
