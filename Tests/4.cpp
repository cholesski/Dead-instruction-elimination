#include <iostream>

int main(){
    int a = 10; //dead store
    if (true){
        std::cout << "hello world!";
    } else{
        int b = a;
        std::cout << "nothing";
    }
    return 0;
}