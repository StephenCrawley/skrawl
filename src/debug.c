#include "a.h"
#include "compile.h"
#include "object.h"
#include "vm.h"

// this function prints 1 line per instruction
// prints opcode, opcode name, op immediate arg (if any), stack. eg:
// 032  OP_ENLIST   2   (3 4;1 2)
void disassemble(VM *vm, uint8_t *instr){
    // print opcode
    printf("%03d  ", instr[0]);

    // print opcode name
    switch(instr[0]){
        case OP_CONSTANT : {
            printf("%-13s", "OP_CONSTANT");
            break;
        }
        case OP_DYAD_START ... OP_DYAD_END : {
            static const char *ops = KOPS;
            printf("OP_DYAD %-10c", ops[ instr[0] ]);
            break;
        }
        case OP_MONAD_START ... OP_MONAD_END : {
            static const char *ops = KOPS;
            printf("OP_MONAD %-9c", ops[ instr[0] - OP_MONAD_START ]);
            break;
        }
        case OP_ENLIST : {
            printf("%-13s", "OP_ENLIST");
            break;
        }
        case OP_RETURN : {
            printf("%-18s", "OP_RETURN");
            break;
        }
        default : printf("%-17s", "OP_UNKNOWN");
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
