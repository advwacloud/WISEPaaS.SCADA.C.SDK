#include <stdlib.h>
#include <stdio.h>

void SetEvent(void (*callback)(void)){
    //while(1){
        sleep(1);
        callback();
        sleep(1);
        callback();
        sleep(1);
        callback();
        sleep(1);
        callback();
    //}
}

void testfunc(){
    printf("like\n");
}

void main(){
    SetEvent(testfunc);

    while(1){
        sleep(1);
        printf("-----\n");
    }
}