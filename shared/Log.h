#pragma once
#include "Shared.h"

namespace AB
{
	struct DateTime;

	enum class LogLevel : uint32 {
		Fatal = 0,
		Error,
		Warn,
		Info,
	};

	u32 DateTimeToString(DateTime* dt, char* buffer, u32 bufferSize);

	// If buffer is big enough returns number of characters written (without null terminator),
	// otherwise returns -1
	AB_API int32 FormatString(char* buffer, uint32 bufferSize, const char* format, ...);
	AB_API void PrintString(const char* format, ...);
	AB_API uint32 ToString(char* buffer, uint32 bufferSize, uint64 value);
	AB_API uint32 ToString(char* buffer, uint32 bufferSize, float64 value, uint32 precision);

	AB_API void Log(LogLevel level, const char* file, const char* func, uint32 line, const char* fmt, ...);
	AB_API void LogAssert(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr);
	AB_API void LogAssert(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr, const char* fmt, ...);
	AB_API void CutFilenameFromEnd(char* str, char separator = '\\');
	// Returns required size if buffer is to small
	struct GetFilenameFromPathRet {
		bool32 succeeded;
		uint64 written;
	};
	
	AB_API GetFilenameFromPathRet GetFilenameFromPath(const char* path, char* buffer, uint32 buffer_size);
	
	struct GetDirectoryRet {
		bool32 succeeded;
		uint64 written;
	};
#if 0	
	AB_API GetDirectoryRet GetDirectory(const char* file_path, char* buffer, uint32 buffer_size);
#endif
}

#if defined(AB_CONFIG_DISTRIB)

#define AB_CORE_INFO(format, ...)	do{}while(false)
#define AB_CORE_WARN(format, ...)	do{}while(false)
#define AB_CORE_ERROR(format, ...)	do{}while(false)
#define AB_CORE_FATAL(format, ...)	do{}while(false)
#define AB_CORE_ASSERT(format, expr, ...)	do{}while(false)

#else

#if defined (__clang__)
#define AB_CORE_INFO(format, ...) AB::Log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_CORE_WARN(format, ...) AB::Log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_CORE_ERROR(format, ...) AB::Log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_CORE_FATAL(format, ...) do { AB::Log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();} while(false)
#define AB_CORE_ASSERT(expr, ...) do { if (!(expr)) {AB::Log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, #expr, ##__VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#else
#define AB_CORE_INFO(format, ...) AB::Log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_WARN(format, ...) AB::Log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_ERROR(format, ...) AB::Log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_FATAL(format, ...) do { AB::Log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, __VA_ARGS__); AB_DEBUG_BREAK();} while(false)
#define AB_CORE_ASSERT(expr, ...) do { if (!(expr)) {AB::LogAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, #expr, __VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#endif
#endif

#define INVALID_DEFAULT_CASE() AB_CORE_FATAL("Invalid default case.")
#define INVALID_CODE_PATH() AB_CORE_FATAL("Invalid code path.")

#define AB_ASSERT(expr, ...) AB_CORE_ASSERT(expr, ##__VA_ARGS__)

//#define AB_CORE_INFO(format, ...) _AB_CORE_INFO(format, __VA_ARGS__)
//#define AB_CORE_WARN(format, ...) _AB_CORE_WARN(format, __VA_ARGS__)
//#define AB_CORE_ERROR(format, ...) _AB_CORE_ERROR(format, __VA_ARGS__)
//#define AB_CORE_FATAL(format, ...) _AB_CORE_FATAL(format, __VA_ARGS__)
//#define AB_CORE_ASSERT(format, expr, ...) _AB_CORE_ASSERT(format, expr, __VA_ARGS__)
