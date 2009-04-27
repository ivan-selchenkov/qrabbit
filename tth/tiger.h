/* tiger.h */
#ifndef TIGER_H
#define TIGER_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define tiger_block_size 64

typedef struct tiger_ctx {
  /* the order of the fields slightly influence the algorithm speed */
  uint64_t state[3];
  /* 512-bit buffer for leftovers */
  unsigned char message[tiger_block_size];
  uint64_t length;
} tiger_ctx;

void tiger_init(tiger_ctx *ctx);
void tiger_update(tiger_ctx *ctx, const unsigned char* msg, unsigned size);
void tiger_final(tiger_ctx *ctx, unsigned char result[24]);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* TIGER_H */
