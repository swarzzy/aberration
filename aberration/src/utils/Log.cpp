#include "Log.h"
#include <cstdarg>
#include <cstring>
#include "src/platform/Console.h"
#include "platform/Platform.h"


namespace AB {

	typedef void(OutCharFn)(char** at, uint32* destSize, char ch);

	static void OutChar(char** at, uint32* destSize, char ch) {
		if (*destSize) {
			(*destSize)--;
			**at = ch;
			*at = *at + 1;
		}
	}

	static void OutCharConsole(char** at, uint32* destSize, char ch) {
		if (ch) {
			ConsolePrint((void*)(&ch), 1);
		}
	}

	uint64 StringLength(const char* str) {
		uint64 count = 0;
		while (*str) {
			count++;
			str++;
		}
		return count;
	}

	static int32 StringToInt32(const char* at, char** newAt) {
		int32 result = 0;
		bool32 negative = false;
		if (*at == '-') {
			negative = true;
			at++;
		}
		while ((*at >= '0') && (*at <= '9')) {
			result *= 10;
			result += (*at - '0');
			at++;
		}
		*newAt = (char*)at;
		return negative ? result * -1 : result;
	}

// NOTE: Clang says that all args that is less than 32 bit will be promoted to 32 bit size.
// There is no such warnings on msvc. So just reading all args that are smaller than 32 bits as 32 bit integers

	static uint64 ReadArgUnsignedInteger(uint32 size, va_list* args) {
		uint64 result = 0;
		switch (size) {
		case 8: {
			result = va_arg(*args, uint32);
		} break;
		case 16: {
			result = va_arg(*args, uint32);
		} break;
		case 32: {
			result = va_arg(*args, uint32);
		} break;
		case 64: {
			result = va_arg(*args, uint64);
		} break;
		default: {
			// TODO: Handle it more properly
			AB_DEBUG_BREAK();
		} break;
		}
		return result;
	}

	static int64 ReadArgSignedInteger(uint32 size, va_list* args) {
		int64 result = 0;
		switch (size) {
		case 8: {
			result = va_arg(*args, int32);
		} break;
		case 16: {
			result = va_arg(*args, int32);
		} break;
		case 32: {
			result = va_arg(*args, int32);
		} break;
		case 64: {
			result = va_arg(*args, int64);
		} break;
		default: {
			// TODO: Handle it more properly
			AB_DEBUG_BREAK();
		} break;
		}
		return result;
	}

	static float64 ReadArgFloat(uint32 size, va_list* args) {
		float64 result = 0;
		switch (size) {
		case 32: {
			// NOTE: Reading float32 as float64 instead of taking address 
			// and casting it to a float64* because floating point 
			// varargs actually implicitly cast to double
			result = (va_arg(*args, float64));
		} break;
		case 64: {
			result = va_arg(*args, float64);
		} break;
		default: {
			// TODO: Handle it more properly
			AB_DEBUG_BREAK();
		} break;
		}
		return result;
	}

	static uint32 Uint64ToASCII(char* buffer, uint32 bufferSize, uint64 value, uint32 base, const char* baseTable, OutCharFn* outFunc) {
		uint32 written = 0;
		char* beg = buffer;
		if (value == 0) {
			outFunc(&buffer, &bufferSize, baseTable[0]);
			written++;
		}
		else {
			while (value != 0) {
				uint64 digitIndex = value % base;
				char digit = baseTable[digitIndex];
				outFunc(&buffer, &bufferSize, digit);
				written++;
				value /= base;
			}
		}
		char* end = buffer - 1;
		while (beg < end) {
			char tmp = *beg;
			*beg = *end;
			*end = tmp;
			beg++;
			end--;
		}
		return written;
	}

	static uint32 Float64ToASCII(char* buffer, uint32 bufferSize, float64 value, uint32 precision, uint32 base, const char* baseTable, OutCharFn* outFunc) {
		if (value < 0) {
			outFunc(&buffer, &bufferSize, '-');
			value = -value;
		}
		uint32 written = 0;
		uint64 intPart = (uint64)value;
		float64 fractPart = value - (float64)intPart;
		written += Uint64ToASCII(buffer, bufferSize, intPart, base, baseTable, outFunc);
		buffer += written;
		bufferSize -= written;

		outFunc(&buffer, &bufferSize, '.');
		written++;

		for (uint32 pIndex = 0; pIndex < precision; pIndex++) {
			fractPart *= float64(base);
			uint32 integer = (uint32)fractPart;
			fractPart -= (float32)integer;
			outFunc(&buffer, &bufferSize, baseTable[integer]);
			written++;
		}
		return written;
	}

	struct FormatProperties {
		bool32 forceSign;
		bool32 padWithZeros;
		bool32 leftJustify;
		bool32 hidePositiveSign;
		bool32 zeroAnnotation;
		bool32 widthSpecified;
		uint32 width;
		bool32 precisionSpecified;
		uint32 precision;
	};

	enum class OutputType : uint16 {
		Unknown = 0,
		Uint,
		Int,
		UintHex,
		UintOct,
		Char,
		String,
		Float,
		FloatHex,
		Uintptr,
		PercentSign,
		QueryWrittenChars,
	};

	struct TypeProperties {
		OutputType type;
		uint16 size;
	};

	static FormatProperties ParseFormat(const char** formatAt, va_list* args) {
		FormatProperties fmt = {};
		const char* at = *formatAt;
		//
		// Parsing flags
		//
		bool32 parsing = true;
		while (parsing) {
			switch (*at) {
			case '+': { fmt.forceSign = true; } break;
			case '0': { fmt.padWithZeros = true; } break;
			case '-': { fmt.leftJustify = true;	} break;
			case ' ': { fmt.hidePositiveSign = true; } break;
			case '#': {	fmt.zeroAnnotation = true; } break;
			default: { parsing = false; } break;
			}
			if (parsing)
				at++;
		}
		//
		// Parsing width
		//
		if (*at == '*') {
			fmt.width = va_arg(*args, int);
			fmt.widthSpecified = true;
			at++;
		}
		else if ((*at >= '0') && (*at <= '9')) {
			char* newAt;
			fmt.width = StringToInt32(at, &newAt);
			at = newAt;
			fmt.widthSpecified = true;
		}
		//
		// Parse precision
		//
		if (*at == '.') {
			at++;
			if (*at == '*') {
				fmt.precision = va_arg(*args, int);
				fmt.precisionSpecified = true;
			}
			else if ((*at >= '0') && (*at <= '9')) {
				fmt.precisionSpecified = true;
				char* newAt;
				fmt.precision = StringToInt32(at, &newAt);
				at = newAt;
			}
		}
		*formatAt = at;
		return fmt;
	}

	static TypeProperties ParseType(const char** formatAt) {
		TypeProperties type = {};
		const char* at = *formatAt;

		if (at[0] == 'i' &&
			at[1] == '8')
		{
			type.type = OutputType::Int;
			type.size = 8;
			*formatAt = at + 2;
		}  else
		if (at[0] == 'i' &&
			at[1] == '1' &&
			at[2] == '6')
		{
			type.type = OutputType::Int;
			type.size = 16;
			*formatAt = at + 3;
		} else
		if (at[0] == 'i' &&
			at[1] == '3' &&
			at[2] == '2')
		{
			type.type = OutputType::Int;
			type.size = 32;
			*formatAt = at + 3;
		} else
		if (at[0] == 'i' &&
			at[1] == '6' &&
			at[2] == '4')
		{
			type.type = OutputType::Int;
			type.size = 64;
			*formatAt = at + 3;
		} else
		if (at[0] == 'u' &&
			at[1] == '8')
		{
			if (at[2] == 'h') {
				type.type = OutputType::UintHex;
				*formatAt = at + 3;
			}
			else if (at[2] == 'o') {
				type.type = OutputType::UintOct;
				*formatAt = at + 3;
			}
			else {
				type.type = OutputType::Uint;
				*formatAt = at + 2;
			}
			type.size = 8;
		} else
		if (at[0] == 'u' &&
			at[1] == '1' &&
			at[2] == '6')
		{
			if (at[3] == 'h') {
				type.type = OutputType::UintHex;
				*formatAt = at + 4;
			}
			else if (at[3] == 'o') {
				type.type = OutputType::UintOct;
				*formatAt = at + 4;
			}
			else {
				type.type = OutputType::Uint;
				*formatAt = at + 3;
			}
			type.size = 16;
		} else
		if (at[0] == 'u' &&
			at[1] == '3' &&
			at[2] == '2')
		{
			if (at[3] == 'h') {
				type.type = OutputType::UintHex;
				*formatAt = at + 4;
			}
			else if (at[3] == 'o') {
				type.type = OutputType::UintOct;
				*formatAt = at + 4;
			}
			else {
				type.type = OutputType::Uint;
				*formatAt = at + 3;
			}
						type.size = 32;
		} else
		if (at[0] == 'u' &&
			at[1] == '6' &&
			at[2] == '4')
		{
			if (at[3] == 'h') {
				type.type = OutputType::UintHex;
				*formatAt = at + 4;
			}
			else if (at[3] == 'o') {
				type.type = OutputType::UintOct;
				*formatAt = at + 4;
			}
			else {
				type.type = OutputType::Uint;
				*formatAt = at + 3;
			}
			type.size = 64;
		} else
		if (at[0] == 'c')
		{
			type.type = OutputType::Char;
			type.size = 0;
			*formatAt = at + 1;
		} else
		if (at[0] == 's')
		{
			type.type = OutputType::String;
			type.size = 0;
			*formatAt = at + 1;
		} else
		if (at[0] == 'p')
		{
			type.type = OutputType::Uintptr;
			type.size = 64;
			*formatAt = at + 1;
		} else
		if (at[0] == 'f' &&
			at[1] == '3' &&
			at[2] == '2')
		{
			if (at[3] == 'h') {
				type.type = OutputType::FloatHex;
				*formatAt = at + 4;
			}
			else {
				type.type = OutputType::Float;
				*formatAt = at + 3;
			}
			type.size = 32;
		} else
		if (at[0] == 'f' &&
			at[1] == '6' &&
			at[2] == '4')
		{
			if (at[3] == 'h') {
				type.type = OutputType::FloatHex;
				*formatAt = at + 4;
			}
			else {
				type.type = OutputType::Float;
				*formatAt = at + 3;
			}
			type.size = 64;
		} else
		if (at[0] == '%')
		{
			type.type = OutputType::PercentSign;
			*formatAt = at + 1;
			type.size = 0;
		} else
		if (at[0] == 'n')
		{
			type.type = OutputType::QueryWrittenChars;
			*formatAt = at + 1;
			type.size = 0;
		}
		else {
			type.type = OutputType::Unknown;
			type.size = 0;
			// NOTE: Error: Unknown type
		}
		return type;
	}

	static void PrintInteger(const TypeProperties* type, const FormatProperties* fmt, char** outputBufferAt, uint32* outputBufferFree, OutCharFn* outFunc, va_list* args) {
		const char decimalTable[] = "0123456789";
		const char upperHexTable[] = "0123456789ABCDEF";
		const char lowerHexTable[] = "0123456789abcdef";
		uint32 defaultFloatPrecision = 5;

		const uint32 tempBufferSize = 64;
		char tempBuffer[tempBufferSize];

		char* tempBufferBegin = tempBuffer;
		uint32 tempBufferFree = 64;
		char* tempBufferAt = tempBuffer;

		char prefix[2] = { 0 };

		switch (type->type) {
		case OutputType::Int: {
			int64 val = ReadArgSignedInteger(type->size, args);
			bool32 negative = val < 0;
			if (negative)
				val = -val;
			uint32 written = Uint64ToASCII(tempBufferAt, tempBufferSize, (uint64)val, 10, decimalTable, OutChar);
			tempBufferAt += written;
			if (negative) {
				prefix[0] = '-';
			}
			else if (fmt->forceSign && !fmt->hidePositiveSign) {
				prefix[0] = '+';
			}
			else if (fmt->hidePositiveSign) {
				prefix[0] = ' ';
			}
		} break;
		case OutputType::Uint: {
			uint64 val = ReadArgUnsignedInteger(type->size, args);
			uint32 written = Uint64ToASCII(tempBufferAt, tempBufferSize, (uint64)val, 10, decimalTable, OutChar);
			tempBufferAt += written;
			if (fmt->forceSign && !fmt->hidePositiveSign) {
				prefix[0] = '+';
			}
			else if (fmt->hidePositiveSign) {
				prefix[0] = ' ';
			}
		} break;
		case OutputType::UintOct: {
			uint64 val = ReadArgUnsignedInteger(type->size, args);
			uint32 written = Uint64ToASCII(tempBufferAt, tempBufferSize, (uint64)val, 8, decimalTable, OutChar);
			tempBufferAt += written;
			if (fmt->zeroAnnotation && (val != 0)) {
				// TODO: should it be like below in hex
				//OutChar(&tempBufferAt, &tempBufferFree, '0');
				if (fmt->zeroAnnotation && (val != 0)) {
					prefix[0] = '0';
				}
			}
		} break;
		case OutputType::UintHex: {
			uint64 val = ReadArgUnsignedInteger(type->size, args);
			uint32 written = Uint64ToASCII(tempBufferAt, tempBufferSize, (uint64)val, 16, lowerHexTable, OutChar);
			tempBufferAt += written;
			if (fmt->zeroAnnotation && (val != 0)) {
				prefix[0] = '0';
				prefix[1] = 'x';
			}
		} break;
		case OutputType::Uintptr: {
			void* val = va_arg(*args, void*);
			uint32 written = Uint64ToASCII(tempBufferAt, tempBufferSize, *(uintptr*)&val, 16, lowerHexTable, OutChar);
			tempBufferAt += written;

		} break;
		default: {
		} break;
		}

		uint32 prefixLength = prefix[0] ? 1 : 0 + prefix[1] ? 1 : 0;
		uint32 usePrecision = fmt->precision;
		if (!fmt->precisionSpecified) {
			usePrecision = (uint32)(tempBufferAt - tempBufferBegin);
		}
		bool32 leftJustify = fmt->padWithZeros ? false : fmt->leftJustify;

		uint32 useWidth = fmt->width;
		if (!fmt->widthSpecified) {
			useWidth = usePrecision + prefixLength;
		}

		// Prefix
		if (fmt->padWithZeros) {
			if (prefix[0] && useWidth) {
				outFunc(outputBufferAt, outputBufferFree, prefix[0]);
				useWidth--;
			}
			if (prefix[1]) {
				outFunc(outputBufferAt, outputBufferFree, prefix[1]);
				useWidth--;
			}

			if (!leftJustify) {
				while (useWidth > usePrecision) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}
		}
		else {
			if (!leftJustify) {
				while (useWidth > (usePrecision + prefixLength)) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}

			if (prefix[0] && useWidth) {
				outFunc(outputBufferAt, outputBufferFree, prefix[0]);
				useWidth--;
			}
			if (prefix[1]) {
				outFunc(outputBufferAt, outputBufferFree, prefix[1]);
				useWidth--;
			}
		}

		if (usePrecision > useWidth) {
			usePrecision = useWidth;
		}

		while ((uint32)(tempBufferAt - tempBufferBegin) < usePrecision) {
			outFunc(outputBufferAt, outputBufferFree, '0');
			usePrecision--;
			useWidth--;
		}

		char* tbufPtr = tempBufferBegin;
		while ((tempBufferAt != tbufPtr) && usePrecision) {
			outFunc(outputBufferAt, outputBufferFree, *(tbufPtr));
			tbufPtr++;
			usePrecision--;
			useWidth--;
		}

		if (leftJustify) {
			while (useWidth) {
				outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
				useWidth--;
			}
		}

	}

	static void PrintChars(const TypeProperties* type, const FormatProperties* fmt, char** outputBufferAt, uint32* outputBufferFree, OutCharFn* outFunc, va_list* args) {
		if (type->type == OutputType::String) {
			char nullBuff[7] = "(null)";
			char* str = va_arg(*args, char*);
			char* begin = str;
			char* at = str;
			if (str) {
				uint32 counter = 0;
				at += StringLength(str);
			}
			else {
				begin = nullBuff;
				at = nullBuff + 6;
			}

			uint32 usePrecision = fmt->precision;
			if (!fmt->precisionSpecified) {
				usePrecision = (uint32)(at - begin);
			}

			bool32 leftJustify = fmt->padWithZeros ? false : fmt->leftJustify;

			uint32 useWidth = fmt->width;
			if (!fmt->widthSpecified) {
				useWidth = usePrecision;
			}

			if (!leftJustify) {
				while (useWidth > usePrecision) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}

			if (usePrecision > useWidth) {
				usePrecision = useWidth;
			}

			while ((uint32)(at - begin) < usePrecision) {
				outFunc(outputBufferAt, outputBufferFree, '0');
				usePrecision--;
				useWidth--;
			}

			char* tbufPtr = begin;
			while ((at != tbufPtr) && usePrecision) {
				outFunc(outputBufferAt, outputBufferFree, *(tbufPtr));
				tbufPtr++;
				usePrecision--;
				useWidth--;
			}

			if (leftJustify) {
				while (useWidth) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}
		}
		else if (type->type == OutputType::PercentSign) {
			outFunc(outputBufferAt, outputBufferFree, '%');
		}
		else if (type->type == OutputType::Char) {
			// NOTE: Chars are converted to int in varargs
			char val = va_arg(*args, int);
			outFunc(outputBufferAt, outputBufferFree, val);
		}
		else {
			// TODO: Error unknown type
		}

	}

	static uint32 CountUint64Digits(uint64 value, uint32 base) {
		if (value == 0) {
			return 1;
		}

		uint32 count = 0;
		while (value != 0) {
			value /= base;
			count++;
		}

		return count;
	}

	static void PrintFloat(const TypeProperties* type, const FormatProperties* fmt, char** outputBufferAt, uint32* outputBufferFree, OutCharFn* outFunc, va_list* args) {
		const uint32 tempBufferSize = 64;
		char tempBuffer[tempBufferSize];
		char* tempBufferBegin = tempBuffer;
		uint32 tempFree = 64;
		char* tempAt = tempBuffer;
		char prefix[2] = { 0 };

		uint32 precision = 5;

		switch (type->type) {
		case OutputType::Float: {
			if (fmt->precisionSpecified) {
				precision = fmt->precision;
			}
			// NOTE: increment precision because of the dot '.'
			precision += 1;
			float64 val = ReadArgFloat(type->size, args);
			uint32 written = Float64ToASCII(tempAt, tempBufferSize, (float64)val, precision, 10, "0123456789", OutChar);
			uint32 intDigits = CountUint64Digits((uint64)(val >= 0 ? val : -val), 10);
			precision += intDigits;
			tempAt += written;

		} break;
#if 0 // Not supported for now
		case OutputType::FloatHex: {
			if (fmt->precisionSpecified) {
				precision = fmt->precision;
			}
			// NOTE: increment precision because of the dot '.'
			precision += 1;
			float64 val = ReadArgFloat(type->size, &args);
			uint32 written = Float64ToASCII(tempAt, tempBufferSize, (float64)val, precision, 16, "0123456789abcdef");
			uint32 intDigits = CountUint64Digits((uint64)val, 16);
			precision += intDigits;
			tempAt += written;
		} break;
#endif
		default: {
			// TODO: Error. Unsupported type
		} break;
		}

		uint32 prefixLength = prefix[0] ? 1 : 0 + prefix[1] ? 1 : 0;
		uint32 usePrecision = precision;

		bool32 leftJustify = fmt->padWithZeros ? false : fmt->padWithZeros;

		uint32 useWidth = fmt->width;
		if (!fmt->widthSpecified) {
			useWidth = usePrecision + prefixLength;
		}

		if (fmt->padWithZeros) {

			if (prefix[0] && useWidth) {
				outFunc(outputBufferAt, outputBufferFree, prefix[0]);
				useWidth--;
			}
			if (prefix[1]) {
				outFunc(outputBufferAt, outputBufferFree, prefix[1]);
				useWidth--;
			}

			if (!leftJustify) {
				while (useWidth > usePrecision) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}
		}
		else {
			if (leftJustify) {
				while (useWidth > (usePrecision + prefixLength)) {
					outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
					useWidth--;
				}
			}

			if (prefix[0] && useWidth) {
				outFunc(outputBufferAt, outputBufferFree, prefix[0]);
				useWidth--;
			}
			if (prefix[1]) {
				outFunc(outputBufferAt, outputBufferFree, prefix[1]);
				useWidth--;
			}
		}

		if (usePrecision > useWidth) {
			usePrecision = useWidth;
		}

		while ((uint32)(tempAt - tempBufferBegin) < usePrecision) {
			outFunc(outputBufferAt, outputBufferFree, '0');
			usePrecision--;
			useWidth--;
		}

		char* tbufPtr = tempBufferBegin;
		while ((tempAt != tbufPtr) && usePrecision) {
			outFunc(outputBufferAt, outputBufferFree, *(tbufPtr));
			tbufPtr++;
			usePrecision--;
			useWidth--;
		}

		if (leftJustify) {
			while (useWidth) {
				outFunc(outputBufferAt, outputBufferFree, fmt->padWithZeros ? '0' : ' ');
				useWidth--;
			}
		}
	}

	static int32 FormatStringV(char* buffer, uint32 bufferSize, const char* format, OutCharFn* outFunc, va_list* args) {
		int result = -1;
		uint32 destSize = bufferSize;
		char* destAt = buffer;
		if (destSize) {
			const char* at = format;
			while (at[0] && destSize > 0) {
				if (at[0] == '%') {
					at++;
					FormatProperties fmt = ParseFormat(&at, args);
					TypeProperties type = ParseType(&at);

					if (type.type != OutputType::Unknown) {

						if (type.type == OutputType::Int     ||
							type.type == OutputType::Uint    ||
							type.type == OutputType::UintOct ||
							type.type == OutputType::UintHex ||
							type.type == OutputType::Uintptr)
						{
							PrintInteger(&type, &fmt, &destAt, &destSize, outFunc, args);
						}
						else
						if (type.type == OutputType::Char   ||
							type.type == OutputType::String ||
							type.type == OutputType::PercentSign)
						{
							PrintChars(&type, &fmt, &destAt, &destSize, outFunc, args);
						}
						else
						if (type.type == OutputType::Float ||
							type.type == OutputType::FloatHex)
						{
							PrintFloat(&type, &fmt, &destAt, &destSize, outFunc, args);
						}
						else
						if (type.type == OutputType::QueryWrittenChars)
						{
							uint32* ptr = va_arg(*args, uint32*);
							*ptr = (uint32)(destAt - buffer);
						}
						else
						{
							// TODO: Error Unsupported type
						}
					}

				}
				else {
					outFunc(&destAt, &destSize, at[0]);
					at++;
				}
			}

			result = destSize ? bufferSize - destSize : -1;

			if (destSize) {
				outFunc(&destAt, &destSize, '\0');
			}
			else {
				//*(destAt - 1) = '\0';
				destAt -= 1;
				destSize += 1;
				outFunc(&destAt, &destSize, '\0');
			}
		}

		return result;
	}

	int32 FormatString(char* buffer, uint32 bufferSize, const char* format, ...) {
		va_list args;
		va_start(args, format);
		uint32 result = FormatStringV(buffer, bufferSize, format, OutChar, &args);
		va_end(args);
		return result;
	}

	// NOTE: OutCharConsole outputs characters one by one.
	// This is might be extremely slow.
	// FormatStringV that takes nullptr look like dirty hack 
	// and might be actually unsafe
	// Fast solution is just make two of these functions:
	// first takes buffer, second just prints to console.
	void PrintString(const char* format, ...) {
		va_list args;
		va_start(args, format);
		FormatStringV(nullptr, 1, format, OutCharConsole, &args);
		va_end(args);
	}

	int32 StringToInt32(const char* at) {
		int32 result = 0;
		bool32 negative = false;
		if (*at == '-') {
			negative = true;
			at++;
		}
		while ((*at >= '0') && (*at <= '9')) {
			result *= 10;
			result += (*at - '0');
			at++;
		}
		return negative ? result * -1 : result;
	}

	uint32 ToString(char* buffer, uint32 bufferSize, uint64 value) {
		return Uint64ToASCII(buffer, bufferSize, value, 10, "0123456789", OutChar);
	}

	uint32 ToString(char* buffer, uint32 bufferSize, float64 value, uint32 precision) {
		return Float64ToASCII(buffer, bufferSize, value, precision, 10, "0123456789", OutChar);
	}

	// LOG
	// TODO: Global variable here is bad!
	static LogLevel g_LogLevel = LogLevel::Info;

	void Log(LogLevel level, const char* file, const char* func, uint32 line, const char* fmt, ...) {
		if (level <= g_LogLevel) {
			va_list args;
			va_start(args, fmt);

			switch (level) {
			case LogLevel::Info: {
				DateTime time = {};
				GetLocalTime(time);
				char timeBuffer[DateTime::DATETIME_STRING_SIZE];
				time.ToString(timeBuffer, DateTime::DATETIME_STRING_SIZE);
				ConsoleSetColor(ConsoleColor::DarkGreen, ConsoleColor::Black);
				PrintString("%s: ", timeBuffer);
				FormatStringV(nullptr, 1, fmt, OutCharConsole, &args);
				PrintString("\n\n");
				ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black);
			} break;

			case LogLevel::Warn: {
				DateTime time = {};
				GetLocalTime(time);
				char timeBuffer[DateTime::DATETIME_STRING_SIZE];
				time.ToString(timeBuffer, DateTime::DATETIME_STRING_SIZE);
				ConsoleSetColor(ConsoleColor::DarkYellow, ConsoleColor::Black);
				PrintString("%s: ", timeBuffer);
				FormatStringV(nullptr, 1, fmt, OutCharConsole, &args);
				char fileBuffer[256];
				memcpy(fileBuffer, file, 256);
				CutFilenameFromEnd(fileBuffer);
				PrintString("\n->FILE: %s\n->FUNC: %s\n->LINE: %u32\n\n", fileBuffer, func, line);
				ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black);
			} break;

			case LogLevel::Error: {
				DateTime time = {};
				GetLocalTime(time);
				char timeBuffer[DateTime::DATETIME_STRING_SIZE];
				time.ToString(timeBuffer, DateTime::DATETIME_STRING_SIZE);
				ConsoleSetColor(ConsoleColor::Red, ConsoleColor::Black);
				PrintString("%s: ", timeBuffer);
				FormatStringV(nullptr, 1, fmt, OutCharConsole, &args);
				char fileBuffer[256];
				memcpy(fileBuffer, file, 256);
				CutFilenameFromEnd(fileBuffer);
				PrintString("\n->FILE: %s\n->FUNC: %s\n->LINE: %u32\n\n", fileBuffer, func, line);
				ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black);
			} break;

			case LogLevel::Fatal: {
				DateTime time = {};
				GetLocalTime(time);
				char timeBuffer[DateTime::DATETIME_STRING_SIZE];
				time.ToString(timeBuffer, DateTime::DATETIME_STRING_SIZE);
				ConsoleSetColor(ConsoleColor::DarkRed, ConsoleColor::Black);
				PrintString("%s: ", timeBuffer);
				FormatStringV(nullptr, 1, fmt, OutCharConsole, &args);
				char fileBuffer[256];
				memcpy(fileBuffer, file, 256);
				CutFilenameFromEnd(fileBuffer);
				PrintString("\n->FILE: %s\n->FUNC: %s\n->LINE: %u32\n\n", fileBuffer, func, line);
				ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black);
			} break;

			default: {
			} break;
			}
			va_end(args);
		}
	}

	void LogAssert(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr, const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		DateTime time = {};
		GetLocalTime(time);
		char timeBuffer[DateTime::DATETIME_STRING_SIZE];
		time.ToString(timeBuffer, DateTime::DATETIME_STRING_SIZE);
		ConsoleSetColor(ConsoleColor::DarkRed, ConsoleColor::Black);
		PrintString("%s: ASSERTION FAILED!\n", timeBuffer);
		FormatStringV(nullptr, 1, fmt, OutCharConsole, &args);
		char fileBuffer[256];
		memcpy(fileBuffer, file, 256);
		CutFilenameFromEnd(fileBuffer);
		PrintString("\n->EXPR: %s \n->FILE: %s\n->FUNC: %s\n->LINE: %u32\n\n",assertStr, fileBuffer, func, line);
		ConsoleSetColor(ConsoleColor::DarkWhite, ConsoleColor::Black);
		va_end(args);
	}

	void CutFilenameFromEnd(char* str, char separator) {
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
			for (uint64 i = beg; i < end; i++) {
				str[fill] = str[i];
				fill++;
			}
			str[end - beg] = '\0';
		}
	}
}