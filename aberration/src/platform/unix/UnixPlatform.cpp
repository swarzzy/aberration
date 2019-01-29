#include "../Platform.h"
#include "src/utils/Log.h"
#include <ctime>
#include <fcntl.h>
#include <unistd.h>

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
						AB_CORE_WARN("File reading error. File: ", filename, ". Failed read data from file");
					}
				}
				else {
					AB_CORE_WARN("File reading error. File: ", filename, ". Memory allocation failed");
				}
			}
			else {
				AB_CORE_WARN("File reading error. File: ", filename);
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: ", filename, ". Failed to open file");
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
				AB_CORE_WARN("File reading error. File: ", filename, ". Write operation failed");
			}
		}
		else {
			AB_CORE_WARN("File reading error. File: ", filename, ". Failed to open file");
			return false;
		}
		close(fileHandle);
		return result;
	}
}