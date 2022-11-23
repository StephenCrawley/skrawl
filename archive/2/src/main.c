// skrawl v2
// new to this implementation:
//   bytecode compilation + interpretation
//   lambdas
//   sym,date types

#include "a.h"
#include "vm.h"
#define SOURCE_MAX 1024

static void runFile(VM *vm, const char * file){
    FILE * fp;
    char * line = NULL;
    size_t len = 0;

    fp = fopen(file, "r");
    if (NULL == fp){
        printf("file error! couldn't read %s", file);
        exit(EXIT_FAILURE);
    }

    while (-1 != getline(&line, &len, fp))
        interpret(vm, line);

    fclose(fp);
    if (line)
        free(line);
}

void repl(VM *vm){
    char source[SOURCE_MAX];
    for (;;){
        putchar(' ');
        if (!fgets(source, sizeof(source), stdin)){
            putchar('\n');
            break;
        }
        interpret(vm, source);
    }
}

int main(int argc, const char **argv){
    
    // if too many command line args passed, exit
    if (2 < argc){
        printf("Too many args. Usage:\n\n\tk [file.k]\n\n");
        exit(1);
    }
    // init a VM instance
    VM *vm = initVM();

    // run file 
    if (2 == argc){
        runFile(vm, argv[1]);
    };

    // run interactive 
    repl(vm);
}
