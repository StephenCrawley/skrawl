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
    for (uint64_t i = 0, n = vm->top - vm->stack; i < n; ++i)
        unref( vm->stack[i] );
    free(vm);
}

static void run(VM *vm){

    // declare variables used by VM
    K x, y;     // objects popped from stack
    K r;        // return object to be pushed on the stack
    M f;        // monadic function 
    D g;        // dyadic  function 
    uint64_t n; // count 

    for (;;){

#ifdef DISASSEMBLE
        disassemble(vm, vm->ip);
#endif

        uint8_t instruction = *vm->ip++;
        switch (instruction){

            // push a constant (source code literal) onto the stack
            case OP_CONSTANT:
                PUSH( ref(vm->chunk->k[*vm->ip++]) );
                break;

            // dyadic operators
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
                g = dyads[instruction];
                if (NULL == g){
                    PUSH(Kerr("nyi"));
                    break;
                }
                x = POP; 
                y = POP;
                r = (*g)(x, y);
                PUSH(r);
                break;

            // monadic operators
            case OP_MONAD_FLIP:
            case OP_MONAD_FIRST:
            case OP_MONAD_NEGATE:
            case OP_MONAD_SQRT:
            case OP_MONAD_VALUE:
            case OP_MONAD_ENUMERATE:
            case OP_MONAD_REVERSE:
                f = monads[instruction - OP_MONAD_START];
                if (NULL == f){
                    PUSH(Kerr("nyi"));
                    break;
                }
                x = POP;
                r = (*f)(x);
                PUSH(r);
                break;

            // enlist 
            case OP_ENLIST:
                n =  *vm->ip++;
                r = k(KK, n);
                for (uint64_t i = 0; i < n; ++i) rk[i] = POP;
                r = squeeze(r);
                PUSH(r);
                break;

            // print top of stack and stop execution
            case OP_RETURN:
                printK( POP );
                return;

            // unknown instruction. print error and stop execution
            default:
                printf("error! VM instruction not recognized: %03d\n", instruction);
                return;
        }

        // short circuit if error
        if (KE == TYPE( *(vm->top - 1) )){
            printK( POP );
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
#ifdef DBG_PARSE
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
