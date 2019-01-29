#include "../Platform.h"
#include <windows.h>
#include "utils/Log.h"

namespace AB {

	String DateTime::ToString() {
		if (hour < 24 && minute < 60 && seconds < 60) {
			char buff[DATETIME_STRING_SIZE];
			sprintf(buff, "%02d:%02d:%02d", hour, minute, seconds);
			return String(buff);
		}
		return ("00:00:00");
	}

	AB_API void GetLocalTime(DateTime& datetime) {
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
