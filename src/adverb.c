#include "adverb.h"
#include "object.h"

// x f\:y
// where f is a function pointer
K mapleft(DYAD f, K x, K y){
    K r=tn(KK,0),t;
    for (i64 i=0,n=KCOUNT(x); i<n; i++){
        t=f(item(i,x),ref(y));
        if (IS_ERROR(t))
            return UNREF_XYR(t);
        r=jk(r,t);
    }
    return UNREF_XY(squeeze(r));
}
