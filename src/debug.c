#include "a.h"
#include "compile.h"
#include "object.h"
#include "vm.h"

// this function prints 1 line per instruction
// prints opcode, opcode name, op immediate arg (if any), stack. eg:
// 032  OP_ENLIST   2   (3 4;1 2)
void disassemble(VM *vm, uint8_t *instr){
    // ops string
    static const char *ops = KOPS;

    // print opcode
    printf("%03d  ", instr[0]);

    // print opcode name
    switch(instr[0]){
        case OP_CONSTANT:
            printf("%-13s", "OP_CONSTANT");
            break;

        case OP_DYAD_ADD:
        case OP_DYAD_MULTIPLY:
        case OP_DYAD_SUBTRACT:
        case OP_DYAD_DIVIDE:
        case OP_DYAD_DOTAPPLY:
        case OP_DYAD_KEY:
        case OP_DYAD_MIN:
        case OP_DYAD_MAX:
        case OP_DYAD_LESS:
        case OP_DYAD_MORE:
        case OP_DYAD_EQUAL:
        case OP_DYAD_MATCH:
        case OP_DYAD_FIND:
            printf("OP_DYAD %-10c", ops[ instr[0] ]);
            break;

        case OP_MONAD_FLIP:
        case OP_MONAD_FIRST:
        case OP_MONAD_NEGATE:
        case OP_MONAD_SQRT:
        case OP_MONAD_VALUE:
        case OP_MONAD_ENUMERATE:
        case OP_MONAD_REVERSE:
        case OP_MONAD_WHERE:
        case OP_MONAD_ASC:
        case OP_MONAD_DESC:
            printf("OP_MONAD %-9c", ops[ instr[0] - OP_MONAD_START ]);
            break;

        case OP_ENLIST:
            printf("%-13s", "OP_ENLIST");
            break;

        case OP_RETURN:
            printf("%-18s", "OP_RETURN");
            break;

        default : printf("%-18s", "OP_UNKNOWN");
    }

    // print immediate argument 
    if (OP_CONSTANT == instr[0] || OP_ENLIST == instr[0]){
        printf("%03d  ", instr[1]);
    }

    // print stack
    K t = k(KK, (uint64_t)(vm->top - vm->stack));
    for (uint64_t i = 0; i < tn; ++i) tk[i] = vm->stack[i];
    printOneLineK(t);
    free(t);

    putchar('\n');
}
