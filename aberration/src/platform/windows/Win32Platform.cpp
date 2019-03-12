#include <windows.h>
#include "utils/Log.h"
#include <cstdlib>

namespace AB {

	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten) {
		auto written = GetModuleFileName(NULL, buffer, bufferSizeBytes);
		auto result = GetLastError();
		if (result == ERROR_INSUFFICIENT_BUFFER)
			return false;
		else
			return true;
	}

	int64 GetCurrentRawTime() {
		static LARGE_INTEGER frequency = {};
		if (!frequency.QuadPart) {
			QueryPerformanceFrequency(&frequency);
		}

		int64 time = 0;
		LARGE_INTEGER currentTime = {};
		if (QueryPerformanceCounter(&currentTime)) {
			AB_CORE_ASSERT(currentTime.QuadPart && frequency.QuadPart, "Failed to get values from windows performance counters.");
			time = (currentTime.QuadPart * 1000000) / frequency.QuadPart;
		}
		return  time;
	}

	void GetLocalTime(DateTime& datetime) {
		SYSTEMTIME time;
		GetLocalTime(&time);

		datetime.year = time.wYear;
		datetime.month = time.wMonth;
		datetime.dayOfWeek = time.wDayOfWeek;
		datetime.day = time.wDay;
		datetime.hour =	time.wHour;
		datetime.minute = time.wMinute;
		datetime.seconds = time.wSecond;
		datetime.milliseconds = time.wMilliseconds;
	}


	// TODO: set bytesRead to zero if reading failed
	void* DebugReadFile(const char* filename, uint32* bytesRead) {
		void* bitmap = nullptr;
		LARGE_INTEGER fileSize = {0};
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			if (GetFileSizeEx(fileHandle, &fileSize)) {
				if (fileSize.QuadPart > 0xffffffff) {
					AB_CORE_ERROR("Can`t read >4GB file.");
					CloseHandle(fileHandle);
					return nullptr;
				}
				bitmap = std::malloc(fileSize.QuadPart);
				if (bitmap) {
					DWORD read;
					if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
						AB_CORE_ERROR("Failed to read file.");
						DebugFreeFileMemory(bitmap);
						bitmap = nullptr;
					}
				}
			}
			CloseHandle(fileHandle);
		}
		*bytesRead = (uint32)fileSize.QuadPart;
		return  bitmap;
	}

	DebugReadFileOffsetRet DebugReadFileOffset(const char* filename, uint32 offset, uint32 size) {
		void* bitmap = nullptr;
		uint32 result_read = 0;
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER fileSize = { 0 };
			if (GetFileSizeEx(fileHandle, &fileSize)) {
				if (fileSize.QuadPart <= 0xffffffff) {
					if (offset + size <= fileSize.QuadPart) {
						void* mem = std::malloc(size);
						if (mem) {
							if (SetFilePointer(fileHandle, offset, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER) {
								DWORD read;
								if (ReadFile(fileHandle, mem, (DWORD)size, &read, 0) && (read == (DWORD)size)) {
									result_read = size;
									bitmap = mem;
								} else {
									AB_CORE_ERROR("Failed to read file.");
									DebugFreeFileMemory(mem);
								}
							} else {
								AB_CORE_ERROR("Failed to set file pointer.");
								DebugFreeFileMemory(mem);
							}
						} else {
							AB_CORE_ERROR("Failed to allocate memory.");
						}
					} else {
						AB_CORE_ERROR("Offset is bigger than file size.");
					}
				} else {
					AB_CORE_ERROR("Can`t read >4GB file.");
				}
			}
			CloseHandle(fileHandle);
		}
		return  {bitmap, result_read };
	}

	void DebugFreeFileMemory(void* memory) {
		if (memory) {
			std::free(memory);
		}
	}

	bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
		HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			DWORD bytesWritten;
			if (WriteFile(fileHandle, data, dataSize, &bytesWritten, 0) && (dataSize == bytesWritten)) {
				CloseHandle(fileHandle);
				return true;
			}
		}
		CloseHandle(fileHandle);
		return false;
	}
}