#include "skrawl.h"
#include "parse.h"
#include "object.h"

#define SRC_MAX 128

int main(){
    K r;
    // repl
    char src[SRC_MAX];
    
    for (;;){
        putchar(' ');
        fgets(src, SRC_MAX, stdin);
        if ((r=parse(src))) unref(printK(r));

#ifdef DBG_WS
        printf("WS:%ld WT:%ld\n", WS, WT);
#endif
    }
}
