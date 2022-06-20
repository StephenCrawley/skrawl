#ifndef COMPILE_H
#define COMPILE_H

#include "a.h"
#include "chunk.h"

#define OP_MONAD_START 0x20

// Bytecode Instructions
enum {
    // dyadic (2 argument) operators
    // opcodes in this range are used as an index into the dyads[] array (defined in verbs.c)
    // eg dyads[opcode]
    OP_DYAD_ADD = 0x00,
    OP_DYAD_MULTIPLY,
    OP_DYAD_SUBTRACT,
    OP_DYAD_DIVIDE,
    OP_DYAD_DOTAPPLY,
    OP_DYAD_KEY,
    OP_DYAD_MIN,
    OP_DYAD_MAX,
    OP_DYAD_LESS,
    OP_DYAD_MORE,
    OP_DYAD_EQUAL,
    OP_DYAD_MATCH,
    OP_DYAD_FIND,
    OP_DYAD_CAT,
    OP_DYAD_ATINDEX,
    OP_DYAD_TAKE,
    OP_DYAD_DROP,
    // monadic (1 argument) operators
    // the opcode is used to index the monads[] array (defined in verbs.c)
    // eg monads[opcode - OP_MONAD_START]
    OP_MONAD_FLIP = OP_MONAD_START,
    OP_MONAD_FIRST,
    OP_MONAD_NEGATE,
    OP_MONAD_SQRT,
    OP_MONAD_VALUE,
    OP_MONAD_ENUMERATE,
    OP_MONAD_REVERSE,
    OP_MONAD_WHERE,
    OP_MONAD_ASC,
    OP_MONAD_DESC,
    OP_MONAD_GROUP,
    OP_MONAD_NOT,
    OP_MONAD_DISTINCT,
    OP_MONAD_ENLIST,
    OP_MONAD_TYPE,
    OP_MONAD_COUNT,
    // put list literal into K object
    OP_ENLIST,
    // load literal value from constants array
    OP_CONSTANT,
    // pop func and apply to top n (immediately encoded) stack objects to the func
    OP_APPLY,
    // pop func and create projection with top n stack onjects
    OP_PROJECT,
    // set global variable
    OP_SETGLOBAL,
    // get global variable
    OP_GETGLOBAL,
    // print top of stack and stop execution
    OP_RETURN,
    // kill the process
    OP_TERMINATE
};

// forward declarations
bool compile(Chunk *chunk);

#endif
