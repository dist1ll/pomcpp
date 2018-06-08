#ifndef RANDOM_AGENT_H
#define RANDOM_AGENT_H

#include <random>

#include "bboard.hpp"

namespace agents
{

/**
 * Use this as an example to implement more sophisticated
 * agents.
 *
 * @brief Randomly selects actions
 */
struct RandomAgent : bboard::Agent
{
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> intDist;

    RandomAgent()
    {
        std::random_device rd;  // non explicit seed
        rng = std::mt19937_64(rd());
        intDist = std::uniform_int_distribution<int>(0, 6);
    }

    bboard::Move act(bboard::State* state) override;
};


/**
 * @brief Selects Idle for every action
 */
struct LazyAgent : bboard::Agent
{
    bboard::Move act(bboard::State* state) override;
};

// more agents to be included?

}

#endif
