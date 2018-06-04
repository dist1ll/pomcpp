#include <iostream>
#include "bboard.hpp"

using std::cout;

int main()
{
    //Init
    bboard::State* init = bboard::InitState(0,1,2,3);
    bboard::PrintState(init);

    delete init;
}
