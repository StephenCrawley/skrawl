#include "a.h"
#include "adverb.h"

// adverbs table
W adverbs[] = {over};

// x f/ y
K over(V f, K x, K y){
    K r = x;
    y = expand(y);
    for (uint64_t i = 0; i < yn; ++i){
        r = f(r, yk[i]);
    }
    free(y);
    return r;
}
