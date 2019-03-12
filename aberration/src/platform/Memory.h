#pragma once
#include "AB.h"
#include <cstring>

#define KILOBYTES(kb) ((kb) * 1024)
#define MEGABYTES(mb) ((mb) * 1024 * 1024)

#define CopyArray(type, elem_count, dest, src) memcpy(dest, src, sizeof(type) * elem_count)
#define SetArray(type, elem_count, dest, val) memset(dest, val, sizeof(type) * elem_count)


namespace AB {
	struct Renderer3D;
	struct WindowProperties;
	struct Renderer2DProperties;
	struct DebugOverlayProperties;
	struct InputMgr;
	struct Application;
	struct AssetManager;
}

namespace AB {

	constexpr uint64 SYS_STORAGE_TOTAL_SIZE = MEGABYTES(6);

	struct PermanentStorage {
		Renderer3D* forward_renderer;
		WindowProperties* window;
		Renderer2DProperties* renderer2d;
		DebugOverlayProperties* debug_overlay;
		InputMgr* input_manager;
		Application* application;
		AssetManager* asset_manager;
	};

	struct _SysAllocatorData {
		void* begin;
		uint64 offset;
		uint64 free;
	};

	struct SystemStorage {
		_SysAllocatorData _internal;
		byte _raw[SYS_STORAGE_TOTAL_SIZE];
	};

	struct Memory {
		PermanentStorage perm_storage;
		SystemStorage sys_storage;
	};

	AB_API Memory* CreateMemoryContext();
	Memory* GetMemory();
	AB_API void* SysStorageAlloc(uint64 size, uint64 aligment = 0);
	AB_API void* SysStorageAllocDebug(uint64 size, const char* file, const char* func, uint32 line, uint64 aligment = 0);

	AB_API const PermanentStorage* PermStorage();

#if defined(AB_CONFIG_DEBUG)
#define SysAlloc(size) SysStorageAllocDebug(size, __FILE__, __func__, __LINE__)
#else
#define SysAlloc(size) SysStorageAlloc(size)
#endif
}