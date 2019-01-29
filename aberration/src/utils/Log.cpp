//#include "Log.h"
//
//bool32 SearchSubStr(const char* str, const char* substr) {
//	bool32 flag = false;
//	uint32 index = 0;
//	const char* strPtr = str;
//	const char* substrPtr = substr;
//
//	if (*substrPtr != '\0') {
//		while (*strPtr != '\0') {
//			if (*strPtr == *substrPtr) {
//				flag = true;
//				if (*(substrPtr + 1) == '\0')
//					return true;
//				else
//					substrPtr++;
//				strPtr++;
//			}
//			else {
//				if (!flag)
//					strPtr++;
//				substrPtr = substr;
//				flag = false;
//			}
//		}
//	}
//	if (*substr != '\0')
//		return false;
//	return flag;
//}
//
//namespace AB::Log {
//	static LogLevel g_LogLevel = LogLevel::Info;
//
//	void Message(LogLevel level, const char* file, const char* func, uint32 line, const char* format, ...) {
//		
//	}
//
//	void Message(LogLevel level, const char* format, ...) {
//		
//	}
//
//	void Print(const char* format, ...) {
//		uint32 index = 0;
//		uint32 openedBraces = 0;
//		const char* formatter
//		const char* argBeg = nullptr;
//		const char* argEnd = nullptr;
//		const char* ch = format;
//		char buf[16];
//		while (*ch != '\0') {
//			if (*ch == '{') {
//				ch++;
//				argBeg = ch;
//				while (*ch != '\0') {
//					if (*ch == '}')
//						argEnd = ch - 1;
//				}
//				if (argEnd == nullptr) {
//					// error braces matching
//				} else {
//					switch (*argBeg) {
//					case 'i': {
//						if (argBeg + 3 <= argEnd) {
//							if (*(argBeg + 1) == 'n' && *(argBeg + 2) == 't' && *(argBeg + 2) == '8') {
//
//							}
//						}
//					} break;
//					case 'u': //unsigned
//					case 'f': // floatts
//					case 'b': // byte
//					}
//				}
//			}
//			ch++;
//		}
//		//vprintf()
//	}
//}