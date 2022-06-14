#include "a.h"
#include "adverb.h"
#include "verb.h"

// adverbs table
W adverbs[] = {over, NULL, NULL, NULL, NULL, NULL};

// x f/ y
K over(K f, K x, K y){
    y = expand(y);
    for (uint64_t i = 0; i < yn; ++i){
        x = cat(enlist(x), enlist(yk[i]));
        x = dotApply(ref(f), x);
    }
    unref(f), free(y);
    return x;
}
