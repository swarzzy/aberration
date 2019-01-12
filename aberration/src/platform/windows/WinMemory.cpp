#include "../Memory.h"
#include <Windows.h>
#include <cstdlib>

#define AB_ALLOC_PROC(size) malloc(size)
#define AB_FREE_PROC(block) free(block)

static uint64 LOGGING_TRESHOLD = 1024; // * 1024

static uint64 CurrentUsedMemory		= 0;
static uint64 CurrentAllocations	= 0;
static uint64 TotalUsedMemory		= 0;
static uint64 TotalAllocations		= 0;

namespace ab::internal {
	AB_API void* allocate_memory(uint64 size) {
		CurrentUsedMemory += size;
		CurrentAllocations++;
		TotalUsedMemory += size;
		TotalAllocations++;
		
		uint64 actualSize = size + sizeof(uint64);
		byte* block = static_cast<byte*>(AB_ALLOC_PROC(actualSize));
		memcpy(block, &size, sizeof(uint64));
		return block + sizeof(uint64);
	}

	AB_API void free_memory(void* block) {
		byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
		uint64 size = *reinterpret_cast<uint64*>(actualBlock);
		CurrentUsedMemory -= size;
		CurrentAllocations--;

		AB_FREE_PROC(actualBlock);
	}

	AB_API void* allocate_memory_debug(uint64 size, const char* file, uint32 line) {
		CurrentUsedMemory += size;
		CurrentAllocations++;
		TotalUsedMemory += size;
		TotalAllocations++;

#if defined(AB_DEBUG_MEMORY)
		if (size > LOGGING_TRESHOLD)
			//TODO: propper logging
			printf("Large allocation(>%lld bytes): %lld bytes in file: %s, line: %d\n", LOGGING_TRESHOLD ,size, file, line);
#endif

		uint64 actualSize = size + sizeof(uint64);
		byte* block = static_cast<byte*>(AB_ALLOC_PROC(actualSize));
		memcpy(block, &size, sizeof(uint64));
		return block + sizeof(uint64);
	}
	AB_API void free_memory_debug(void* block, const char* file, uint32 line) {
		byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
		uint64 size = *reinterpret_cast<uint64*>(actualBlock);
		CurrentUsedMemory -= size;
		CurrentAllocations--;

#if defined(AB_DEBUG_MEMORY)
		if (size > LOGGING_TRESHOLD)
			//TODO: propper logging
			printf("Large deallocation(>%lld bytes): %lld bytes in file: %s, line: %d\n", LOGGING_TRESHOLD, size, file, line);
#endif

		AB_FREE_PROC(actualBlock);
	}
}

namespace ab {
	AB_API void get_system_memory_info(SystemMemoryInfo& info) {
		MEMORYSTATUSEX status;
		status.dwLength = sizeof(MEMORYSTATUSEX);
		GlobalMemoryStatusEx(&status);
		
		info.memoryLoad = status.dwMemoryLoad;
		info.totalPhys = status.ullTotalPhys;
		info.availablePhys = status.ullAvailPhys;
		info.totalSwap = status.ullTotalPageFile;
		info.availableSwap = status.ullAvailPageFile;
		info.totalVirtual = status.ullTotalVirtual;
		info.availableVirtual = status.ullAvailVirtual;
	}

	void get_app_memory_info(AppMemoryInfo& info) {
		info.currentUsed = CurrentUsedMemory;
		info.currentAllocations = CurrentAllocations;
		info.totalUsed = TotalUsedMemory;
		info.totalAllocations = TotalAllocations;
	}
}