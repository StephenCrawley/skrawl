#include "a.h"
#include "compile.h"
#include "chunk.h"
#include "object.h"
#include "token.h"

// forward declarations
static void compileBranch(Chunk *chunk, K x);

static void addByte(Chunk *chunk, uint8_t code){
    if(chunk->codeCount == chunk->codeCapacity){
        growArray(chunk, sizeof(*chunk->code));
    }
    chunk->code[chunk->codeCount] = code;
    chunk->codeCount++;
}

// add a constant load instruction to the code
// the instruction is followed by the constant's index in chunk->k
static void addConstant(Chunk *chunk, K x){
    if (MAX_K_CONSTS <= chunk->kCount){
        if (!chunk->compileError){ // report only once
            chunk->compileError = true;
            printf("Compile error! MAX_K_CONSTS (%d) reached.\n", MAX_K_CONSTS);
        }
        return;
    }
    addByte(chunk, OP_CONSTANT);
    chunk->k[chunk->kCount] = ref(x);
    addByte(chunk, (uint8_t)chunk->kCount);
    chunk->kCount++;
}

static void compileLeaf(Chunk *chunk, K x){
	if (KI == ABS(xt) || KF == ABS(xt) || KC == ABS(xt) || KN == ABS(xt) || KU == xt || KV == xt || KA == xt || KP == xt){
        addConstant(chunk, x);
	}
	// sym literal
    // NB: when object is created at compile time we need to make refcount=0  
    // because addConstant ref's an object before it's added to the constant pool
    // otherwise there's a memleak when the chunk is freed 
	else if (KS == xt){
        K t;
        addConstant(chunk, ( 1 == xn ) ? 
                           ( t = Ks(xi[0]), tr = 0, t ) : 
                           x
                    );
    }
    // get a global variable
    else if (-KS == xt){
        addByte(chunk, OP_GETGLOBAL);
        chunk->k[chunk->kCount] = ref(x);
        addByte(chunk, (uint8_t)chunk->kCount);
        chunk->kCount++;
    }
    else {
        chunk->compileError = true;
        printf("Compile error! Unrecognised type.\n");
    }
}

// pop n objects from stack and apply to x
static void compileApply(Chunk *chunk, K x, uint8_t n){
	compileBranch(chunk, x);
	addByte(chunk, OP_APPLY);
	addByte(chunk, n);
}

// pop n objects from stack and project on x
// this is for when the parser creates -KN objects ("magic values"). eg parsing "1+" or "@[1;;2]"
// m is the number of magic values present. this is the rank of the resultant projection
// magic values are only created at compile time, so best to count+encode them now too
static void compileProject(Chunk *chunk, uint8_t n, uint8_t m){
	addByte(chunk, OP_PROJECT);
	addByte(chunk, n);
	addByte(chunk, m);
}

static void compileMonad(Chunk *chunk, K x, uint8_t n){
	if (TOKEN_COMMA == xc[0]){
        addByte(chunk, OP_ENLIST);
		addByte(chunk, n);
        return;
    }

    if (1 == n){
        uint8_t opcode = OP_MONAD_START + (uint8_t) xc[0];
        addByte(chunk, opcode);
    }
    else {
        compileApply(chunk, x, xn-1);
    }
}

static void compileDyad(Chunk *chunk, K x){
    if (TOKEN_COLON == xc[0]){
        addByte(chunk, OP_SETGLOBAL);
        return;
    }

	uint8_t opcode = (uint8_t) xc[0];
    addByte(chunk, opcode);
}

// protected eval is an error-trapping construct
// .[f;a;e] -> apply f to a, if error evaluate e 
static void compileProtectedEval(Chunk *chunk, K x){
    // hacky but simple and works
    // compile (.;f;args;err) as (.;f;args) then compile the jump+error code
    xn--;
    compileBranch(chunk, x);
    xn++;
    addByte(chunk, OP_JUMP_IF_NOT_ERROR);
    // placeholder byte for # of bytecodes to jump over if error
    addByte(chunk, 0xff);
    uint16_t n = chunk->codeCount;
    // compile the error branch
    compileBranch(chunk, xk[xn-1]);
    // overwrite the placeholder
    chunk->code[n-1] = chunk->codeCount - n;
}

// check if x is (\;\). if so then generate OP_TERMINATE instruction
static bool isDoubleBackslash(K x){
    return KK == xt && 2 == xn && 
           KA == TYPE(xk[0]) && TOKEN_BSLASH == CHAR(xk[0])[0] &&
           KA == TYPE(xk[1]) && TOKEN_BSLASH == CHAR(xk[1])[0];
}

static void compileBranch(Chunk *chunk, K x){
	// if terminal value (literal or variable)
	if (KK != xt){
		compileLeaf(chunk, x);
	}
	// else branch (func; arg1; ... ; argn)
	else {
        // compile empty generic list ()
        if (0 == xn){
            addConstant(chunk, x);
            return;
        }

        // check for \\ (terminate process)
        if (isDoubleBackslash(x) || isDoubleBackslash(xk[0])){
                addByte(chunk, OP_TERMINATE);
                return;
            }
            
        // compile sym list literal
        if (1 == xn && KS == TYPE(xk[0])){
            compileLeaf(chunk, xk[0]);
            return;
        }

        // compile protected eval
        if (4 == xn && KV == TYPE(xk[0]) && TOKEN_DOT == CHAR(xk[0])[0]){
            compileProtectedEval(chunk, x);
            return;
        }

		K t;
		bool project = false;
		uint8_t mval = 0; // magic value count
	
		// first compile the args
		for (uint64_t i = xn-1; i >= 1; --i){
			t = xk[i];
            
            // special case: sym atoms (variables)
            if (-KS == tt){
                // if (:;`sym;...) then we're assigning a variable
                if (1 == i && (KV == TYPE(xk[0]) && TOKEN_COLON == CHAR(xk[0])[0])){
                    addConstant(chunk, t);
                    continue;
                }
                // else we're loading a variable
                else {
                    compileLeaf(chunk, t);
                    continue;
                }
            }

			( KK == tt ) ? compileBranch(chunk, t) : compileLeaf(chunk, t);

            // keep track of magic values
			if (-KN == tt) { project = true; mval++; }
		}

		// then compile the applicable value
		t = xk[0];

        // compile projections
        if (project){
            compileBranch(chunk, t);
            compileProject(chunk, xn, mval);
            return;
        }
		
        // or compile 
		if (KU == tt){
			compileMonad(chunk, t, xn-1);
		}
		else if (KV == tt){
			( 3 == xn ) ? compileDyad(chunk, t) : compileApply(chunk, t, xn-1);
		}
        else if (KA == tt && TOKEN_APOSTROPHE == tc[0] && 3 == xn){ // composition
            addByte(chunk, OP_COMPOSE);
        }
		else {
			compileApply(chunk, t, xn-1);
		}
	}
}

static void compileExpressions(Chunk *chunk, K x){
    if (KK == xt && 0 < xn && -KC == TYPE(xk[0]) && ';' == CHAR(xk[0])[0]){ // if ; separated expressions
        for(uint64_t i = 1; i < xn; ++i)
            compileBranch(chunk, xk[i]);
    }
    else {
        compileBranch(chunk, x);
    }
}

bool compile(Chunk *chunk){
    compileExpressions(chunk, chunk->parseTree);
    addByte(chunk, OP_RETURN);
    return !(chunk->compileError);
}
