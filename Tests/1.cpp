#include <iostream>

int main(){
    int x,y,z;
    x = 5; //dead store
    y = 12;
    z = 11;
    for (int i = 0; i < 0; i++){
        z += 3*y;
        x = 7; //dead store
        std::cout<<"OVDE";
    }
    std::cout << z << '\n';
    x = 50;
    z = x+5;
    std::cout << z << '\n';
    return 0;
}