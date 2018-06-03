#include <iostream>
#include "bboard.hpp"

using std::cout;

int main()
{
    bboard::State* init = new bboard::State;
    bboard::PrintState(init);
}
