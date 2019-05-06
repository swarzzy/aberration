#pragma once 
#include "AB.h"

namespace AB
{
	i32 ConsolePrint(const void* data, u32 count);
	i32 ConsolePrint(const char* string);
	i32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor);
	// NOTE: If buffer is to small, writes bufferSizeBytes chars,
	// set bytesWritten to bufferSizeBytes and returns false
	b32 GetExecutablePath(char* buffer, u32 bufferSizeBytes, u32* bytesWritten);

	// Returns microseconds
	i64 GetCurrentRawTime();

	void GetLocalTime(DateTime* datetime);

	u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename);
	u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize,
								  const char* filename);
	
	void* DebugReadFilePermanent(MemoryArena* memory,
								 const char* filename, u32* bytesRead);

	u32 DebugGetFileSize(const char* filename);
	
	b32 DebugWriteFile(const char* filename,  void* data, u32 dataSize);
}
