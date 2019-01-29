#pragma once
#include "src/Types.h"
#include "src/Common.h"

#include <memory>

#if !defined(_MSC_VER)
#include <cstring>
#endif

#if defined (AB_DEBUG_MEMORY)
#define ab_create			new
#define ab_delete_scalar	delete
#define ab_delete_array		delete[]
#else
#define ab_create			new
#define ab_delete_scalar	delete		// delete scalar
#define ab_delete_array		delete[]	// delete array
#endif

namespace AB::internal {
	AB_API void* AllocateMemory(uint64 size);
	AB_API void FreeMemory(void* block);


	AB_API void* AllocateMemoryDebug(uint64 size, const char* file, uint32 line);
	AB_API void FreeMemoryDebug(void* block, const char* file, uint32 line);

	// Dummy struct for new and delete operator overloading
	struct ABCustomAllocator_t {};
	inline constexpr ABCustomAllocator_t ABCustomAllocator = ABCustomAllocator_t();
}


namespace AB {

	template<typename T>
	class Allocator {
	public:
		using value_type = T;
		using size_type = uint64;

		Allocator() = default;
		~Allocator() = default;
		Allocator(const Allocator&) {}
		template<typename A>
		Allocator(const Allocator<A>&) {}

		T* allocate(size_type count) {
#if defined(AB_DEBUG_MEMORY)
			//void* block = internal::AllocateMemoryDebug(count * sizeof(T), "class AB::Allocator<T>", 1);
			void* block = std::malloc(count * sizeof(T));
#else
			//void* block = internal::AllocateMemory(count * sizeof(T));
			void* block = std::malloc(count * sizeof(T));
#endif
			return static_cast<T*>(block);
		}

		void deallocate(T* ptr, size_type n) {
#if defined(AB_DEBUG_MEMORY)
			//internal::FreeMemoryDebug(ptr, "class AB::Allocator<T>", 1);
			std::free(ptr);
#else
			//internal::FreeMemory(ptr);
			std::free(ptr);
#endif
		}
	};

	template<typename T1, typename T2>
	inline bool operator==(const Allocator<T1>& left, const Allocator<T2>& right) { return true; }

	template<typename T1, typename T2>
	inline bool operator!=(const Allocator<T1>& left, const Allocator<T2>& right) { return false; }

	struct AB_API SystemMemoryInfo {
		uint32 memoryLoad;
		uint64 totalPhys;
		uint64 availablePhys;
		uint64 totalSwap;
		uint64 availableSwap;
	};

	struct AB_API AppMemoryInfo {
		uint64 currentUsed;
		uint64 currentAllocations;
		uint64 totalUsed;
		uint64 totalAllocations;
	};

	AB_API void GetSystemMemoryInfo(SystemMemoryInfo& info);
	AB_API void GetAppMemoryInfo(AppMemoryInfo& info);
}

// warning C4595: 'operator new': non-member operator new or delete functions may not be declared inline
#if defined(_MSC_VER)
#pragma warning(disable : 4595)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-new-delete"
#endif

//#define AB_MEMORY_OVERRIDE_NEW_DELETE
#if defined (AB_MEMORY_OVERRIDE_NEW_DELETE) //&& defined(AB_BUILD_DLL)

inline void* operator new(uint64 size) {
	return AB::internal::AllocateMemory(size);
}

inline void* operator new(uint64 size, const char* file, uint32 line) {
	return AB::internal::AllocateMemoryDebug(size, file, line);
}

inline void* operator new[](uint64 size) {
	return AB::internal::AllocateMemory(size);
}

inline void* operator new[](uint64 size, const char* file, uint32 line) {
	return AB::internal::AllocateMemoryDebug(size, file, line);
}

inline void operator delete(void* block) noexcept {
	AB::internal::FreeMemory(block);
}

inline void operator delete(void* block, const char* file, uint32 line) {
	AB::internal::FreeMemoryDebug(block, file, line);
}

inline void operator delete[](void* block) noexcept {
	AB::internal::FreeMemory(block);
}

inline void operator delete[](void* block, const char* file, uint32 line) {
	AB::internal::FreeMemoryDebug(block, file, line);
}
#endif

// warning C4595: 'operator new': non-member operator new or delete functions may not be declared inline
#if defined(_MSC_VER)
#pragma warning(default : 4595)
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif