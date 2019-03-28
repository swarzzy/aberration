#pragma once 

namespace std {
	template <typename T, typename Alloc>
	class vector;
}

namespace AB {

	struct Mesh {
		uint64 num_vertices;
		uint64 num_normals;
		uint64 num_uv;

		uint64 num_vertex_indices;
		uint64 num_normal_indices;
		uint64 num_uv_indices;

		// Theese all should be freed
		char* name;
		char* material_name;

		hpm::Vector3* vertices;
		hpm::Vector3* normals;
		hpm::Vector2* uv;

		uint32* vertex_indices;
		uint32* normal_indices;
		uint32* uv_indices;

	};

	struct Material {
		char* name;
		char* diff_map_name;
		char* amb_map_name;
		char* spec_map_name;
		hpm::Vector3 ka;
		hpm::Vector3 kd;
		hpm::Vector3 ks;
		hpm::Vector3 ke;
		float32 shininess;
	};

	void FreeMesh(Mesh* mesh);
	void LoadMTL(const char* path, std::vector<Material>* material_stack);
	void ParseOBJ(const char* file_path, std::vector<Mesh>* mesh_stack, std::vector<Material>* material_stack);
}