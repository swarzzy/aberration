#pragma once

#include <xmmintrin.h>
#include <cmath>

#if defined(_MSC_VER)
typedef __int8				int8;  
typedef __int16				int16; 
typedef __int32				int32; 
typedef __int64				int64; 

typedef unsigned __int8		uint8;	
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

typedef uint32				bool32;
typedef unsigned char		uchar;

#undef byte // Defined as unsigned char in rpcndr.h
typedef unsigned __int8		byte;

#else
#include <cstdint>
typedef int8_t				int8;	
typedef int16_t				int16;	
typedef int32_t				int32;	
typedef int64_t				int64;	

typedef uint8_t				uint8;	
typedef uint16_t			uint16;	
typedef uint32_t			uint32;	
typedef uint64_t			uint64;	

typedef uint32				bool32;
typedef unsigned char		uchar;

#undef byte // Defined as unsigned char in rpcndr.h
typedef uint8_t				byte;

#endif

typedef float				float32;
typedef double				float64;

// SIMD registers
typedef __m128				float128;


#define HPM_CALL //__vectorcall
#define HPM_INLINE inline

namespace hpm {

	template <typename T>
	HPM_INLINE T Abs(T value) {
		if (value < 0) {
			return value * (T)-1;
		}
		return value;
	}

	HPM_INLINE float32 Map(float32 t, float32 a, float32 b, float32 c, float32 d) {
		if (a == b || d == c) return 0.0f;
		return c + (d - c) / (b - a) * (t - a);
	}

	HPM_INLINE constexpr float32 Pi() {
		return 3.14159265358979323846f;
	}
	
	HPM_INLINE float32 Sin(float32 radians) {
		return sinf(radians);
	}

	HPM_INLINE float32 Cos(float32 radians) {
		return cosf(radians);
	}

	HPM_INLINE float32 Tan(float32 radians) {
		return tanf(radians);
	}

	HPM_INLINE float32 Sqrt(float32 num) {
		return sqrtf(num);
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
	};

	struct Rectangle {
		Vector2 min;
		Vector2 max;
	};

	union Matrix4 {
		Vector4 columns[4];
		float32 data[16];
		struct {
			float32 _11, _21, _31, _41;
			float32 _12, _22, _32, _42;
			float32 _13, _23, _33, _43;
			float32 _14, _24, _34, _44;
		};
	};

	union Matrix3 {
		Vector3 columns[3];
		float32 data[9];
		struct {
			float32 _11, _21, _31;
			float32 _12, _22, _32;
			float32 _13, _23, _33;
		};
	};

	HPM_INLINE Vector2 HPM_CALL Add(Vector2 left, Vector2 right) {
		return Vector2{ left.x + right.x, left.y + right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Add(Vector2 left, float32 scalar) {
		return Vector2{ left.x + scalar, left.y + scalar };
	}

	HPM_INLINE Vector2 HPM_CALL Subtract(Vector2 left, Vector2 right) {
		return Vector2{ left.x - right.x, left.y - right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Subtract(Vector2 left, float32 scalar) {
		return Vector2{ left.x - scalar, left.y - scalar };
	}

	HPM_INLINE Vector2 HPM_CALL Multiply(Vector2 left, Vector2 right) {
		return Vector2{ left.x * right.x, left.y * right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Multiply(Vector2 left, float32 scalar) {
		return Vector2{ left.x * scalar, left.y * scalar};
	}

	HPM_INLINE Vector2 HPM_CALL Divide(Vector2 left, Vector2 right) {
		return Vector2{ left.x / right.x, left.y / right.y };
	}

	HPM_INLINE Vector2 HPM_CALL Divide(Vector2 left, float32 scalar) {
		return Vector2{ left.x / scalar, left.y / scalar};
	}


	HPM_INLINE Vector3 HPM_CALL Add(Vector3 left, Vector3 right) {
		return Vector3{ left.x + right.x, left.y + right.y, left.z + right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Add(Vector3 left, float32 scalar) {
		return Vector3{ left.x + scalar, left.y + scalar, left.z + scalar};
	}

	HPM_INLINE Vector3 HPM_CALL Subtract(Vector3 left, Vector3 right) {
		return Vector3{ left.x - right.x, left.y - right.y, left.z - right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Subtract(Vector3 left, float32 scalar) {
		return Vector3{ left.x - scalar, left.y - scalar, left.z - scalar};
	}

	HPM_INLINE Vector3 HPM_CALL Multiply(Vector3 left, Vector3 right) {
		return Vector3{ left.x * right.x, left.y * right.y, left.z * right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Multiply(Vector3 left, float32 scalar) {
		return Vector3{ left.x * scalar, left.y * scalar, left.z * scalar};
	}

	HPM_INLINE Vector3 HPM_CALL Divide(Vector3 left, Vector3 right) {
		return Vector3{ left.x / right.x, left.y / right.y, left.z / right.z };
	}

	HPM_INLINE Vector3 HPM_CALL Divide(Vector3 left, float32 scalar) {
		return Vector3{ left.x / scalar, left.y / scalar, left.z / scalar };
	}


	HPM_INLINE Vector4 HPM_CALL Add(Vector4 left, Vector4 right) {
		return Vector4{ left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Add(Vector4 left, float32 scalar) {
		return Vector4{ left.x + scalar, left.y + scalar, left.z + scalar, left.w + scalar};
	}

	HPM_INLINE Vector4 HPM_CALL Subtract(Vector4 left, Vector4 right) {
		return Vector4{ left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Subtract(Vector4 left, float32 scalar) {
		return Vector4{ left.x - scalar, left.y - scalar, left.z - scalar, left.w - scalar};
	}

	HPM_INLINE Vector4 HPM_CALL Multiply(Vector4 left, Vector4 right) {
		return Vector4{ left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Multiply(Vector4 left, float32 scalar) {
		return Vector4{ left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar };
	}

	HPM_INLINE Vector4 HPM_CALL Divide(Vector4 left, Vector4 right) {
		return Vector4{ left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w };
	}

	HPM_INLINE Vector4 HPM_CALL Divide(Vector4 left, float32 scalar) {
		return Vector4{ left.x / scalar, left.y / scalar, left.z / scalar, left.w / scalar};
	}


	HPM_INLINE float32 HPM_CALL Length(Vector2 vector) {
		return Sqrt(vector.x * vector.x + vector.y * vector.y);
	}

	HPM_INLINE float32 HPM_CALL Length(Vector3 vector) {
		return Sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	}

	HPM_INLINE float32 HPM_CALL Length(Vector4 vector) {
		return Sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w);
	}

	HPM_INLINE Vector2 HPM_CALL Normalize(Vector2 vector) {
		Vector2 result;
		float32 len = Length(vector);
		result.x = vector.x / len;
		result.y = vector.y / len;
		return result;
	}

	HPM_INLINE Vector3 HPM_CALL Normalize(Vector3 vector) {
		Vector3 result;
		float32 len = Length(vector);
		result.x = vector.x / len;
		result.y = vector.y / len;
		result.z = vector.z / len;
		return result;
	}

	HPM_INLINE Vector4 HPM_CALL Normalize(Vector4 vector) {
		Vector4 result;
		float32 len = Length(vector);
		result.x = vector.x / len;
		result.y = vector.y / len;
		result.z = vector.z / len;
		result.w = vector.w / len;
		return result;
	}


	HPM_INLINE float32 HPM_CALL Dot(Vector2 left, Vector2 right) {
		return left.x * right.x + left.y * right.y;
	}

	HPM_INLINE float32 HPM_CALL Dot(Vector3 left, Vector3 right) {
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	HPM_INLINE float32 HPM_CALL Dot(Vector4 left, Vector4 right) {
		return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
	}


	HPM_INLINE Vector3 HPM_CALL Cross(Vector3 left, Vector3 right) {
		return {left.y * right.z - left.z * right.y,
				left.z * right.x - left.x * right.z,
				left.x * right.y - left.y * right.x};
	}

	HPM_INLINE Matrix3 HPM_CALL Identity3() {
		Matrix3 result = {};
		result._11 = 1.0f;
		result._22 = 1.0f;
		result._33 = 1.0f;
		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Identity4() {
		Matrix4 result = {};
		result._11 = 1.0f;
		result._22 = 1.0f;
		result._33 = 1.0f;
		result._44 = 1.0f;
		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL OrthogonalRH(float32 left, float32 right, float32 bottom, float32 top, float32 near, float32 far) {
		Matrix4 result = {};

		result._11 = 2.0f / (right - left);
		result._22 = 2.0f / (top - bottom);
		result._33 = -2.0f / (far - near);
		result._14 = -(right + left) / (right - left);
		result._24 = -(top + bottom) / (top - bottom);
		result._34 = -(far + near) / (far - near);
		result._44 = 1.0f;

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL PerspectiveRH(float32 fovDeg, float32 aspectRatio, float32 near, float32 far) {
		Matrix4 result = {};

		float32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
		result._33 = - (far + near) / (far - near);
		result._43 = -1.0f;
		result._34 = (-2.0f * near * far) / (far - near);

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Translation(Vector3 trans) {
		Matrix4 result = {};
		result._11 = 1.0f;
		result._22 = 1.0f;
		result._33 = 1.0f;
		result._44 = 1.0f;

		result._14 = trans.x;
		result._24 = trans.y;
		result._34 = trans.z;

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL LookAtRH(Vector3 from, Vector3 at, Vector3 up) {
		Vector3 zAxis = Normalize(Subtract(at, from));
		Vector3 xAxis = Normalize(Cross(zAxis, up));
		Vector3 yAxis = Cross(xAxis, zAxis);

		Matrix4 result;
		result._11 = xAxis.x;
		result._12 = xAxis.y;
		result._13 = xAxis.z;
		result._14 = -Dot(xAxis, from);

		result._21 = yAxis.x;
		result._22 = yAxis.y;
		result._23 = yAxis.z;
		result._24 = -Dot(yAxis, from);

		result._31 = -zAxis.x;
		result._32 = -zAxis.y;
		result._33 = -zAxis.z;
		result._34 = Dot(zAxis, from);

		result._41 = 0.0f;
		result._42 = 0.0f;
		result._43 = 0.0f;
		result._44 = 1.0f;

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Translate(Matrix4 mtx, Vector3 trans) {
		mtx.columns[3] = Add(Add(Multiply(mtx.columns[0], trans.x), Multiply(mtx.columns[1], trans.y)), 
							 Add(Multiply(mtx.columns[2], trans.z), mtx.columns[3]));
		return mtx;
	}

	HPM_INLINE Matrix4 HPM_CALL Scaling(Vector3 scale) {
		Matrix4 result = {};
		result._11 = scale.x;
		result._22 = scale.y;
		result._33 = scale.z;
		result._44 = 1.0f;

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Scale(Matrix4 mtx, Vector3 scale) {
		mtx.columns[0] = Multiply(mtx.columns[0], scale.x);
		mtx.columns[1] = Multiply(mtx.columns[1], scale.y);
		mtx.columns[2] = Multiply(mtx.columns[2], scale.z);
		return mtx;
	}

	HPM_INLINE Matrix4 HPM_CALL Rotation(float32 angleDeg, Vector3 axis) {
		Matrix4 result = {};

		float32 c = Cos(ToRadians(angleDeg));
		float32 oc = 1.0f - c;
		float32 s = Sin(ToRadians(angleDeg));
		Vector3 an = Normalize(axis);

		result._11 = c + an.x * an.x * oc;
		result._12 = (an.x * an.y * oc) + (an.z * s);
		result._13 = (an.x * an.z * oc) - (an.y * s);

		result._21 = (an.x * an.y * oc) - (an.z * s);
		result._22 = c + an.y * an.y * oc;
		result._23 = (an.y * an.z * oc) + (an.x * s);

		result._31 = (an.x * an.z * oc) + (an.y * s);
		result._32 = (an.y * an.z * oc) - (an.x * s);
		result._33 = c + an.z * an.z * oc;

		result._44 = 1.0f;
		
		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Rotate(Matrix4 mtx, float32 angleDeg, Vector3 axis) {
		Matrix4 rotation;

		float32 c = Cos(ToRadians(angleDeg));
		float32 oc = 1.0f - c;
		float32 s = Sin(ToRadians(angleDeg));
		Vector3 an = Normalize(axis);

		rotation._11 = c + an.x * an.x * oc;
		rotation._12 = (an.x * an.y * oc) + (an.z * s);
		rotation._13 = (an.x * an.z * oc) - (an.y * s);

		rotation._21 = (an.x * an.y * oc) - (an.z * s);
		rotation._22 = c + an.y * an.y * oc;
		rotation._23 = (an.y * an.z * oc) + (an.x * s);

		rotation._31 = (an.x * an.z * oc) + (an.y * s);
		rotation._32 = (an.y * an.z * oc) - (an.x * s);
		rotation._33 = c + an.z * an.z * oc;

		Matrix4 result;
		result._11 = mtx._11 * rotation._11 + mtx._12 * rotation._21 + mtx._13 * rotation._31;
		result._12 = mtx._11 * rotation._12 + mtx._12 * rotation._22 + mtx._13 * rotation._32;
		result._13 = mtx._11 * rotation._13 + mtx._12 * rotation._23 + mtx._13 * rotation._33;

		result._21 = mtx._21 * rotation._11 + mtx._22 * rotation._21 + mtx._23 * rotation._31;
		result._22 = mtx._21 * rotation._12 + mtx._22 * rotation._22 + mtx._23 * rotation._32;
		result._23 = mtx._21 * rotation._13 + mtx._22 * rotation._23 + mtx._23 * rotation._33;

		result._31 = mtx._31 * rotation._11 + mtx._32 * rotation._21 + mtx._33 * rotation._31;
		result._32 = mtx._31 * rotation._12 + mtx._32 * rotation._22 + mtx._33 * rotation._32;
		result._33 = mtx._31 * rotation._13 + mtx._32 * rotation._23 + mtx._33 * rotation._33;

		result.columns[3] = mtx.columns[3];
		result._41 = mtx._41;
		result._42 = mtx._42;
		result._43 = mtx._43;
		result._44 = mtx._44;

		return result;
	}

	// TODO: Is passing matrices by value is efficient?
	HPM_INLINE Matrix3 HPM_CALL Multiply(Matrix3 left, Matrix3 right) {
		Matrix3 result;
		result._11 = left._11 * right._11 + left._12 * right._21 + left._13 * right._31;
		result._12 = left._11 * right._12 + left._12 * right._22 + left._13 * right._32;
		result._13 = left._11 * right._13 + left._12 * right._23 + left._13 * right._33;

		result._21 = left._21 * right._11 + left._22 * right._21 + left._23 * right._31;
		result._22 = left._21 * right._12 + left._22 * right._22 + left._23 * right._32;
		result._23 = left._21 * right._13 + left._22 * right._23 + left._23 * right._33;

		result._31 = left._31 * right._11 + left._32 * right._21 + left._33 * right._31;
		result._32 = left._31 * right._12 + left._32 * right._22 + left._33 * right._32;
		result._33 = left._31 * right._13 + left._32 * right._23 + left._33 * right._33;

		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL Multiply(Matrix4 left, Matrix4 right) {
		Matrix4 result;

		result._11 = left._11 * right._11 + left._12 * right._21 + left._13 * right._31 + left._14 * right._41;
		result._12 = left._11 * right._12 + left._12 * right._22 + left._13 * right._32 + left._14 * right._42;
		result._13 = left._11 * right._13 + left._12 * right._23 + left._13 * right._33 + left._14 * right._43;
		result._14 = left._11 * right._14 + left._12 * right._24 + left._13 * right._34 + left._14 * right._44;

		result._21 = left._21 * right._11 + left._22 * right._21 + left._23 * right._31 + left._24 * right._41;
		result._22 = left._21 * right._12 + left._22 * right._22 + left._23 * right._32 + left._24 * right._42;
		result._23 = left._21 * right._13 + left._22 * right._23 + left._23 * right._33 + left._24 * right._43;
		result._24 = left._21 * right._14 + left._22 * right._24 + left._23 * right._34 + left._24 * right._44;

		result._31 = left._31 * right._11 + left._32 * right._21 + left._33 * right._31 + left._34 * right._41;
		result._32 = left._31 * right._12 + left._32 * right._22 + left._33 * right._32 + left._34 * right._42;
		result._33 = left._31 * right._13 + left._32 * right._23 + left._33 * right._33 + left._34 * right._43;
		result._34 = left._31 * right._14 + left._32 * right._24 + left._33 * right._34 + left._34 * right._44;
		
		result._41 = left._41 * right._11 + left._42 * right._21 + left._43 * right._31 + left._44 * right._41;
		result._42 = left._41 * right._12 + left._42 * right._22 + left._43 * right._32 + left._44 * right._42;
		result._43 = left._41 * right._13 + left._42 * right._23 + left._43 * right._33 + left._44 * right._43;
		result._44 = left._41 * right._14 + left._42 * right._24 + left._43 * right._34 + left._44 * right._44;

		return result;
	}
}