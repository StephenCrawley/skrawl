#include "vm.h"
#include "object.h"
#include "compile.h"

#define BYTECODE_PTR(x)  CHR( *OBJ(x) )

K run(K x){
    //instruction pointer, current instruction
    const u8 *ip = BYTECODE_PTR(x);
    u8 instr;

    for (;;){
        instr = *ip++;

        switch (32u>instr-OP_MONAD ? OP_MONAD : 32u>instr-OP_DYAD ? OP_DYAD : instr){
        case OP_MONAD:
            printf("%03d OP_MONAD (%c)\n", instr, cverb(instr-OP_MONAD));
            break;
        case OP_DYAD:
            printf("%03d OP_DYAD (%c)\n", instr, cverb(instr-OP_DYAD));
            break;
        case OP_CONSTANT:
            printf("%03d OP_CONSTANT (%d)\n", instr, *ip++);
            break;
        case OP_APPLY_N:
            printf("%03d OP_APPLY_N (%d)\n", instr, *ip++);
            break;
        case OP_POP:
            printf("%03d OP_POP\n",instr);
            break; 
        case OP_RETURN:
            printf("%03d OP_RETURN\n",instr);
            return x;
        default:
            printf("%03d unknown instruction\n",instr);
        }
    }
    return x;
}

// accepts parse tree (or error/null) as argument
// compiles to bytecode and runs on VM
K evalK(K x){
    K r;
    return (IS_ERROR(x) || IS_NULL(x)) ? x : UNREF_X(IS_ERROR(r=compile(x)) ? r : run(r));
}
