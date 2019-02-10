#pragma once
#include "src/ABHeader.h"

#if defined(AB_PLATFORM_WINDOWS)
#define AB_DEBUG_BREAK() __debugbreak()
#elif defined(AB_PLATFORM_LINUX)
#define AB_DEBUG_BREAK() __builtin_debugtrap()
#endif

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

#if 0

namespace AB {
	struct DateTime;
}

namespace AB::Log {

	

	void Initialize(LogLevel level);
	void Message(LogLevel level, const char* file, const char* func, uint32 line, const char* format, ...);
	void Message(LogLevel level, const char* format, ...);
	void Print(const char* format, ...);
}

namespace AB::utils {

	enum class LogLevel : uint32 {
		Fatal = 0,
		Error,
		Warn,
		Info,
	};

	class Log {
		AB_DISALLOW_COPY_AND_MOVE(Log)
	private:
		static const uint32 LOG_BUFFER_SIZE = 1024;

		inline static Log* s_Instance = nullptr;

		LogLevel m_LogLevel;
		char m_Buffer[LOG_BUFFER_SIZE];
		uint64 m_Filled;

		inline ~Log() {}

	public:
		inline Log(LogLevel level);
		inline static void Initialize(LogLevel level);
		inline static void Destroy();
		inline static Log* Instance();

		template<typename... Args>
		void Message(LogLevel level, Args&&... args);
		inline void SetLevel(LogLevel level);

	private:
		template<typename Arg>
		void HandleArgs(Arg&& arg);
		template<typename First, typename... Args>
		void HandleArgs(First&& first, Args&&... args);
	};

	template<typename... Args>
	void LogMessage(LogLevel level, const char* file, const char* func, int32 line, Args&&... args) {
		switch(level) {
		case LogLevel::Info : {Log::Instance()->Message(level, std::forward<Args>(args)...); } break;
		case LogLevel::Warn: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									CutFilenameFromEnd(fl);
								}
								else
									fl[0] = '\0';
								Log::Instance()->Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							 } break;
		case LogLevel::Error: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									CutFilenameFromEnd(fl);
								}
								else
									fl[0] = '\0';
								Log::Instance()->Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							  } break;
		case LogLevel::Fatal: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									CutFilenameFromEnd(fl);
								}
								else
								fl[0] = '\0';
								Log::Instance()->Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							  } break;
	}
	}



	inline Log::Log(LogLevel level) 
		: m_LogLevel(level)
		, m_Filled(0)
	{}

	inline void Log::Initialize(LogLevel level) {
		if (s_Instance) {
			AB_CORE_WARN("Log system already initialized.");
		} else {
			s_Instance = ab_create Log(level);
		}
	}

	inline void Log::Destroy() {
		if (s_Instance) {
			ab_delete_scalar s_Instance;
			s_Instance = nullptr;
		}
	}

	inline Log* Log::Instance() {
		if (s_Instance) {
			return s_Instance;
		} else {
			ConsolePrint("ERROR: Log system is not initialized.");
			return nullptr;
		}
	}

	inline void Log::SetLevel(LogLevel level) {
		m_LogLevel = level;
	}

	template <typename ... Args>
	void Log::Message(LogLevel level, Args&&... args) {
		if (level <= m_LogLevel) {
			//AB::DateTime time = {};
			//GetLocalTime(time);
			//memcpy(m_Buffer + m_Filled, time.ToString().c_str(), AB::DateTime::DATETIME_STRING_SIZE - 1);
			//m_Buffer[AB::DateTime::DATETIME_STRING_SIZE - 1] = ':';
		//	m_Buffer[AB::DateTime::DATETIME_STRING_SIZE] = ' ';
			//m_Filled += DateTime::DATETIME_STRING_SIZE + 1;

			HandleArgs(std::forward<Args>(args)...);
			
			switch (level) {
			case LogLevel::Info: { ConsoleSetColor(ConsoleColor::DarkGreen, ConsoleColor::Black); } break;
			case LogLevel::Warn: { ConsoleSetColor(ConsoleColor::DarkYellow, ConsoleColor::Black); } break;
			case LogLevel::Error: { ConsoleSetColor(ConsoleColor::Red, ConsoleColor::Black); } break;
			case LogLevel::Fatal: { ConsoleSetColor(ConsoleColor::DarkRed, ConsoleColor::Black); } break;
			default: { ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black); }
			}
			ConsolePrint(m_Buffer);
			//console_print(m_Buffer, static_cast<uint32>(m_Filled));
			ConsoleSetColor(ConsoleColor::Default, ConsoleColor::Default);
			m_Filled = 0;
		}
	}
	
	// Expand args with recursion
	template <typename Arg>
	void Log::HandleArgs(Arg&& arg) {
		String str = ToString(arg);
		// TODO: get rid of string
		if (m_Filled < LOG_BUFFER_SIZE) {
			if (str.size() < LOG_BUFFER_SIZE - (m_Filled + 2)) {
				memcpy(m_Buffer + m_Filled, str.c_str(), sizeof(char) * str.size());
				m_Filled += str.size() + 2;
				memset(m_Buffer + (m_Filled - 2), '\n', sizeof(char));
				memset(m_Buffer + (m_Filled - 1), '\0', sizeof(char));
			}
			else {
				memcpy(m_Buffer + m_Filled, str.c_str(), LOG_BUFFER_SIZE - (m_Filled + 2));
				m_Filled += LOG_BUFFER_SIZE - m_Filled;
				memset(m_Buffer + (m_Filled - 2), '\n', sizeof(char));
				memset(m_Buffer + (m_Filled - 1), '\0', sizeof(char));
			}
		}
	}
	
	template <typename First, typename... Args>
	void Log::HandleArgs(First&& first, Args&&... args) {
		String str = ToString(first);
		if (m_Filled < LOG_BUFFER_SIZE) {
			if (str.size() < LOG_BUFFER_SIZE - (m_Filled + 2)) {
				memcpy(m_Buffer + m_Filled, str.c_str(), sizeof(char) * str.size());
				m_Filled += str.size();
				HandleArgs(std::forward<Args>(args)...);
			}
			else {
				memcpy(m_Buffer + m_Filled, str.c_str(), LOG_BUFFER_SIZE - (m_Filled + 2));
				m_Filled += LOG_BUFFER_SIZE - m_Filled;
				memset(m_Buffer + (m_Filled - 2), '\n', sizeof(char));
				memset(m_Buffer + (m_Filled - 1), '\0', sizeof(char));
			}
		}
	}
}
#endif