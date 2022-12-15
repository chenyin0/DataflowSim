#pragma once

#include <memory>
#include <vector>
#include <array>
#include <list>
#include <string>
#include <functional>
#include <cmath>
#include <chrono>
#include <random>
#include <queue>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <tuple>
#include <utility>
#include <stack>
#include <bitset>
#include <algorithm>
#include <numeric>
#include <typeinfo>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <ctime>
#include <ratio>
#include <chrono>
#include <limits>
#include <time.h>

// #include <stdio.h>
#include "./Para.h"

// using uint64_t = std::size_t;
using uint64_t = uint64_t;
// using uint64 = uint64_t;
using std::accumulate;
using std::array;
using std::begin;
using std::bitset;
using std::copy;
using std::copy_if;
using std::deque;
using std::end;
using std::find;
using std::find_if;
using std::for_each;
using std::function;
using std::list;
using std::make_shared;
using std::make_unique;
using std::map;
using std::ostream;
using std::pair;
using std::queue;
using std::set;
using std::shared_ptr;
using std::stack;
using std::string;
using std::transform;
using std::tuple;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#define interface struct
#ifdef WINDOWS
#define DEBUG_ASSERT(x)  \
    if (!((void)0, (x))) \
    {                    \
        __debugbreak();  \
    }
#endif
#ifdef LINUX
#define DEBUG_ASSERT(x)  \
    if (!((void)0, (x))) \
    {                    \
        void __builtin_trap(); \
    }
#endif
#define DEBUG_CONSOLE true

#define str(s) #s
#define xstr(s) str(s)

#define ULONG_LONG_MAX (std::numeric_limits<uint64_t>::max)()

class Bool
{
public:
    Bool()
        : _value()
    {
    }

    Bool(bool value_)
        : _value(value_)
    {
    }

    operator bool() const
    {
        return _value;
    }

    bool *operator&()
    {
        return &_value;
    }

    const bool *operator&() const
    {
        return &_value;
    }

private:
    bool _value;
};

#define Bool Bool