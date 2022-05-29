#include "a.h"
#include "vm.h"
#include "parse.h" 
#include "verb.h"
#include "object.h"
#include "compile.h"
#include "debug.h"

#define PUSH(k)  (*vm->top = (k), vm->top++)
#define POP      (*(--vm->top))

VM *initVM(){
    VM *vm = malloc(sizeof(struct vm));
    vm->chunk = NULL;
    vm->ip = NULL;
    vm->top = vm->stack;
    return vm;
}

static void addChunkToVM(VM *vm, Chunk *chunk){
    vm->chunk = chunk;
    vm->ip = chunk->code;
}

static void cleanup(VM *vm){
    // unref stack members
    for (uint64_t i = 0, n = vm->top - vm->stack; i < n; ++i)
        unref( vm->stack[i] );
    // reset stackTop
    vm->top = vm->stack;
}

static void run(VM *vm){

    // declare variables used by VM
    K x, y;     // objects popped from stack
    K r;        // return object to be pushed on the stack
    K t;        // temp object
    U f;        // monadic function 
    V g;        // dyadic  function 
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
            case OP_DYAD_CAT:
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
            case OP_MONAD_WHERE:
            case OP_MONAD_ASC:
            case OP_MONAD_DESC:
            case OP_MONAD_NOT:
            case OP_MONAD_TYPE:
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

            // create projection
            case OP_PROJECT:
                n = *vm->ip++;
                t = k(KK, n);
                for (uint8_t i = 0; i < tn; ++i) tk[i] = POP;
                r = Kp();
                rk[0] = Ki( *vm->ip++ );
                rk[1] = t;
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

InterpretResult interpret(VM *vm, const char *source){
    // init new Chunk
    Chunk *chunk = initNewChunk();

    // parse source. parse tree is saved in chunk->parseTree
    // the tree is a K object
    bool parseSuccess = parse(source, chunk);
    if (!parseSuccess){
        return INTERPRET_PARSE_ERROR;
    }

// if flag is set, parse input and print the parse tree only
#ifdef DBG_PARSE
    printK( ref(chunk->parseTree) );
    freeChunk(chunk);
    return INTERPRET_OK;
#endif

    bool compileSuccess = compile(chunk);
    if (!compileSuccess){
        freeChunk(chunk);
        return INTERPRET_COMPILE_ERROR;
    }
    // run bytecode in VM
    addChunkToVM(vm, chunk);
    run(vm);

    // cleanup and return success
    freeChunk(chunk);
    cleanup(vm);
    return INTERPRET_OK;
}
