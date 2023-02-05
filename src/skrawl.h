#ifndef SKRAWL_H
#define SKRAWL_H

// shared includes
#include <stdio.h>
#include <stdlib.h>    //mmap
#include <inttypes.h>
#include <string.h>    //memcpy
#include <stdbool.h>

// type macros
typedef uint64_t  K;
typedef int8_t    i8;
typedef uint8_t   u8;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef int64_t   i64;
typedef uint64_t  u64;

// K object types
enum {
    KK,                        //generic list
    KC,                        //char
    KI,                        //int64
    KF,                        //double
    KS,                        //symbol
    K_SIMPLE_LIST_END = KS,    //simple lists are composed of same-type atoms
    KL,                        //lambda
    KU,                        //monad
    KV,                        //dyad
    KW,                        //adverb
    K_ADVERB_START,            //start of adverb types
    KEACH = K_ADVERB_START,    // '
    KOVER,                     // /
    KSCAN,                     /* \ */ 
    KEP,                       // ':
    KER,                       // /:
    KEL,                       // \: 
    K_ADVERB_END = KEL,        //end of adverb types
    KM,                        //magic value(internal)
    KN,                        //null(internal)
    KE                         //error(internal)
};

#define VERB_STR    ":+-*%,?.@!$#_^&?=<>~|;"
#define ADVERB_STR  "'/\\'/\\"

// K header accessors
// the header is 16 bytes
// --mtrrrrnnnnnnnn
// - unused, m membucket, t type, r refcount, n count
#define HDR_MEM(x)    (( i8*)(x))[-14]
#define HDR_TYP(x)    (( i8*)(x))[-13]
#define HDR_REF(x)    ((u32*)(x))[-3]
#define HDR_CNT(x)    ((i64*)(x))[-1]
// tagged pointer type get/set. type encoded in upper 8 bits
#define TAG_TYP(x)    ((i8)((x)>>56))
#define TAG_VAL(x)    ((x) & 0x00ffffffffffffff)
#define SET_TAG(t,v)  ((((i64)(t))<<56) | (0x00ffffffffffffff & (i64)(v)))
// wrapper accessors
#define MEM(x)  HDR_MEM(x)  
#define TYP(x)  __extension__({K _a=(x); TAG_TYP(_a) ? TAG_TYP(_a) : HDR_TYP(_a);})  
#define REF(x)  HDR_REF(x)  
#define CNT(x)  __extension__({K _a=(x); TAG_TYP(_a) ? 1 : HDR_CNT(_a);})

// K object accessors
#define OBJ(x)  ((     K*)(x))  //pointer to generic K object list
#define CHR(x)  ((    u8*)(x))  //pointer to int8 
#define INT(x)  ((   i64*)(x))  //pointer to int64 
#define FLT(x)  ((double*)(x))  //pointer to double 

// shared utility macros
#define ABS(a)             __extension__({__typeof__(a)_a=(a); _a > 0 ? _a : -_a ;}) 
#define MAX(a,b)           __extension__({__typeof__(a)_a=(a);__typeof__(b)_b=(b);_a>_b?_a:_b;})
#define IS_SIMPLE_LIST(x)  __extension__({K _x=(x); 0<TYP(_x) && TYP(_x)<=K_SIMPLE_LIST_END;})
#define IS_VERB(a)         __extension__({i8 t=TYP(a); KU==t || KV==t;}) 
#define IS_ADVERB(a)       __extension__({i8 t=TYP(a); K_ADVERB_START<=t && t<=K_ADVERB_END;})
#define IS_GENERIC(x)      __extension__({K _x=(x); i8 t=TYP(_x); KK==t || KL==t || IS_ADVERB(_x);}) //has other K objects as children
#define IS_ERROR(x)        __extension__({K _x=(x); KE==TAG_TYP(_x);})
#define IS_NULL(x)         __extension__({K _x=(x); KN==TAG_TYP(_x);})
// shared utility functions
static inline char* sc(char *s,char c){ while(*s!=c)if(!*s++)return (char*)0; return s; }
static inline u64   ic(char *s,char c){ return sc(s,c)-s; }
static inline K     tx(i8 t,K x){ return TAG_TYP(x) ? SET_TAG(t,x) : (HDR_TYP(x)=t,x); }

#endif
