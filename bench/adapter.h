#ifndef BENCH_ADAPTER_H
#define BENCH_ADAPTER_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include "hawk.h"
#ifndef LOGN
#define LOGN 9
#endif
#define B_N (1<<LOGN)
#define B_Q (-1)
#define B_PK HAWK_PUBKEY_SIZE(LOGN)
#define B_SK HAWK_PRIVKEY_SIZE(LOGN)
#define B_SIG HAWK_SIG_SIZE(LOGN)
#define B_EXPANDED 1
extern int bench_testing;
void bench_fill(void *,size_t);
static shake_context brng;
static int rng_ready;
static uint64_t tmp[(HAWK_TMPSIZE_KEYGEN(LOGN)+7)/8];
static uint8_t decoded[HAWK_PRIVKEY_DECODED_SIZE(LOGN)];
static void rng(void*c,void*p,size_t n){(void)c;if(bench_testing){bench_fill(p,n);return;}if(!rng_ready){uint8_t seed[32];if(getrandom(seed,32,0)!=32)abort();shake_init(&brng,256);shake_inject(&brng,seed,32);shake_flip(&brng);rng_ready=1;}shake_extract(&brng,p,n);}
static int b_keygen(uint8_t*p,uint8_t*s){return !hawk_keygen(LOGN,s,p,rng,0,tmp,sizeof tmp);}
static int b_sign(uint8_t*s,size_t*l,const uint8_t*m,size_t n,const uint8_t*k){shake_context sc;hawk_sign_start(&sc);shake_inject(&sc,m,n);*l=B_SIG;return !hawk_sign_finish(LOGN,rng,0,s,&sc,k,tmp,sizeof tmp);}
static int b_verify(const uint8_t*s,size_t l,const uint8_t*m,size_t n,const uint8_t*k){shake_context sc;hawk_verify_start(&sc);shake_inject(&sc,m,n);return !hawk_verify_finish(LOGN,s,l,&sc,k,B_PK,tmp,sizeof tmp);}
static int b_expand(const uint8_t*k){hawk_decode_private_key(LOGN,decoded,k);return 0;}
static int b_expanded_sign(uint8_t*s,size_t*l,const uint8_t*m,size_t n){shake_context sc;hawk_sign_start(&sc);shake_inject(&sc,m,n);*l=B_SIG;return !hawk_sign_finish_alt(LOGN,rng,0,s,&sc,decoded,sizeof decoded,tmp,sizeof tmp);}
static size_t b_unpadded(const uint8_t*s,size_t n){size_t bit=(LOGN==9?24:40)*8+(size_t)B_N*(LOGN==10?7:6);for(int i=0;i<B_N;i++){for(;;){if(bit>=n*8)return 0;unsigned v=(s[bit/8]>>(bit%8))&1;bit++;if(v)break;}}size_t used=(bit+7)/8;int16_t decoded_sig[HAWK_DECODED_SIG_LENGTH(LOGN)];if(!hawk_decode_signature(LOGN,decoded_sig,s,n))return 0;size_t expected=(LOGN==9?24:40)*8+(size_t)B_N*(LOGN==10?8:7);for(int i=0;i<B_N;i++){int x=decoded_sig[i];unsigned a=x<0?(unsigned)(-x-1):(unsigned)x;expected+=a>>(LOGN==10?6:5);}return (expected+7)/8==used?used:0;}
#endif
