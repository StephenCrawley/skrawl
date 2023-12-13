#include "compile.h"
#include "object.h"
#include "verb.h"

#define IMM_ARG_MAX  255  //max value of immediate arg (max representable by 8bits)
#define BYTES(x)     LAMBDA_OPCODE(x) //bytecodes accessor
#define CONSTS(x)    LAMBDA_CONSTS(x) //constants accessor

#define COMPILING_LAMBDA(x) (HDR_CNT(x) != 2)

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

static K compileVarGet(K x, K r){
    // global get
    if (!COMPILING_LAMBDA(r))
        return addByte(compileConstant(r,x),OP_GET_GLOBAL);

    // if compiling lambda, either we're getting an arg or a local
    K args=LAMBDA_ARGS(r);
    i64 ix=symIndex(args,x);
    if (ix < HDR_CNT(args))
        return addByte(r,OP_GET_ARG+ix);
    else 
        return UNREF_R(kerr("'compile! lambda vars nyi"));
}

static K compileExprs(K x, K r){
    i8  xt=TYP(x); 
    i64 xn=CNT(x);

    // compile literals and variables
    // special case: (,`a) TYP==KS && CNT==1 is a  as we create a new atomic symbol
    // which we must unref because it's not part of the parse tree which is unref'd later 
    if (xt==KS && xn==1) return x=ks(*INT(x)), r=compileConstant(r,x), UNREF_X(r);
    // compile variable get
    if (xt==-KS) return compileVarGet(x,r);
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

static K compile0(K x, K r){
    // if compiling single expression i.e. not(;:;...)
    if (TYP(x)!=KK || !CNT(x) || TYP(*OBJ(x))!=KU || TAG_VAL(*OBJ(x))!=TOK_SEMICOLON){
        r=compileExprs(x,r);
        return UNREF_X(IS_ERROR(r) ? r : addByte(r,OP_RETURN));
    }
    
    // else compile each expression from 1 .. n-1
    for (i64 i=1,n=CNT(x)-1; i<=n; i++){
        r=compileExprs(OBJ(x)[i],r);
        if (IS_ERROR(r)) return r;
        // append OP_RETURN if last expression, else append OP_POP
        r=addByte(r, i==n ? OP_RETURN : OP_POP);
    }
    return UNREF_X(r);
}

// given a pointer to bytecode, determine the stack size
static u8 getMaxStackSize(u8 *b){
    u8 s=0,m=0;
    while (*b != OP_RETURN){
        u8 instr=*b++;
        switch (20u > (u32)(instr-OP_MONAD  ) ? OP_MONAD   :
                20u > (u32)(instr-OP_DYAD   ) ? OP_DYAD    :
                 6u > (u32)(instr-OP_ADVERB ) ? OP_ADVERB  : 
                 3u > (u32)(instr-OP_GET_ARG) ? OP_GET_ARG : instr){
        case OP_CONSTANT:
            s++;
            m=MAX(s,m);
            b++;
            break;
        case OP_SET_GLOBAL:
            b++;
            break;
        case OP_APPLY_N:
            s -= *b++;
            break;
        case OP_DYAD:    //fall through
        case OP_POP:     //fall through
            s--;
            break;
        case OP_GET_ARG:
            s++;
            m=MAX(s,m);
            break;
        }
    }
    return m;
}

// recurse thru parse tree x, generating bytecode
// returns a K object (byetcode;consts)
K compile(K x){ 
    x=compile0(x, k2(tn(KX,1),tn(KK,0)));
    ((u8*)BYTES(x))[0]=getMaxStackSize((u8*)BYTES(x)+1);
    return x;
}

// x - parse tree
// f - ("{x}";args) of lambda
K compileLambda(K x, K f){
    K r=j2(k2(tn(KX,1),tn(KK,0)),f); //(opcodes;consts;"{x}";args)
    r=compile0(x,r);
    ((u8*)BYTES(r))[0]=getMaxStackSize((u8*)BYTES(r)+1);
    HDR_RNK(r)=HDR_CNT(LAMBDA_ARGS(r));
    return r;
}
