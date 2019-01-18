#include "../Console.h"
#include <unistd.h>

namespace AB {
	static const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = ConsoleColor::DarkWhite;
	static const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = ConsoleColor::Black;

	AB_API int32 console_print(const void* data, uint32 count) {
		ssize_t result = write(1, data, count);
		if (result != count)
			return 0;
		return 1;
	}

    AB_API int32 ConsolePrint(const char* string) {
	    // TODO: Better console output
        auto result = printf("%s", string);
        if (result >= 0)
            return 1;
        else
            return 0;
	}

	AB_API int32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor) {
		return 0;
	}
}