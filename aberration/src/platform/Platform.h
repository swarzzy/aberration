#pragma once 
#include "src/ABHeader.h"

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

		std::string ToString();
		static const uint16 DATETIME_STRING_SIZE = 9; // hh:mm:ss\0
	};

	AB_API void get_local_time(DateTime& datetime);
}