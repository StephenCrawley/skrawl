#include "apply.h"
#include "object.h"

// f . x
// generic apply function
// f - some applicable value (primitive, lambda, list, etc)
// x - a list of arguments to f 
K apply(K x, K y){

    // enlist is special, can take any number of arguments
    if (IS_OP(x,KU,TOK_COMMA))
        return UNREF_X(squeeze(y));

    return UNREF_XY(kerr(kC0("'nyi! apply")));
}
