#include <iostream>

int f(int p, int q){
    p = 11; //dead store
    return q + 3;
}

int main(){
    int x;
    int p = 3;
    int q = 4; //dead store
    q = 5;
    x = f(p,q); 
    x = f(1,1);
    std::cout << x;
    return 0;
}
