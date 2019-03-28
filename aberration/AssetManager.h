#pragma once
#include "AB.h"
#include <hypermath.h>

namespace AB {
	constexpr uint32 MESH_STORAGE_CAPACITY = 128;
	constexpr uint32 TEXTURE_STORAGE_CAPACITY = 128;
	constexpr int32 ASSET_INVALID_HANDLE = -1;

	struct Material {
		hpm::Vector3 ambient;
		hpm::Vector3 diffuse;
		hpm::Vector3 specular;
		hpm::Vector3 emission;
		float32 shininess;
		int32 diff_map_handle;
		int32 spec_map_handle;
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

	// TODO: Hash map for associating texture names and handles

	struct Texture {
		uint32 api_handle;
		uint64 mem_size;
		byte* mem_begin;
		byte* bitmap;
		char* name;
	};

	struct AssetManager {
		byte mesh_storage_usage[MESH_STORAGE_CAPACITY];
		byte texture_storage_usage[TEXTURE_STORAGE_CAPACITY];
		Mesh meshes[MESH_STORAGE_CAPACITY];
		Texture textures[TEXTURE_STORAGE_CAPACITY];
	};

	enum class TextureFormat : byte {
		RED,
		RGB,
		RGBA
	};

	AB_API AssetManager* AssetInitialize();
	AB_API int32 AssetCreateTexture(AssetManager* mgr, byte* bitmap, uint16 w, uint16 h, uint32 bits_per_pixel, const char* name);
	AB_API int32 AssetCreateTextureBMP(AssetManager* mgr, const char* bmp_path);
	AB_API int32 AssetCreateMesh(AssetManager* mgr, uint32 number_of_vertices, hpm::Vector3* positions, hpm::Vector2* uvs, hpm::Vector3* normals, uint32 num_of_indices, uint32* indices, Material* material);
	AB_API int32 AssetCreateMeshAAB(AssetManager* mgr, const char* aab_path);
	Mesh* AssetGetMeshData(AssetManager* mgr, int32 mesh_handle);
	Texture* AssetGetTextureData(AssetManager* mgr, int32 texture_handle);
}
