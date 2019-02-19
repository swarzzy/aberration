#pragma once
#include "ABHeader.h"

namespace AB {

	enum class ConsoleColor : uint16 {
		Black = 0, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkPurple, DarkYellow,
		DarkWhite, Gray, Blue, Green, Cyan, Red, Purple, Yellow, White, Default
	};

	AB_API int32 ConsolePrint(const void* data, uint32 count);
	AB_API int32 ConsolePrint(const char* string);
	AB_API int32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor);
}