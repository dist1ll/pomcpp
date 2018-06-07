#include <iostream>
#include "bboard.hpp"
#include "agents.hpp"

using std::cout;

int main()
{
    // Init
    bboard::State* init = bboard::InitState(0,1,2,3);
    bboard::PrintState(init);

    bboard::Move moves[4];
    // loop this
    moves[0] = bboard::Move::RIGHT;
    moves[1] = bboard::Move::IDLE;
    moves[2] = bboard::Move::IDLE;
    moves[3] = bboard::Move::IDLE;

    bboard::Step(init, moves);

    bboard::PrintState(init);

    delete init;
}
