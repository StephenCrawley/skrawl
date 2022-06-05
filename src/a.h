#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// K object
typedef struct k {
    int8_t   t;   // type
    uint16_t r;   // refcount
    uint64_t n;   // array count
    uint8_t  d[]; // data
} *K;

// the below are used for K types. the 1-letter names correspond to the typenames within k itself
// for example the k expression:
// @ 1 2 3
// meaning "type of 1 2 3", returns `I, indicating a 64-bit signed integer array
// the below abbreviations are only used in the context of K object data
typedef char    C; 
typedef int64_t I; 
typedef double  F;
typedef K (*U)(K);     // monadic function
typedef K (*V)(K, K);  // dyadic  function

// K accessors
// these provide shorthand access to K struct members, significantly cleaning up the code
// x, y, z are reserved exclusively for function arguments
// r is reserved exclusively for the function return value
// t is used as a temp / placeholder variable of any kind
// it is important to be familiar with these macro shorthands to read this codebase

// type
#define TYPE(k)  ((k)->t)
#define xt       TYPE(x)
#define yt       TYPE(y)
#define rt       TYPE(r)
#define tt       TYPE(t)
// refcount
#define REFC(k)  ((k)->r)
#define xr       REFC(x)
#define yr       REFC(y)
#define rr       REFC(r)
// object count
#define COUNT(k) ((k)->n)
#define xn       COUNT(x)
#define yn       COUNT(y)
#define rn       COUNT(r)
#define tn       COUNT(t)
// object data pointer
#define DATA(k)   ((k)->d)
// char pointer
#define CHAR(k)   ((C*)DATA(k))
#define xc        CHAR(x)
#define yc        CHAR(y)
#define rc        CHAR(r)
#define tc        CHAR(t)
// 64-bit signed int pointer
#define INT(k)    ((I*)DATA(k))
#define xi        INT(x)
#define yi        INT(y)
#define ri        INT(r)
#define ti        INT(t)
// double precision float pointer
#define FLOAT(k)  ((F*)DATA(k))
#define xf        FLOAT(x)
#define yf        FLOAT(y)
#define rf        FLOAT(r)
#define tf        FLOAT(t)
// general K pointer (for a K object containing K objects)
#define KOBJ(k)   ((K*)DATA(k))
#define xk        KOBJ(x)
#define yk        KOBJ(y)
#define rk        KOBJ(r)
#define tk        KOBJ(t)
// dictionary accessors
#define DKEYS(k)  KOBJ(k)[0]
#define DVALS(k)  KOBJ(k)[1]
// table accessors
#define TKEYS(k)  DKEYS(KOBJ(k)[0])
#define TVALS(k)  DVALS(KOBJ(k)[0])

// K types
enum {
    KE = -128, // error
    KK = 0,    // general list
    KC,        // char
    KI,        // int
    KF,        // float
    KS,        // symbol
    KD,        // dictionary
    KT,        // table
    KU,        // monadic function
    KV,        // dyadic function
    KA,        // adverb
    KP,        // projection
    KN         // null
};
#define KWIDTHS 8,1,8,8,8,8,8,1,1,1,8,0 // in bytes
#define KOPS "+*-%.!|&<>=~?,@#^$_:/\\'\\/'";
#define KLISTTYPES "KCIFSDTUVAPN"
#define KATOMTYPES " cifs"

// utility macros
#define MAX(x, y) __extension__({__typeof__(x) _x = (x);__typeof__(y) _y = (y); _x > _y ? _x : _y;})
#define MIN(x, y) __extension__({__typeof__(x) _x = (x);__typeof__(y) _y = (y); _x < _y ? _x : _y;})
#define ABS(x) MAX((x), -(x))
#define SGN(x) __extension__({__typeof__(x) _x = (x); (_x > 0) - (_x < 0);})

// K object utilities
#define K_COUNT(x) ((KD==TYPE(x)) ? COUNT(DKEYS(x)) : (KT==TYPE(x)) ? COUNT(KOBJ(TVALS(x))[0]) : COUNT(x))

#endif
