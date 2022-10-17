#ifndef ASCON_H_
#define ASCON_H_

#include <stdint.h>

#include "word.h"

typedef struct {
  word_t x0, x1, x2, x3, x4;
} state_t;

struct asconhasha_ctx {
  state_t s;          /* ASCON alg. State structure */
  uint8_t ipad[64];   /* HMAC: inner padding */
  uint8_t opad[64];   /* HMAC: outer padding */
};

void ascon_hashinit(state_t* s);
void ascon_absorb(state_t* s, const uint8_t* in, uint64_t inlen);
void ascon_squeeze(state_t* s, uint8_t* out, uint64_t outlen);

void asconhasha_hmac_init(struct asconhasha_ctx *ctx,const uint8_t *key, uint32_t keylen);
void asconhasha_hmac_update(struct asconhasha_ctx *ctx, const uint8_t *input, uint32_t ilen);
void asconhasha_hmac_final(struct asconhasha_ctx *ctx, uint8_t* out, uint64_t outlen);

#endif /* ASCON_H */
