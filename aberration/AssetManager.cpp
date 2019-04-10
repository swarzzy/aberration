#include "AssetManager.h"
#include "platform/Memory.h"
#include "utils/Log.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Common.h"
#include "FileFormats.h"
#include <vector>
#include "utils/ImageLoader.h"
#include "platform/API/GraphicsAPI.h"

namespace AB {

	AssetManager* AssetInitialize() {
		AssetManager** mgr = &GetMemory()->perm_storage.asset_manager;
		if (!(*mgr)) {
			(*mgr) = (AssetManager*)SysAlloc(sizeof(AssetManager));
		}
		return (*mgr);
	}

	struct VBufferLayout {
		hpm::Vector3 vertex;
		hpm::Vector2 uv;
		hpm::Vector3 normal;
	};

	static uint32 GenAPIVertexBuffer(AssetManager* mgr, byte* mesh_mem_begin, uint64 size, uint32 num_vertices) {
		uint32 vbo_handle;
		GLCall(glGenBuffers(1, &vbo_handle));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo_handle));
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, mesh_mem_begin, GL_STATIC_DRAW));		
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		return vbo_handle;
	}

	static uint32 GenAPIVertexBufferShuffle(AssetManager* mgr, Mesh* mesh) {
		uint32 vbo_handle;
		GLCall(glGenBuffers(1, &vbo_handle));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, vbo_handle));
		GLCall(glBufferData(GL_ARRAY_BUFFER, mesh->mem_size + (8 * mesh->num_vertices), nullptr, GL_STATIC_DRAW));
		VBufferLayout* buffer;
		GLCall(buffer = (VBufferLayout*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
		for (uint32 i = 0; i < mesh->num_vertices; i++) {
			if (mesh->positions)
				buffer[i].vertex = mesh->positions[i];
			if (mesh->uvs)
				buffer[i].uv = mesh->uvs[i];
			if (mesh->normals)
				buffer[i].normal = mesh->normals[i];
		}
		GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		return vbo_handle;
	}

	static uint32 GenAPIIndexBuffer(AssetManager* mgr, uint32* indices, uint64 number_of_indices) {
		uint32 ibo_handle;
		GLCall(glGenBuffers(1, &ibo_handle));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_handle));
		uint64 size = number_of_indices* sizeof(uint32);
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		return ibo_handle;
	}

	int32 AssetCreateMesh(AssetManager* mgr, uint32 number_of_vertices, hpm::Vector3* positions, hpm::Vector2* uvs, hpm::Vector3* normals, uint32 num_of_indices, uint32* indices, Material* material) {
		AB_CORE_ASSERT(number_of_vertices, "Mesh should have more than 0 vertices.");
		AB_CORE_ASSERT(positions, "Cannot create mesh witout vertices.");

		int32 free_index = -1;
		bool32 found = false;
		for (uint32 i = 0; i < MESH_STORAGE_CAPACITY; i++) {
			if (!mgr->mesh_storage_usage[i]) {
				free_index = i; 
				found = true;
				break;
			}
		}

		if (found) {
			mgr->mesh_storage_usage[free_index] = true;

			bool32 has_uvs = uvs != nullptr;
			bool32 has_hormals = normals != nullptr;
			bool32 has_indices = indices != nullptr;
			bool32 has_material = material != nullptr;

			uintptr vert_mem_size = number_of_vertices * sizeof(hpm::Vector3);
			uintptr uv_mem_size = has_uvs ? number_of_vertices * sizeof(hpm::Vector2) : 0;
			uintptr norm_mem_size = has_hormals ? number_of_vertices * sizeof(hpm::Vector3) : 0;
			uintptr ind_mem_size = has_indices ? num_of_indices * sizeof(uint32) : 0;
			uintptr mat_mem_size = sizeof(Material);

			uintptr mem_size = vert_mem_size + uv_mem_size + norm_mem_size + ind_mem_size + mat_mem_size;
			mgr->meshes[free_index].mem_begin = (byte*)malloc(mem_size);
			mgr->meshes[free_index].mem_size = mem_size;

			mgr->meshes[free_index].num_vertices = number_of_vertices;
			mgr->meshes[free_index].num_indices = num_of_indices;

			mgr->meshes[free_index].positions = (hpm::Vector3*)mgr->meshes[free_index].mem_begin;

			byte* uv_beg = has_uvs ? (mgr->meshes[free_index].mem_begin + vert_mem_size) : nullptr;
			byte* norm_beg = has_hormals ? (mgr->meshes[free_index].mem_begin + vert_mem_size + uv_mem_size) : nullptr;
			byte* ind_beg = has_indices ? (mgr->meshes[free_index].mem_begin + vert_mem_size + uv_mem_size + norm_mem_size) : nullptr;
			byte* mat_beg = mgr->meshes[free_index].mem_begin + vert_mem_size + uv_mem_size + norm_mem_size + ind_mem_size;
			
			mgr->meshes[free_index].uvs	= (hpm::Vector2*)uv_beg;
			mgr->meshes[free_index].normals = (hpm::Vector3*)norm_beg;
			mgr->meshes[free_index].indices = (uint32*)ind_beg;
			mgr->meshes[free_index].material = (Material*)mat_beg;

			CopyArray(hpm::Vector3, number_of_vertices, mgr->meshes[free_index].positions, positions);

			if (has_material) {
				CopyScalar(Material, mgr->meshes[free_index].material, material);
			} else {
				Material dummy = {};
				dummy.shininess = 32.0f;
				dummy.ambient = { 0.2f, 0.0f, 0.2f };
				dummy.diffuse = { 0.8f, 0.0f, 0.8f };
				dummy.specular = { 1.0f, 0.0f, 1.0f };
				dummy.emission = {};
				dummy.diff_map_handle = ASSET_INVALID_HANDLE;
				dummy.spec_map_handle = ASSET_INVALID_HANDLE;

				CopyScalar(Material, mgr->meshes[free_index].material, &dummy);
			}
			
			if (has_uvs) {
				CopyArray(hpm::Vector2, number_of_vertices, mgr->meshes[free_index].uvs, uvs);
			}
			if (has_hormals) {
				CopyArray(hpm::Vector3, number_of_vertices, mgr->meshes[free_index].normals, normals);
			}
			if (has_indices) {
				CopyArray(uint32, num_of_indices, mgr->meshes[free_index].indices, indices);
			}

			mgr->meshes[free_index].api_vb_handle = GenAPIVertexBuffer(mgr, mgr->meshes[free_index].mem_begin, mem_size, number_of_vertices);
			//mgr->meshes[free_index].api_vb_handle = GenAPIVertexBufferShuffle(mgr, &mgr->meshes[free_index]);
			AB_CORE_ASSERT(mgr->meshes[free_index].api_vb_handle, "Failed to create vertex buffer");

			if (has_indices) {
				mgr->meshes[free_index].api_ib_handle = GenAPIIndexBuffer(mgr, mgr->meshes[free_index].indices, num_of_indices);
				AB_CORE_ASSERT(mgr->meshes[free_index].api_ib_handle, "Failed to create index buffer");
			} else {
				mgr->meshes[free_index].api_ib_handle = 0;
			}

		} else {
			AB_CORE_ERROR("Failed to load mesh. Storage is full.");
		}

		return free_index;
	}

	int32 AssetCreateMeshAAB(AssetManager * mgr, const char * aab_path) {
		int32 result_handle = ASSET_INVALID_HANDLE;

		auto[_header, header_read] = DebugReadFileOffset(aab_path, 0, sizeof(AABMeshHeader));
		if (_header) {
			AABMeshHeader* header = (AABMeshHeader*)_header;
			if (header->magic_value == AAB_FILE_MAGIC_VALUE && header->version == AAB_FILE_VERSION) {
				uint32 asset_type = header->asset_type;
				if (asset_type == AAB_FILE_TYPE_MESH) {
					uint32 vertices_count = header->vertices_count;
					uint32 normals_count = header->normals_count;
					uint32 uvs_count = header->uvs_count;
					uint32 indices_count = header->indices_count;

					uint64 vertices_offset = header->vertices_offset;
					uint64 normals_offset = header->normals_offset;
					uint64 uvs_offset = header->uvs_offset;
					uint64 indices_offset = header->indices_offset;
					uint64 material_name_offset = header->material_name_offset;
					uint64 material_properties_offset = header->material_properties_offset;
					uint64 material_diff_map_name_offset = header->material_diff_bitmap_name_offset;
					uint64 material_spec_map_name_offset = header->material_spec_bitmap_name_offset;
					
					bool32 has_uv = header->uvs_count;
					bool32 has_material = header->material_name_offset != 0;


					uint64 asset_size = header->asset_size;
					DebugFreeFileMemory(_header);

#pragma warning(push)
#pragma warning(disable:4244) // uint64 to uint32 conversion. Asset will do his job if bad things happens.
					AB_CORE_ASSERT(asset_size <= 0xffffffff, "Can't read >4GB files.");
					auto[asset_data, asset_read] = DebugReadFileOffset(aab_path, vertices_offset, (uint32)asset_size);
#pragma warning(pop)
					if (asset_data) {
						// NOTE: Assume that all data properly alligned
						hpm::Vector3* vertices = (hpm::Vector3*)((byte*)asset_data);
						hpm::Vector3* normals = (hpm::Vector3*)((byte*)asset_data + (normals_offset - vertices_offset));
						hpm::Vector2* uvs = has_uv ? (hpm::Vector2*)((byte*)asset_data + (uvs_offset - vertices_offset) ) : nullptr;
						uint32* indices = (uint32*)((byte*)asset_data + (indices_offset - vertices_offset));
						AABMeshMaterialProperties* material_props = has_material ? (AABMeshMaterialProperties*)((byte*)asset_data + (material_properties_offset - vertices_offset)) : nullptr;

						if (has_material) {
							Material material = {};
							if (material_diff_map_name_offset) {
								char* diff_name_ptr = (char*)((byte*)asset_data + (material_diff_map_name_offset - vertices_offset));
								// TODO: This is all temporary
								char path_buff[512];
								auto[result, written] = GetDirectory(aab_path, path_buff, 256);
								AB_CORE_ASSERT(result, "Too long path.");
								strcat(path_buff, diff_name_ptr);
								material.diff_map_handle = AssetCreateTextureBMP(mgr, path_buff, MapType::Diffuse);
							} else {
								material.diff_map_handle = ASSET_INVALID_HANDLE;
							}
							if (material_spec_map_name_offset) {
								char* spec_name_ptr = (char*)((byte*)asset_data + (material_spec_map_name_offset- vertices_offset));
								// TODO: This is all temporary
								char path_buff[512];
								auto[result, written] = GetDirectory(aab_path, path_buff, 256);
								AB_CORE_ASSERT(result, "Too long path.");
								strcat(path_buff, spec_name_ptr);
								material.spec_map_handle = AssetCreateTextureBMP(mgr, path_buff, MapType::Specular);
							}
							else {
								material.spec_map_handle = ASSET_INVALID_HANDLE;
							}

							material.ambient = material_props->k_a;
							material.diffuse = material_props->k_d;
							material.specular = material_props->k_s;
							material.emission = material_props->k_e;
							material.shininess = material_props->shininess;
							
							result_handle = AssetCreateMesh(mgr, vertices_count, vertices, uvs, normals, indices_count, indices, &material);
						} else {
							result_handle = AssetCreateMesh(mgr, vertices_count, vertices, uvs, normals, indices_count, indices, nullptr);
						}

					} else {
						AB_CORE_ERROR("Failed to read seet data");
					}
				}
				else {
					AB_CORE_ERROR("Asset : %s is not a mesh.", aab_path);
				}
			}
			else {
				AB_CORE_ERROR("Unknown file format in file: %s", aab_path);
			}
		} else {
			AB_CORE_ERROR("Failed to read file: %s", aab_path);
		}
		
		return result_handle;
	}

	Mesh* AssetGetMeshData(AssetManager* mgr, int32 mesh_handle) {
		if (mesh_handle != ASSET_INVALID_HANDLE && mesh_handle < MESH_STORAGE_CAPACITY) {
			return &mgr->meshes[mesh_handle];
		} else {
			return nullptr;
		}
	}

	Texture* AssetGetTextureData(AssetManager* mgr, int32 texture_handle) {
		if (texture_handle != ASSET_INVALID_HANDLE && texture_handle < TEXTURE_STORAGE_CAPACITY) {
			return &mgr->textures[texture_handle];
		} else {
			return nullptr;
		}
	}

	int32 AssetCreateTexture(AssetManager* mgr, byte* bitmap, uint16 w, uint16 h,
							 uint32 bits_per_pixel, const char* name,
							 MapType mapType) {
		int32 result_handle = ASSET_INVALID_HANDLE;
		int32 free_index = ASSET_INVALID_HANDLE;
		bool32 found = false;
		for (uint32 i = 0; i < TEXTURE_STORAGE_CAPACITY; i++) {
			if (!mgr->texture_storage_usage[i]) {
				free_index = i;
				found = true;
				break;
			}
		}

		if (found) {
			uint64 bitmap_size = w * h * (bits_per_pixel / 8);
			uint64 name_size = strlen(name) + 1;
			uint64 mem_size = bitmap_size + name_size;
			void* mem_ptr = malloc(mem_size);
			if (mem_ptr) {
				auto* tx = &mgr->textures[free_index];
				tx->mem_size = mem_size;
				// TODO: strict aliasing might create problems here?
				tx->mem_begin = (byte*)mem_ptr;
				tx->bitmap = (byte*)mem_ptr;
				tx->name = (char*)(tx->mem_begin + bitmap_size);

				CopyArray(byte, bitmap_size, tx->bitmap, bitmap);
				CopyArray(char, name_size , tx->name, name);
				API::ColorSpace cs;
				if (mapType == MapType::Diffuse) {
					cs = API::TEX_COLOR_SPACE_SRGB;
				} else {
					cs = API::TEX_COLOR_SPACE_LINEAR;
				}
				
				API::TextureFormat format = API::GetTextureFormat(bits_per_pixel, cs);
				
				API::TextureParameters p = {
					API::TEX_FILTER_LINEAR,
					API::TEX_WRAP_CLAMP_TO_EDGE,
				};
				tx->api_handle = API::CreateTexture(p, format, w, h, bitmap);
				if (!tx->api_handle) {
					AB_CORE_ERROR("Texture loading error. OpenGL API Error. Texture: %s", name);
				}

				mgr->texture_storage_usage[free_index] = true;
				result_handle = free_index;
			}
			else {
				AB_CORE_ERROR("Failed to allocate block with size: %u64", mem_size);
			}
		} else {
			AB_CORE_ERROR("Failed to create texture. Storage is full");
		}
		return result_handle;
	}

	int32 AssetCreateTextureBMP(AssetManager* mgr, const char* bmp_path,
								MapType mapType) {
		API::ColorSpace cs;
		if (mapType == MapType::Diffuse) {
			cs = API::TEX_COLOR_SPACE_SRGB;
		} else {
			cs = API::TEX_COLOR_SPACE_LINEAR;			
		}
		
		Image bmp = LoadBMP(bmp_path, cs);

		if (bmp.bitmap) {
			char name_buf[256];
			auto[result, written] = GetFilenameFromPath(bmp_path, name_buf, 256);
			// TODO: this in only test code
			AB_CORE_ASSERT(result, "Too long filename: %s", bmp_path);
			return AssetCreateTexture(mgr, bmp.bitmap, bmp.width, bmp.height,
									  bmp.bit_per_pixel, name_buf, mapType);
		} else {
			AB_CORE_ERROR("Faied to create texture forom file: %s", bmp_path);
			return ASSET_INVALID_HANDLE;
		}
	}

}
