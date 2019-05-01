#include "ExtendedMath.h"

namespace AB {

#define Min(v1, v2) (v1 < v2 ? v1 : v2)
#define Max(v1, v2) (v1 > v2 ? v1 : v2)

	BBoxAligned RealignBBoxAligned(BBoxAligned aabb) {
		BBoxAligned realigned = {};

		realigned.min.x = Min(aabb.min.x, aabb.max.x);
		realigned.min.y = Min(aabb.min.y, aabb.max.y);
		realigned.min.z = Min(aabb.min.z, aabb.max.z);

		realigned.max.x = Max(aabb.min.x, aabb.max.x);
		realigned.max.y = Max(aabb.min.y, aabb.max.y);
		realigned.max.z = Max(aabb.min.z, aabb.max.z);

		return realigned;
	}

}
