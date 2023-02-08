#include "parse.h"   //readK
#include "vm.h"      //evalK
#include "object.h"  //printK, WS, WT

int main(){
    for (;;){
        printK(evalK(readK()));
#ifdef DBG_WS
        printf("WS:%ld WT:%ld\n", WS, WT);
#endif
    }
}
