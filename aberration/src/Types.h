#pragma once

// Aberration basic sized types
// Integers
#if defined(_MSC_VER)
typedef __int8	int8;  // char (byte)
typedef __int16	int16; // short
typedef __int32	int32; // int
typedef __int64	int64; // long long

typedef unsigned __int8		uint8;	// unsigned char
typedef unsigned __int16	uint16; // unsigned short
typedef unsigned __int32	uint32; // unsigned int
typedef unsigned __int64	uint64; // unsigned long long (size_t)

typedef __int8			byte;
typedef unsigned __int8	ubyte;

#else
#include <cstdint>
typedef int8_t	int8;	// char (byte)
typedef int16_t	int16;	// short
typedef int32_t	int32;	// int
typedef int64_t	int64;	// long long

typedef uint8_t		uint8;	// unsigned char
typedef uint16_t	uint16;	// unsigned short
typedef uint32_t	uint32;	// unsigned int
typedef uint64_t	uint64;	// unsigned long long (size_t)

typedef int8_t		byte;
typedef uint8_t		ubyte;

#endif

// Floating point
typedef float	float32; // float
typedef double	float64; // double