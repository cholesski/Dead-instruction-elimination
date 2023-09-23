#include <iostream>

bool b = true; 

int main(){
    b = false; //dead store
    b = true;
    std::cout << b << '\n';
    return 0;
}
