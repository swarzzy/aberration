#pragma once 
#include "src/ABHeader.h"
#include <string>

#if defined(AB_PLATFORM_WINDOWS)
#define AB_DEBUG_BREAK() __debugbreak()
#elif defined(AB_PLATFORM_LINUX)
#define AB_DEBUG_BREAK() __builtin_debugtrap()
#endif

namespace AB {

	struct AB_API DateTime {
		uint16 year;
		uint16 month;
		uint16 dayOfWeek;
		uint16 day;
		uint16 hour;
		uint16 minute;
		uint16 seconds;
		uint16 milliseconds;

		String ToString();
		static const uint16 DATETIME_STRING_SIZE = 9; // hh:mm:ss\0
	};

	AB_API void GetLocalTime(DateTime& datetime);

}