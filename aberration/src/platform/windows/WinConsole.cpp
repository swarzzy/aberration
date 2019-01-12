#include "../Console.h"

// Windows implemetation
#include <Windows.h>

namespace ab {
	static const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = ConsoleColor::DarkWhite;
	static const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = ConsoleColor::Black;

	// NOTE: Might be unsafe store handle as static variable
	static HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	int32 console_print(const void* data, uint32 count) {
		DWORD written;
		BOOL result = WriteConsoleA(consoleHandle, data, count, &written, NULL);
		if (written != count || result == 0)
			return 0;
		return 1;
	}

	int32 console_set_color(ConsoleColor textColor, ConsoleColor backColor) {
		if (textColor == ConsoleColor::Default)
			textColor = CONSOLE_DEFAULT_TEXT_COLOR;
		if (backColor == ConsoleColor::Default)
			backColor = CONSOLE_DEFAULT_BACK_COLOR;

		WORD consoleColor = static_cast<WORD>(textColor) | (static_cast<WORD>(backColor) << 4);
		BOOL result = SetConsoleTextAttribute(consoleHandle, consoleColor);
		if (result == 0)
			return 0;
		return 1;
	}
}