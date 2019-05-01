#pragma once
#include "AB.h"
#include <cstring>
#include "Log.h"

#define KILOBYTES(kb) ((kb) * 1024)
#define MEGABYTES(mb) ((mb) * 1024 * 1024)

#define CopyArray(type, elem_count, dest, src) memcpy(dest, src, sizeof(type) * elem_count)
#define CopyScalar(type, dest, src) memcpy(dest, src, sizeof(type))
#define CopyBytes(numBytes, dest, src) memcpy(dest, src, numBytes)
#define SetArray(type, elem_count, dest, val) memset(dest, val, sizeof(type) * elem_count)
#define SetZeroScalar(type, dest) memset(dest, 0, sizeof(type))


namespace AB {

	// Using 16 byte aligment as default as malloc does (according to specs)
	// There are crashes when using 8 byte aligment and optimizations are enabled
	static constexpr uint64 DEFAULT_ALIGMENT = 16;//alignof(std::max_align_t);

	struct MemoryArena
	{
		uptr free;
		uptr offset;
		uptr size;
		void* stackMark;
		void* begin;
		uptr _pad;
	};

	// TODO: Enable all logging!!!!
	inline uptr CalculatePadding(uptr offset, uptr aligment)
	{
		// TODO: Calculate padding properly
		AB_CORE_ASSERT(aligment, "Aligment is zero.");
#if 1
		u64 padding = (aligment - offset % aligment) % aligment;
		return padding;
#else
		uptr mul = (offset / aligment) + 1;
		uptr aligned = mul * aligment;
		uptr padding = aligned - offset;
		return padding;
#endif
	}

	inline void BeginTemporaryMemory(MemoryArena* arena)
	{
		arena->stackMark = (void*)((byte*)arena->begin + arena->offset);	
	}

	inline void EndTemporaryMemory(MemoryArena* arena)
	{
		AB_CORE_ASSERT(arena->stackMark,
					   "Calling EndTemporaryMemory without BeginTemporaryMemory");
		uptr markOffset = (uptr)arena->stackMark - (uptr)arena->begin;
		arena->free += arena->offset - markOffset;
		arena->offset = markOffset;
		arena->stackMark = nullptr;
	}

	inline void* PushSize(MemoryArena* arena, uptr size, uptr aligment)
	{
		uptr nextAdress = 0;
		uptr padding = 0;
		uptr useAligment = 0;
		uptr currentOffset = arena->offset;
		uptr currentAddress = (uptr)arena->begin + currentOffset;
		
		if (aligment == 0)
		{
			useAligment = DEFAULT_ALIGMENT;
		}
		else
		{
			useAligment = aligment;
		}

		if (currentOffset % useAligment != 0)
		{
			padding = CalculatePadding(currentAddress, useAligment);
		}

		if (size + padding <= arena->free)
		{
			// TODO: Why +1 is here?
			arena->offset += size + padding + 1;
			arena->free -= size + padding + 1;
			nextAdress = currentAddress + padding;
			// TODO: Padding!!!!!!
			//AB_CORE_ASSERT(nextAdress % useAligment == 0, "Wrong aligment");
		}

		return (void*)nextAdress;
	}

	inline MemoryArena* AllocateSubArena(MemoryArena* arena, uptr size)
	{
		MemoryArena* result = nullptr;
		uptr chunkSize = size + sizeof(arena);
		void* chunk = PushSize(arena, chunkSize, alignof(MemoryArena));
		if (chunk)
		{
			MemoryArena header = {};
			header.free = size;
			header.offset = 0;
			header.stackMark = nullptr;
			header.size = size;
			header.begin = (void*)((byte*)chunk + sizeof(MemoryArena));
			CopyScalar(MemoryArena, chunk, &header);
			result = (MemoryArena*)chunk;
		}
		return result;
	}
}
