#include "../Common.h"
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dlfcn.h>

namespace AB {

	static const ConsoleColor CONSOLE_DEFAULT_TEXT_COLOR = ConsoleColor::DarkWhite;
	static const ConsoleColor CONSOLE_DEFAULT_BACK_COLOR = ConsoleColor::Black;

	AB_API int32 ConsolePrint(const void* data, uint32 count) {
		ssize_t result = write(1, data, count);
		if (result != count)
			return 0;
		return 1;
	}

    AB_API int32 ConsolePrint(const char* string) {
	    // TODO: Better console output
        auto result = printf("%s", string);
        if (result >= 0)
            return 1;
        else
            return 0;
	}

	AB_API int32 ConsoleSetColor(ConsoleColor textColor, ConsoleColor backColor) {
		return 0;
	}
	
	bool32 GetExecutablePath(char* buffer, uint32 bufferSizeBytes, uint32* bytesWritten) {
		ssize_t len = readlink("/proc/self/exe", buffer, bufferSizeBytes - 1);
		if (len == bufferSizeBytes -1) {
			*bytesWritten = len + 1;
			return false;
		}
		if (len != -1) {
			buffer[len] = '\0';
			*bytesWritten = len + 1;
			return true;
		} else {
			*bytesWritten = 0;
			return  false;
		}
	}

	int64 GetCurrentRawTime() {
		int64 time = 0;
		struct timespec tp;
		if (clock_gettime(CLOCK_MONOTONIC_RAW, &tp) == 0) {
			time = ((int64)tp.tv_sec * 1000000) + ((int64)tp.tv_nsec / 1000);
		}
		return time;
	}

	AB_API void GetLocalTime(DateTime& datetime) {
		std::time_t t = std::time(nullptr);
		auto tm = std::localtime(&t);

		datetime.year = static_cast<uint16>(tm->tm_year);
		datetime.month = static_cast<uint16>(tm->tm_mon);
		datetime.dayOfWeek = static_cast<uint16>(tm->tm_wday);
		datetime.day = static_cast<uint16>(tm->tm_mday);
		datetime.hour = static_cast<uint16>(tm->tm_hour);
		datetime.minute = static_cast<uint16>(tm->tm_min);
		datetime.seconds = static_cast<uint16>(tm->tm_sec);
		datetime.milliseconds = 0;
	}

	AB_API void* DebugReadFile(const char* filename, uint32* bytesRead) {
		void* ptr = nullptr;
		*bytesRead = 0;
		int fileHandle = open(filename, O_RDONLY);
		if (fileHandle) {
			off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
			if (fileEnd) {
				lseek(fileHandle, 0, SEEK_SET);
				void* data = std::malloc(fileEnd);
				if (data) {
					ssize_t result = read(fileHandle, data, fileEnd);
					if (result == fileEnd) {
						ptr = data;
						*bytesRead = (uint32)result;
					}
					else {
						std::free(data);
						AB_CORE_WARN("File reading error. File: %s. Failed read data from file", filename);
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: %s. Memory allocation failed", filename);
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: %s", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file.", filename);
			return ptr;
		}
		close(fileHandle);
		return ptr;
	}

	AB_API void DebugFreeFileMemory(void* memory) {
		if (memory) {
			std::free(memory);
		}
	}

	AB_API bool32 DebugWriteFile(const char* filename, void* data, uint32 dataSize) {
		bool32 result = false;
		int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
		if (fileHandle) {
			ssize_t written = write(fileHandle, data, dataSize);
			if (written == dataSize) {
				result = true;
			}
			else {
				AB_CORE_WARN("File reading error. File: %s. Write operation failed.", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file", filename);
			return false;
		}
		close(fileHandle);
		return result;
	}

	DebugReadFileOffsetRet DebugReadFileOffset(const char* filename, uint32 offset, uint32 size) {
		void* ptr = nullptr;
		uint32 bytesRead = 0;
		int fileHandle = open(filename, O_RDONLY);
		if (fileHandle) {
			off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
			if (fileEnd >= (off_t)(offset + size)) {
				lseek(fileHandle, offset, SEEK_SET);
				void* data = std::malloc(size);
				if (data) {
					ssize_t result = read(fileHandle, data, size);
					if (result == size) {
						ptr = data;
						bytesRead = (uint32)result;
					}
					else {
						free(data);
						AB_CORE_WARN("File reading error. File: %s. Failed read data from file", filename);
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: %s. Memory allocation failed", filename);
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: %s", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file.", filename);
			return {};
		}
		close(fileHandle);
		return {ptr, bytesRead};
	}

	DebugReadTextFileRet DebugReadTextFile(const char* filename) {
		char* ptr = nullptr;
		uint32 bytesRead = 0;
		int fileHandle = open(filename, O_RDONLY);
		if (fileHandle) {
			off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
			if (fileEnd) {
				lseek(fileHandle, 0, SEEK_SET);
				void* data = std::malloc(fileEnd + 1);
				if (data) {
					ssize_t result = read(fileHandle, data, fileEnd);
					if (result == fileEnd) {
						ptr = (char*)data;
						ptr[fileEnd + 1] = '\0';
						// NOTE: read can read less than fileEnd bytes
						bytesRead = (uint32)(fileEnd + 1);
					}
					else {
						std::free(data);
						AB_CORE_WARN("File reading error. File: %s. Failed read data from file", filename);
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: %s. Memory allocation failed", filename);
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: %s", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: %s. Failed to open file.", filename);
			return {};
		}
		close(fileHandle);
		return {ptr, bytesRead};
	}


}
