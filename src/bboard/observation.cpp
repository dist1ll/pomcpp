#include <thread>
#include <chrono>
#include <functional>
#include <algorithm>
#include <random>

#include "bboard.hpp"

namespace bboard
{

inline void copyBoardTo(const State& state, Observation& observation)
{
    std::copy_n(&state.board[0][0], BOARD_SIZE * BOARD_SIZE, &observation.board[0][0]);
    observation.bombs = state.bombs;
    observation.flames = state.flames;
}

inline void copyAgentsTo(const State& state, Observation& observation)
{
    observation.agentInfos.count = 0;
    for(int i = 0; i < AGENT_COUNT; i++)
    {
        observation.agentIDMapping[i] = i;
        observation.agentInfos.AddElem(state.agents[i]);
    }
}

inline void copyTo(const State& state, Observation& observation)
{
    copyBoardTo(state, observation);
    copyAgentsTo(state, observation);
}

void Observation::Get(const State& state, const uint agentID, const ObservationParameters obsParams, Observation& observation)
{
    // fully observable environment
    if(obsParams.exposePowerUps && !obsParams.agentPartialMapView && obsParams.agentInfoVisibility == AgentInfoVisibility::All)
    {
        copyTo(state, observation);
        return;
    }

    // board observations

    if(obsParams.agentPartialMapView)
    {
        // hide invisible area
        AgentInfo info = state.agents[agentID];

        int leftFogCount = std::max(0, info.x - obsParams.agentViewSize);
        int rightFogBegin = std::min(BOARD_SIZE, info.x + obsParams.agentViewSize + 1);
        int rightFogCount = BOARD_SIZE - rightFogBegin;
        int viewRowLength = rightFogBegin - leftFogCount;

        for(int y = 0; y < BOARD_SIZE; y++)
        {
            if(std::abs(y - info.y) > obsParams.agentViewSize)
            {
                // fill the whole row with fog
                std::fill_n(&observation.board[y][0], BOARD_SIZE, bboard::Item::FOG);
            }
            else
            {
                // row is inside the agent's view, fill the row partially with fog

                // cells on the left
                std::fill_n(&observation.board[y][0], leftFogCount, bboard::Item::FOG);

                // copy board items
                if(obsParams.exposePowerUps)
                {
                    std::copy_n(&state.board[y][leftFogCount], viewRowLength, &observation.board[y][leftFogCount]);
                }
                else
                {
                    for(int x = leftFogCount; x < rightFogBegin; x++)
                    {
                        int item = state.board[y][x];
                        if(IS_WOOD(item))
                        {
                            // erase the powerup information
                            observation.board[y][x] = Item::WOOD;
                        }
                        else
                        {
                            observation.board[y][x] = item;
                        }
                    }
                }

                // cells on the right
                std::fill_n(&observation.board[y][rightFogBegin], rightFogCount, bboard::Item::FOG);
            }
        }

        // filter bomb objects
        observation.bombs.count = 0;
        for(int i = 0; i < state.bombs.count; i--)
        {
            int b = state.bombs[i];

            if(InViewRange(info.x, info.y, BMB_POS_X(b), BMB_POS_Y(b), obsParams.agentViewSize))
            {
                observation.bombs.AddElem(b);
            }
        }

        // and flames
        observation.flames.count = 0;
        for(int i = 0; i < state.flames.count; i--)
        {
            Flame f = state.flames[i];

            if(InViewRange(f.position, info.x, info.y, obsParams.agentViewSize))
            {
                observation.flames.AddElem(f);
            }
        }
    }
    else
    {
        // full view on the arena
        copyBoardTo(state, observation);

        if(!obsParams.exposePowerUps)
        {
            for(int y = 0; y < BOARD_SIZE; y++)
            {
                for(int x = 0; x < BOARD_SIZE; x++)
                {
                    int item = observation.board[y][x];
                    if(IS_WOOD(item) && item != Item::WOOD)
                    {
                        // erase the powerup information
                        observation.board[y][x] = Item::WOOD;
                    }
                }
            }
        }
    }

    // agent observations

    switch (obsParams.agentInfoVisibility) {
        case bboard::AgentInfoVisibility::All:
            copyAgentsTo(state, observation);
            break;

        case bboard::AgentInfoVisibility::OnlySelf:
            observation.agentInfos.AddElem(state.agents[agentID]);
            std::fill_n(observation.agentIDMapping, AGENT_COUNT, -1);
            observation.agentIDMapping[agentID] = 0;
            break;

        case bboard::AgentInfoVisibility::InView:
            AgentInfo self = state.agents[agentID];
            // add self
            observation.agentIDMapping[agentID] = 0;
            observation.agentInfos.AddElem(self);
            // add others if visible
            for(uint i = 0; i < AGENT_COUNT; i++)
            {
                if(i == agentID) continue;

                AgentInfo other = state.agents[i];
                if(InViewRange(self.x, self.y, other.x, other.y, obsParams.agentViewSize))
                {
                    // visible
                    observation.agentIDMapping[i] = observation.agentInfos.count;
                    observation.agentInfos.AddElem(other);
                }
                else
                {
                    // not visible
                    observation.agentIDMapping[i] = -1;
                }
            }
            break;
    }
}

}
