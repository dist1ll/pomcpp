#include <chrono>
#include <utility>
#include <iostream>
#include <cmath>

#include "catch.hpp"
#include "testing_utilities.hpp"

#include "bboard.hpp"
#include "agents.hpp"

using bboard::FixedQueue;

template<typename F, typename... Args>
double timeMethod(int times, F func, Args&&... args)
{
    std::chrono::duration<double, std::milli> total;
    auto t1 = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < times; i++)
    {
        func(std::forward<Args>(args)...);
    }
    total = std::chrono::high_resolution_clock::now() - t1;
    return total.count();
}

void Proxy(bboard::State* s, bboard::Move* m, bboard::Agent* a)
{
    m[0] = a->act(s);
    m[1] = a->act(s);
    m[2] = a->act(s);
    m[3] = a->act(s);
    bboard::Step(s, m);
}

TEST_CASE("Step Function", "[performance]")
{
    std::unique_ptr<bboard::State> sx = std::make_unique<bboard::State>();
    sx->PutAgentsInCorners(0, 1, 2, 3);

    agents::HarmlessAgent a;
    bboard::Move r = bboard::Move::RIGHT;
    bboard::Move m[4] = {r, r, r, r};

    int times = 100000;
    double t = timeMethod(times, Proxy, sx.get(), m, &a);

    std::cout << std::endl
              << "bboard::Step(s, m): "
              << int(std::floor(times/(t/100.0)))
              << " it.\t per 100ms"
              << std::endl
              << "Agent: "
              << type_name<decltype(a)>()
              << std::endl << std::endl;
}

inline void QueueRemovalSTD(FixedQueue<int, 20>& queue)
{
    queue.count = 20;
    for(int i = queue.count/2 - 1; i >= 0; i--)
    {
        queue.RemoveAt(i * 2);
    }
}

inline void QueueRemovalMEM(FixedQueue<int, 20>& queue)
{
    queue.count = 20;
    for(int i = 9; i >= 0; i--)
    {
        queue.RemoveAt_MEMCPY(i * 2);
    }
}

TEST_CASE("Fixed Queue Remove Copies", "[performance]")
{
    int times = 1000;
    int avgRuns = 100;
    std::cout << "FixedQueue::RemoveAt (Avg. over "
              << avgRuns << " runs)"
              << std::endl;

    FixedQueue<int, 20> q;

    double t1 = 0;
    double t2 = 0;

    for(int i = 0; i < avgRuns; i++)
    {
        t1 += timeMethod(times, QueueRemovalSTD, q);
    }
    for(int i = 0; i < avgRuns; i++)
    {
        t2 += timeMethod(times, QueueRemovalMEM, q);
    }

    t1 /= avgRuns;

    std::cout << times / 100 << "k"
              << " removal calls for STDCPY:\t"
              << t1 << "ms"
              << std::endl << std::endl;

}
