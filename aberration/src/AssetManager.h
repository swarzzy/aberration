#pragma once
#include "AB.h"
#include <hypermath.h>

namespace AB {
	constexpr uint32 MESH_STORAGE_CAPACITY = 128;
	constexpr int32 MESH_INVALID_HANDLE = -1;

	struct Material {
		hpm::Vector3 ambient;
		hpm::Vector3 diffuse;
		hpm::Vector3 specular;
		hpm::Vector3 emission;
		float32 shininess;
		static constexpr uint32 DIFF_BITMAP_NAME_SIZE = 256;
		char diff_bitmap_name[DIFF_BITMAP_NAME_SIZE];
	};

	struct Mesh {
		uint32 api_vb_handle;
		uint32 api_ib_handle;
		uint32 num_vertices;
		uint32 num_indices;
		uint64 mem_size;
		byte* mem_begin;
		hpm::Vector3* positions;
		hpm::Vector2* uvs;
		hpm::Vector3* normals;
		uint32* indices;
		Material* material;
	};

	struct AssetManager {
		byte mesh_storage_usage[MESH_STORAGE_CAPACITY];
		Mesh meshes[MESH_STORAGE_CAPACITY];
	};


	AB_API AssetManager* AssetInitialize();
	AB_API int32 AssetCreateMesh(AssetManager* mgr, uint32 number_of_vertices, hpm::Vector3* positions, hpm::Vector2* uvs, hpm::Vector3* normals, uint32 num_of_indices, uint32* indices, Material* material);
	AB_API int32 AssetCreateMeshOBJ(AssetManager* mgr, const char* obj_path);
	AB_API int32 AssetCreateMeshAAB(AssetManager* mgr, const char* aab_path);
	Mesh* AssetGetMeshData(AssetManager* mgr, int32 mesh_handle);
}
