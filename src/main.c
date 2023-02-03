#include "skrawl.h"
#include "parse.h"
#include "object.h"

int main(){
    for (;;){
        unref(printK(readK()));
#ifdef DBG_WS
        printf("WS:%ld WT:%ld\n", WS, WT);
#endif
    }
}
