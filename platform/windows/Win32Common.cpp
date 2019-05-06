#include "../Common.h"
#include "Log.h"

#include <windows.h>
#include <cstdlib>

namespace AB
{

	static const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = ConsoleColor::DarkWhite;
	static const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = ConsoleColor::Black;

	// NOTE: Might be unsafe store handle as static variable
	static HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	i32 ConsolePrint(const void* data, u32 count)
	{
		DWORD written;
		BOOL result = WriteConsole(consoleHandle, data, count, &written, NULL);
		if (written != count || result == 0)
			return 0;
		return 1;
	}

	i32 ConsolePrint(const char* string)
	{
		DWORD written;
		BOOL result = WriteConsole(consoleHandle, string,
								   (DWORD)strlen(string), &written, NULL);
		if (result == 0)
			return 0;
		return 1;
	}

	i32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor)
	{
		if (textColor == ConsoleColor::Default)
			textColor = CONSOLE_DEFAULT_TEXT_COLOR;
		if (backColor == ConsoleColor::Default)
			backColor = CONSOLE_DEFAULT_BACK_COLOR;

		WORD consoleColor = static_cast<WORD>(textColor) |
			(static_cast<WORD>(backColor) << 4);
		BOOL result = SetConsoleTextAttribute(consoleHandle, consoleColor);
		if (result == 0)
			return 0;
		return 1;
	}

	bool32 GetExecutablePath(char* buffer, u32 bufferSizeBytes, u32* bytesWritten)
	{
		auto written = GetModuleFileName(NULL, buffer, bufferSizeBytes);
		auto result = GetLastError();
		if (result == ERROR_INSUFFICIENT_BUFFER)
			return false;
		else
			return true;
	}

	i64 GetCurrentRawTime()
	{
		static LARGE_INTEGER frequency = {};
		if (!frequency.QuadPart)
		{
			QueryPerformanceFrequency(&frequency);
		}

		i64 time = 0;
		LARGE_INTEGER currentTime = {};
		if (QueryPerformanceCounter(&currentTime))
		{
			AB_CORE_ASSERT(currentTime.QuadPart && frequency.QuadPart,
						   "Failed to get values from windows performance counters.");
			time = (currentTime.QuadPart * 1000000) / frequency.QuadPart;
		}
		return  time;
	}

	void GetLocalTime(DateTime* datetime)
	{
		SYSTEMTIME time;
		::GetLocalTime(&time);

		datetime->year = time.wYear;
		datetime->month = time.wMonth;
		datetime->dayOfWeek = time.wDayOfWeek;
		datetime->day = time.wDay;
		datetime->hour =	time.wHour;
		datetime->minute = time.wMinute;
		datetime->seconds = time.wSecond;
		datetime->milliseconds = time.wMilliseconds;
	}

	u32 DebugReadFileToBuffer(void* buffer, u32 bufferSize, const char* filename)
	{
		u32 written = 0;
		LARGE_INTEGER fileSize = {0};
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ,
									   NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			if (GetFileSizeEx(fileHandle, &fileSize))
			{
				if (fileSize.QuadPart > bufferSize)
				{
					CloseHandle(fileHandle);
				}
				if (buffer)
				{
					DWORD read;
					BOOL result = ReadFile(fileHandle, buffer, (DWORD)fileSize.QuadPart, &read, 0);
					if (!result && !(read == (DWORD)fileSize.QuadPart))
					{
						AB_CORE_ERROR("Failed to read file.");
					}
					else
					{
						written = SafeCastU64U32(fileSize.QuadPart);
					}
				}
			}
			CloseHandle(fileHandle);
		}
		return written;		
	}

	u32 DebugReadTextFileToBuffer(void* buffer, u32 bufferSize,
								  const char* filename)
	{
		u32 bytesRead = 0;
		char* string = nullptr;
		LARGE_INTEGER fileSize = { 0 };
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ,
									   FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			if (GetFileSizeEx(fileHandle, &fileSize))
			{
				if (fileSize.QuadPart + 1 > bufferSize)
				{
					AB_CORE_ERROR("Can`t read >4GB file.");
					CloseHandle(fileHandle);
					return 0;
				}
				if (buffer)
				{
					DWORD read;
					BOOL result = ReadFile(fileHandle, buffer,
										   (DWORD)fileSize.QuadPart, &read, 0);
					if (!result && !(read == (DWORD)fileSize.QuadPart))
					{
						AB_CORE_ERROR("Failed to read file.");
						return 0;
					}
					else
					{
						((char*)buffer)[fileSize.QuadPart] = '\0';
						bytesRead = (u32)fileSize.QuadPart + 1;
					}
				}
			}
			CloseHandle(fileHandle);
		}
		return bytesRead;
	}
	
    void* DebugReadFilePermanent(MemoryArena* memory,
								 const char* filename, u32* bytesRead)
	{
		void* bitmap = nullptr;
		LARGE_INTEGER fileSize = {0};
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ,
									   FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			if (GetFileSizeEx(fileHandle, &fileSize))
			{
				if (fileSize.QuadPart > 0xffffffff)
				{
					AB_CORE_ERROR("Can`t read >4GB file.");
					CloseHandle(fileHandle);
					return nullptr;
				}
				bitmap = PushSize(memory, fileSize.QuadPart, 0);
				if (bitmap)
				{
					DWORD read;
					BOOL readResult = ReadFile(fileHandle, bitmap,
											   (DWORD)fileSize.QuadPart,
											   &read, 0);
					if (!readResult || !(read == (DWORD)fileSize.QuadPart))
					{
						AB_CORE_ERROR("Failed to read file.");
					}
				}
			}
			CloseHandle(fileHandle);
		}
		*bytesRead = (u32)fileSize.QuadPart;
		return  bitmap;
	}

	u32 DebugGetFileSize(const char* filename)
	{
		u32 fileSize = 0;
		HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0,
								   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (handle != INVALID_HANDLE_VALUE)
		{
			DWORD sz = (u32)GetFileSize(handle, 0);
			if (sz != INVALID_FILE_SIZE)
			{
				fileSize = (u32)sz;
			}
			else
			{
				AB_CORE_ERROR("Failed to get file size.");
			}
			CloseHandle(handle);
		}
		else
		{
			AB_CORE_ERROR("Failed to open file.");
		}
		return fileSize;
	}

	b32 DebugWriteFile(const char* filename, void* data, u32 dataSize)
	{
		HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0,
									   CREATE_ALWAYS, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD bytesWritten;
			BOOL writeResult = WriteFile(fileHandle, data,
										 dataSize, &bytesWritten, 0);
			if (writeResult && (dataSize == bytesWritten))
			{
				CloseHandle(fileHandle);
				return true;
			}
		}
		CloseHandle(fileHandle);
		return false;
	}

#if 0
		
	DebugReadFileOffsetRet DebugReadFileOffset(const char* filename, u32 offset, u32 size) {
		void* bitmap = nullptr;
		u32 result_read = 0;
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

	DebugReadTextFileRet DebugReadTextFile(const char* filename) {
		u32 bytesRead = 0;
		char* string = nullptr;
		LARGE_INTEGER fileSize = { 0 };
		HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
		if (fileHandle != INVALID_HANDLE_VALUE) {
			if (GetFileSizeEx(fileHandle, &fileSize)) {
				if (fileSize.QuadPart > 0xffffffff) {
					AB_CORE_ERROR("Can`t read >4GB file.");
					CloseHandle(fileHandle);
					return { nullptr, 0 };
				}
				void* bitmap = malloc(fileSize.QuadPart + 1);
				if (bitmap) {
					DWORD read;
					if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
						AB_CORE_ERROR("Failed to read file.");
						DebugFreeFileMemory(bitmap);
						bitmap = nullptr;
					}
					else {
						string = (char*)bitmap;
						string[fileSize.QuadPart] = '\0';
						bytesRead = (u32)fileSize.QuadPart + 1;
					}
				}
			}
			CloseHandle(fileHandle);
		}
		return  { string , bytesRead };
	}

	// TODO: set bytesRead to zero if reading failed
	void* DebugReadFile(const char* filename, u32* bytesRead) {
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
		*bytesRead = (u32)fileSize.QuadPart;
		return  bitmap;
	}
#endif

}
