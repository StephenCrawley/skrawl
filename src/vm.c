#include "vm.h"
#include "object.h"
#include "compile.h"
#include "verb.h"

#define POP()            ( *--top )
#define PUSH(x)          ( *top++ = (x) )
#define BYTECODE_PTR(x)  CHR( *OBJ(x) )
#define CONSTANT_PTR(x)  OBJ( OBJ(x)[1] )

K run(K r){
    // VM registers and variables
    K stack[256];
    K *top=stack;
    K *consts=CONSTANT_PTR(r);
    const u8 *ip=BYTECODE_PTR(r);
    u8 instr, b;  //instr:current instruction, b:temp byte variable

    // useful variables to execute opcodes
    K x,y;
    i64 n;
    DYAD v;

    for (;;){
        instr=*ip++;

        // ternary step function to convert monads/dyads/adverbs opcodes to base opcode
        switch (20u>instr-OP_MONAD  ? OP_MONAD  :
                20u>instr-OP_DYAD   ? OP_DYAD   :
                 6u>instr-OP_ADVERB ? OP_ADVERB : instr)
        {
        case OP_MONAD:
            printf("%03d OP_MONAD (%c)\n", instr, cverb(instr-OP_MONAD));
            break;
        case OP_DYAD:
            //printf("%03d OP_DYAD (%c)\n", instr, cverb(instr-OP_DYAD));
            b=instr-OP_DYAD;
            v=dyad_table[b];
            x=POP();
            x=(*v)(x,POP());
            if (IS_ERROR(x))
                goto run_error;
            PUSH(x);
            break;
        case OP_ADVERB:
            printf("%03d OP_ADVERB (%c%s)\n", instr, cadverb(instr-OP_ADVERB), instr-OP_ADVERB>2 ? ":" : ""); 
            break;    
        case OP_CONSTANT:
            //printf("%03d OP_CONSTANT (%d)\n", instr, *ip);
            PUSH(ref(consts[*ip++]));
            break;
        case OP_APPLY_N:
            //printf("%03d OP_APPLY_N (%d)\n", instr, *ip);
            n=*ip++;
            x=POP();
            y=tn(KK,n);
            for (i64 i=0; i<n; i++) 
                OBJ(y)[i]=POP();
            x=apply(x,y);
            if (IS_ERROR(x))
                goto run_error;
            PUSH(x);
            break;
        case OP_POP:
            //printf("%03d OP_POP\n",instr);
            unref(POP());
            break; 
        case OP_RETURN:
            //printf("%03d OP_RETURN\n",instr);
            return UNREF_R(POP());
        default:
            printf("%03d unknown instruction\n",instr);
        }
    }

run_error:
    // clean the stack
    while (stack!=top--) unref(*top);
    // return error
    return UNREF_R(x);
}

// accepts parse tree (or error/null) as argument
// compiles to bytecode and runs on VM
K evalK(K x){
    K r;
    return (IS_ERROR(x) || IS_NULL(x)) ? x : UNREF_X(IS_ERROR(r=compile(x)) ? r : run(r));
}
