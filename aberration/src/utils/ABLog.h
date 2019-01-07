#pragma once
#include "src/ABHeader.h"
#include "src/utils/String.h"

namespace ab::utils {

	enum class LogLevel {
		Info,
		Warn,
		Error,
		Fatal
	};

	class Log : public Singleton<Log> {
		AB_DISALLOW_COPY_AND_MOVE(Log)
	private:
		friend class Singleton<Log>;

		static const uint32 LOG_BUFFER_SIZE = 15;

		LogLevel m_LogLevel;
		char m_Buffer[LOG_BUFFER_SIZE];
		int32 m_Filled;

	protected:
		Log(LogLevel level);
		~Log() {}

	public:
		template<typename... Args>
		void Message(LogLevel level, Args&&... args);
		void SetLevel(LogLevel level);

	//private:
		template<typename Arg>
		void HandleArgs(Arg&& arg);
		template<typename First, typename... Args>
		void HandleArgs(First&& first, Args&&... args);
	};

	inline Log::Log(LogLevel level)
		: m_LogLevel(level)
		, m_Filled(0)
	{
		
	}

	template <typename ... Args>
	void Log::Message(LogLevel level, Args&&... args) {
		HandleArgs(std::forward<Args>(args)...);
		console_print(m_Buffer, m_Filled);
		m_Filled = 0;
	}
	
	// Expand args with recursion
	template <typename Arg>
	void Log::HandleArgs(Arg&& arg) {
		std::string str = to_string(arg);
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
		std::string str = to_string(first);
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
