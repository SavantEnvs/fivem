#pragma once

// Minimal stand-in for the game's precompiled StdInc.h so net-base/net-packet build standalone on Linux.
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include <unistd.h>

#define DLL_IMPORT
#define DLL_EXPORT
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

// identical to the definition in code/client/shared/Utils.h
inline constexpr uint32_t HashRageString(std::string_view string)
{
	uint32_t hash = 0;

	for (char ch : string)
	{
		hash += ch;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}
