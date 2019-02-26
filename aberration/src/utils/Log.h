#pragma once
#include "ABHeader.h"

namespace AB {

	enum class LogLevel : uint32 {
		Fatal = 0,
		Error,
		Warn,
		Info,
	};

	// If buffer is big enough returns number of characters written (without null terminator),
	// otherwise returns -1
	int32 FormatString(char* buffer, uint32 bufferSize, const char* format, ...);
	void PrintString(const char* format, ...);
	uint32 ToString(char* buffer, uint32 bufferSize, uint64 value);
	uint32 ToString(char* buffer, uint32 bufferSize, float64 value, uint32 precision);

	void Log(LogLevel level, const char* file, const char* func, uint32 line, const char* fmt, ...);
	void LogAssert(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr, const char* fmt, ...);
	void CutFilenameFromEnd(char* str, char separator = '\\');
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
// TODO: print assertions
#define AB_CORE_ASSERT(expr, format, ...) do { if (!(expr)) {AB::Log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#else
#define AB_CORE_INFO(format, ...) AB::Log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_WARN(format, ...) AB::Log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_ERROR(format, ...) AB::Log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_CORE_FATAL(format, ...) do { AB::Log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, __VA_ARGS__); AB_DEBUG_BREAK();} while(false)
// TODO: print assertions
#define AB_CORE_ASSERT(expr, format, ...) do { if (!(expr)) {AB::LogAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, #expr ,format, __VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#endif
#endif

//#define AB_CORE_INFO(format, ...) _AB_CORE_INFO(format, __VA_ARGS__)
//#define AB_CORE_WARN(format, ...) _AB_CORE_WARN(format, __VA_ARGS__)
//#define AB_CORE_ERROR(format, ...) _AB_CORE_ERROR(format, __VA_ARGS__)
//#define AB_CORE_FATAL(format, ...) _AB_CORE_FATAL(format, __VA_ARGS__)
//#define AB_CORE_ASSERT(format, expr, ...) _AB_CORE_ASSERT(format, expr, __VA_ARGS__)