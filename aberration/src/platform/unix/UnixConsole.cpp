#include "../Console.h"
#include <unistd.h>

namespace ab {
	static const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = ConsoleColor::DarkWhite;
	static const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = ConsoleColor::Black;

	int32 console_print(const void* data, uint32 count) {
		ssize_t result = write(1, data, count);
		if (result != count)
			return 0;
		return 1;
	}

	int32 console_set_color(ConsoleColor textColor, ConsoleColor backColor) {
		return 0;
	}
}