#pragma once

#include <xmmintrin.h>
#include <math.h>

#include <stdint.h>
typedef int8_t				i8;
typedef int16_t				i16;
typedef int32_t				i32;
typedef int64_t				i64;

typedef uint8_t				u8;
typedef uint16_t			u16;
typedef uint32_t			u32;
typedef uint64_t			u64;

typedef uintptr_t			uptr;

#undef byte // Defined as unsigned char in rpcndr.h
typedef char				byte;
typedef u32             	b32;
typedef unsigned char		uchar;

typedef float				f32;
typedef double				f64;

typedef u32             	color32;
typedef byte				b8;
typedef __m128				f128;

#define HPM_USE_NAMESPACE

#define HPM_CALL //__vectorcall
namespace hpm
{
	constexpr f32 PI_32 = 3.14159265358979323846f;
	constexpr f32 FLOAT_EPS = 0.000001f;

	inline f32 AbsF32(f32 value)
	{
		return value >= 0.0f ? value : -value;
	}

	inline u32 AbsI32(i32 value)
	{
		return value >= 0 ? (u32)value : (u32)(-value);
	}

	inline u32 SignBit(f32 val)
	{
		return signbit(val);
	}

	inline f32 Square(f32 v)
	{
		f32 result = v * v;
		return result;
	}

	inline f32 Map(f32 t, f32 a, f32 b, f32 c, f32 d)
	{
		if (a == b || d == c) return 0.0f;
		return c + (d - c) / (b - a) * (t - a);
	}

	inline f32 Sin(f32 radians)
	{
		return sinf(radians);
	}

	inline f32 Cos(f32 radians)
	{
		return cosf(radians);
	}

	inline f32 Tan(f32 radians)
	{
		return tanf(radians);
	}

	inline f32 Sqrt(f32 num)
	{
		return sqrtf(num);
	}

	inline i32 Floor(f32 value)
	{
		return (i32)floorf(value);
	}

	inline i32 Ceil(f32 value)
	{
		return (i32)ceilf(value);
	}

	inline i32 RoundF32I32(f32 value)
	{
		return (i32)roundf(value);
	}

	inline float Minimum(float a, float b)
	{
		return a < b ? a : b;
	}

	inline float Maximum(float a, float b)
	{
		return a > b ? a : b;
	}
	
	inline f32 ToDegrees(f32 radians)
	{
		return 180.0f / PI_32 * radians;
	}

	inline f32 ToRadians(f32 degrees)
	{
		return PI_32 / 180.0f * degrees;
	}

	inline f32 Lerp(f32 a, f32 b, f32 t)
	{
		f32 result;
		if (t < 0.0f)
		{
			result = a;
		}
		else if (t > 1.0f)
		{
			result = b;
		}
		else
		{
			result = (1 - t) * a + t * b;
		}

		return result;
	}
	
	union Vector2
	{
		struct
		{
			f32 x;
			f32 y;
		};
		struct
		{
			f32 r;
			f32 g;
		};
		f32 data[2];
	};

	union Vector3
	{
		struct
		{
			f32 x;
			f32 y;
			f32 z;
		};
		struct
		{
			f32 r;
			f32 g;
			f32 b;
		};
		f32 data[3];
	};

	union Vector4
	{
		struct
		{
			f32 x;
			f32 y;
			f32 z;
			f32 w;
		};
		struct
		{
			f32 r;
			f32 g;
			f32 b;
			f32 a;
		};
		struct
		{
			Vector3 xyz;
			f32 _w;
		};
		f32 data[4];
		f128 _packed;
	};


	union Matrix4
	{
		Vector4 columns[4];
		f32 data[16];
		struct
		{
			f32 _11, _21, _31, _41;
			f32 _12, _22, _32, _42;
			f32 _13, _23, _33, _43;
			f32 _14, _24, _34, _44;
		};
	};

	union Matrix3
	{
		Vector3 columns[3];
		f32 data[9];
		struct
		{
			f32 _11, _21, _31;
			f32 _12, _22, _32;
			f32 _13, _23, _33;
		};
	};

	union Quaternion
	{
		struct
		{
			f32 x, y, z, w;
		};
		struct
		{
			Vector3 xyz;
			f32 _w;
		};		
	};

	inline Vector2 V2(f32 x, f32 y)
	{
		return Vector2{x, y};
	}
	
	inline Vector2 V2(f32 val)
	{
		return Vector2{val, val};
	}

	inline Vector3 V3(f32 x, f32 y, f32 z)
	{
		return Vector3{x, y, z};
	}
	
	inline Vector3 V3(f32 val)
	{
		return Vector3{val, val, val};
	}

	inline Vector3 V3(Vector2 v, f32 z)
	{
		return Vector3{v.x, v.y, z};
	}

	inline Vector3 V3(Vector4 v)
	{
		return Vector3{v.x, v.y, v.z};
	}

	inline Vector3 V3(Quaternion q)
	{
		return Vector3{q.x, q.y, q.z};
	}

	inline Vector4 V4(f32 x, f32 y, f32 z, f32 w)
	{
		return Vector4{x, y ,z, w};
	}
	
	inline Vector4 V4(f32 val)
	{
		return Vector4{val, val , val, val};
	}

	inline Vector4 V4(Vector2 v, f32 z, f32 w)
	{
		return Vector4{v.x, v.y ,z, w};
	}

	inline Vector4 V4(Vector3 v, f32 w)
	{
		return Vector4{v.x, v.y ,v.z, w};
	}

	inline Vector2 operator+(Vector2 l, Vector2 r)
	{
		return Vector2{ l.x + r.x, l.y + r.y };
	}

	inline Vector2 operator+(Vector2 v, f32 s)
	{
		return Vector2{ v.x + s, v.y + s };		
	}

 	inline Vector2 operator+(f32 s, Vector2 v)
	{
		return Vector2{ v.x + s, v.y + s };		
	}

	inline Vector2& operator+=(Vector2& l, Vector2 r)
	{
		l.x += r.x;
		l.y += r.y;
		return l;
	}

	inline Vector2& operator+=(Vector2& l, f32 s)
	{
		l.x += s;
		l.y += s;
		return l;
	}

	inline Vector2 operator-(Vector2 l, Vector2 r)
	{
		return Vector2{ l.x - r.x, l.y - r.y };
	}

	inline Vector2 operator-(Vector2 v, f32 s)
	{
		return Vector2{ v.x - s, v.y - s };		
	}

	inline Vector2 operator-(f32 s, Vector2 v)
	{
		return Vector2{ v.x - s, v.y - s };		
	}

	inline Vector2& operator-=(Vector2& l, Vector2 r)
	{
		l.x -= r.x;
		l.y -= r.y;
		return l;
	}

	inline Vector2& operator-=(Vector2& l, f32 s)
	{
		l.x -= s;
		l.y -= s;
		return l;
	}

	inline Vector2 operator-(Vector2 v)
	{
		v.x = -v.x;
		v.y = -v.y;
		return v;
	}

	inline Vector2 operator*(Vector2 v, f32 s)
	{
		return Vector2{ v.x * s, v.y * s };
	}
	
	inline Vector2 operator*(f32 s, Vector2 v)
	{
		return Vector2{ v.x * s, v.y * s };
	}

	inline Vector2& operator*=(Vector2& l, f32 s)
	{
		l.x *= s;
		l.y *= s;
		return l;
	}

	inline Vector2 HPM_CALL AddV2V2(Vector2 left, Vector2 right)
	{
		return Vector2{ left.x + right.x, left.y + right.y };
	}

	inline Vector2 HPM_CALL AddV2F32(Vector2 left, f32 scalar)
	{
		return Vector2{ left.x + scalar, left.y + scalar };
	}

	inline Vector2 HPM_CALL SubV2V2(Vector2 left, Vector2 right)
	{
		return Vector2{ left.x - right.x, left.y - right.y };
	}

	inline Vector2 HPM_CALL SubV2F32(Vector2 left, f32 scalar)
	{
		return Vector2{ left.x - scalar, left.y - scalar };
	}

	inline Vector2 HPM_CALL MulV2V2(Vector2 left, Vector2 right)
	{
		return Vector2{ left.x * right.x, left.y * right.y };
	}

	inline Vector2 HPM_CALL MulV2F32(Vector2 left, f32 scalar)
	{
		return Vector2{ left.x * scalar, left.y * scalar};
	}

	inline Vector2 HPM_CALL DivV2V2(Vector2 left, Vector2 right)
	{
		return Vector2{ left.x / right.x, left.y / right.y };
	}

	inline Vector2 HPM_CALL DivV2F32(Vector2 left, f32 scalar)
	{
		return Vector2{ left.x / scalar, left.y / scalar};
	}

	inline Vector2 operator/(Vector2 v, f32 s)
	{
		return Vector2{ v.x / s, v.y / s };
	}

	inline Vector2 operator/(f32 s, Vector2 v)
	{
		return Vector2{ v.x / s, v.y / s };
	}


	
	inline Vector3 operator+(Vector3 l, Vector3 r)
	{
		return Vector3{ l.x + r.x, l.y + r.y, l.z + r.z };
	}

	inline Vector3 operator+(Vector3 v, f32 s)
	{
		return Vector3{ v.x + s, v.y + s, v.z + s };		
	}

 	inline Vector3 operator+(f32 s, Vector3 v)
	{
		return Vector3{ v.x + s, v.y + s, v.z + s };		
	}

	inline Vector3& operator+=(Vector3& l, Vector3 r)
	{
		l.x += r.x;
		l.y += r.y;
		l.z += r.z;
		return l;
	}

	inline Vector3& operator+=(Vector3& l, f32 s)
	{
		l.x += s;
		l.y += s;
		l.z += s;
		return l;
	}

	inline Vector3 operator-(Vector3 l, Vector3 r)
	{
		return Vector3{ l.x - r.x, l.y - r.y, l.z - r.z };
	}

	inline Vector3 operator-(Vector3 v, f32 s)
	{
		return Vector3{ v.x - s, v.y - s, v.z - s };		
	}

	inline Vector3 operator-(f32 s, Vector3 v)
	{
		return Vector3{ v.x - s, v.y - s, v.z - s };		
	}

	inline Vector3& operator-=(Vector3& l, Vector3 r)
	{
		l.x -= r.x;
		l.y -= r.y;
		l.z -= r.z;
		return l;
	}

	inline Vector3& operator-=(Vector3& l, f32 s)
	{
		l.x -= s;
		l.y -= s;
		l.z -= s;
		return l;
	}

	inline Vector3 operator-(Vector3 v)
	{
		v.x = -v.x;
		v.y = -v.y;
		v.z = -v.z;
		return v;
	}

	inline Vector3 operator*(Vector3 v, f32 s)
	{
		return Vector3{ v.x * s, v.y * s, v.z * s };
	}
	
	inline Vector3 operator*(f32 s, Vector3 v)
	{
		return Vector3{ v.x * s, v.y * s, v.z * s };
	}

	inline Vector3& operator*=(Vector3& l, f32 s)
	{
		l.x *= s;
		l.y *= s;
		l.z *= s;
		return l;
	}
 
	inline Vector3 operator/(Vector3 v, f32 s)
	{
		return Vector3{ v.x / s, v.y / s, v.z / s };
	}
 
	inline Vector3 operator/(f32 s, Vector3 v)
	{
		return Vector3{ v.x / s, v.y / s, v.z / s };
	}

	inline Vector3& operator/=(Vector3& v, f32 s)
	{
		v.x /= s;
		v.y /= s;
		v.z /= s;
		return v;
	}
 


	inline Vector4& operator/=(Vector4& v, f32 s)
	{
		v.x /= s;
		v.y /= s;
		v.z /= s;
		v.w /= s;
		return v;
	}

	inline Vector4& operator*=(Vector4& v, f32 s)
	{
		v.x *= s;
		v.y *= s;
		v.z *= s;
		v.w *= s;
		return v;
	}
 



	inline Vector3 HPM_CALL AddV3V3(Vector3 left, Vector3 right)
	{
		return Vector3{ left.x + right.x, left.y + right.y, left.z + right.z };
	}

	inline Vector3 HPM_CALL AddV3F32(Vector3 left, f32 scalar)
	{
		return Vector3{ left.x + scalar, left.y + scalar, left.z + scalar};
	}

	inline Vector3 HPM_CALL SubV3V3(Vector3 left, Vector3 right)
	{
		return Vector3{ left.x - right.x, left.y - right.y, left.z - right.z };
	}

	inline Vector3 HPM_CALL SubV3F32(Vector3 left, f32 scalar)
	{
		return Vector3{ left.x - scalar, left.y - scalar, left.z - scalar};
	}

	inline Vector3 HPM_CALL MulV3V3(Vector3 left, Vector3 right)
	{
		return Vector3{ left.x * right.x, left.y * right.y, left.z * right.z };
	}

	inline Vector3 HPM_CALL MulV3F32(Vector3 left, f32 scalar)
	{
		return Vector3{ left.x * scalar, left.y * scalar, left.z * scalar};
	}

	inline Vector3 HPM_CALL DivV3V3(Vector3 left, Vector3 right)
	{
		return Vector3{ left.x / right.x, left.y / right.y, left.z / right.z };
	}

	inline Vector3 HPM_CALL DivV3F32(Vector3 left, f32 scalar)
	{
		return Vector3{ left.x / scalar, left.y / scalar, left.z / scalar };
	}

	inline Vector4 HPM_CALL AddV4V4(Vector4 left, Vector4 right)
	{
		return Vector4{ left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
	}

	inline Vector4 HPM_CALL AddV4F32(Vector4 left, f32 scalar)
	{
		return Vector4{ left.x + scalar, left.y + scalar, left.z + scalar, left.w + scalar};
	}

	inline Vector4 HPM_CALL SubV4V4(Vector4 left, Vector4 right)
	{
		return Vector4{ left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
	}

	inline Vector4 HPM_CALL SubV4F32(Vector4 left, f32 scalar)
	{
		return Vector4{ left.x - scalar, left.y - scalar, left.z - scalar, left.w - scalar};
	}

	inline Vector4 HPM_CALL MulV4V4(Vector4 left, Vector4 right)
	{
		return Vector4{ left.x * right.x, left.y * right.y, left.z * right.z, left.w * right.w };
	}

	inline Vector4 HPM_CALL MulV4F32(Vector4 left, f32 scalar)
	{
		return Vector4{ left.x * scalar, left.y * scalar, left.z * scalar, left.w * scalar };
	}

	inline Vector4 HPM_CALL DivV4V4(Vector4 left, Vector4 right)
	{
		return Vector4{ left.x / right.x, left.y / right.y, left.z / right.z, left.w / right.w };
	}

	inline Vector4 HPM_CALL DivV4V4(Vector4 left, f32 scalar)
	{
		return Vector4{ left.x / scalar, left.y / scalar, left.z / scalar, left.w / scalar};
	}

	inline f32 HPM_CALL Length(Vector2 vector)
	{
		return Sqrt(vector.x * vector.x + vector.y * vector.y);
	}

	inline f32 HPM_CALL Length(Vector3 vector)
	{
		return Sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
	}

	inline f32 HPM_CALL LengthSq(Vector3 vector)
	{
		return vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
	}


	inline f32 HPM_CALL Length(Vector4 vector)
	{
		return Sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w);
	}

	inline f32 HPM_CALL DistanceSq(Vector2 v1, Vector2 v2)
	{
		return (v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y);
	}

	inline f32 HPM_CALL DistanceSq(Vector3 v1, Vector3 v2)
	{
		return (v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z);
	}

	inline Vector2 HPM_CALL Normalize(Vector2 vector)
	{
		Vector2 result;
		f32 len = Length(vector);
		if (len > FLOAT_EPS)
		{
			result.x = vector.x / len;
			result.y = vector.y / len;
		}
		else
		{
			result = vector;
		}
		return result;
	}

	inline Vector3 HPM_CALL Normalize(Vector3 vector)
	{
		Vector3 result;
		f32 len = Length(vector);
		if (len > FLOAT_EPS)
		{
			result.x = vector.x / len;
			result.y = vector.y / len;
			result.z = vector.z / len;
		}
		else
		{
			result = vector;
		}
		return result;
	}

	inline Vector4 HPM_CALL Normalize(Vector4 vector)
	{
		Vector4 result;
		f32 len = Length(vector);
		if (len > FLOAT_EPS)
		{
			result.x = vector.x / len;
			result.y = vector.y / len;
			result.z = vector.z / len;
			result.w = vector.w / len;
		}
		else
		{
			result = vector;
		}
		
		return result;
	}

	inline Vector2 Lerp(Vector2 a, Vector2 b, f32 t)
	{
		Vector2 result;
		result.x = Lerp(a.x, b.x ,t);
		result.y = Lerp(a.y, b.y ,t);
		return result;
	}

	inline Vector3 Lerp(Vector3 a, Vector3 b, f32 t)
	{
		Vector3 result;
		result.x = Lerp(a.x, b.x ,t);
		result.y = Lerp(a.y, b.y ,t);
		result.z = Lerp(a.z, b.z ,t);
		return result;
	}

	inline f32 HPM_CALL Dot(Vector2 left, Vector2 right)
	{
		return left.x * right.x + left.y * right.y;
	}

	inline f32 HPM_CALL Dot(Vector3 left, Vector3 right)
	{
		return left.x * right.x + left.y * right.y + left.z * right.z;
	}

	inline f32 HPM_CALL Dot(Vector4 left, Vector4 right)
	{
		return left.x * right.x + left.y * right.y + left.z * right.z + left.w * right.w;
	}
	
	inline Vector2 Reflect(Vector2 v, Vector2 normalizedAxis)
	{
		Vector2 result = {};
		Vector2 n = normalizedAxis;
		Vector2 NdotN2 = MulV2F32(n, 2.0f * Dot(v, n));
		result = SubV2V2(v, NdotN2);
		return result;
	}

	inline Vector3 HPM_CALL Cross(Vector3 left, Vector3 right)
	{
		return {left.y * right.z - left.z * right.y,
				left.z * right.x - left.x * right.z,
				left.x * right.y - left.y * right.x};
	}

	inline Matrix3 HPM_CALL Identity3()
	{
		Matrix3 result = {};
		result._11 = 1.0f;
		result._22 = 1.0f;
		result._33 = 1.0f;
		return result;
	}

	inline Matrix4 HPM_CALL Identity4()
	{
		Matrix4 result = {};
		result._11 = 1.0f;
		result._22 = 1.0f;
		result._33 = 1.0f;
		result._44 = 1.0f;
		return result;
	}

	inline Matrix3 HPM_CALL M3x3(Matrix4 m)
	{
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

	inline Matrix4 HPM_CALL M4x4(Matrix3 m)
	{
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
	inline Matrix2 HPM_CALL Transpose(Matrix2 matrix)
	{
		Matrix2 result;
		
		result._11 = matrix._11;
		result._12 = matrix._21;
		result._21 = matrix._12;
		result._22 = matrix._22;

		return result;
	}
#endif
	
	inline Matrix3 HPM_CALL Transpose(Matrix3 matrix)
	{
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
	
	inline Matrix4 HPM_CALL Transpose(Matrix4 matrix)
	{
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
	inline f32 HPM_CALL Determinant(Matrix2 m)
	{
		return m._11 * m._22 - m._21 * m._12;
	}
#endif	

	inline f32 HPM_CALL Determinant(Matrix3 m)
	{
		return 	m._11 * m._22 * m._33 - m._11 * m._23 * m._32
			- m._12 * m._21 * m._33 + m._12 * m._23 * m._31
			+ m._13 * m._21 * m._32 - m._13 * m._22 * m._31;
	}

	inline Matrix4 HPM_CALL OrthogonalRH(f32 left, f32 right,
										 f32 bottom, f32 top, f32 n, f32 f)
	{
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

	inline Matrix4 HPM_CALL PerspectiveRH(f32 fovDeg, f32 aspectRatio, f32 n, f32 f)
	{
		Matrix4 result = {};

		f32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
#if 0
		result._33 = - (f + n) / (f - n);
		result._34 = (-2.0f * n * f) / (f - n);
#endif
		result._33 = f / (n - f);
		result._43 = -1.0f;
		result._34 = (n * f) / (n - f);

		return result;
	}

	inline Matrix4 HPM_CALL PerspectiveOpenGLRH(f32 fovDeg, f32 aspectRatio,
												f32 n, f32 f)
	{
		Matrix4 result = {};

		f32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
		result._33 = -(f + n) / (f - n);
		result._43 = -1.0f;
		result._34 = (-2 * n * f) / (f - n);

		return result;
	}

	inline Matrix4 HPM_CALL PerspectiveOpenGLLH(f32 fovDeg, f32 aspectRatio,
			f32 n, f32 f)
	{
		Matrix4 result = {};

		f32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
		result._33 = -(f + n) / (f - n);
		result._43 = 1.0f;
		result._34 = (2 * n * f) / (f - n);

		return result;
	}


	inline Matrix4 HPM_CALL PerspectiveLH(f32 fovDeg, f32 aspectRatio, f32 n, f32 f)
	{
		Matrix4 result = {};

		f32 tanHalfFov = Tan(ToRadians(fovDeg / 2.0f));

		result._11 = 1.0f / (aspectRatio * tanHalfFov);
		result._22 = 1.0f / tanHalfFov;
		result._33 = f / (f - n);
		result._43 = 1.0f;
		result._34 = (-n * f) / (f - n);

		return result;
	}

	inline Matrix4 HPM_CALL Translation(Vector3 trans)
	{
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

	inline Vector3 HPM_CALL GetPosition(Matrix4* m)
	{
		return Vector3{m->_14, m->_24, m->_34};
	}

	inline Matrix4 HPM_CALL Translate(Matrix4 mtx, Vector3 trans)
	{
		mtx.columns[3] = AddV4V4(AddV4V4(MulV4F32(mtx.columns[0], trans.x), MulV4F32(mtx.columns[1], trans.y)), 
								 AddV4V4(MulV4F32(mtx.columns[2], trans.z), mtx.columns[3]));
		return mtx;
	}

	inline Matrix4 HPM_CALL Scaling(Vector3 scale)
	{
		Matrix4 result = {};
		result._11 = scale.x;
		result._22 = scale.y;
		result._33 = scale.z;
		result._44 = 1.0f;

		return result;
	}

	inline Matrix4 HPM_CALL Scale(Matrix4 mtx, Vector3 scale)
	{
		mtx.columns[0] = MulV4F32(mtx.columns[0], scale.x);
		mtx.columns[1] = MulV4F32(mtx.columns[1], scale.y);
		mtx.columns[2] = MulV4F32(mtx.columns[2], scale.z);
		return mtx;
	}

	inline Vector3 HPM_CALL MulM3V3(Matrix3 m, Vector3 v)
	{
		Vector3 r;
		r.x = m._11 * v.x + m._12 * v.y + m._13 * v.z;
		r.y = m._21 * v.x + m._22 * v.y + m._23 * v.z;
		r.z = m._31 * v.x + m._32 * v.y + m._33 * v.z;
		return r;
	}

	inline Vector4 HPM_CALL MulM4V4(Matrix4 m, Vector4 v)
	{
		Vector4 r;
		r.x = m._11 * v.x + m._12 * v.y + m._13 * v.z + m._14 * v.w;
		r.y = m._21 * v.x + m._22 * v.y + m._23 * v.z + m._24 * v.w;
		r.z = m._31 * v.x + m._32 * v.y + m._33 * v.z + m._34 * v.w;
		r.w = m._41 * v.x + m._42 * v.y + m._43 * v.z + m._44 * v.w;
		return r;
	}

	b32 Inverse(Matrix3* m);
	
#if defined(HYPERMATH_IMPL)

	b32 Inverse(Matrix3* m)
	{
		b32 result = false;
		f32 a11 = m->_22 * m->_33 - m->_23 * m->_32; 
		f32 a12 = - (m->_21 * m->_33 - m->_23 * m->_31);
		f32 a13 = m->_21 * m->_32 - m->_22 * m->_31;
		
		f32 det = (m->_11 * a11 + m->_12 * a12 + m->_13 * a13);
		// TODO: define this in hypemath
		if (AbsF32(det) > FLOAT_EPS)
		{
					
			f32 a21 = - (m->_12 * m->_33 - m->_32 * m->_13);
			f32 a22 = m->_11 * m->_33 - m->_13 * m->_31;
			f32 a23 = - (m->_11 * m->_32 - m->_12 * m->_31);
		
			f32 a31 = m->_12 * m->_23 - m->_22 * m->_13;
			f32 a32 = - (m->_11 * m->_23 - m->_21 * m->_13);
			f32 a33 = m->_11 * m->_22 - m->_21 * m->_12;
						
			f32 oneOverDet = 1.0f / det;

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

			*m = inv;
			result = true;
		}

		return result;
	}

#endif

	Matrix4 HPM_CALL Inverse(Matrix4 m);

#if defined(HYPERMATH_IMPL)
	
	Matrix4 HPM_CALL Inverse(Matrix4 m)
	{
		f32 a11_22 = m._33 * m._44 - m._34 * m._43;
		f32 a11_23 = m._32 * m._44 - m._34 * m._42;
		f32 a11_24 = m._32 * m._43 - m._33 * m._42;

		f32 A11 = m._22 * a11_22 - m._23 * a11_23 + m._24 * a11_24;

		f32 a12_21 = m._33 * m._44 - m._34 * m._43;
		f32 a12_23 = m._31 * m._44 - m._34 * m._41;
		f32 a12_24 = m._31 * m._43 - m._33 * m._41;

		f32 A12 = -(m._21 * a12_21 - m._23 * a12_23 + m._24 * a12_24);

		f32 a13_21 = m._32 * m._44 - m._34 * m._42;
		f32 a13_22 = m._31 * m._44 - m._34 * m._41;
		f32 a13_24 = m._31 * m._42 - m._32 * m._41;

		f32 A13 = m._21 * a13_21 - m._22 * a13_22 + m._24 * a13_24;

		f32 a14_21 = m._32 * m._43 - m._33 * m._42;
		f32 a14_22 = m._31 * m._43 - m._33 * m._41;
		f32 a14_23 = m._31 * m._42 - m._32 * m._41;

		f32 A14 = -(m._21 * a14_21 - m._22 * a14_22 + m._23 * a14_23);

		f32 a21_12 = m._33 * m._44 - m._34 * m._43;
		f32 a21_13 = m._32 * m._44 - m._34 * m._42;
		f32 a21_14 = m._32 * m._43 - m._33 * m._42;

		f32 A21 = -(m._12 * a21_12 - m._13 * a21_13 + m._14 * a21_14);

		f32 a22_11 = m._33 * m._44 - m._34 * m._43;
		f32 a22_13 = m._31 * m._44 - m._34 * m._41;
		f32 a22_14 = m._31 * m._43 - m._33 * m._41;

		f32 A22 = m._11 * a22_11 - m._13 * a22_13 + m._14 * a22_14;

		f32 a23_11 = m._32 * m._44 - m._34 * m._42;
		f32 a23_12 = m._31 * m._44 - m._34 * m._41;
		f32 a23_14 = m._31 * m._42 - m._32 * m._41;

		f32 A23 = -(m._11 * a23_11 - m._12 * a23_12 + m._14 * a23_14);

		f32 a24_11 = m._32 * m._43 - m._33 * m._42;
		f32 a24_12 = m._31 * m._43 - m._33 * m._41;
		f32 a24_13 = m._31 * m._42 - m._32 * m._41;

		f32 A24 = m._11 * a24_11 - m._12 * a24_12 + m._13 * a24_13;

		f32 a31_12 = m._23 * m._44 - m._24 * m._43;
		f32 a31_13 = m._22 * m._44 - m._24 * m._42;
		f32 a31_14 = m._22 * m._43 - m._23 * m._42;

		f32 A31 = m._12 * a31_12 - m._13 * a31_13 + m._14 * a31_14;

		f32 a32_11 = m._23 * m._44 - m._24 * m._43;
		f32 a32_13 = m._21 * m._44 - m._24 * m._41;
		f32 a32_14 = m._21 * m._43 - m._23 * m._41;

		f32 A32 = -(m._11 * a32_11 - m._13 * a32_13 + m._14 * a32_14);

		f32 a33_11 = m._22 * m._44 - m._24 * m._42;
		f32 a33_12 = m._21 * m._44 - m._24 * m._41;
		f32 a33_14 = m._21 * m._42 - m._22 * m._41;

		f32 A33 = m._11 * a33_11 - m._12 * a33_12 + m._14 * a33_14;

		f32 a34_11 = m._22 * m._43 - m._23 * m._42;
		f32 a34_12 = m._21 * m._43 - m._23 * m._41;
		f32 a34_13 = m._21 * m._42 - m._22 * m._41;

		f32 A34 = -(m._11 * a34_11 - m._12 * a34_12 + m._13 * a34_13);

		f32 a41_12 = m._23 * m._34 - m._24 * m._33;
		f32 a41_13 = m._22 * m._34 - m._24 * m._32;
		f32 a41_14 = m._22 * m._33 - m._23 * m._32;

		f32 A41 = -(m._12 * a41_12 - m._13 * a41_13 + m._14 * a41_14);

		f32 a42_11 = m._23 * m._34 - m._24 * m._33;
		f32 a42_13 = m._21 * m._34 - m._24 * m._31;
		f32 a42_14 = m._21 * m._33 - m._23 * m._31;

		f32 A42 = m._11 * a42_11 - m._13 * a42_13 + m._14 * a42_14;

		f32 a43_11 = m._22 * m._34 - m._24 * m._32;
		f32 a43_12 = m._21 * m._34 - m._24 * m._31;
		f32 a43_14 = m._21 * m._32 - m._22 * m._31;

		f32 A43 = -(m._11 * a43_11 - m._12 * a43_12 + m._14 * a43_14);

		f32 a44_11 = m._22 * m._33 - m._23 * m._32;
		f32 a44_12 = m._21 * m._33 - m._23 * m._31;
		f32 a44_13 = m._21 * m._32 - m._22 * m._31;

		f32 A44 = m._11 * a44_11 - m._12 * a44_12 + m._13 * a44_13;

		f32 det = m._11 * A11 + m._12 * A12 + m._13 * A13 + m._14 * A14;

		// NOTE: Quietly return an indetity matix if no inverse matrix exist
		if (det == 0) {
			return Identity4();
		}
		else
		{
			f32 oneOverDet = 1.0f / det;
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

	f32 HPM_CALL Determinant(Matrix4 m);

#if defined(HYPERMATH_IMPL)

	f32 HPM_CALL Determinant(Matrix4 m)
	{
		f32 minor1 = m._11 * (m._22 * m._33 * m._44 - m._22 * m._34 * m._43
								  - m._23 * m._32 * m._44 + m._23 * m._34 * m._42
								  + m._24 * m._32 * m._43 - m._24 * m._33 * m._42);

		f32 minor2 = m._12 * (m._21 * m._33 * m._44 - m._21 * m._34 * m._43
								  - m._23 * m._31 * m._44 + m._23 * m._41 * m._34
								  +m._24 * m._31 * m._43 - m._24 * m._33 * m._41);

		f32 minor3 = m._13 * (m._21 * m._32 * m._44 - m._21 * m._34 * m._42
								  - m._22 * m._31 * m._44 + m._22 * m._34 * m._41
								  + m._24 * m._31 * m._42 - m._24 * m._32 * m._41);

		f32 minor4 = m._14 * (m._21 * m._32 * m._43 - m._21 * m._33 * m._42
								  - m._22 * m._31 * m._43 + m._22 * m._41 * m._33
								  + m._23 * m._31 * m._42 - m._23 * m._32 * m._41);

		return minor1 - minor2 + minor3 - minor4;
	}

#endif

	Matrix4 HPM_CALL LookAtRH(Vector3 from, Vector3 at, Vector3 up);
	Matrix4 HPM_CALL LookAtLH(Vector3 from, Vector3 at, Vector3 up);
	Matrix4 HPM_CALL LookAtDirRH(Vector3 position, Vector3 dir, Vector3 up);
#if defined(HYPERMATH_IMPL) 
	
	Matrix4 HPM_CALL LookAtRH(Vector3 from, Vector3 at, Vector3 up)
	{
		Vector3 zAxis = Normalize(SubV3V3(from, at));
		Vector3 xAxis = Normalize(Cross(up, zAxis)); 
		Vector3 yAxis = Cross(zAxis, xAxis);

		Matrix4 result;
		result._11 = xAxis.x;
		result._12 = xAxis.y;
		result._13 = xAxis.z;
		result._14 = -Dot(xAxis, from);

		result._21 = yAxis.x;
		result._22 = yAxis.y;
		result._23 = yAxis.z;
		result._24 = -Dot(yAxis, from);

		result._31 = zAxis.x;
		result._32 = zAxis.y;
		result._33 = zAxis.z;
		result._34 = -Dot(zAxis, from);

		result._41 = 0.0f;
		result._42 = 0.0f;
		result._43 = 0.0f;
		result._44 = 1.0f;

		return result;
	}

	
	Matrix4 HPM_CALL LookAtLH(Vector3 from, Vector3 at, Vector3 up)
	{
		Vector3 zAxis = Normalize(SubV3V3(at, from));
		Vector3 xAxis = Normalize(Cross(up, zAxis)); 
		Vector3 yAxis = Cross(zAxis, xAxis);

		Matrix4 result;
		result._11 = xAxis.x;
		result._12 = xAxis.y;
		result._13 = xAxis.z;
		result._14 = -Dot(xAxis, from);

		result._21 = yAxis.x;
		result._22 = yAxis.y;
		result._23 = yAxis.z;
		result._24 = -Dot(yAxis, from);

		result._31 = zAxis.x;
		result._32 = zAxis.y;
		result._33 = zAxis.z;
		result._34 = -Dot(zAxis, from);

		result._41 = 0.0f;
		result._42 = 0.0f;
		result._43 = 0.0f;
		result._44 = 1.0f;

		return result;
	}


	Matrix4 HPM_CALL LookAtDirRH(Vector3 position, Vector3 dir, Vector3 up)
	{
		Vector3 zAxis = Normalize(dir);
		Vector3 xAxis = Normalize(Cross(zAxis, up));
		Vector3 yAxis = Cross(xAxis, zAxis);

		Matrix4 result;
		result._11 = xAxis.x;
		result._12 = xAxis.y;
		result._13 = xAxis.z;
		result._14 = -Dot(xAxis, position);

		result._21 = yAxis.x;
		result._22 = yAxis.y;
		result._23 = yAxis.z;
		result._24 = -Dot(yAxis, position);

		result._31 = -zAxis.x;
		result._32 = -zAxis.y;
		result._33 = -zAxis.z;
		result._34 = Dot(zAxis, position);

		result._41 = 0.0f;
		result._42 = 0.0f;
		result._43 = 0.0f;
		result._44 = 1.0f;

		return result;
	}


#endif

	Matrix4 HPM_CALL Rotation(f32 angleDeg, Vector3 axis);

#if defined(HYPERMATH_IMPL)

	Matrix4 HPM_CALL Rotation(f32 angleDeg, Vector3 axis)
	{
		Matrix4 result = {};

		f32 c = Cos(ToRadians(angleDeg));
		f32 oc = 1.0f - c;
		f32 s = Sin(ToRadians(angleDeg));
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

	Matrix4 HPM_CALL Rotate(Matrix4 mtx, f32 angleDeg, Vector3 axis);

#if defined(HYPERMATH_IMPL)

	Matrix4 HPM_CALL Rotate(Matrix4 mtx, f32 angleDeg, Vector3 axis)
	{
		Matrix4 rotation;

		f32 c = Cos(ToRadians(angleDeg));
		f32 oc = 1.0f - c;
		f32 s = Sin(ToRadians(angleDeg));
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
	Matrix3 HPM_CALL MulM3M3(Matrix3 left, Matrix3 right)
	{
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
	
	Matrix4 HPM_CALL MulM4M4(Matrix4 left, Matrix4 right)
	{
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

	inline HPM_CALL Quaternion Quat(f32 x, f32 y, f32 z, f32 w)
	{
		return Quaternion{x, y, z, w};
	}

	inline HPM_CALL Quaternion Quat(Vector3 v)
	{
		return Quaternion{v.x, v.y, v.z, 0.0f};
	}

	inline HPM_CALL Quaternion QuatFromAxisAngle(Vector3 axis, f32 angleRad)
	{
		Vector3 a = MulV3F32(axis, Sin(angleRad / 2.0f));
		//a = Normalize(a);
		return Quaternion{a.x, a.y, a.z, Cos(angleRad / 2.0f)};
	}

	inline HPM_CALL Quaternion QuatFromEuler(f32 yaw, f32 pitch, f32 roll)
	{
		f32 cy = Cos(yaw * 0.5f);
		f32 cp = Cos(pitch * 0.5f);
		f32 cr = Cos(roll * 0.5f);

		f32 sy = Sin(yaw * 0.5f);
		f32 sp = Sin(pitch * 0.5f);
		f32 sr = Sin(roll * 0.5f);

		Quaternion result;
		result.x = cy * cp * sr - sy * sp * cr;
		result.y = sy * cp * sr + cy * sp * cr;
		result.z = sy * cp * cr - cy * sp * sr;
		result.w = cy * cp * cr + sy * sp * sr;
		
		return result;
	}

	inline HPM_CALL Quaternion IdentityQuat()
	{
		return Quaternion{0.0f, 0.0f, 0.0f, 1.0f};
	}

	inline HPM_CALL Quaternion MulQQ(Quaternion l, Quaternion r)
	{
		Quaternion result;
		result.x = (l.x * r.w) + (l.y * r.z) - (l.z * r.y) + (l.w * r.x);
		result.y = (-l.x * r.z) + (l.y * r.w) + (l.z * r.x) + (l.w * r.y);
		result.z = (l.x * r.y) - (l.y * r.x) + (l.z * r.w) + (l.w * r.z);
		result.w = (-l.x * r.x) - (l.y * r.y) - (l.z * r.z) + (l.w * r.w);
		return result;
	}

	inline HPM_CALL Vector3 Rotate(Quaternion q, Vector3 v)
	{
		Quaternion conj = Quaternion{-q.x, -q.y, -q.z, q.w};

		Quaternion premul = MulQQ(q, Quat(v));
		Quaternion result = MulQQ(premul, conj);

		return V3(result);
	}

	inline HPM_CALL Quaternion Conjugate(Quaternion q)
	{
		return Quaternion{-q.x, -q.y, -q.z, q.w};
	}

	inline HPM_CALL Matrix3 QuatToM3x3(Quaternion q)
	{
		f32 xSq = q.x * q.x * 2.0f;
		f32 ySq = q.y * q.y * 2.0f;
		f32 zSq = q.z * q.z * 2.0f;
		
		Matrix3 result = {};
		result._11 = 1.0f - ySq - zSq;
		result._12 = 2.0f * q.x * q.y + 2.0f * q.w * q.z;
		result._13 = 2.0f * q.x * q.z - 2.0f * q.y * q.w;
		result._21 = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
		result._22 = 1.0f - xSq - zSq;
		result._23 = 2.0f * q.y * q.z + 2.0f * q.x * q.w;
		result._31 = 2.0f * q.x * q.z + 2.0f * q.y * q.w;
		result._32 = 2.0f * q.y * q.z - 2.0f * q.x * q.w;
		result._33 = 1.0f - xSq - ySq;

		return result;
	}

	inline HPM_CALL f32 HPM_CALL Length(Quaternion q)
	{
		return Sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}

	inline HPM_CALL Quaternion Normalize(Quaternion q)
	{
		f32 len = Length(q);
		Quaternion result;
		if (len > FLOAT_EPS)
		{
			result.x = q.x / len;
			result.y = q.y / len;
			result.z = q.z / len;
			result.w = q.w / len;
		}
		else
		{
			result = q;
		}

		return result;
	}

	inline HPM_CALL Quaternion LerpQuat(Quaternion a, Quaternion b, f32 t)
	{
		Quaternion result;
		if (t < 0.0f)
		{
			result = a;
		}
		else if (t > 1.0f) 
		{
			result = b;
		}
		else
		{
			result.x = (1 - t) * a.x + t * b.x;
			result.y = (1 - t) * a.y + t * b.y;
			result.z = (1 - t) * a.z + t * b.z;
			result.w = (1 - t) * a.w + t * b.w;
			result = Normalize(result);
		}
		return result;
	}	
}
#if defined(HPM_USE_NAMESPACE)
using namespace hpm;
#endif

typedef Vector2 v2;
typedef Vector3 v3;
typedef Vector4 v4;
typedef Matrix3 m3x3;
typedef Matrix4 m4x4;
typedef Quaternion quat;
