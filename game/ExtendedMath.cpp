#include "ExtendedMath.h"

namespace AB
{

#define Min(v1, v2) (v1 < v2 ? v1 : v2)
#define Max(v1, v2) (v1 > v2 ? v1 : v2)

	BBoxAligned RealignBBoxAligned(BBoxAligned aabb)
	{
		BBoxAligned realigned = {};

		realigned.min.x = Min(aabb.min.x, aabb.max.x);
		realigned.min.y = Min(aabb.min.y, aabb.max.y);
		realigned.min.z = Min(aabb.min.z, aabb.max.z);

		realigned.max.x = Max(aabb.min.x, aabb.max.x);
		realigned.max.y = Max(aabb.min.y, aabb.max.y);
		realigned.max.z = Max(aabb.min.z, aabb.max.z);

		return realigned;
	}

	// NOTE: Implementation based on:
	// https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
	Frustum FrustumFromProjRH(const m4x4* perspMtx)
	{
		Frustum result = {};
		const m4x4* m = perspMtx;

		result.leftPlane.a = m->_41 + m->_11;
		result.leftPlane.b = m->_42 + m->_12;
		result.leftPlane.c = m->_43 + m->_13;
		result.leftPlane.d = m->_44 + m->_14;

		result.rightPlane.a = m->_41 - m->_11;
		result.rightPlane.b = m->_42 - m->_12;
		result.rightPlane.c = m->_43 - m->_13;
		result.rightPlane.d = m->_44 - m->_14;

		result.topPlane.a = m->_41 - m->_21;
		result.topPlane.b = m->_42 - m->_22;
		result.topPlane.c = m->_43 - m->_23;
		result.topPlane.d = m->_44 - m->_24;

		result.bottomPlane.a = m->_41 + m->_21;
		result.bottomPlane.b = m->_42 + m->_22;
		result.bottomPlane.c = m->_43 + m->_23;
		result.bottomPlane.d = m->_44 + m->_24;

		result.farPlane.a = m->_41 - m->_31;
		result.farPlane.b = m->_42 - m->_32;
		result.farPlane.c = m->_43 - m->_33;
		result.farPlane.d = m->_44 - m->_34;

		result.nearPlane.a = m->_41 + m->_31;
		result.nearPlane.b = m->_42 + m->_32;
		result.nearPlane.c = m->_43 + m->_33;
		result.nearPlane.d = m->_44 + m->_34;

		result.leftPlane = Normalize(result.leftPlane);
		result.rightPlane = Normalize(result.rightPlane);
		result.farPlane = Normalize(result.farPlane);
		result.nearPlane = Normalize(result.nearPlane);
		result.topPlane = Normalize(result.topPlane);
		result.bottomPlane = Normalize(result.bottomPlane);
		
		return result;
	}
		
	b32 GenFrustumVertices(const Frustum* f, FrustumVertices* v)
	{
		b32 nlb = IntersectPlanes3(f->leftPlane, f->nearPlane,
								   f->bottomPlane, &v->nearLeftBottom);
		
		b32 nlt = IntersectPlanes3(f->leftPlane, f->nearPlane,
								   f->topPlane, &v->nearLeftTop);

		b32 nrb = IntersectPlanes3(f->rightPlane, f->nearPlane,
								   f->bottomPlane, &v->nearRightBottom);

		b32 nrt = IntersectPlanes3(f->rightPlane, f->nearPlane,
								   f->topPlane, &v->nearRightTop);

		b32 flb = IntersectPlanes3(f->leftPlane, f->farPlane,
								   f->bottomPlane, &v->farLeftBottom);

		b32 flt = IntersectPlanes3(f->leftPlane, f->farPlane,
								   f->topPlane, &v->farLeftTop);

		b32 frb = IntersectPlanes3(f->rightPlane, f->farPlane,
								   f->bottomPlane, &v->farRightBottom);

		b32 frt = IntersectPlanes3(f->rightPlane, f->farPlane,
								   f->topPlane, &v->farRightTop);

		return nlb && nlt && nrb && nrt && flb && flt && frb && frt;
	}

}
