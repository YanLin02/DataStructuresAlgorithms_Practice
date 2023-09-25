
#pragma once
#include "Definition.h"

#include <unordered_map>

using namespace std;

typedef pair<unsigned int, unsigned int> Pa2;

/// @brief Pa2's hash, used for hashmap
struct Pa2_hash
{
	template <class T, class U>
	int operator()(const pair<T, U>& a) const
	{
		return hash<int>()(a.first) * 17 ^ hash<int>()(a.second) * 71;
	}
};

/// @brief Êý×é»º´æ
class Buffer
{
public:

	unordered_map<Pa2, double, Pa2_hash> Buf;
	
};

