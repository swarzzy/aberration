#pragma once

#include <xmmintrin.h>
#include <cmath>

#if defined(_MSC_VER)
typedef __int8				int8;  // char (byte)
typedef __int16				int16; // short
typedef __int32				int32; // int
typedef __int64				int64; // long long

typedef unsigned __int8		uint8;	// unsigned char
typedef unsigned __int16	uint16; // unsigned short
typedef unsigned __int32	uint32; // unsigned int
typedef unsigned __int64	uint64; // unsigned long long (size_t)

typedef uint32				bool32;
typedef unsigned char		uchar;

#undef byte // Defined as unsigned char in rpcndr.h
typedef unsigned __int8		byte;

#else
#include <cstdint>
typedef int8_t				int8;	// char (byte)
typedef int16_t				int16;	// short
typedef int32_t				int32;	// int
typedef int64_t				int64;	// long long

typedef uint8_t				uint8;	// unsigned char
typedef uint16_t			uint16;	// unsigned short
typedef uint32_t			uint32;	// unsigned int
typedef uint64_t			uint64;	// unsigned long long (size_t)

typedef uint32				bool32;
typedef unsigned char		uchar;

#undef byte // Defined as unsigned char in rpcndr.h
typedef uint8_t				byte;

#endif
// Floating point
typedef float				float32;
typedef double				float64;

// SIMD registers
typedef __m128				float128;


#define HPM_CALL //__vectorcall
#define HPM_INLINE inline

namespace hpm {

	HPM_INLINE constexpr float32 Pi() {
		return 3.14159265358979323846f;
	}
	
	HPM_INLINE float32 Sin(float32 radians) {
		return std::sin(radians);
	}

	HPM_INLINE float32 Cos(float32 radians) {
		return std::cos(radians);
	}

	HPM_INLINE constexpr float32 ToDegrees(float32 radians) {
		return 180.0f / Pi() * radians;
	}

	HPM_INLINE constexpr float32 ToRadians(float32 degrees) {
		return Pi() / 180.0f * degrees;
	}

	union Vector2 {
		struct {
			float32 x;
			float32 y;
		};
		struct {
			float32 r;
			float32 g;
		};
		float32 data[2];

		Vector2(float32 x_, float32 y_)
			: x(x_)
			, y(y_)
		{}

		Vector2() {}
	};

	union Vector3 {
		struct {
			float32 x;
			float32 y;
			float32 z;
		};
		struct {
			float32 r;
			float32 g;
			float32 b;
		};
		float32 data[3];

		Vector3(float32 x_, float32 y_, float32 z_)
			: x(x_)
			, y(y_)
			, z(z_)
		{}

		Vector3() {}
	};

	union Vector4 {
		struct {
			float32 x;
			float32 y;
			float32 z;
			float32 w;
		};
		struct {
			float32 r;
			float32 g;
			float32 b;
			float32 a;
		};
		float32 data[4];
		float128 _packed;

		Vector4(float32 x_, float32 y_, float32 z_, float32 w_)
			: x(x_)
			, y(y_)
			, z(z_)
			, w(w_)
		{}

		Vector4() {}
	};

	struct Rectangle {
		Vector2 min;
		Vector2 max;
	};

	union Matrix4 {
		Vector4 columns[4];
		float32 data[16];
	};

	union Matrix3 {
		Vector3 columns[3];
		float32 data[9];
	};

	HPM_INLINE Vector2 HPM_CALL Add(Vector2 left, Vector2 right) {
		return Vector2{ left.x + right.x, left.y + right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Subtract(Vector2 left, Vector2 right) {
		return Vector2{ left.x - right.x, left.y - right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Multiply(Vector2 left, Vector2 right) {
		return Vector2{ left.x * right.x, left.y * right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Divide(Vector2 left, Vector2 right) {
		return Vector2{ left.x / right.x, left.y / right.y };
	}


	HPM_INLINE Vector3 HPM_CALL Add(Vector3 left, Vector3 right) {
		return Vector3{ left.x + right.x, left.y + right.y, left.z + right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Subtract(Vector3 left, Vector3 right) {
		return Vector3{ left.x - right.x, left.y - right.y, left.z - right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Multiply(Vector3 left, Vector3 right) {
		return Vector3{ left.x * right.x, left.y * right.y, left.z * right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Divide(Vector3 left, Vector3 right) {
		return Vector3{ left.x / right.x, left.y / right.y, left.z / right.z };
	}


	HPM_INLINE Vector4 HPM_CALL Add(Vector4 left, Vector4 right) {
		return Vector4{ left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Subtract(Vector4 left, Vector4 right) {
		return Vector4{ left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Multiply(Vector4 left, Vector4 right) {
		return Vector4{ left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Divide(Vector4 left, Vector4 right) {
		return Vector4{ left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w };
	}
}