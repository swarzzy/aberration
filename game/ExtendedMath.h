#pragma once
#include "Shared.h"
#include <hypermath.h>

namespace AB
{
	struct Rectangle
	{
		v2 min;
		v2 max;
	};
		
	struct Transform
	{
		// Quat rotation;
		m4x4 worldMatrix;
	};

	struct BBoxAligned
	{
		v3 min;
		v3 max;
	};

	union Plane
	{
		struct
		{
			f32 a, b, c, d;
		};
		
		struct
		{
			Vector3 normal;
			f32 _d;
		};
	};

	struct Frustum
	{
		Plane farPlane;
		Plane leftPlane;
		Plane rightPlane;
		Plane topPlane;
		Plane bottomPlane;
		Plane nearPlane;
	};

	union v3i
	{
		struct
		{
			i32 x, y, z;
		};
	};

	inline v3i V3I(i32 x, i32 y, i32 z)
	{
		v3i result;
		result.x = x;
		result.y = y;
		result.z = z;
		return result;
	}

	union v2u
	{
		struct
		{
			u32 x, y;
		};
	};

	inline Rectangle RectLeftCornerDim(v2 leftCorner, v2 dim)
	{
		Rectangle result;
		result.min = leftCorner;
		result.max = leftCorner + dim;
		return result;
	}

	BBoxAligned RealignBBoxAligned(BBoxAligned aabb);
	Frustum FrustumFromProjRH(const m4x4* perspMtx);

	b32 IntersectPlanes3(Plane p1, Plane p2, Plane p3, v3* out);

	inline Plane Normalize(Plane p)
	{
		Plane result = p;
		f32 length = Length(p.normal);
		result.normal /= length;
		result.d /= length;
		return result;
	}

	inline b32 IntersectPlanes3(Plane p1, Plane p2, Plane p3, v3* out)
	{
		b32 result = false;
		m3x3 a = {};
		a._11 = p1.normal.x;
		a._12 = p1.normal.y;
		a._13 = p1.normal.z;
		a._21 = p2.normal.x;
		a._22 = p2.normal.y;
		a._23 = p2.normal.z;
		a._31 = p3.normal.x;
		a._32 = p3.normal.y;
		a._33 = p3.normal.z;

		v3 b = V3(p1.d, p2.d, p3.d);

		if (Inverse(&a))
		{
			*out = MulM3V3(a, b);
			result = true;
		}

		return result;
	}


	inline b32 Contains(Rectangle rect, v2 point)
	{
		return	point.x > rect.min.x && 
			point.y > rect.min.y && 
			point.x < rect.max.x &&
			point.y < rect.max.y;
	}
}
