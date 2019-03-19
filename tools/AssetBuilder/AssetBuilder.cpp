#include <cstdint>
#include <cassert>
#include <vector>

#include "../../aberration/src/ABFileFormats.h"
#include "../../hypermath/hypermath.h"

#include "OBJLoader.cpp"

namespace AB {
	
	struct ABMesh {
		// UVs are optional
		hpm::Vector3* vertices;
		hpm::Vector3* normals;
		hpm::Vector2* uv;

		uint32 num_vertices;
		uint32 num_indices;

		uint32* indices;
		int32 material_index;
	};

	void FreeABMesh(ABMesh* mesh) {
		if (mesh->vertices) {
			free(mesh->vertices);
		}
		if (mesh->normals) {
			free(mesh->normals);
		}
		if (mesh->uv) {
			free(mesh->uv);
		}
		if (mesh->indices) {
			free(mesh->indices);
		}

		*mesh = {};
	}

	ABMesh GenABMesh(Mesh* mesh, std::vector<Material>* material_stack) {
		ABMesh ab_mesh = {};
		assert(mesh->vertices != 0);
		assert(mesh->num_vertex_indices % 3 == 0); // Mesh has non triangle faces

		bool32 generated_normals = false;

		if (!mesh->num_normals) {
			printf("File does not contains normals. Generating normals...\n");
			mesh->normals = (hpm::Vector3*)malloc(sizeof(hpm::Vector3) * mesh->num_vertex_indices);
			mesh->num_normals = GenNormals(mesh->vertices, mesh->num_vertices, mesh->vertex_indices, mesh->num_vertex_indices, mesh->normals);
			generated_normals = true;
		}

		bool32 has_uv = mesh->uv != nullptr;

		hpm::Vector3* vertices = nullptr;
		hpm::Vector3* normals = nullptr;
		hpm::Vector2* uvs = nullptr;
		uint32* indices = nullptr;

		uint64 vertices_at = 0;
		uint64 normals_at = 0;
		uint64 uvs_at = 0;
		uint64 indices_at = 0;

		vertices = (hpm::Vector3*)malloc(mesh->num_vertex_indices * sizeof(hpm::Vector3));
		indices = (uint32*)malloc(mesh->num_vertex_indices * sizeof(uint32));
		if (has_uv) {
			uvs = (hpm::Vector2*)malloc(mesh->num_vertex_indices * sizeof(hpm::Vector2));
		}

		if (generated_normals) {
			normals = mesh->normals;
			normals_at = mesh->num_normals;
		}
		else {
			normals = (hpm::Vector3*)malloc(mesh->num_vertex_indices * sizeof(hpm::Vector3));
		}

		for (uint32 i = 0; i < mesh->num_vertex_indices; i += 3) {
			assert(mesh->vertex_indices[i + 2] < mesh->num_vertices); // Out of range
			if (mesh->uv_indices) {
				assert(mesh->uv_indices[i + 2] < mesh->num_uv); // Out of range
			}
			if (mesh->normal_indices) {
				assert(mesh->normal_indices[i + 2] < mesh->num_normals); // Out of range
			}

			vertices[vertices_at] = mesh->vertices[mesh->vertex_indices[i]];
			vertices[vertices_at + 1] = mesh->vertices[mesh->vertex_indices[i + 1]];
			vertices[vertices_at + 2] = mesh->vertices[mesh->vertex_indices[i + 2]];
			vertices_at += 3;

			if (has_uv) {
				uvs[uvs_at] = mesh->uv[mesh->uv_indices[i]];
				uvs[uvs_at + 1] = mesh->uv[mesh->uv_indices[i + 1]];
				uvs[uvs_at + 2] = mesh->uv[mesh->uv_indices[i + 2]];
				uvs_at += 3;
			}

			if (!generated_normals) {
				normals[normals_at] = mesh->normals[mesh->normal_indices[i]];
				normals[normals_at + 1] = mesh->normals[mesh->normal_indices[i + 1]];
				normals[normals_at + 2] = mesh->normals[mesh->normal_indices[i + 2]];
				normals_at += 3;
			}

			indices[indices_at] = i;
			indices[indices_at + 1] = i + 1;
			indices[indices_at + 2] = i + 2;
			indices_at += 3;
		}

		int32 material_index = -1;
		if (mesh->material_name) { // has material
			// Fetch material
			for (uint32 i = 0; i < material_stack->size(); i++) {
				if (strcmp(mesh->material_name, (*material_stack)[i].name) == 0) {
					material_index = i;
				}
			}
		}

		ab_mesh.vertices = vertices;
		ab_mesh.normals = normals;
		ab_mesh.uv = uvs;
		ab_mesh.indices = indices;
		ab_mesh.num_vertices = (uint32)vertices_at;
		ab_mesh.num_indices = (uint32)indices_at;
		ab_mesh.material_index = material_index;

		return ab_mesh;
	}

	// NOTE: This is crappy temporary solution
	void WriteAABMesh(const char* file_name, ABMesh* mesh, std::vector<Material>* material_stack) {
		// TODO: Temporary: writing matrial to mesh asset
		uint64 material_name_size = 0;
		uint64 material_props_size = 0;
		uint64 material_diff_name_size = 0;
		char* material_diff_map_name = nullptr;
		Material* material = {};
		if (mesh->material_index != -1) { // has material
			material = &(*material_stack)[mesh->material_index];
			material_name_size = strlen(material->name) + 1;
			// TODO: For now its assumed that amb and diff map is always the same texture
			if (material->amb_map_name) {
				material_diff_name_size = strlen(material->amb_map_name) + 1;
				material_diff_map_name = material->amb_map_name;
			}
			else if (material->diff_map_name) {
				material_diff_name_size = strlen(material->diff_map_name) + 1;
				material_diff_map_name = material->diff_map_name;
			}
		}
		material_props_size = sizeof(AB::AABMeshMaterialProperties);

		bool32 has_uv = mesh->uv != nullptr;
		uint32 num_uv = has_uv ? mesh->num_vertices : 0;

		uint64 data_size = sizeof(hpm::Vector3) * mesh->num_vertices
			+ sizeof(hpm::Vector2) * num_uv
			+ sizeof(hpm::Vector3) * mesh->num_vertices
			+ sizeof(uint32) * mesh->num_indices
			+ material_name_size
			+ material_props_size
			+ material_diff_name_size;

		uint64 file_size = data_size + sizeof(AB::AABMeshHeader);

		void* file_buffer = malloc(file_size);
		assert(file_buffer); // malloc failed

		AB::AABMeshHeader* header_ptr = (AB::AABMeshHeader*)file_buffer;
		header_ptr->magic_value = AB::AAB_FILE_MAGIC_VALUE;
		header_ptr->version = AB::AAB_FILE_VERSION;
		header_ptr->asset_size = data_size;
		header_ptr->asset_type = AB::AAB_FILE_TYPE_MESH;
		header_ptr->vertices_count = mesh->num_vertices;
		header_ptr->normals_count = mesh->num_vertices;
		header_ptr->uvs_count = num_uv;
		header_ptr->indices_count = mesh->num_indices;

		void* vertices_ptr = (void*)(header_ptr + 1);
		uint64 vertices_size = sizeof(hpm::Vector3) * mesh->num_vertices;
		memcpy(vertices_ptr, mesh->vertices, vertices_size);


		void* normals_ptr = (void*)((byte*)vertices_ptr + vertices_size);
		uint64 normals_size = sizeof(hpm::Vector3) * mesh->num_vertices;
		memcpy(normals_ptr, mesh->normals, normals_size);

		void* uvs_ptr = (void*)((byte*)normals_ptr + normals_size);
		uint64 uvs_size = sizeof(hpm::Vector2) * num_uv;
		if (uvs_size) {
			memcpy(uvs_ptr, mesh->uv, uvs_size);
		}

		void* indices_ptr = (void*)((byte*)uvs_ptr + uvs_size);
		uint64 indices_size = sizeof(uint32) * mesh->num_indices;
		memcpy(indices_ptr, mesh->indices, indices_size);

		if (mesh->material_index != -1) {
			// TODO: Strings in the middle of file can break aligment.
			// Should write strings at the end of file or introduce padding
			void* mat_ptr = (void*)((byte*)indices_ptr + indices_size);
			memcpy(mat_ptr, material->name, material_name_size);
			mat_ptr = (void*)((byte*)mat_ptr + material_name_size);

			AB::AABMeshMaterialProperties aab_material = {};
			aab_material.k_a = material->ka;
			aab_material.k_d = material->kd;
			aab_material.k_s = material->ks;
			aab_material.k_e = material->ke;
			aab_material.shininess = material->shininess;

			memcpy(mat_ptr, &aab_material, sizeof(aab_material));

			if (material_diff_name_size) {
				mat_ptr = (void*)((byte*)mat_ptr + material_props_size);

				memcpy(mat_ptr, material_diff_map_name, material_diff_name_size);
			}
		}

		header_ptr->vertices_offset = sizeof(AB::AABMeshHeader);
		header_ptr->normals_offset = header_ptr->vertices_offset + vertices_size;
		header_ptr->uvs_offset = header_ptr->normals_offset + normals_size;
		header_ptr->indices_offset = header_ptr->uvs_offset + uvs_size;
		if (mesh->material_index != -1) {
			header_ptr->material_name_offset = header_ptr->indices_offset + indices_size;
			header_ptr->material_properties_offset = header_ptr->material_name_offset + material_name_size;
			if (material_diff_map_name) {
				header_ptr->material_diff_bitmap_name_offset = header_ptr->material_properties_offset + material_props_size;
			}
			else {
				header_ptr->material_diff_bitmap_name_offset = 0;
			}
		}
		else {
			header_ptr->material_name_offset = 0;
			header_ptr->material_properties_offset = 0;
			header_ptr->material_diff_bitmap_name_offset = 0;
		}

		assert(file_size < 0xffffffff); // Can`t write bigger than 4gb
		bool32 result = WriteFile("test.aab", file_buffer, (uint32)file_size);
		assert(result); // Failed to write file

		free(file_buffer);
	}
}

using namespace AB;
int main(int argc, char** argv) {
	if (argc > 1) {
		auto[data, size] = ReadEntireFile(argv[1]);
		// TODO: check reading
		constexpr uint32 file_dir_sz = 512;
		char file_dir[file_dir_sz];
		auto[success, written] = GetDirectory(argv[1], file_dir, file_dir_sz);

		if (success) {
			std::vector<Mesh> mesh_stack;
			std::vector<Material> material_stack;

			ParseOBJ(argv[1], &mesh_stack, &material_stack);
			FreeFileMemory(data);
			for (uint32 i = 0; i < mesh_stack.size(); i++) {
				ABMesh mesh = GenABMesh(&(mesh_stack[i]), &material_stack);
				char* file_name = (char*)malloc(strlen(mesh_stack[i].name) + 4 + 2);
				strcpy(file_name, mesh_stack[i].name);
				strcat(file_name, ".aab");
				WriteAABMesh(file_name, &mesh, &material_stack);
				free(file_name);
				FreeMesh(&(mesh_stack[i]));
				FreeABMesh(&mesh);

			}
			//VertexData vertex_data = ParseOBJ((char*)data, size, file_dir);
			//WriteAABMesh(vertex_data);
		} else {
			printf("Too long file path.\n");
		}
	} else {
		printf("No input.\n");
	}
	return 0;
}