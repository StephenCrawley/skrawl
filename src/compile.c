#include "compile.h"
#include "object.h"

#define IMM_ARG_MAX  255 //max value of immediate arg (max representable by 8bits)
#define RETURN_IF_ERROR(x) __extension__({ K _e=(x); if(IS_ERROR((_e))) return _e; }) 

// in this script x is the parse tree (or any child object within it)
// and r is the object to be returned to the VM (see compile() below)

static K addByte(K r, u8 x)        { return *OBJ(r)=j2(*OBJ(r),kx(x))   , r; }
static K add2Bytes(K r, u8 x, u8 y){ return *OBJ(r)=j2(*OBJ(r),kx2(x,y)), r; }
static K addConstant(K r, K y){ return OBJ(r)[1]=jk(OBJ(r)[1],ref(y)), r; }
static K compileConstant(K r, K y){ 
    u8 n=CNT(OBJ(r)[1]);
    if (IMM_ARG_MAX==n) return UNREF_R(kerr(kC0("'compile! CONST MAX")));
    r=add2Bytes(r,OP_CONSTANT,n);
    return addConstant(r,y); 
}

// instruction to pop top of stack and apply it to next n items on top of stack
static K compileApplyN(K r, u8 n){
    if (IMM_ARG_MAX==n) return kerr(kC0("'compile! APPLY MAX"));
    return add2Bytes(r,OP_APPLY_N,n);
}

static K compileExprs(K x, K r){
    i8 t=TYP(x); 
    i64 n=CNT(x), i=n-1;

    // KS==TYP && 1==CNT (,`a) is a special case as we create a new atomic symbol
    // which we must unref because it's not part of the parse tree which is unref'd later 
    if (KS==t&&1==n) return x=ks(*INT(x)), r=compileConstant(r,x), UNREF_X(r);
    // if not generic, compile constant
    if (KK!=t) return compileConstant(r,x);
    // compile () 
    if (!n)    return compileConstant(r,x);
    // compile ,`a`b 
    if (1==n)  return compileConstant(r,*OBJ(x));

    // handle (f;...) where f is applied to the subsequent elements
    // we compile elements n-1 .. 1 first
    while (i) RETURN_IF_ERROR(r=compileExprs(OBJ(x)[i--],r));
    // then we compile f
    K f=*OBJ(x); t=TYP(f);
    return 
        (KU==t&&2==n) ? addByte(r,OP_MONAD +TAG_VAL(f)) : //monad instruction
        (KV==t&&3==n) ? addByte(r,OP_DYAD  +TAG_VAL(f)) : //dyad instruction
        (KW==t&&2==n) ? addByte(r,OP_ADVERB+TAG_VAL(f)) : //adverb instruction
        IS_ERROR(r=compileExprs(f,r)) ? r :                   //error
        compileApplyN(r,n-1);                                 //general apply
}

// recurse thru parse tree, generating bytecode
// returns a K object (x,y)
// x - bytecodes
// y - constants
K compile(K x){
    K r=k2(tn(KX,0),tn(KK,0));
    
    // if compiling single expression i.e. not(;:;...)
    if (KK!=TYP(x) || !CNT(x) || KU!=TYP(*OBJ(x)) || ';'!=cverb(TAG_VAL(*OBJ(x))))
        return IS_ERROR(r=compileExprs(x,r)) ? r : addByte(r, OP_RETURN);
    
    // else compile each expression from 1 .. n-1
    for (i64 i=1,n=CNT(x)-1; i<=n; i++){
        RETURN_IF_ERROR(r=compileExprs(OBJ(x)[i],r));
        r=addByte(r, i==n ? OP_RETURN : OP_POP);
    }
    return r;
}
