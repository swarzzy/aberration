#pragma once 
#include "AB.h"

namespace AB {


	AB_API int32 ConsolePrint(const void* data, uint32 count);
	AB_API int32 ConsolePrint(const char* string);
	AB_API int32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor);
	// If buffer is to small, writes bufferSizeBytes chars, set bytesWritten to bufferSizeBytes and returns false
	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten);

	// Returns microseconds
	int64 GetCurrentRawTime();

	AB_API void GetLocalTime(DateTime& datetime);

	struct DebugReadTextFileRet
	{
		char* data;
		u32 size;
	};

	AB_API DebugReadTextFileRet DebugReadTextFile(const char* filename);

	AB_API void* DebugReadFile(const char* filename, uint32* bytesRead);

	u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename);
	u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize,
								  const char* filename);
	
	void* DebugReadFilePermanent(MemoryArena* memory,
								 const char* filename, uint32* bytesRead);

	u32 DebugGetFileSize(const char* filename);
	
	struct DebugReadFileOffsetRet {
		void* data;
		uint32 read;
	};
	
	AB_API DebugReadFileOffsetRet DebugReadFileOffset(const char* filename, uint32 offset, uint32 size);

	AB_API void DebugFreeFileMemory(void* memory);
	AB_API bool32 DebugWriteFile(const char* filename,  void* data, uint32 dataSize);
}
