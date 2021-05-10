#include "pymethods.h"

#include <iostream>

int hello_world(char* str) 
{
    std::string str_cpp(str);
    std::cout << "Hello World! Input: " << str_cpp << std::endl;

    return 123;
}
