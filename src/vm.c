#include "a.h"
#include "vm.h"
#include "parse.h" 
#include "verb.h"
#include "object.h"
#include "compile.h"
#include "debug.h"

#define PUSH(k)  (*vm->top = (k), vm->top++)
#define POP      (*(--vm->top))

static VM *initVM(Chunk *chunk){
    VM *vm = malloc(sizeof(struct vm));
    vm->chunk = chunk;
    vm->ip = chunk->code;
    vm->top = vm->stack;
    return vm;
}

static void freeVM(VM *vm){
    free(vm);
}

static void run(VM *vm){
    for (;;){
        uint8_t instruction = *vm->ip++;

#ifdef DBGCODE
        disassemble(vm, instruction);
#endif
        switch (instruction){

            // push a constant (source code literal) onto the stack
            case OP_CONSTANT : {
                PUSH( ref(vm->chunk->k[*vm->ip++]) );
                break;
            }

            case OP_DYAD_START ... OP_DYAD_END : {
                K x = POP; 
                K y = POP;
                D f = dyads[instruction];
                K r = (*f)(x, y);
                PUSH(r);
                break;
            }

            case OP_ENLIST : {
                uint64_t count =  *vm->ip++;
                K r = k(KK, count);
                for (uint64_t i = 0; i < count; ++i) rk[i] = POP;
                r = squeeze(r);
                PUSH(r);
                break;
            }

            case OP_RETURN : {
                printK( POP );
                return;
            }

            default:
                printf("error! op not recognized.\n");
                return;
        }
    }
}

InterpretResult interpret(const char *source){
    // init new Chunk
    Chunk *chunk = initNewChunk();

    // parse source. parse tree is saved in chunk->parseTree
    // the tree is a K object
    bool parseSuccess = parse(source, chunk);

    // return error if parsing failed
    if (!parseSuccess) return INTERPRET_PARSE_ERROR;

// if flag is set, parse input and print the parse tree only
#ifdef DBGPARSE
    printK( ref(chunk->parseTree) );
    freeChunk(chunk);
    return INTERPRET_OK;
#endif

    compile(chunk);

    // run bytecode in VM
    VM *vm = initVM(chunk);
    run(vm);

    // cleanup and return success
    freeChunk(chunk);
    freeVM(vm);
    return INTERPRET_OK;
}
