#include <chrono>
#include <utility>
#include <iostream>
#include <cmath>

#include "catch.hpp"
#include "testing_utilities.hpp"

#include "bboard.hpp"
#include "agents.hpp"

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
    sx.get()->PutAgentsInCorners(0, 1, 2, 3);

    agents::RandomAgent a;
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
              << std::endl;
}
