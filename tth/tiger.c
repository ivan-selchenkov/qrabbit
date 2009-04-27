/* tiger.c - an implementation of Tiger Hash Function
 * based on the article by
 * Ross Anderson and Eli Biham "Tiger: A Fast New Hash Function".
 *
 * Implementation written by Alexei Kravchenko.
 */
#include <string.h>
#include "byte_order.h"
#include "tiger.h"

/* initialize context */
void tiger_init(tiger_ctx *ctx) {
  ctx->state[0] = 0x0123456789ABCDEFLL;
  ctx->state[1] = 0xFEDCBA9876543210LL;
  ctx->state[2] = 0xF096A5B4C3B2E187LL;
  ctx->length = 0;
}

/* lookup tables */
extern uint64_t tiger_sboxes[4*256];
#define t1 tiger_sboxes
#define t2 (tiger_sboxes+256)
#define t3 (tiger_sboxes+512)
#define t4 (tiger_sboxes+768)

/*#define round(a,b,c,x,mul) \
  c ^= x; \
  a -= t1[((c)>>(0*8))&0xFF] ^ t2[((c)>>(2*8))&0xFF] ^ \
       t3[((c)>>(4*8))&0xFF] ^ t4[((c)>>(6*8))&0xFF] ; \
  b += t4[((c)>>(1*8))&0xFF] ^ t3[((c)>>(3*8))&0xFF] ^ \
       t2[((c)>>(5*8))&0xFF] ^ t1[((c)>>(7*8))&0xFF] ; \
  b *= mul;*/

/* for IA32 */
#define round(a,b,c,x,mul) \
  c ^= x; \
  a -= t1[(uint8_t)(c)] ^ \
       t2[(uint8_t)(((uint32_t)(c))>>(2*8))] ^ \
       t3[(uint8_t)((c)>>(4*8))] ^ \
       t4[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(2*8))] ; \
  b += t4[(uint8_t)(((uint32_t)(c))>>(1*8))] ^ \
       t3[(uint8_t)(((uint32_t)(c))>>(3*8))] ^ \
       t2[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(1*8))] ^ \
       t1[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(3*8))]; \
       b *= mul;

#define pass(a,b,c,mul) \
  round(a,b,c,x0,mul) \
  round(b,c,a,x1,mul) \
  round(c,a,b,x2,mul) \
  round(a,b,c,x3,mul) \
  round(b,c,a,x4,mul) \
  round(c,a,b,x5,mul) \
  round(a,b,c,x6,mul) \
  round(b,c,a,x7,mul)

#define key_schedule { \
  x0 -= x7 ^ 0xA5A5A5A5A5A5A5A5LL; \
  x1 ^= x0; \
  x2 += x1; \
  x3 -= x2 ^ ((~x1)<<19); \
  x4 ^= x3; \
  x5 += x4; \
  x6 -= x5 ^ ((~x4)>>23); \
  x7 ^= x6; \
  x0 += x7; \
  x1 -= x0 ^ ((~x7)<<19); \
  x2 ^= x1; \
  x3 += x2; \
  x4 -= x3 ^ ((~x2)>>23); \
  x5 ^= x4; \
  x6 += x5; \
  x7 -= x6 ^ 0x0123456789ABCDEFLL; \
}

/* process a 512-bit block */
static void tiger_process_message_block(uint64_t state[3], uint64_t* block) {
  /* Optimized for GCC IA32.
     The order of declarations is important for compiler. */
  register uint64_t a, b, c;
  register uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
  register uint64_t tmp;
  char i;
  
  x0=le2me_64(block[0]); x1=le2me_64(block[1]); 
  x2=le2me_64(block[2]); x3=le2me_64(block[3]);
  x4=le2me_64(block[4]); x5=le2me_64(block[5]);
  x6=le2me_64(block[6]); x7=le2me_64(block[7]);
  
  a = state[0];
  b = state[1];
  c = state[2];

  /* passes and key shedules */
  for(i=0; i<3; i++) {
    if(i != 0) key_schedule;
    pass(a, b, c, ( i==0 ? 5 : i==1 ? 7 : 9 ));
    tmp=a; a=c; c=b; b=tmp;
  }

  /* feedforward operation */
  state[0] = a ^ state[0];
  state[1] = b - state[1];
  state[2] = c + state[2];
}

void tiger_update(tiger_ctx *ctx, const unsigned char* msg, unsigned size) {
  unsigned index = (unsigned)ctx->length & 63;
  unsigned left;
  ctx->length += size;

  /* Try to fill partial block */
  if (index) {
    left = tiger_block_size - index;
    if (size < left) {
      memcpy(ctx->message + index, msg, size);
      return;
    } else {
      memcpy(ctx->message + index, msg, left);
      tiger_process_message_block(ctx->state, (uint64_t*)ctx->message);
      msg += left;
      size -= left;
    }
  }
  while (size >= tiger_block_size) {
    if( IS_ALIGNED_64(msg) ) {
      /* the most common case is processing of an already aligned message 
         without copying it */
      tiger_process_message_block(ctx->state, (uint64_t*)msg);
    } else {
      memcpy(ctx->message, msg, tiger_block_size);
      tiger_process_message_block(ctx->state, (uint64_t*)ctx->message);
    }

    msg += tiger_block_size;
    size -= tiger_block_size;
  }
  if(size) {
    /* save leftovers */
    memcpy(ctx->message, msg, size);
  }
}

void tiger_final(tiger_ctx *ctx, unsigned char result[24]) {
  unsigned index = (unsigned)ctx->length & 63;

  /* pad message and run for last block */
  ctx->message[index++] = 0x01;
  while( index&7 ) {
      ctx->message[index++] = 0;
  }

  /* if no room left in the message to store 64-bit message length */
  if(index>56) {
    /* then pad the rest with zeros and process it */
    while(index < 64) {
      ctx->message[index++] = 0;
    }
    tiger_process_message_block(ctx->state, (uint64_t*)ctx->message);
    index = 0;
  }
  while(index < 56) {
    ctx->message[index++] = 0;
  }
  ((uint64_t*)(&(ctx->message[56])))[0] = le2me_64(ctx->length << 3);
  tiger_process_message_block(ctx->state, (uint64_t*)ctx->message);
  
  /* save result hash */
  memcpy(result, &ctx->state, 24);
}
