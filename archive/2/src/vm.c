#include "a.h"
#include "vm.h"
#include "parse.h" 
#include "verb.h"
#include "object.h"
#include "compile.h"
#include "debug.h"

#define PUSH(k)  (r = (k), vm->error = (KE==TYPE(r)), *vm->top = r, vm->top++)
#define POP      (*(--vm->top))

VM *initVM(){
    VM *vm = malloc(sizeof(struct vm));
    vm->chunk = NULL;
    vm->ip = NULL;
    vm->top = vm->stack;
    vm->globals = key(enlist(Ks((int64_t)'`')), enlist(KNUL));
    vm->retval = KNUL;
    vm->silent = false;
    vm->error = false;
    vm->terminate = false;
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

void freeVM(VM *vm){
    unref(vm->globals);
    unref(vm->retval);
    free(vm);
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
                r = ref(vm->chunk->k[*vm->ip++]);
                PUSH(r);
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
            case OP_DYAD_ATINDEX:
            case OP_DYAD_TAKE:
            case OP_DYAD_DROP:
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
            case OP_MONAD_DISTINCT:
            case OP_MONAD_TYPE:
            case OP_MONAD_COUNT:
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

            // apply. f x
            case OP_APPLY:
                x = POP;
                n = *vm->ip++; // number of objects to pop
                y = k(KK, n);
                for (uint8_t i = 0; i < yn; ++i) yk[i] = POP;
                r = dotApply(x, squeeze(y));
                PUSH(r);
                break;

            // create projection
            case OP_PROJECT:
                n = *vm->ip++;
                t = k(KK, n);
                for (uint8_t i = 0; i < tn; ++i) tk[i] = POP;
                r = Kp( Ki(*vm->ip++), t );
                PUSH(r);
                break;

            // set global variable
            case OP_SETGLOBAL:
                x = POP;
                y = POP;
                if (NULL == vm->globals){
                    vm->globals = key(enlist(x), enlist(ref(y)));
                }
                else {
                    vm->globals = upsertDicts(vm->globals, key(enlist(x), enlist(ref(y))));
                }
                PUSH(y);
                break;

            // push a global variable onto the stack
            case OP_GETGLOBAL:
                // get the variable name from constants pool and search the globals dict
                x = vm->chunk->k[*vm->ip++];
                t = find(ref(DKEYS(vm->globals)), ref(x));
                // if variable not defined, push error
                if (ti[0] == K_COUNT(vm->globals)){
                    char var[9]; // syms are max 8 bytes. +1 for \0
                    for (uint8_t i = 0; i < 8; ++i) var[i] = xc[i];
                    var[8] = '\0';
                    PUSH(Kerr(var));
                }
                else {
                    r = ref(KOBJ(DVALS(vm->globals))[ti[0]]);
                    PUSH(r);
                }
                unref(t);
                break;

            // push composition onto the stack
            case OP_COMPOSE:
                x = POP;
                y = POP;
                PUSH(Kq(x, y));
                break;

            case OP_JUMP_IF_NOT_ERROR:
                vm->error = false;
                n = *vm->ip++; // # bytecodes to jump if not error
                x = POP;
                // if error, we fall through to the error code
                if (KE == xt){
                    unref(x);
                }
                // else jump over the error code
                else {
                    PUSH(x);
                    vm->ip += n;
                }
                break;

            // print top of stack and stop execution
            case OP_RETURN:
                r = POP;
                if (!vm->silent) printK(r);
                unref(vm->retval);
                vm->retval = r;
                return;

            case OP_TERMINATE:
                vm->terminate = true;
                return;

            // unknown instruction. print error and stop execution
            default:
                printf("error! VM instruction not recognized: %03d\n", instruction);
                return;
        }

        // short circuit if error
        if (vm->error && 
            OP_JUMP_IF_NOT_ERROR != *vm->ip){
            unref(printK(POP));
            vm->error = false;
            return;
        }
    }
}

InterpretResult interpret(VM *vm, const char *source){
    // init new Chunk
    Chunk *chunk = initNewChunk();

    // parse source and save in chunk->parseTree. the tree is a K object
    chunk->parseTree = parse(source);

    if (NULL == chunk->parseTree){
        freeChunk(chunk);
        return INTERPRET_PARSE_ERROR;
    }

// if flag is set, parse input and print the parse tree only
#ifdef DBG_PARSE
    printK(chunk->parseTree);
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

    // cleanup
    freeChunk(chunk);
    cleanup(vm);

    // if OP_TERMINATE called, free the VM and exit
    if (vm->terminate){
        freeVM(vm);
        exit(0);
    }

    // return success
    return INTERPRET_OK;
}
