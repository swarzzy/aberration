#pragma once
#include "src/ABHeader.h"

namespace ab {
	enum class ConsoleColor : uint16 {
		Black = 0, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkPurple, DarkYellow,
		DarkWhite, Gray, Blue, Green, Cyan, Red, Purple, Yellow, White
	};

	AB_API int32 console_print(const void* data, int64 count);
	AB_API int32 console_set_color(ConsoleColor textColor, ConsoleColor backColor);
}