#include "compile.h"
#include "object.h"

#define IMM_ARG_MAX  255  //max value of immediate arg (max representable by 8bits)
#define BYTES(x)     (OBJ(x)[0]) //bytecodes accessor
#define CONSTS(x)    (OBJ(x)[1]) //constants accessor

// in this script x is the parse tree (or any child object within it)
// and r is the object to be returned to the VM (see compile() below)

static K addByte(K r, u8 x){
    return BYTES(r)=j2(BYTES(r),kx(x)), r; 
}

static K add2Bytes(K r, u8 x, u8 y){
    return BYTES(r)=j2(BYTES(r),kx2(x,y)), r; 
}

static K addConstant(K r, K y){
    return CONSTS(r)=jk(CONSTS(r),ref(y)), r;
}

static K compileConstAndOpcode(K r, K y, u8 op){
    // check we haven't exceeded constant pool limit
    u8 n=CNT(CONSTS(r));
    if (n==IMM_ARG_MAX)
        return UNREF_R(kerr("'compile! CONST MAX"));

    // add the opcode and append the constant 
    r=add2Bytes(r,op,n);
    return addConstant(r,y); 
}

static K compileConstant(K r, K y){ 
    return compileConstAndOpcode(r,y,OP_CONSTANT); 
}

// instruction to pop top of stack and apply it to next n items on top of stack
static K compileApplyN(K r, i64 n){
    // check we haven't exceeded immediate arg limit
    if (n>IMM_ARG_MAX) 
        return UNREF_R(kerr("'compile! APPLY MAX"));

    return add2Bytes(r,OP_APPLY_N,n);
}

static K compileExprs(K x, K r){
    i8  xt=TYP(x); 
    i64 xn=CNT(x);

    // compile literals and variables
    // special case: (,`a) TYP==KS && CNT==1 is a  as we create a new atomic symbol
    // which we must unref because it's not part of the parse tree which is unref'd later 
    if (xt==KS && xn==1) return x=ks(*INT(x)), r=compileConstant(r,x), UNREF_X(r);
    // compile variable get
    if (xt==-KS) return addByte(compileConstant(r,x),OP_GET_GLOBAL);
    // compile constant or ()
    if (xt!=KK || !xn) return compileConstant(r,x);
    // compile ,`a`b
    if (xn==1)  return compileConstant(r,*OBJ(x));
    
    // special case: (:;`a;...) compile variable set
    if (xn==3 && IS_DYAD(*OBJ(x),TOK_COLON) && TYP(OBJ(x)[1])==-KS){
        r=compileExprs(OBJ(x)[2],r);
        if (IS_ERROR(r)) return r;
        return compileConstAndOpcode(r,OBJ(x)[1],OP_SET_GLOBAL);
    }

    // handle (f;...) where f is applied to the subsequent elements
    // first we compile elements n-1 .. 1
    bool m=0;  //any magic values?
    for (i64 i=xn-1; i>0; i--){
        K t=OBJ(x)[i];
        m|=IS_MAGIC_VAL(t);   //any magic values?
        r=compileExprs(t,r);  //compile
        if (IS_ERROR(r)) return r;
    }

    // then we compile f
    K f=*OBJ(x); 
    i8 ft=TYP(f);
    return 
        (ft==KU && xn==2)       ? addByte(r,OP_MONAD +TAG_VAL(f)) : //monad instruction
        (ft==KV && xn==3 && !m) ? addByte(r,OP_DYAD  +TAG_VAL(f)) : //dyad instruction
        (ft==KW && xn==2)       ? addByte(r,OP_ADVERB+TAG_VAL(f)) : //adverb instruction
        IS_ERROR(r=compileExprs(f,r)) ? r                         : //error
        compileApplyN(r,xn-1);                                      //general apply
}

// recurse thru parse tree, generating bytecode
// returns a K object (x,y)
// x - bytecodes
// y - constants
K compile(K x){
    //    (BYTES   ;CONSTS  ) 
    K r=k2(tn(KX,0),tn(KK,0));
    
    // if compiling single expression i.e. not(;:;...)
    if (TYP(x)!=KK || !CNT(x) || TYP(*OBJ(x))!=KU || TAG_VAL(*OBJ(x))!=TOK_SEMICOLON)
        return UNREF_X(IS_ERROR(r=compileExprs(x,r)) ? r : addByte(r,OP_RETURN));
    
    // else compile each expression from 1 .. n-1
    for (i64 i=1,n=CNT(x)-1; i<=n; i++){
        r=compileExprs(OBJ(x)[i],r);
        if (IS_ERROR(r)) return r;
        // append OP_RETURN if last expression, else append OP_POP
        r=addByte(r, i==n ? OP_RETURN : OP_POP);
    }
    return UNREF_X(r);
}
