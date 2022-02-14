#ifndef MACHINE_H
#define MACHINE_H

#include "a.h"
#include "chunk.h"

#define STACK_MAX 128

typedef enum {
    INTERPRET_OK,
    INTERPRET_PARSE_ERROR,
    INTERPRET_COMPILE_ERROR
} InterpretResult;

typedef struct vm {
    Chunk   *chunk;            // chunk to execute
    uint8_t *ip;               // instruction pointer
    K       stack[STACK_MAX];  // K stack
    K       *top;              // K stack top
} VM;

// public function declarations
InterpretResult interpret(VM *vm, const char *source);
VM *initVM();

#endif
