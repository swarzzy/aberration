#pragma once
#include "src/ABHeader.h"

namespace AB {

	// Aberration string class
	using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;

	// Based on https://habr.com/post/205772/
	// AB::String ToString()
	template<typename T> struct HasToString {
	private:
		template<typename U> 
		static decltype(std::declval<U>().ToString()) Detect(const U&); // Will be called if type has AB::String ToString()
		static void Detect(...); // Will be called for all other types
	public:
		// Checking type of Detect() that was called
		static constexpr bool value = std::is_same<String, decltype(Detect(std::declval<T>()))>::value;
	};

	inline void CutFilenameFromEnd(char* str, char separator = '\\') {
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
	inline String ToString(T arg) {
		if constexpr (std::is_fundamental<T>::value)
			// TODO: sprintf
			return String(std::to_string(arg));
		else if constexpr (HasToString<T>::value)
			return arg.ToString();
		else
			return "<ERROR::STRING: UNKNOWN TYPE>";
	}

	inline String ToString(const String& string) {
		return String(string);
	}

	//inline String ToString(const std::string& string) {
	//	return String(string);
	//}
	inline String ToString(std::string_view string) {
		return String(string);
	}
	inline String ToString(char* str) {
		return String(str);
	}
	inline String ToString(char ch) {
		// TODO: get rid of std::string (sprintf)
		return String(std::to_string(ch));
	}
	inline String ToString(const char* str) {
		return String(str);
	}
}