#pragma once 
#include "AB.h"
int main();

namespace AB {

	enum class ConsoleColor : uint16 {
		Black = 0, DarkBlue, DarkGreen, DarkCyan, DarkRed, DarkPurple, DarkYellow,
		DarkWhite, Gray, Blue, Green, Cyan, Red, Purple, Yellow, White, Default
	};

	AB_API int32 ConsolePrint(const void* data, uint32 count);
	AB_API int32 ConsolePrint(const char* string);
	AB_API int32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor);
	// If buffer is to small, writes bufferSizeBytes chars, set bytesWritten to bufferSizeBytes and returns false
	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten);

	// Returns microseconds
	int64 GetCurrentRawTime();

	struct AB_API DateTime {
		uint16 year;
		uint16 month;
		uint16 dayOfWeek;
		uint16 day;
		uint16 hour;
		uint16 minute;
		uint16 seconds;
		uint16 milliseconds;

		uint32 ToString(char* buffer, uint32 bufferSize);
		static const uint16 DATETIME_STRING_SIZE = 9; // hh:mm:ss\0
	};

	AB_API void GetLocalTime(DateTime& datetime);

	struct DebugReadTextFileRet {
		char* data;
		uint32 size;
	};

	AB_API DebugReadTextFileRet DebugReadTextFile(const char* filename);

	AB_API void* DebugReadFile(const char* filename, uint32* bytesRead);

	struct DebugReadFileOffsetRet {
		void* data;
		uint32 read;
	};
	
	AB_API DebugReadFileOffsetRet DebugReadFileOffset(const char* filename, uint32 offset, uint32 size);

	AB_API void DebugFreeFileMemory(void* memory);
	AB_API bool32 DebugWriteFile(const char* filename,  void* data, uint32 dataSize);
}
