#include "../Platform.h"
#include <windows.h>

namespace AB {

	std::string DateTime::ToString() {
		if (hour < 24 && minute < 60 && seconds < 60) {
			char buff[DATETIME_STRING_SIZE];
			sprintf(buff, "%02d:%02d:%02d", hour, minute, seconds);
			return std::string(buff);
		}
		return ("00:00:00");
	}

	AB_API void GetLocalTime(DateTime& datetime) {
		SYSTEMTIME time;
		GetLocalTime(&time);

		datetime.year = time.wYear;
		datetime.month = time.wMonth;
		datetime.dayOfWeek = time.wDayOfWeek;
		datetime.day = time.wDay;
		datetime.hour =	time.wHour;
		datetime.minute = time.wMinute;
		datetime.seconds = time.wSecond;
		datetime.milliseconds = time.wMilliseconds;
	}
}