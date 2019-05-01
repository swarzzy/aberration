#pragma once
#include "AB.h"
#include "Memory.h"

namespace AB
{
	constexpr uint64 SYS_STORAGE_TOTAL_SIZE = MEGABYTES(6);

	struct _SysAllocatorData {
		void* begin;
		uint64 offset;
		uint64 free;
		uint64 _pad;
	};

	struct SystemStorage {
		_SysAllocatorData _internal;
		byte _raw[SYS_STORAGE_TOTAL_SIZE];
	};

	struct Memory {
		//PermanentStorage perm_storage;
		SystemStorage sys_storage;
	};

	MemoryArena* AllocateArena(uptr size);

	
	AB_API Memory* CreateMemoryContext();
	//AB_API Memory* GetMemory();
	AB_API void* SysStorageAlloc(uint64 size, uint64 aligment = 0);
	AB_API void* SysStorageAllocDebug(uint64 size, const char* file, const char* func, uint32 line, uint64 aligment = 0);

	//AB_API const PermanentStorage* PermStorage();

#if defined(AB_CONFIG_DEBUG)
#define SysAlloc(size) SysStorageAllocDebug(size, __FILE__, __func__, __LINE__)
#define SysAllocAligned(size, aligment) SysStorageAllocDebug(size, __FILE__, __func__, __LINE__, aligment)
#else
#define SysAlloc(size) SysStorageAlloc(size)
#define SysAllocAligned(size, aligment) SysStorageAlloc(size, aligment)
#endif
}
