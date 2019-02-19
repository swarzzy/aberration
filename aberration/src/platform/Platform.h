#pragma once 
#include "ABHeader.h"
#include <hypermath.h>
#include <../Aberration.h>


typedef void(GameInitializeFn)(AB::Engine* engine, AB::GameContext* gameContext);
typedef void(GameUpdateFn)(AB::Engine* engine, AB::GameContext* gameContext);
typedef void(GameRenderFn)(AB::Engine* engine, AB::GameContext* gameContext);

int main();

namespace AB {
	// If buffer is to small, writes bufferSizeBytes chars, set bytesWritten to bufferSizeBytes and returns false
	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten);

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

	AB_API void* DebugReadFile(const char* filename, uint32* bytesRead);
	AB_API void DebugFreeFileMemory(void* memory);
	AB_API bool32 DebugWriteFile(const char* filename,  void* data, uint32 dataSize);

}