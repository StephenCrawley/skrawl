#include "a.h"
#include "compile.h"
#include "object.h"
#include "vm.h"

// this function prints 1 line per instruction
// prints opcode, opcode name, stack, eg:
// 032  OP_ENLIST      (3 4;1 2)
void disassemble(VM *vm, uint8_t instruction){
    // print opcode
    printf("%03d  ", instruction);

    // print opcode name
    switch(instruction){
        case OP_CONSTANT : {
            printf("%-15s", "OP_CONSTANT");
            break;
        }
        case OP_DYAD_START ... OP_DYAD_END : {
            static const char *ops = KOPS;
            printf("OP_DYAD %-7c", ops[instruction]);
            break;
        }
        case OP_ENLIST : {
            printf("%-15s", "OP_ENLIST");
            break;
        }
        case OP_RETURN : {
            printf("%-15s", "OP_RETURN");
            break;
        }
        default : printf("%-15s", "OP_UNKNOWN");
    }

    // print stack
    K t = k(KK, (uint64_t)(vm->top - vm->stack));
    for (uint64_t i = 0; i < tn; ++i) tk[i] = vm->stack[i];
    printOneLineK(t);
    free(t);

    putchar('\n');
}
