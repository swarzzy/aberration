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
#include "utils/Log.h"

//#include "platform/Platform.h"