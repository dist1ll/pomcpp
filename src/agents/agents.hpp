#ifndef RANDOM_AGENT_H
#define RANDOM_AGENT_H

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
