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
typedef int32_t   i32;
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
    K_ADVERB_END = KEL         //end of adverb types
};

#define VERB_STR    ":+-*%,;"
#define ADVERB_STR  "'/\\'/\\"

// K header accessors
// the header is 16 bytes
// --mtrrrrnnnnnnnn
// - unused, m membucket, t type, r refcount, n count
#define MEM(x)  (( i8*)x)[-14]
#define TYP(x)  (( i8*)x)[-13]
#define REF(x)  ((i32*)x)[-3]
#define CNT(x)  ((i64*)x)[-1]

// K object accessors
#define OBJ(x)  ((     K*) x)  //pointer to generic K object list
#define CHR(x)  ((    i8*) x)  //pointer to int8 
#define INT(x)  ((   i64*) x)  //pointer to int64 
#define FLT(x)  ((double*) x)  //pointer to double 

// shared utility macros
#define ABS(a)   __extension__({__typeof__(a)_a=(a); _a > 0 ? _a : -_a ;}) 
#define MAX(a,b) __extension__({__typeof__(a)_a=(a);__typeof__(b)_b=(b);_a>_b?_a:_b;})
#define IS_SIMPLE_LIST(x)  __extension__({__typeof__(x)_x=(x); 0<TYP(_x) && TYP(_x)<=K_SIMPLE_LIST_END;})
#define IS_VERB(a)    __extension__({__typeof__(a)_a=(a); i8 t=TYP(_a); KU==t || KV==t;}) 
#define IS_ADVERB(a)  __extension__({__typeof__(a)_a=(a); i8 t=TYP(_a); K_ADVERB_START<=t && t<=K_ADVERB_END;})
#define IS_GENERIC(x) __extension__({__typeof__(x)_x=(x); i8 t=TYP(_x); KK==t || IS_ADVERB(_x);}) //has other K objects as children
// shared utility functions
static inline char* sc(char *s,char c){ while(*s!=c)if(!*s++)return (char*)0; return s; }

#endif
