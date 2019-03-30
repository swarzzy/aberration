#include "../../aberration/AB.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

inline uint32 SafeTruncateU64U32(uint64 val) {
	assert(val <= 0xffffffff);
	return (uint32)val;
}

struct GetDirectoryRet { bool32 succeeded;
	uint64 written;
};

GetDirectoryRet GetDirectory(const char* file_path, char* buffer, uint32 buffer_size) {
#if defined(AB_PLATFORM_WINDOWS)
	char sep_1 = '\\';
	char sep_2 = '/';
#elif defined(AB_PLATFORM_LINUX)
	char sep_1 = '/';
	char sep_2 = '/';

#else
#error Unsupported OS
#endif
	GetDirectoryRet result = {};
	uint64 last_sep_index = 0;
	char used_separator = '/';

	uint64 i = 0;
	char at = file_path[i];
	while (at != '\0') {
		if (at == sep_1 || at == sep_2) {
			last_sep_index = i;
			used_separator = at;
		}
		i++;
		at = file_path[i];
	}

	uint64 dir_num_chars = last_sep_index + 2;

	if (buffer_size >= dir_num_chars) {
		memcpy(buffer, file_path, dir_num_chars - 2);
		buffer[dir_num_chars - 2] = used_separator;
		buffer[dir_num_chars - 1] = '\0';
		result.succeeded = true;
	}
	else {
		result.succeeded = false;
	}
	result.written = dir_num_chars;
	return result;
}


#if defined(AB_PLATFORM_WINDOWS)
#include <windows.h>

void FreeFileMemory(void* memory) {
	if (memory) {
		free(memory);
	}
}

struct ReadFileRet {
	void* data;
	uint64 size;
};

ReadFileRet ReadEntireFile(const char* filename) {
	uint32 bytesRead = 0;
	void* bitmap = nullptr;
	LARGE_INTEGER fileSize = { 0 };
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			if (fileSize.QuadPart > 0xffffffff) {
				printf("Can`t read >4GB file.");
				CloseHandle(fileHandle);
				return { nullptr, 0 };
			}
			bitmap = malloc(fileSize.QuadPart);
			if (bitmap) {
				DWORD read;
				if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
					printf("Failed to read file.");
					FreeFileMemory(bitmap);
					bitmap = nullptr;
				}
				else {
					bytesRead = (uint32)fileSize.QuadPart;
				}
			}
		}
		CloseHandle(fileHandle);
	}
	return  { bitmap, bytesRead };
}

struct ReadFileAsTextRet {
	char* data;
	uint64 size;
};

ReadFileAsTextRet ReadEntireFileAsText(const char* filename) {
	uint32 bytesRead = 0;
	char* string = nullptr;
	LARGE_INTEGER fileSize = { 0 };
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			if (fileSize.QuadPart > 0xffffffff) {
				printf("Can`t read >4GB file.");
				CloseHandle(fileHandle);
				return { nullptr, 0 };
			}
			void* bitmap = malloc(fileSize.QuadPart + 1);
			if (bitmap) {
				DWORD read;
				if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
					printf("Failed to read file.");
					FreeFileMemory(bitmap);
					bitmap = nullptr;
				}
				else {
					string = (char*)bitmap;
					string[fileSize.QuadPart] = '\0';
					bytesRead = (uint32)fileSize.QuadPart + 1;
				}
			}
		}
		CloseHandle(fileHandle);
	}
	return  { string , bytesRead };
}

bool32 WriteFile(const char* filename, void* data, uint32 dataSize) {
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

#elif defined(AB_PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void FreeFileMemory(void* memory) {
	if (memory) {
		std::free(memory);
	}
}

struct ReadFileRet {
	void* data;
	uint64 size;
};

ReadFileRet ReadEntireFile(const char* filename) {
	uint32 bytesRead = 0;
	void* ptr = nullptr;
	bytesRead = 0;
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
					bytesRead = (uint32)result;
				}
				else {
					std::free(data);
					printf("File reading error. File: %s. Failed read data from file", filename);
				}
			}
			else {
				printf("File reading error. File: %s. Memory allocation failed", filename);
			}
		}
		else {
			printf("File reading error. File: %s", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file.", filename);
		return { nullptr, 0 };
	}
	close(fileHandle);
	return { ptr, bytesRead };
}

bool32 WriteFile(const char* filename, void* data, uint32 dataSize) {
	bool32 result = false;
	int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
	if (fileHandle) {
		ssize_t written = write(fileHandle, data, dataSize);
		if (written == dataSize) {
			result = true;
		}
		else {
			printf("File reading error. File: %s. Write operation failed.", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file", filename);
		return false;
	}
	close(fileHandle);
	return result;
}

struct ReadFileAsTextRet {
	char* data;
	uint64 size;
};

ReadFileAsTextRet ReadEntireFileAsText(const char* filename) {
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
					printf("File reading error. File: %s. Failed read data from file", filename);
				}
			}
			else {
				printf("File reading error. File: %s. Memory allocation failed", filename);
			}
		}
		else {
			printf("File reading error. File: %s", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file.", filename);
		return {};
	}
	close(fileHandle);
	return {ptr, bytesRead};
}

#else
#error Unsupported platform.
#endif
