#include "skrawl.h"
#include "parse.h"
#define SRC_MAX 128

int main(){

    // repl
    char src[SRC_MAX];
    for (;;){
        putchar(' ');
        fgets(src, SRC_MAX, stdin);
        parse(src);
    }
}
