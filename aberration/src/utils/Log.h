#pragma once
#include "src/ABHeader.h"
#include "src/utils/String.h"
#include "src/platform/Console.h"
#include "src/platform/Platform.h"

// TODO:
// -- Time stamps
// -- Logging to file

namespace ab::utils {

	enum class LogLevel : uint32 {
		Fatal = 0,
		Error,
		Warn,
		Info,
	};

	class Log {
		AB_DISALLOW_COPY_AND_MOVE(Log)
	private:
		friend class Singleton<Log>;

		static const uint32 LOG_BUFFER_SIZE = 1024;

		inline static LogLevel m_LogLevel = LogLevel::Info;
		inline static char m_Buffer[LOG_BUFFER_SIZE];
		inline static uint64 m_Filled = 0;

		Log(LogLevel level);
		~Log() {}

	public:
		template<typename... Args>
		static void Message(LogLevel level, Args&&... args);
		inline static void SetLevel(LogLevel level);

	private:
		template<typename Arg>
		static void HandleArgs(Arg&& arg);
		template<typename First, typename... Args>
		static void HandleArgs(First&& first, Args&&... args);
	};

	template<typename... Args>
	void log_stamp(LogLevel level, const char* file, const char* func, int32 line, Args&&... args) {
		switch(level) {
		case LogLevel::Info : {Log::Message(level, std::forward<Args>(args)...); } break;
		case LogLevel::Warn: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									cut_filename_from_end(fl);
								}
								else
									fl[0] = '\0';
								Log::Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							 } break;
		case LogLevel::Error: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									cut_filename_from_end(fl);
								}
								else
									fl[0] = '\0';
								Log::Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							  } break;
		case LogLevel::Fatal: {	char fl[256];
								if (strlen(file) < 255) {
									memcpy(fl, file, strlen(file) + 1);
									cut_filename_from_end(fl);
								}
								else
								fl[0] = '\0';
								Log::Message(level, std::forward<Args>(args)..., "\n->  FILE: ", fl, "\n->  FUNC: ", func, "\n->  LINE: ", line);
							  } break;
	}
	}
#define AB_CORE_INFO(...) do { ab::utils::log_stamp(ab::utils::LogLevel::Info, __FILE__, "", 0, __VA_ARGS__); } while(false)
#define AB_CORE_WARN(...) do { ab::utils::log_stamp(ab::utils::LogLevel::Warn, __FILE__, __func__, __LINE__, __VA_ARGS__); } while(false)
#define AB_CORE_ERROR(...) do { ab::utils::log_stamp(ab::utils::LogLevel::Error, __FILE__, __func__, __LINE__, __VA_ARGS__); } while(false)
#define AB_CORE_FATAL(...) do { ab::utils::log_stamp(ab::utils::LogLevel::Fatal, __FILE__, __func__, __LINE__, __VA_ARGS__); __debugbreak();} while(false)

	inline void Log::SetLevel(LogLevel level) {
		m_LogLevel = level;
	}

	template <typename ... Args>
	void Log::Message(LogLevel level, Args&&... args) {
		if (level <= m_LogLevel) {
			DateTime time = {};
			get_local_time(time);
			memcpy(m_Buffer + m_Filled, time.ToString().c_str(), DateTime::DATETIME_STRING_SIZE - 1);
			m_Buffer[DateTime::DATETIME_STRING_SIZE - 1] = ':';
			m_Buffer[DateTime::DATETIME_STRING_SIZE] = ' ';
			m_Filled += DateTime::DATETIME_STRING_SIZE + 1;

			HandleArgs(std::forward<Args>(args)...);
			
			switch (level) {
			case LogLevel::Info: { console_set_color(ConsoleColor::DarkGreen, ConsoleColor::Black); } break;
			case LogLevel::Warn: { console_set_color(ConsoleColor::DarkYellow, ConsoleColor::Black); } break;
			case LogLevel::Error: { console_set_color(ConsoleColor::Red, ConsoleColor::Black); } break;
			case LogLevel::Fatal: { console_set_color(ConsoleColor::DarkRed, ConsoleColor::Black); } break;
			default: { console_set_color(ConsoleColor::DarkWhite, ConsoleColor::Black); }
			}
			
			console_print(m_Buffer, static_cast<uint32>(m_Filled));
			console_set_color(ConsoleColor::Default, ConsoleColor::Default);
			m_Filled = 0;
		}
	}
	
	// Expand args with recursion
	template <typename Arg>
	void Log::HandleArgs(Arg&& arg) {
		String str = to_string(arg);
		// TODO: get rid of string
		if (m_Filled < LOG_BUFFER_SIZE) {
			if (str.size() < LOG_BUFFER_SIZE - (m_Filled + 1)) {
				memcpy(m_Buffer + m_Filled, str.c_str(), sizeof(char) * str.size());
				m_Filled += str.size() + 1;
				memset(m_Buffer + (m_Filled - 1), '\n', sizeof(char));
			}
			else {
				memcpy(m_Buffer + m_Filled, str.c_str(), LOG_BUFFER_SIZE - (m_Filled + 1));
				m_Filled += LOG_BUFFER_SIZE - m_Filled;
				memset(m_Buffer + (m_Filled - 1), '\n', sizeof(char));
			}
		}
	}
	
	template <typename First, typename... Args>
	void Log::HandleArgs(First&& first, Args&&... args) {
		String str = to_string(first);
		if (m_Filled < LOG_BUFFER_SIZE) {
			if (str.size() < LOG_BUFFER_SIZE - m_Filled) {
				memcpy(m_Buffer + m_Filled, str.c_str(), sizeof(char) * str.size());
				m_Filled += str.size();
				HandleArgs(std::forward<Args>(args)...);
			}
			else {
				memcpy(m_Buffer + m_Filled, str.c_str(), LOG_BUFFER_SIZE - (m_Filled + 1));
				m_Filled += LOG_BUFFER_SIZE - m_Filled;
				memset(m_Buffer + (m_Filled - 1), '\n', sizeof(char));
			}
		}
	}
}
