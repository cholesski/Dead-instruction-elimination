#include <iostream>

int x = 1; //dead store

int main(){
    x = 5;
    std::cout << x;
    return 0;
}