#pragma once

#include <xmmintrin.h>
#include <cmath>

#include <cstdint>
typedef int8_t				int8;
typedef int16_t				int16;
typedef int32_t				int32;
typedef int64_t				int64;

typedef uint8_t				uint8;
typedef uint16_t			uint16;
typedef uint32_t			uint32;
typedef uint64_t			uint64;

typedef uintptr_t			uintptr;

#undef byte // Defined as unsigned char in rpcndr.h
typedef uint8				byte;
typedef uint32				bool32;
typedef unsigned char		uchar;

typedef float				float32;
typedef double				float64;

typedef uint32				color32;
typedef byte				bool8;
typedef __m128				float128;

#define HPM_USE_NAMESPACE

#define HPM_CALL //__vectorcall
#define HPM_INLINE inline

namespace hpm {

	constexpr float32 PI_32 = 3.14159265358979323846f;

	HPM_INLINE float32 Map(float32 t, float32 a, float32 b, float32 c, float32 d) {
		if (a == b || d == c) return 0.0f;
		return c + (d - c) / (b - a) * (t - a);
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

#if 1
	
	HPM_INLINE int32 Abs(int32 val) {
		return val > 0 ? val : -val;
	}

	HPM_INLINE float32 Abs(float32 val) {
		return val > 0 ? val : -val;
	}

	HPM_INLINE int16 Abs(int16 val) {
		return val > 0 ? val : -val;
	}

	HPM_INLINE long Abs(long val) {
		return val > 0 ? val : -val;
	}


#else

	// TODO: This is temporary. Because clang for some reason complains on overloads.

	template<typename T>
		HPM_INLINE T Abs(T val) {
		return val > 0 ? val : -val;
	}

#endif

	
	HPM_INLINE constexpr float32 ToDegrees(float32 radians) {
		return 180.0f / PI_32 * radians;
	}

	HPM_INLINE constexpr float32 ToRadians(float32 degrees) {
		return PI_32 / 180.0f * degrees;
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

	HPM_INLINE Vector2 V2(float32 x, float32 y) {
		return Vector2{x, y};
	}
	
	HPM_INLINE Vector2 V2(float32 val) {
		return Vector2{val, val};
	}

	HPM_INLINE Vector3 V3(float32 x, float32 y, float32 z) {
		return Vector3{x, y, z};
	}
	
	HPM_INLINE Vector3 V3(float32 val) {
		return Vector3{val, val, val};
	}

	HPM_INLINE Vector3 V3(Vector2 v, float32 z) {
		return Vector3{v.x, v.y, z};
	}

	HPM_INLINE Vector3 V3(Vector4 v) {
		return Vector3{v.x, v.y, v.z};
	}

	HPM_INLINE Vector4 V4(float32 x, float32 y, float32 z, float32 w) {
		return Vector4{x, y ,z, w};
	}
	
	HPM_INLINE Vector4 V4(float32 val) {
		return Vector4{val, val , val, val};
	}

	HPM_INLINE Vector4 V4(Vector2 v, float32 z, float32 w) {
		return Vector4{v.x, v.y ,z, w};
	}

	HPM_INLINE Vector4 V4(Vector3 v, float32 w) {
		return Vector4{v.x, v.y ,v.z, w};
	}
 
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

	HPM_INLINE Vector2 HPM_CALL MulV2V2(Vector2 left, Vector2 right) {
		return Vector2{ left.x * right.x, left.y * right.y };
	}

	HPM_INLINE Vector2 HPM_CALL MulV2F32(Vector2 left, float32 scalar) {
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

	HPM_INLINE Vector3 HPM_CALL MulV3V3(Vector3 left, Vector3 right) {
		return Vector3{ left.x * right.x, left.y * right.y, left.z * right.z };
	}

	HPM_INLINE Vector3 HPM_CALL MulV3F32(Vector3 left, float32 scalar) {
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

	HPM_INLINE Vector4 HPM_CALL MulV4V4(Vector4 left, Vector4 right) {
		return Vector4{ left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w };
	}

	HPM_INLINE Vector4 HPM_CALL MulV4F32(Vector4 left, float32 scalar) {
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

	HPM_INLINE float32 HPM_CALL DistanceSq(Vector2 v1, Vector2 v2) {
		return (v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y);
	}

	HPM_INLINE float32 HPM_CALL DistanceSq(Vector3 v1, Vector3 v2) {
		return (v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z);
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

	HPM_INLINE Matrix3 HPM_CALL M3x3(Matrix4 m) {
		Matrix3 result;
		
		result._11 = m._11;
		result._12 = m._12;
		result._13 = m._13;
		result._21 = m._21;
		result._22 = m._22;
		result._23 = m._23;
		result._31 = m._31;
		result._32 = m._32;
		result._33 = m._33;
		
		return result;
	}

	HPM_INLINE Matrix4 HPM_CALL M4x4(Matrix3 m) {
		Matrix4 result = {};

		
		result._11 = m._11;
		result._12 = m._12;
		result._13 = m._13;
		result._21 = m._21;
		result._22 = m._22;
		result._23 = m._23;
		result._31 = m._31;
		result._32 = m._32;
		result._33 = m._33;
		result._44 = 1.0f;

		return result;
	}

#if 0
	HPM_INLINE Matrix2 HPM_CALL Transpose(Matrix2 matrix) {
		Matrix2 result;
		
		result._11 = matrix._11;
		result._12 = matrix._21;
		result._21 = matrix._12;
		result._22 = matrix._22;

		return result;
	}
#endif
	
	HPM_INLINE Matrix3 HPM_CALL Transpose(Matrix3 matrix) {
		Matrix3 result;
		
		result._11 = matrix._11;
		result._12 = matrix._21;
		result._13 = matrix._31;
		result._21 = matrix._12;
		result._22 = matrix._22;
		result._23 = matrix._32;
		result._31 = matrix._13;
		result._32 = matrix._23;
		result._33 = matrix._33;

		return result;
	}
	
	HPM_INLINE Matrix4 HPM_CALL Transpose(Matrix4 matrix) {
		Matrix4 result;
		
		result._11 = matrix._11;
		result._12 = matrix._21;
		result._13 = matrix._31;
		result._14 = matrix._41;
		result._21 = matrix._12;
		result._22 = matrix._22;
		result._23 = matrix._32;
		result._24 = matrix._42;
		result._31 = matrix._13;
		result._32 = matrix._23;
		result._33 = matrix._33;
		result._34 = matrix._43;
		result._41 = matrix._14;
		result._42 = matrix._24;
		result._43 = matrix._34;
		result._44 = matrix._44;
		
		return result;
	}
#if 0
	HPM_INLINE float32 HPM_CALL Determinant(Matrix2 m) {
		return m._11 * m._22 - m._21 * m._12;
	}
#endif	

	HPM_INLINE float32 HPM_CALL Determinant(Matrix3 m) {
		return 	m._11 * m._22 * m._33 - m._11 * m._23 * m._32
			- m._12 * m._21 * m._33 + m._12 * m._23 * m._31
			+ m._13 * m._21 * m._32 - m._13 * m._22 * m._31;
	}

	HPM_INLINE Matrix4 HPM_CALL OrthogonalRH(float32 left, float32 right, float32 bottom, float32 top, float32 n, float32 f) {
		Matrix4 result = {};

		result._11 = 2.0f / (right - left);
		result._22 = 2.0f / (top - bottom);
		result._33 = -2.0f / (f - n);
		result._14 = -(right + left) / (right - left);
		result._24 = -(top + bottom) / (top - bottom);
		result._34 = -(f + n) / (f - n);
		result._44 = 1.0f;

		return result;
	}

		HPM_INLINE Matrix4 HPM_CALL PerspectiveRH(float32 fovDeg, float32 aspectRatio, float32 n, float32 f) {
		Matrix4 result = {};

		float32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
		result._33 = - (f + n) / (f - n);
		result._43 = -1.0f;
		result._34 = (-2.0f * n * f) / (f - n);

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

	HPM_INLINE Vector3 HPM_CALL GetPosition(Matrix4* m) {
		return Vector3{m->_14, m->_24, m->_34};
	}

		HPM_INLINE Matrix4 HPM_CALL Translate(Matrix4 mtx, Vector3 trans) {
		mtx.columns[3] = Add(Add(MulV4F32(mtx.columns[0], trans.x), MulV4F32(mtx.columns[1], trans.y)), 
							 Add(MulV4F32(mtx.columns[2], trans.z), mtx.columns[3]));
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
		mtx.columns[0] = MulV4F32(mtx.columns[0], scale.x);
		mtx.columns[1] = MulV4F32(mtx.columns[1], scale.y);
		mtx.columns[2] = MulV4F32(mtx.columns[2], scale.z);
		return mtx;
	}

	HPM_INLINE Vector3 HPM_CALL MulM3V3(Matrix3 m, Vector3 v) {
		Vector3 r;
		r.x = m._11 * v.x + m._12 * v.y + m._13 * v.z;
		r.y = m._21 * v.x + m._22 * v.y + m._23 * v.z;
		r.z = m._31 * v.x + m._32 * v.y + m._33 * v.z;
		return r;
	}

	HPM_INLINE Vector4 HPM_CALL MulM4V4(Matrix4 m, Vector4 v) {
		Vector4 r;
		r.x = m._11 * v.x + m._12 * v.y + m._13 * v.z + m._14 * v.w;
		r.y = m._21 * v.x + m._22 * v.y + m._23 * v.z + m._24 * v.w;
		r.z = m._31 * v.x + m._32 * v.y + m._33 * v.z + m._34 * v.w;
		r.w = m._41 * v.x + m._42 * v.y + m._43 * v.z + m._44 * v.w;
		return r;
	}

	Matrix3 HPM_CALL Inverse(Matrix3 m);
	
#if defined(HYPERMATH_IMPL)

	Matrix3 HPM_CALL Inverse(Matrix3 m) {

		float32 a11 = m._22 * m._33 - m._23 * m._32; 
		float32 a12 = - (m._21 * m._33 - m._23 * m._31);
		float32 a13 = m._21 * m._32 - m._22 * m._31;
		
		float32 a21 = - (m._12 * m._33 - m._32 * m._13);
		float32 a22 = m._11 * m._33 - m._13 * m._31;
		float32 a23 = - (m._11 * m._32 - m._12 * m._31);
		
		float32 a31 = m._12 * m._23 - m._22 * m._13;
		float32 a32 = - (m._11 * m._23 - m._21 * m._13);
		float32 a33 = m._11 * m._22 - m._21 * m._12;

		float32 det = (m._11 * a11 + m._12 * a12 + m._13 * a13);
		// NOTE: Quietly return an indetity matix if no inverse matrix exist
		if (det == 0) {
			return Identity3();
		} else {
			
			float32 oneOverDet = 1.0f / det;

			Matrix3 inv;
			inv._11 = a11 * oneOverDet;
			inv._12 = a21 * oneOverDet;
			inv._13 = a31 * oneOverDet;
			inv._21 = a12 * oneOverDet;
			inv._22 = a22 * oneOverDet;
			inv._23 = a32 * oneOverDet;
			inv._31 = a13 * oneOverDet;
			inv._32 = a23 * oneOverDet;
			inv._33 = a33 * oneOverDet;

			return inv;
		}
	}

#endif

	Matrix4 HPM_CALL Inverse(Matrix4 m);

#if defined(HYPERMATH_IMPL)
	
	Matrix4 HPM_CALL Inverse(Matrix4 m) {

		float32 a11_22 = m._33 * m._44 - m._34 * m._43;
		float32 a11_23 = m._32 * m._44 - m._34 * m._42;
		float32 a11_24 = m._32 * m._43 - m._33 * m._42;

		float32 A11 = m._22 * a11_22 - m._23 * a11_23 + m._24 * a11_24;

		float32 a12_21 = m._33 * m._44 - m._34 * m._43;
		float32 a12_23 = m._31 * m._44 - m._34 * m._41;
		float32 a12_24 = m._31 * m._43 - m._33 * m._41;

		float32 A12 = -(m._21 * a12_21 - m._23 * a12_23 + m._24 * a12_24);

		float32 a13_21 = m._32 * m._44 - m._34 * m._42;
		float32 a13_22 = m._31 * m._44 - m._34 * m._41;
		float32 a13_24 = m._31 * m._42 - m._32 * m._41;

		float32 A13 = m._21 * a13_21 - m._22 * a13_22 + m._24 * a13_24;

		float32 a14_21 = m._32 * m._43 - m._33 * m._42;
		float32 a14_22 = m._31 * m._43 - m._33 * m._41;
		float32 a14_23 = m._31 * m._42 - m._32 * m._41;

		float32 A14 = -(m._21 * a14_21 - m._22 * a14_22 + m._23 * a14_23);

		float32 a21_12 = m._33 * m._44 - m._34 * m._43;
		float32 a21_13 = m._32 * m._44 - m._34 * m._42;
		float32 a21_14 = m._32 * m._43 - m._33 * m._42;

		float32 A21 = -(m._12 * a21_12 - m._13 * a21_13 + m._14 * a21_14);

		float32 a22_11 = m._33 * m._44 - m._34 * m._43;
		float32 a22_13 = m._31 * m._44 - m._34 * m._41;
		float32 a22_14 = m._31 * m._43 - m._33 * m._41;

		float32 A22 = m._11 * a22_11 - m._13 * a22_13 + m._14 * a22_14;

		float32 a23_11 = m._32 * m._44 - m._34 * m._42;
		float32 a23_12 = m._31 * m._44 - m._34 * m._41;
		float32 a23_14 = m._31 * m._42 - m._32 * m._41;

		float32 A23 = -(m._11 * a23_11 - m._12 * a23_12 + m._14 * a23_14);

		float32 a24_11 = m._32 * m._43 - m._33 * m._42;
		float32 a24_12 = m._31 * m._43 - m._33 * m._41;
		float32 a24_13 = m._31 * m._42 - m._32 * m._41;

		float32 A24 = m._11 * a24_11 - m._12 * a24_12 + m._13 * a24_13;

		float32 a31_12 = m._23 * m._44 - m._24 * m._43;
		float32 a31_13 = m._22 * m._44 - m._24 * m._42;
		float32 a31_14 = m._22 * m._43 - m._23 * m._42;

		float32 A31 = m._12 * a31_12 - m._13 * a31_13 + m._14 * a31_14;

		float32 a32_11 = m._23 * m._44 - m._24 * m._43;
		float32 a32_13 = m._21 * m._44 - m._24 * m._41;
		float32 a32_14 = m._21 * m._43 - m._23 * m._41;

		float32 A32 = -(m._11 * a32_11 - m._13 * a32_13 + m._14 * a32_14);

		float32 a33_11 = m._22 * m._44 - m._24 * m._42;
		float32 a33_12 = m._21 * m._44 - m._24 * m._41;
		float32 a33_14 = m._21 * m._42 - m._22 * m._41;

		float32 A33 = m._11 * a33_11 - m._12 * a33_12 + m._14 * a33_14;

		float32 a34_11 = m._22 * m._43 - m._23 * m._42;
		float32 a34_12 = m._21 * m._43 - m._23 * m._41;
		float32 a34_13 = m._21 * m._42 - m._22 * m._41;

		float32 A34 = -(m._11 * a34_11 - m._12 * a34_12 + m._13 * a34_13);

		float32 a41_12 = m._23 * m._34 - m._24 * m._33;
		float32 a41_13 = m._22 * m._34 - m._24 * m._32;
		float32 a41_14 = m._22 * m._33 - m._23 * m._32;

		float32 A41 = -(m._12 * a41_12 - m._13 * a41_13 + m._14 * a41_14);

		float32 a42_11 = m._23 * m._34 - m._24 * m._33;
		float32 a42_13 = m._21 * m._34 - m._24 * m._31;
		float32 a42_14 = m._21 * m._33 - m._23 * m._31;

		float32 A42 = m._11 * a42_11 - m._13 * a42_13 + m._14 * a42_14;

		float32 a43_11 = m._22 * m._34 - m._24 * m._32;
		float32 a43_12 = m._21 * m._34 - m._24 * m._31;
		float32 a43_14 = m._21 * m._32 - m._22 * m._31;

		float32 A43 = -(m._11 * a43_11 - m._12 * a43_12 + m._14 * a43_14);

		float32 a44_11 = m._22 * m._33 - m._23 * m._32;
		float32 a44_12 = m._21 * m._33 - m._23 * m._31;
		float32 a44_13 = m._21 * m._32 - m._22 * m._31;

		float32 A44 = m._11 * a44_11 - m._12 * a44_12 + m._13 * a44_13;

		float32 det = m._11 * A11 + m._12 * A12 + m._13 * A13 + m._14 * A14;

		// NOTE: Quietly return an indetity matix if no inverse matrix exist
		if (det == 0) {
			return Identity4();
		}
		else {
			float32 oneOverDet = 1.0f / det;
			Matrix4 result;
			result._11 = A11 * oneOverDet;
			result._12 = A21 * oneOverDet;
			result._13 = A31 * oneOverDet;
			result._14 = A41 * oneOverDet;
			result._21 = A12 * oneOverDet;
			result._22 = A22 * oneOverDet;
			result._23 = A32 * oneOverDet;
			result._24 = A42 * oneOverDet;
			result._31 = A13 * oneOverDet;
			result._32 = A23 * oneOverDet;
			result._33 = A33 * oneOverDet;
			result._34 = A43 * oneOverDet;
			result._41 = A14 * oneOverDet;
			result._42 = A24 * oneOverDet;
			result._43 = A34 * oneOverDet;
			result._44 = A44 * oneOverDet;

			return result;
		}
	}

#endif

	float32 HPM_CALL Determinant(Matrix4 m);

#if defined(HYPERMATH_IMPL)

	float32 HPM_CALL Determinant(Matrix4 m) {
		float32 minor1 = m._11 * (m._22 * m._33 * m._44 - m._22 * m._34 * m._43
								  - m._23 * m._32 * m._44 + m._23 * m._34 * m._42
								  + m._24 * m._32 * m._43 - m._24 * m._33 * m._42);

		float32 minor2 = m._12 * (m._21 * m._33 * m._44 - m._21 * m._34 * m._43
								  - m._23 * m._31 * m._44 + m._23 * m._41 * m._34
								  +m._24 * m._31 * m._43 - m._24 * m._33 * m._41);

		float32 minor3 = m._13 * (m._21 * m._32 * m._44 - m._21 * m._34 * m._42
								  - m._22 * m._31 * m._44 + m._22 * m._34 * m._41
								  + m._24 * m._31 * m._42 - m._24 * m._32 * m._41);

		float32 minor4 = m._14 * (m._21 * m._32 * m._43 - m._21 * m._33 * m._42
								  - m._22 * m._31 * m._43 + m._22 * m._41 * m._33
								  + m._23 * m._31 * m._42 - m._23 * m._32 * m._41);

		return minor1 - minor2 + minor3 - minor4;
	}

#endif

	Matrix4 HPM_CALL LookAtRH(Vector3 from, Vector3 at, Vector3 up);
	
#if defined(HYPERMATH_IMPL) 
	
	Matrix4 HPM_CALL LookAtRH(Vector3 from, Vector3 at, Vector3 up) {
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

#endif

	Matrix4 HPM_CALL Rotation(float32 angleDeg, Vector3 axis);

#if defined(HYPERMATH_IMPL)

	Matrix4 HPM_CALL Rotation(float32 angleDeg, Vector3 axis) {
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

#endif

	Matrix4 HPM_CALL Rotate(Matrix4 mtx, float32 angleDeg, Vector3 axis);

#if defined(HYPERMATH_IMPL)

	Matrix4 HPM_CALL Rotate(Matrix4 mtx, float32 angleDeg, Vector3 axis) {
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

#endif

	Matrix3 HPM_CALL MulM3M3(Matrix3 left, Matrix3 right);

#if defined(HYPERMATH_IMPL)
	// TODO: Is passing matrices by value is efficient?
	Matrix3 HPM_CALL MulM3M3(Matrix3 left, Matrix3 right) {
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

#endif

	Matrix4 HPM_CALL MulM4M4(Matrix4 left, Matrix4 right);

#if defined(HYPERMATH_IMPL)
	
	Matrix4 HPM_CALL MulM4M4(Matrix4 left, Matrix4 right) {
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
	
#endif
	
}
#if defined(HPM_USE_NAMESPACE)
using namespace hpm;
#endif

typedef Vector2 v2;
typedef Vector3 v3;
typedef Vector4 v4;
typedef Matrix3 m3x3;
typedef Matrix4 m4x4;
