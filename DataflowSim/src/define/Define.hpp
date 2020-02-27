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

#include <ctime>
#include <ratio>
#include <chrono>

using uint = std::size_t;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;
using std::vector;
using std::array;
using std::list;
using std::string;
using std::queue;
using std::deque;
using std::ostream;
using std::stack;
using std::set;
using std::map;
using std::function;
using std::tuple;
using std::pair;
using std::unordered_set;
using std::unordered_map;
using std::bitset;
using std::for_each;
using std::begin;
using std::end;
using std::find;
using std::find_if;
using std::copy;
using std::copy_if;
using std::transform;
using std::accumulate;

#define interface struct
#define DEBUG_ASSERT(x) if ( !((void)0,(x))) { __debugbreak(); }
#define DEBUG_CONSOLE true

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

	bool* operator& ()
	{
		return &_value;
	}

	const bool* operator&() const
	{
		return &_value;
	}

private:

	bool _value;

};

#define Bool Bool