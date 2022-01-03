#include "a.h"
#include "vm.h"
#include "parse.h" 
#include "verb.h"
#include "object.h"

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
        switch (instruction){

            // push a constant (source code literal) onto the stack
            case OP_CONSTANT : {
                PUSH( ref(vm->chunk->k[*vm->ip++]) );
                break;
            }

            // pop 2 objects and add them. push result onto the stack
            case OP_ADD : {
                K y = POP; 
                K x = POP; 
                K r = add(x, y);
                PUSH(r);
                break;
            }

            // pop an object and print it
            case OP_RETURN : {
                printK( POP );
                return;
            }
        }
    }
}

InterpretResult interpret(const char *source){
    // init new Chunk
    Chunk *chunk = initNewChunk();

    // compile source into chunk
    bool compileSuccess = compile(source, chunk);

    // return error if compilation failed
    if (!compileSuccess) return INTERPRET_COMPILE_ERROR;
    else { printK( ref(chunk->parseTree) ); freeChunk(chunk); return INTERPRET_OK;}

    // *** CURRENTLY VM IS NOT USED *** //
    // source is parsed and parse tree is returned if successful
    // TODO : compile to bytecode from parse tree

    // run bytecode in VM
    VM *vm = initVM(chunk); 
    run(vm);

    // cleanup and return success
    freeChunk(chunk);
    freeVM(vm);
    return INTERPRET_OK;
}
