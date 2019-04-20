#pragma once
#include "AB.h"
#include <hypermath.h>

namespace AB {
	struct Rectangle {
		Vector2 min;
		Vector2 max;
	};
		
	struct Transform {
		// Quat rotation;
		Matrix4 worldMatrix;
	};

	struct BBoxAligned {
		Vector3 min;
		Vector3 max;
	};

	inline bool32 Contains(Rectangle rect, Vector2 point) {
		return	point.x > rect.min.x && 
			point.y > rect.min.y && 
			point.x < rect.max.x &&
			point.y < rect.max.y;
	}

	AB_API BBoxAligned RealignBBoxAligned(BBoxAligned aabb);
}
