#pragma once
#include "src/ABHeader.h"

namespace ab {

	// Based on https://habr.com/post/205772/
	// std::string ToString()
	template<typename T> struct HasToString {
	private:
		template<typename U> 
		static decltype(std::declval<U>().ToString()) Detect(const U&); // Will be called if type has std::string ToString()
		static void Detect(...); // Will be called for all other types
	public:
		// Checking type of Detect() that was called
		static constexpr bool value = std::is_same<std::string, decltype(Detect(std::declval<T>()))>::value;
	};

	inline void cut_filename_from_end(char* str, char separator = '\\') {
		uint64 beg = 0;
		uint64 end = strlen(str);
		bool found = false;

		for (uint64 i = end; i > 0; i--) {
			if (str[i] == separator) {
				beg = i + 1;
				found = true;
				break;
			}
		}

		if (found) {
			uint32 fill = 0;
			for(uint64 i = beg; i < end; i++) {
				str[fill] = str[i];
				fill++;
			}
			str[end - beg] = '\0';
		}
	}

	template<typename T>
	inline std::string to_string(T arg) {
		if constexpr (std::is_fundamental<T>::value)
			// TODO: sprintf
			return std::to_string(arg);
		else if constexpr (HasToString<T>::value)
			return arg.ToString();
		else
			return "<ERROR::STRING: UNKNOWN TYPE>";
	}

	inline std::string to_string(const std::string& string) {
		return std::string(string);
	}
	inline std::string to_string(std::string_view string) {
		return std::string(string);
	}
	inline std::string to_string(char* str) {
		return std::string(str);
	}
	inline std::string to_string(char ch) {
		return std::to_string(ch);
	}
	inline std::string to_string(const char* str) {
		return std::string(str);
	}
}