/**
  Taken from https://stackoverflow.com/a/20170989
 */

#ifndef TESTING_UTILITIES_HPP
#define TESTING_UTILITIES_HPP

#include <iostream>
#include <iomanip>

#include "bboard.hpp"
#include "agents.hpp"

using namespace agents;

inline std::array<SimpleAgent, 4> CreateAgents(std::mt19937& rng)
{
    return {SimpleAgent(rng()), SimpleAgent(rng()), SimpleAgent(rng()), SimpleAgent(rng())};
}

inline std::array<SimpleUnbiasedAgent, 4> CreateUnbiasedAgents(std::mt19937& rng)
{
    return {SimpleUnbiasedAgent(rng()), SimpleUnbiasedAgent(rng()), SimpleUnbiasedAgent(rng()), SimpleUnbiasedAgent(rng())};
}

template<class T>
void RecursiveCommas(std::ostream& os, T n)
{
    T rest = n % 1000; //"last 3 digits"
    n /= 1000;         //"begining"

    if (n > 0)
    {
        RecursiveCommas(os, n); //printing "begining"

        //and last chunk
        os << ',' << std::setfill('0') << std::setw(3) << rest;
    }
    else
        os << rest; //first chunk of the number
}

template <class T>
constexpr
std::string_view
type_name()
{
    using namespace std;
#ifdef __clang__
    string_view p = __PRETTY_FUNCTION__;
    return string_view(p.data() + 34, p.size() - 34 - 1);
#elif defined(__GNUC__)
    string_view p = __PRETTY_FUNCTION__;
#  if __cplusplus < 201402
    return string_view(p.data() + 36, p.size() - 36 - 1);
#  else
    return string_view(p.data() + 49, p.find(';', 49) - 49);
#  endif
#elif defined(_MSC_VER)
    string_view p = __FUNCSIG__;
    return string_view(p.data() + 84, p.size() - 84 - 7);
#endif
}

#endif // TESTING_UTILITIES_HPP
