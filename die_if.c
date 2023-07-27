#include <stdio.h>

int main(){
    int a = 5;
    if (5){
        printf("hello world");
    } else {
        printf("ovo brisemo");
    }

    if (0){
        printf("ovo brisemo");
        a = 10;
    } else {
        printf("%d", a);
    }

    return 0;
}