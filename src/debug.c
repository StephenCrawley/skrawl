#include "a.h"
#include "compile.h"
#include "object.h"
#include "vm.h"

void disassemble(VM *vm, uint8_t instruction){
    printf("%03d  ", instruction);
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

    K t = k(KK, (uint64_t)(vm->top - vm->stack));
    for (uint64_t i = 0; i < tn; ++i) tk[i] = vm->stack[i];
    printOneLineK(t);
    putchar('\n');
}
