#include <iostream>

bool b = true; //dead store

int main(){
    b = false;
    b = true;
    std::cout << b << '\n';
    return 0;
}
