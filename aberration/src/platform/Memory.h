#pragma once
#include "src/ABHeader.h"
#include <memory>

#if defined (AB_DEBUG_MEMORY)
#define AB_NEW		new(__FILE__, __LINE__)
#define AB_DELS		delete
#define AB_DELA		delete[]
#else
#define AB_NEW		new
#define AB_DELS		delete		// delete scalar
#define AB_DELA		delete[]	// delete array
#endif

namespace AB::internal {
	AB_API void* allocate_memory(uint64 size);
	AB_API void free_memory(void* block);

	AB_API void* allocate_memory_debug(uint64 size, const char* file, uint32 line);
	AB_API void free_memory_debug(void* block, const char* file, uint32 line);
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
			void* block = internal::allocate_memory_debug(count * sizeof(T), "class AB::Allocator<T>", 1);
#else
			void* block = internal::allocate_memory(count * sizeof(T));
#endif
			return static_cast<T*>(block);
		}

		void deallocate(T* ptr, size_type n) {
#if defined(AB_DEBUG_MEMORY)
			internal::free_memory_debug(ptr, "class AB::Allocator<T>", 1);
#else
			internal::free_memory(ptr);
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

	AB_API void get_system_memory_info(SystemMemoryInfo& info);
	AB_API void get_app_memory_info(AppMemoryInfo& info);
}

// warning C4595: 'operator new': non-member operator new or delete functions may not be declared inline
#if defined(_MSC_VER)
#pragma warning(disable : 4595)
#endif
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winline-new-delete"
#endif

#define AB_MEMORY_OVERRIDE_NEW_DELETE
#if defined (AB_MEMORY_OVERRIDE_NEW_DELETE)
inline void* operator new(uint64 size) {
	return AB::internal::allocate_memory(size);
}
inline void* operator new(uint64 size, const char* file, uint32 line) {
	return AB::internal::allocate_memory_debug(size, file ,line);
}
inline void* operator new[](uint64 size) {
	return AB::internal::allocate_memory(size);
}
inline void* operator new[](uint64 size, const char* file, uint32 line) {
	return AB::internal::allocate_memory_debug(size, file, line);
}
inline void operator delete(void* block) noexcept {
	AB::internal::free_memory(block);
	//free(block);
}
inline void operator delete(void* block, const char* file, uint32 line) noexcept {
	AB::internal::free_memory_debug(block, file, line);
}
inline void operator delete[](void* block) noexcept {
	AB::internal::free_memory(block);
}
inline void operator delete[](void* block, const char* file, uint32 line) noexcept {
	AB::internal::free_memory_debug(block, file, line);
}
#endif

// warning C4595: 'operator new': non-member operator new or delete functions may not be declared inline
#if defined(_MSC_VER)
#pragma warning(default : 4595)
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif