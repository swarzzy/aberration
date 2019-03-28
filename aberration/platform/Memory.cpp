#include "Memory.h"
#include "utils/Log.h"

#include <cstdlib>
#include <cstring>
#include <cstddef>

namespace AB {

	// Using 16 byte aligment as default as malloc does (according to specs)
	// There are crashes when using 8 byte aligment and optimizations are enabled
	static constexpr uint64 DEFAULT_ALIGMENT = 16;//alignof(std::max_align_t);

	static Memory* g_MemoryContext = nullptr;

	static uint64 CalculatePadding(uintptr offset, uint64 aligment) {
		AB_CORE_ASSERT(aligment, "Aligment is zero.");
		uint64 padding = (aligment - offset % aligment) % aligment;
		return padding;
	}

	Memory* CreateMemoryContext() {
		if (!g_MemoryContext) {
			g_MemoryContext = (Memory*)malloc(sizeof(Memory));
			AB_CORE_ASSERT(g_MemoryContext, "Failed to allocate game memory.");
			memset(g_MemoryContext, 0, sizeof(Memory));

			uint64 sys_chunk_pad = CalculatePadding(g_MemoryContext->sys_storage._raw[0], DEFAULT_ALIGMENT);
			AB_CORE_ASSERT((g_MemoryContext->sys_storage._raw[0] + sys_chunk_pad) % DEFAULT_ALIGMENT == 0, "Aligment error");
			g_MemoryContext->sys_storage._internal.begin = g_MemoryContext->sys_storage._raw + sys_chunk_pad;
			g_MemoryContext->sys_storage._internal.free = SYS_STORAGE_TOTAL_SIZE - sys_chunk_pad - sizeof(_SysAllocatorData);
		}
		return g_MemoryContext;
	}

	Memory* GetMemory() {
		return g_MemoryContext;
	}

	void* SysStorageAlloc(uint64 size, uint64 aligment) {
		uint64 padding = 0;
		uint64 use_aligment = 0;
		uint64 current_offset = g_MemoryContext->sys_storage._internal.offset;
		uintptr current_address = (uintptr)g_MemoryContext->sys_storage._internal.begin + current_offset;
		if (aligment == 0) {
			use_aligment = DEFAULT_ALIGMENT;
		} else {
			use_aligment = aligment;
		}

		if (g_MemoryContext->sys_storage._internal.offset % use_aligment != 0) {
			padding = CalculatePadding(current_address, use_aligment);
		}

		AB_CORE_ASSERT(size + padding < g_MemoryContext->sys_storage._internal.free, "Not enough system memory.");

		g_MemoryContext->sys_storage._internal.offset += size + padding + 1;
		g_MemoryContext->sys_storage._internal.free -= size + padding + 1;
		uintptr next_address = current_address + padding;

		AB_CORE_ASSERT(next_address % use_aligment == 0, "Wrong aligment");
		//PrintString("Allocation: size: %u64, at address %u64, free: %u64, at: %u64, before: %u64, padding: %u64\n", size, next_address, g_MemoryContext->sys_storage._internal.free, next_address, current_address, next_address - current_address);

		return (void*)next_address;
	}

	void* SysStorageAllocDebug(uint64 size, const char* file, const char* func, uint32 line, uint64 aligment) {
		//AB_CORE_INFO("Allocation in system storage: size: %u64; file: %s; func: %s; line: %u32", size, file, func, line);
		return SysStorageAlloc(size, aligment);
	}

	const PermanentStorage* PermStorage() {
		return &g_MemoryContext->perm_storage;
	}
}
