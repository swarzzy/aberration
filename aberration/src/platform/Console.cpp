#include "Console.h"

// Windows implemetation
#if defined(AB_PLATFORM_WINDOWS)
#include <Windows.h>

namespace ab {
	// NOTE: Might be unsafe store handle as static variable
	static HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	int32 console_print(const void* data, int64 count) {
		DWORD written;
		BOOL result = WriteConsoleA(consoleHandle, data, count, &written, NULL);
		if (written != count || written == 0)
			return 0;
		return 1;
	}

	int32 console_set_color(ConsoleColor textColor, ConsoleColor backColor) {
		WORD consoleColor = static_cast<WORD>(textColor) | (static_cast<WORD>(backColor) << 4);
		BOOL result = SetConsoleTextAttribute(consoleHandle, consoleColor);
		if (result == 0)
			return 0;
		return 1;
	}
}

#endif
