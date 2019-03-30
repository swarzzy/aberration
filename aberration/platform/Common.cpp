#include "Common.h"

namespace AB {

	uint32 DateTime::ToString(char* buffer, uint32 bufferSize) {
		if (hour < 24 && minute < 60 && seconds < 60) {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				int32 written = FormatString(buffer, bufferSize, "%02u16:%02u16:%02u16", hour, minute, seconds);
				return DATETIME_STRING_SIZE - 1;
			}
		}
		else {
			if (bufferSize >= DATETIME_STRING_SIZE) {
				FormatString(buffer, bufferSize, "00:00:00");
				return DATETIME_STRING_SIZE - 1;
			}
		}
		return 1;
	}

}
