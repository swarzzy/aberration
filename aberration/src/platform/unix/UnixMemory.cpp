#include "../Memory.h"
#include "../Console.h"

#include <sys/sysinfo.h>
#include <cstdlib>
#include <cstring>

#define AB_ALLOC_PROC(size) std::malloc(size)
#define AB_FREE_PROC(block) std::free(block)

static uint64 LOGGING_TRESHOLD = 1024; // * 1024

static uint64 CurrentUsedMemory = 0;
static uint64 CurrentAllocations = 0;
static uint64 TotalUsedMemory = 0;
static uint64 TotalAllocations = 0;

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
		byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
		uint64 size = *reinterpret_cast<uint64*>(actualBlock);
		CurrentUsedMemory -= size;
		CurrentAllocations--;

		AB_FREE_PROC(actualBlock);
	}

	AB_API void* AllocateMemoryDebug(uint64 size, const char* file, uint32 line) {
		CurrentUsedMemory += size;
		CurrentAllocations++;
		TotalUsedMemory += size;
		TotalAllocations++;

#if defined(AB_DEBUG_MEMORY)
		if (size > LOGGING_TRESHOLD) {
            //TODO: propper logging
            //TODO: propper logging
            char buff[256];
            sprintf(buff, "Large allocation(>%lu bytes): %lu bytes in file: %s, line: %u\n", LOGGING_TRESHOLD, size,
                    file, line);
            ConsolePrint(buff);
        }
    #endif

		uint64 actualSize = size + sizeof(uint64);
		byte* block = static_cast<byte*>(AB_ALLOC_PROC(actualSize));
		memcpy(block, &size, sizeof(uint64));
		return block + sizeof(uint64);
	}
	AB_API void FreeMemoryDebug(void* block, const char* file, uint32 line) {
		byte* actualBlock = static_cast<byte*>(block) - sizeof(uint64);
		uint64 size = *reinterpret_cast<uint64*>(actualBlock);
		CurrentUsedMemory -= size;
		CurrentAllocations--;

#if defined(AB_DEBUG_MEMORY)
		if (size > LOGGING_TRESHOLD) {
            //TODO: propper logging
            char buff[256];
            sprintf(buff, "Large deallocation(>%lu bytes): %lu bytes in file: %s, line: %u\n", LOGGING_TRESHOLD, size, file, line);
            ConsolePrint(buff);
        }
#endif

		AB_FREE_PROC(actualBlock);
	}
}

namespace AB {
	AB_API void GetSystemMemoryInfo(SystemMemoryInfo& info) {
		struct sysinfo sysInfo = {};
		sysinfo(&sysInfo);
		info.totalPhys = sysInfo.totalram;
		info.totalSwap = sysInfo.totalswap;
		info.availablePhys = sysInfo.freeram;
		info.availableSwap = sysInfo.freeswap;
		info.memoryLoad = info.availablePhys * 100 / info.totalPhys;
	}

	AB_API void GetAppMemoryInfo(AppMemoryInfo& info) {
		info.currentUsed = CurrentUsedMemory;
		info.currentAllocations = CurrentAllocations;
		info.totalUsed = TotalUsedMemory;
		info.totalAllocations = TotalAllocations;
	}
}