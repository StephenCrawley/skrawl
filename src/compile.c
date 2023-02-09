#include "compile.h"
#include "object.h"

#define IMM_ARG_MAX  255 //max number of immediate args (max representable by 8bits)
#define RETURN_IF_ERROR(x) __extension__({ K _e=(x); if(IS_ERROR((_e))) return _e; }) 

// in this script, x is the parse tree (or any child object within it)
// and r is the object to be returned to the VM (see compile() below)

static K addBytecode(K r, u8 c){ return *OBJ(r)=j2(*OBJ(r),kx(c)), r; }
static K addConstant(K r,  K y){ return OBJ(r)[1]=jk(OBJ(r)[1],ref(y)), r; }
static K compileConstant(K r, K y){ 
    u8 n=CNT(OBJ(r)[1]);
    if (IMM_ARG_MAX==n) return printf("'CONST MAX\n"),UNREF_R(ke());
    r=addBytecode(r,OP_CONSTANT);
    return addBytecode(addConstant(r,y), n); 
}

// instruction to pop top of stack and apply it to next n items on top of stack
static K compileApplyN(K r, u8 n){
    if (n==IMM_ARG_MAX) return printf("'APPLY MAX\n"),ke();
    return addBytecode(addBytecode(r, OP_APPLY_N), n);
}

static K compileExprs(K x, K r){
    i8 t=TYP(x); 
    i64 n=CNT(x), i=n-1;

    // if not generic, compile constant
    // KS==TYP && 1==CNT (,`a) is a special case
    if (KK!=t) return compileConstant(r, KS==t&&1==n ? ks(*INT(x)) : x);

    // compile () or ,`a`b 
    if (!n)   return compileConstant(r, x);
    if (1==n) return compileConstant(r, *OBJ(x));

    // handle (f;...) where f is applied to the subsequent elements
    // we compile elements n-1 .. 1 first
    while (i) RETURN_IF_ERROR(r=compileExprs(OBJ(x)[i--], r));
    // then we compile f
    K f=*OBJ(x); t=TYP(f);
    return 
        (KU==t&&2==n) ? addBytecode(r, OP_MONAD +TAG_VAL(f)) : //monad instruction
        (KV==t&&3==n) ? addBytecode(r, OP_DYAD  +TAG_VAL(f)) : //dyad instruction
        (KW==t&&2==n) ? addBytecode(r, OP_ADVERB+TAG_VAL(f)) : //adverb instruction
        IS_ERROR(r=compileExprs(f,r)) ? r :                    //error
        compileApplyN(r,n-1);                                  //general apply
}

// recurse thru parse tree, generating bytecode
// returns a K object (x,y)
// x - bytecodes
// y - constants
K compile(K x){
    K r=k2(tn(KX,0),tn(KK,0));
    
    // if compiling single expression (i.e. not(;:;...))
    if (KK!=TYP(x) || !CNT(x) || KU!=TYP(*OBJ(x)) || ';'!=cverb(TAG_VAL(*OBJ(x))))
        return IS_ERROR(r=compileExprs(x,r)) ? r : addBytecode(r, OP_RETURN);
    
    // else compile each expression from 1 .. n-1
    for (i64 i=1,n=CNT(x)-1; i<=n; i++){
        if (IS_ERROR(r=compileExprs(OBJ(x)[i],r))) return r;
        r=addBytecode(r, i==n ? OP_RETURN : OP_POP);
    }
    return r;
}
