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

    RandomAgent();

    bboard::Move act(const bboard::State* state) override;
};


/**
 * @brief Randomly selects actions that are not laying bombs
 */
struct HarmlessAgent : bboard::Agent
{
    std::mt19937_64 rng;
    std::uniform_int_distribution<int> intDist;

    HarmlessAgent();

    bboard::Move act(const bboard::State* state) override;
};

/**
 * @brief Selects Idle for every action
 */
struct LazyAgent : bboard::Agent
{
    bboard::Move act(const bboard::State* state) override;
};

// more agents to be included?

}

#endif
