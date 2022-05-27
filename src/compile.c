#include "a.h"
#include "compile.h"
#include "chunk.h"
#include "object.h"
#include "token.h"

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
    addByte(chunk, OP_CONSTANT);
    chunk->k[chunk->kCount] = ref(x);
    addByte(chunk, (uint8_t)chunk->kCount);
    chunk->kCount++;
}

static void compileLeaf(Chunk *chunk, K x){
	if (KI == ABS(xt) || KF == ABS(xt) || KC == ABS(xt) || KN == ABS(xt) || KU == ABS(xt) || KV == ABS(xt)){
        addConstant(chunk, x);
	}
	// sym literal
	else if (KS == xt){
        addConstant(chunk, 1 == xn ? Ks(xi[0]) : x);
    }
    // TODO : implement variables
    else if (-KS == xt){
        chunk->compileError = true;
        printf("Compile error! variables not yet implemented\n");
    }
    else {
        chunk->compileError = true;
        printf("Compile error! Unrecognised type.\n");
    }
}

// pop n objects from stack and apply to x
static void compileApply(Chunk *chunk, K x, uint8_t n){
	addConstant(chunk, x);
	addByte(chunk, OP_APPLY);
	addByte(chunk, n);
}

// pop n objects from stack and project on x
// this is for when the parser creates -KN objects ("magic values"). eg parsing "1+" or "@[1;;2]"
// m is the number of magic values present. this is the rank of the resultant projection
// magic values are only created at compile time, so best to count+encode them now too
static void compileProject(Chunk *chunk, K x, uint8_t n, uint8_t m){
	addConstant(chunk, x);
	addByte(chunk, OP_PROJECT);
	addByte(chunk, n);
	addByte(chunk, m);
}

static void compileMonad(Chunk *chunk, K x, uint8_t n){
	if (TOKEN_COMMA == xc[0]){
        addByte(chunk, OP_ENLIST);
		addByte(chunk, n);
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
	uint8_t opcode = (uint8_t) xc[0];
    addByte(chunk, opcode);
}

static void compileBranch(Chunk *chunk, K x){
	// if terminal value (literal or variable)
	if (KK != xt){
		compileLeaf(chunk, x);
	}
	// else branch (func; arg1; ... ; argn)
	else {
        // compile sym list literal
        if (1 == xn && KS == TYPE(xk[0])){
            compileLeaf(chunk, xk[0]);
            return;
        }

		K t;
		bool project = false;
		uint8_t mval = 0; // magic value count
	
		// first compile the args
		for (uint64_t i = xn-1; i >= 1; --i){
			t = xk[i];
			( KK == tt ) ? compileBranch(chunk, t) : compileLeaf(chunk, t);
            // keep track of magic values
			if (-KN == tt) { project = true; mval++; }
		}
		
		// then compile the func
		t = xk[0];
		if (KK == tt){
			compileBranch(chunk, t);
		}
		else if (KU == tt){
			( project ) ? compileProject(chunk, t, xn, mval) : compileMonad(chunk, t, xn-1);
		}
		else if (KV == tt){
			( project ) ? compileProject(chunk, t, xn, mval) : 
			( 3 == xn ) ? compileDyad(chunk, t) : 
			              compileApply(chunk, t, xn-1);
		}
		else {
			compileApply(chunk, t, xn-1);
		}
	}
}

static void compileExpressions(Chunk *chunk, K x){
    if(KK == xt && -KC == TYPE(xk[0]) && ';' == CHAR(xk[0])[0]){ // if ; separated expressions
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
