// skrawl v2
// new to this implementation:
//   bytecode compilation + interpretation
//   lambdas
//   sym,date types

#include "a.h"
#include "vm.h"
#define SOURCE_MAX 1024

// static void runFile(){} // TODO

void repl(){
    char source[SOURCE_MAX];
    for(;;){
        putchar(' ');
        if(!fgets(source, sizeof(source), stdin)){
            putchar('\n');
            break;
        }
        interpret(source);
    }
}

int main(int argc, const char* argv[]){
    
    // if too many command line args passed, exit
    if (2 < argc){
        printf("Too many args. Usage:\n\n\tk [file.k]\n\n");
        exit(1);
    }
    
    // run file 
    if (2 == argc){
        printf("file load not yet implemented. exiting...\n");
        // runFile(argv[1]); TODO : implement load from file
        exit(1);
    };

    // run interactive 
    repl();
}
