#pragma once
#include "src/ABHeader.h"
//#include "src/platform/Platform.h"
#include "src/platform/Console.h"

// TODO:
// -- Time stamps
// -- Logging to file

namespace AB {
	struct DateTime;
}

namespace AB::Log {

	enum class LogLevel : uint32 {
		Fatal = 0,
		Error,
		Warn,
		Info,
	};

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

#if !defined(AB_CONFIG_DISTRIB)

#define _AB_CORE_INFO(...)	do{}while(false)
#define _AB_CORE_WARN(...)	do{}while(false)
#define _AB_CORE_ERROR(...)	do{}while(false)
#define _AB_CORE_FATAL(...)	do{}while(false)
#define _AB_CORE_ASSERT(expr, ...)	do{}while(false)

#else

#define _AB_CORE_INFO(...) do { AB::utils::LogMessage(AB::utils::LogLevel::Info, __FILE__, "", 0, __VA_ARGS__); } while(false)
#define _AB_CORE_WARN(...) do { AB::utils::LogMessage(AB::utils::LogLevel::Warn, __FILE__, __func__, __LINE__, __VA_ARGS__); } while(false)
#define _AB_CORE_ERROR(...) do { AB::utils::LogMessage(AB::utils::LogLevel::Error, __FILE__, __func__, __LINE__, __VA_ARGS__); } while(false)
#define _AB_CORE_FATAL(...) do { AB::utils::LogMessage(AB::utils::LogLevel::Fatal, __FILE__, __func__, __LINE__, __VA_ARGS__); AB_DEBUG_BREAK();} while(false)
#define _AB_CORE_ASSERT(expr, ...) do { if (!(expr)) {AB::utils::LogMessage(AB::utils::LogLevel::Fatal, __FILE__, __func__, __LINE__, \
										"ASSERTION FAILED: ", #expr, "\n", __VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#endif

#define AB_CORE_INFO(...) _AB_CORE_INFO(__VA_ARGS__) 
#define AB_CORE_WARN(...) _AB_CORE_WARN(__VA_ARGS__)
#define AB_CORE_ERROR(...) _AB_CORE_ERROR(__VA_ARGS__)
#define AB_CORE_FATAL(...) _AB_CORE_FATAL(__VA_ARGS__)
#define AB_CORE_ASSERT(expr, ...) _AB_CORE_ASSERT(expr, __VA_ARGS__)

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


