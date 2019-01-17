#pragma once
#if defined(_MSC_VER) 
#pragma warning(disable : 4530)
// Because exceptions are disabled

#pragma warning(disable : 4251)
// Problems with using STL in dllexport stuff
// MSDN says: C4251 can be ignored if you are deriving from a 
// type in the C++ Standard Library, compiling a debug release (/MTd) 
// and where the compiler error message refers to _Container_base.

#endif
// Aberration headers
#include "Common.h"
#include "Types.h"
#include "platform/Memory.h"

// STL
#include <string_view>
#include <sstream>
#include <vector>
#include <utility>

#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <functional>

#if !defined(_MSC_VER)
#include <cstring>
#endif

//#include "platform/Platform.h"
#include "utils/String.h"
//#include "utils/Log.h"