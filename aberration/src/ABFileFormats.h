#pragma once

#include "AB.h"
#include "../../hypermath/hypermath.h"

namespace AB {
	enum AABFileType : uint32 {
		AAB_FILE_TYPE_MESH = 0x01020304
	};
	constexpr uint32 AAB_FILE_MAGIC_VALUE = 0xaabaabaa;
	constexpr uint32 AAB_FILE_VERSION = 0;

#pragma pack(push, 1)
	struct AABMeshMaterialProperties {
		hpm::Vector3 k_a;
		hpm::Vector3 k_d;
		hpm::Vector3 k_s;
		hpm::Vector3 k_e;
		float32 shininess;
	};

	struct AABMeshHeader {
		uint32 magic_value;
		uint32 version;
		uint64 asset_size;
		uint32 asset_type;
		uint32 vertices_count;
		uint32 normals_count;
		uint32 uvs_count;
		uint32 indices_count;
		uint64 vertices_offset;
		uint64 normals_offset;
		uint64 uvs_offset;
		uint64 indices_offset;
		uint64 material_name_offset;		// Both are zero if there is no material
		uint64 material_diff_bitmap_name_offset;
		uint64 material_properties_offset;
	};
#pragma pack (pop)
}
