#include "AssetManager.h"
#include "platform/Memory.h"
#include "utils/Log.h"
#include "platform/API/OpenGL/ABOpenGL.h"
#include "platform/Platform.h"
#include "ABFileFormats.h"
#include <vector>

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

	int32 AssetCreateMesh(AssetManager* mgr, uint32 number_of_vertices, hpm::Vector3* positions, hpm::Vector2* uvs, hpm::Vector3* normals, uint32 num_of_indices, uint32* indices) {
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

			uintptr vert_mem_size = number_of_vertices * sizeof(hpm::Vector3);
			uintptr uv_mem_size = has_uvs ? number_of_vertices * sizeof(hpm::Vector2) : 0;
			uintptr norm_mem_size = has_hormals ? number_of_vertices * sizeof(hpm::Vector3) : 0;
			uintptr ind_mem_size = has_indices ? num_of_indices * sizeof(uint32) : 0;

			uintptr mem_size = vert_mem_size + uv_mem_size + norm_mem_size + ind_mem_size;
			mgr->meshes[free_index].mem_begin = (byte*)malloc(mem_size);
			mgr->meshes[free_index].mem_size = mem_size;

			mgr->meshes[free_index].num_vertices = number_of_vertices;
			mgr->meshes[free_index].num_indices = num_of_indices;

			mgr->meshes[free_index].positions = (hpm::Vector3*)mgr->meshes[free_index].mem_begin;

			byte* uv_beg = has_uvs ? (mgr->meshes[free_index].mem_begin + vert_mem_size) : nullptr;
			byte* norm_beg = has_hormals ? (mgr->meshes[free_index].mem_begin + vert_mem_size + uv_mem_size) : nullptr;
			byte* ind_beg = has_indices ? (mgr->meshes[free_index].mem_begin + vert_mem_size + uv_mem_size + norm_mem_size) : nullptr;
			
			mgr->meshes[free_index].uvs	= (hpm::Vector2*)uv_beg;
			mgr->meshes[free_index].normals = (hpm::Vector3*)norm_beg;
			mgr->meshes[free_index].indices = (uint32*)ind_beg;

			CopyArray(hpm::Vector3, number_of_vertices, mgr->meshes[free_index].positions, positions);
			
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

	struct ParsePositionRet {
		hpm::Vector3 position;
		char* next;
	};

	ParsePositionRet ParsePosition(char* at) {  // after v
		char* next;
		float32 x = strtof(at, &next);
		AB_CORE_ASSERT(at != next, "Failed to read position");
		at = next;
		float32 y = strtof(at, &next);
		AB_CORE_ASSERT(at != next, "Failed to read position");
		at = next;
		float32 z = strtof(at, &next);
		AB_CORE_ASSERT(at != next, "Failed to read position");

		return { hpm::Vector3{ x, y, z }, next };
	}

	struct ParseFaceRet {
		uint32 vertices[3];
		char* next;
	};

	ParseFaceRet ParseFace (char* at) {  // after v
		char* next;
		ParseFaceRet result;
		result.vertices[0] = strtoul(at, &next, 10) - 1;
		AB_CORE_ASSERT(at != next, "Failed to read face indices");
		at = next;
		result.vertices[1] = strtoul (at, &next, 10) - 1;
		AB_CORE_ASSERT(at != next, "Failed to read face indices");
		at = next;
		result.vertices[2] = strtoul(at, &next, 10) - 1;
		AB_CORE_ASSERT(at != next, "Failed to read face indices");
		at = next;

		strtoul(at, &next, 10);
		AB_CORE_ASSERT(at == next, "Failed to read face indices. This model has non triangle faces");
		result.next = at;
		return result;
	}

	void GenNormals(std::vector<hpm::Vector3>& vertices, std::vector<uint32>& indices, std::vector<hpm::Vector3>& normals) {
		AB_CORE_ASSERT(indices.size() % 3 == 0, "Wrong number of indices");
		for (uint32 i = 0; i < indices.size(); i+= 3) {
			hpm::Vector3 first = hpm::Subtract(vertices[indices[i + 1]], vertices[indices[i]]);
			hpm::Vector3 second = hpm::Subtract(vertices[indices[i + 2]], vertices[indices[i]]);
			hpm::Vector3 normal = hpm::Normalize(hpm::Cross(first, second));
			normals.push_back(normal);
			normals.push_back(normal);
			normals.push_back(normal);
		}
	}

	int32 AssetCreateMeshOBJ(AssetManager* mgr, const char* obj_path) {
		uint32 read = 0;
		char* data = (char*)DebugReadFile(obj_path, &read);
		AB_CORE_ASSERT(data, "Failed to read file: %s", obj_path);
		std::vector<hpm::Vector3> tmp_vertices;
		std::vector<uint32> tmp_indices;
		std::vector<hpm::Vector3> tmp_normals;
		tmp_vertices.reserve(read);
		tmp_indices.reserve(read);
		tmp_normals.reserve(read);

		for (uint64 i = 0; i < read;) {
			char* at = data + i;
			switch (*at) {
			case 'm': { // mtllib
				while (*(data + i) != '\n') i++;
				i++;
			} break;
			case 'u': { // usemtl
				while (*(data + i) != '\n') i++;
				i++;
			} break;
			case 'g': { // g - group 
				while (*(data + i) != '\n') i++;
				i++;
			} break;
			case '#': { // comment
				while (*(data + i) != '\n') i++;
				i++;
			} break;
			case 'v': { // vertex (position)
				auto[pos, next] = ParsePosition(at + 1);
				i += next - at;
				tmp_vertices.push_back(pos);
			} break;
			case 'f': { // face indices
				auto[ind, next] = ParseFace(at + 1);
				i += next - at;
				tmp_indices.push_back(ind[0]);
				tmp_indices.push_back(ind[1]);
				tmp_indices.push_back(ind[2]);
			} break;
			default: {
				i++;
			} break;
			}
		}
		tmp_vertices.shrink_to_fit();
		tmp_indices.shrink_to_fit();

		DebugFreeFileMemory(data);
		GenNormals(tmp_vertices, tmp_indices, tmp_normals);
		tmp_normals.shrink_to_fit();

		std::vector<hpm::Vector3> vertices;
		std::vector<hpm::Vector3> normals;
		std::vector<uint32> indices;

		for (uint32 i = 0; i < tmp_indices.size(); i += 3) {
			vertices.push_back(tmp_vertices[tmp_indices[i]]);
			vertices.push_back(tmp_vertices[tmp_indices[i + 1]]);
			vertices.push_back(tmp_vertices[tmp_indices[i + 2]]);

			normals.push_back(tmp_normals[i]);
			normals.push_back(tmp_normals[i + 1]);
			normals.push_back(tmp_normals[i + 2]);

			indices.push_back(i);
			indices.push_back(i + 1);
			indices.push_back(i + 2);
		}

		return AssetCreateMesh(mgr, (uint32)vertices.size(), vertices.data(), nullptr, normals.data(), (uint32)indices.size(), indices.data());
	}

	int32 AssetCreateMeshAAB(AssetManager * mgr, const char * aab_path) {
		int32 result_handle = MESH_INVALID_HANDLE;

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

					bool32 has_uv = header->uvs_count;

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

						result_handle = AssetCreateMesh(mgr, vertices_count, vertices, uvs, normals, indices_count, indices);
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
		return &mgr->meshes[mesh_handle];
	}
}
