#include "../Memory.h"
#include "src/platform/Console.h"
#include <Windows.h>
#include <cstdlib>

#define AB_ALLOC_PROC(size) malloc(size)
#define AB_FREE_PROC(block) free(block)

static uint64 LOGGING_TRESHOLD = 1024; // * 1024

static uint64 CurrentUsedMemory		= 0;
static uint64 CurrentAllocations	= 0;
static uint64 TotalUsedMemory		= 0;
static uint64 TotalAllocations		= 0;

namespace AB::internal {
	AB_API void* AllocateMemory(uint64 size) {
		CurrentUsedMemory += size;
		CurrentAllocations++;
		TotalUsedMemory += size;
		TotalAllocations++;
		
		uint64 actualSize = size + sizeof(uint64);
		byte* block = static_cast<byte*>(AB_ALLOC_PROC(actualSize));
		memcpy(block, &size, sizeof(uint64));
		return block + sizeof(uint64);
	}

	AB_API void FreeMemory(void* block) {
		if (block != nullptr) {
			byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
			uint64 size = *reinterpret_cast<uint64*>(actualBlock);
			CurrentUsedMemory -= size;
			CurrentAllocations--;

			AB_FREE_PROC(actualBlock);
		}
	}

	AB_API void* AllocateMemoryDebug(uint64 size, const char* file, uint32 line) {
		CurrentUsedMemory += size;
		CurrentAllocations++;
		TotalUsedMemory += size;
		TotalAllocations++;

#if defined(AB_DEBUG_MEMORY)
		if (size > LOGGING_TRESHOLD) {
			//TODO: propper logging
			char buff[256];
			sprintf(buff, "Large allocation(>%llu bytes): %llu bytes in file: %s, line: %u\n", LOGGING_TRESHOLD, size, file, line);
			ConsolePrint(buff);
		}
#endif
		uint64 actualSize = size + sizeof(uint64);
		byte* block = static_cast<byte*>(AB_ALLOC_PROC(actualSize));
		memcpy(block, &size, sizeof(uint64));
		return block + sizeof(uint64);
	}
	AB_API void FreeMemoryDebug(void* block, const char* file, uint32 line) {
		if (block != nullptr) {
			byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
			uint64 size = *reinterpret_cast<uint64*>(actualBlock);
			CurrentUsedMemory -= size;
			CurrentAllocations--;

#if defined(AB_DEBUG_MEMORY)
			if (size > LOGGING_TRESHOLD) {
				//TODO: propper logging
				char buff[256];
				sprintf(buff, "Large deallocation(>%llu bytes): %llu bytes in file: %s, line: %u\n", LOGGING_TRESHOLD, size, file, line);
				ConsolePrint(buff);
			}
#endif

			AB_FREE_PROC(actualBlock);
		}
#if defined(AB_DEBUG_MEMORY)
		if (block == nullptr) {
			//TODO: propper logging
			char buff[128];
			sprintf(buff, "Attempt to delete nullptr in file: %s, line: %u\n", file, line);
			ConsolePrint(buff);
		}
#endif
	}
}

namespace AB {
	AB_API void GetSystemMemoryInfo(SystemMemoryInfo& info) {
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&status);
		
		info.memoryLoad = status.dwMemoryLoad;
		info.totalPhys = status.ullTotalPhys;
		info.availablePhys = status.ullAvailPhys;
		info.totalSwap = status.ullTotalPageFile;
		info.availableSwap = status.ullAvailPageFile;
	}

	void GetAppMemoryInfo(AppMemoryInfo& info) {
		info.currentUsed = CurrentUsedMemory;
		info.currentAllocations = CurrentAllocations;
		info.totalUsed = TotalUsedMemory;
		info.totalAllocations = TotalAllocations;
	}
}