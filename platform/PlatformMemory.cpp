#include "PlatformMemory.h"
#include "Log.h"

#include <cstdlib>
#include <cstring>
#include <cstddef>


namespace AB
{
	MemoryArena* AllocateArena(uptr size)
	{
		uptr headerSize = sizeof(MemoryArena);
		// TODO: Clang (it works on clang actually)
		void* mem = _aligned_malloc(size + headerSize, 128);
		AB_CORE_ASSERT(mem, "Allocation failed");
		memset(mem, 0, size + headerSize);
		MemoryArena header = {};
		header.free = size;
		header.offset = 0;
		header.begin = (void*)((byte*)mem + headerSize);
		header.stackMark = nullptr;
		header.size = size;
		CopyScalar(MemoryArena, mem, &header);
		return (MemoryArena*)mem;
	}
}
