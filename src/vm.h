#ifndef MACHINE
#define MACHINE

#include "a.h"
#include "chunk.h"

#define STACK_MAX 128

typedef enum {
    INTERPRET_OK,   
    INTERPRET_COMPILE_ERROR
} InterpretResult;

typedef struct vm {
    Chunk   *chunk;            // chunk to execute
    uint8_t *ip;               // instruction pointer
    K       stack[STACK_MAX];  // K stack
    K       *top;              // K stack top
} VM;

// public function declarations
InterpretResult interpret(const char *source);

#endif
