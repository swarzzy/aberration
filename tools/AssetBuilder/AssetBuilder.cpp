#include <cstdint>
#include <cstdio>
#include <cassert>
#include <cstdlib> 
#include <cstring>

#include "../../aberration/src/ABFileFormats.h"

#include "../../hypermath/hypermath.h"

#if 0
struct ReadFileRet { void* data; uint64 size; } ReadFile(const char* path) {
	void* data = nullptr;
	uint64 size = 0;
	FILE* file_handle = fopen(path, "r");
	if (file_handle) {
		if (fseek(file_handle, 0, SEEK_END) == 0) {
			long int file_size = ftell(file_handle);
			if (file_size != -1L) {
				void* tmp_data = malloc(sizeof(byte) * file_size);
				if (tmp_data) {
					uint64 read = fread(tmp_data, sizeof(byte), file_size, file_handle);
					if (read == file_size) {
						size = (uint64)file_size;
						data = tmp_data;
					} else {
						free(tmp_data);
					}
				}
				else {
					// malloc error
				}
			}
			else {
				// ftell error
			}
		}
		else {
			// fseek error

		}
		fclose(file_handle);
	} else {
		printf("Failed to open file\n");
	}
	return { data, size };
}
#endif

#if defined(AB_PLATFORM_WINDOWS)
#include <windows.h>

void FreeFileMemory(void* memory) {
	if (memory) {
		std::free(memory);
	}
}

struct ReadFileRet { void* data; uint64 size; } ReadEntireFile(const char* filename) {
	uint32 bytesRead = 0;
	void* bitmap = nullptr;
	LARGE_INTEGER fileSize = { 0 };
	HANDLE fileHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		if (GetFileSizeEx(fileHandle, &fileSize)) {
			if (fileSize.QuadPart > 0xffffffff) {
				printf("Can`t read >4GB file.");
				CloseHandle(fileHandle);
				return {nullptr, 0};
			}
			bitmap = std::malloc(fileSize.QuadPart);
			if (bitmap) {
				DWORD read;
				if (!ReadFile(fileHandle, bitmap, (DWORD)fileSize.QuadPart, &read, 0) && !(read == (DWORD)fileSize.QuadPart)) {
					printf("Failed to read file.");
					FreeFileMemory(bitmap);
					bitmap = nullptr;
				}
				else {
					bytesRead = (uint32)fileSize.QuadPart;
				}
			}
		}
		CloseHandle(fileHandle);
	}
	return  {bitmap, bytesRead};
}

bool32 WriteFile(const char* filename, void* data, uint32 dataSize) {
	HANDLE fileHandle = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if (fileHandle != INVALID_HANDLE_VALUE) {
		DWORD bytesWritten;
		if (WriteFile(fileHandle, data, dataSize, &bytesWritten, 0) && (dataSize == bytesWritten)) {
			CloseHandle(fileHandle);
			return true;
		}
	}
	CloseHandle(fileHandle);
	return false;
}

#elif defined(AB_PLATFORM_LINUX)
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

void FreeFileMemory(void* memory) {
	if (memory) {
		std::free(memory);
	}
}

struct ReadFileRet { void* data; uint64 size; } ReadEntireFile(const char* filename) {
	uint32 bytesRead = 0;
	void* ptr = nullptr;
	*bytesRead = 0;
	int fileHandle = open(filename, O_RDONLY);
	if (fileHandle) {
		off_t fileEnd = lseek(fileHandle, 0, SEEK_END);
		if (fileEnd) {
			lseek(fileHandle, 0, SEEK_SET);
			void* data = std::malloc(fileEnd);
			if (data) {
				ssize_t result = read(fileHandle, data, fileEnd);
				if (result == fileEnd) {
					ptr = data;
					bytesRead = (uint32)result;
				}
				else {
					std::free(data);
					printf("File reading error. File: %s. Failed read data from file", filename);
				}
			}
			else {
				printf("File reading error. File: %s. Memory allocation failed", filename);
			}
		}
		else {
			printf("File reading error. File: %s", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file.", filename);
		return {nullptr, 0};
	}
	close(fileHandle);
	return {ptr, bytes_read};
}

bool32 WriteFile(const char* filename, void* data, uint32 dataSize) {
	bool32 result = false;
	int fileHandle = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IROTH | S_IRWXU | S_IRGRP);
	if (fileHandle) {
		ssize_t written = write(fileHandle, data, dataSize);
		if (written == dataSize) {
			result = true;
		}
		else {
			printf("File reading error. File: %s. Write operation failed.", filename);
		}
	}
	else {
		printf("File reading error. File: %s. Failed to open file", filename);
		return false;
	}
	close(fileHandle);
	return result;
}
#else
#error Unsupported platform.
#endif

struct ParseVertexRet { hpm::Vector3 v; char* next; } 
inline static ParseVertex(char* at) {  // after v
	char* next;
	float32 x = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 y = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 z = strtof(at, &next);
	assert(at != next);

	return { { x, y, z }, next };
}

struct ParseNormalRet { hpm::Vector3 v; char* next; }
inline static ParseNormal(char* at) {  // after v
	char* next;
	float32 x = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 y = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 z = strtof(at, &next);
	assert(at != next);

	auto result = hpm::Normalize(hpm::Vector3{ x, y, z });

	return { result, next };
}

struct ParseUVRet { hpm::Vector2 uv; char* next; } 
inline static ParseUV(char* at) {
	char* next;
	float32 u = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 v = strtof(at, &next);
	assert(at != next);

	return { { u, v }, next };
}

enum class FaceIndexType : byte {
	Undefined = 0, Vertex, VertexNormal, VertexUV, VertexUVNormal
};

struct ParseFaceRet { uint32 v[3]; uint32 uv[3]; uint32 n[3]; char* next; }
inline static ParseFace(FaceIndexType type, char* at) {  // after v
	
	char* next;
	ParseFaceRet result = {};

	for (uint32 i = 0; i < 3; i++) {
		result.v[i] = strtoul(at, &next, 10) - 1;
		assert(at != next);
		at = next;
		if (type == FaceIndexType::VertexUV || type == FaceIndexType::VertexUVNormal) {
			while (*at == '/') at++;
			result.uv[i] = strtoul(at, &next, 10) - 1;
			assert(at != next);
			at = next;
		}
		if (type == FaceIndexType::VertexNormal || type == FaceIndexType::VertexUVNormal) {
			while (*at == '/') at++;
			result.n[i] = strtoul(at, &next, 10) - 1;
			assert(at != next);
			at = next;
		}
	}

	strtoul(at, &next, 10);
	assert(at == next); // Face has more than 3 vertices
	
	result.next = at;
	return result;
}

inline static FaceIndexType DefineFaceIndexType(char* at) {
	FaceIndexType type = FaceIndexType::Undefined;
	while (*at == ' ') at++;
	uint32 slash_counter = 0;
	bool32 has_vt = false;
	while (*at != ' ' && *at != '\n') {
		if (slash_counter == 1 && !(*at == '/')) {
			has_vt = true;
		}
		if (*at == '/') {
			slash_counter++;
		}
		at++;
	}

	if (slash_counter == 0) {
		type = FaceIndexType::Vertex;
	} else if (slash_counter == 1) {
		type = FaceIndexType::VertexUV;
	} else if (slash_counter == 2) {
		if (has_vt) {
			type = FaceIndexType::VertexUVNormal;
		} else {
			type = FaceIndexType::VertexNormal;
		}
	} else {
		type = FaceIndexType::Undefined;
	}

	return type;
}

// TODO: This is very bag normals generation
uint32 GenNormals(hpm::Vector3* beg_vertices, uint32 size_vertices, uint32* beg_indices, uint32 size_indices, hpm::Vector3* beg_normals) {
	uint32 normals_at = 0;

	for (uint32 i = 0; i < size_indices; i += 3) {
		assert(beg_indices[i + 2] < size_vertices); // Out of range

		hpm::Vector3 first = hpm::Subtract(beg_vertices[beg_indices[i + 1]], beg_vertices[beg_indices[i]]);
		hpm::Vector3 second = hpm::Subtract(beg_vertices[beg_indices[i + 2]], beg_vertices[beg_indices[i]]);
		hpm::Vector3 normal = hpm::Normalize(hpm::Cross(first, second));
		beg_normals[normals_at] = normal;
		beg_normals[normals_at + 1] = normal;
		beg_normals[normals_at + 2] = normal;
		normals_at += 3;
	}
	return normals_at;
}

struct Mtl {
	static constexpr uint32 TEXT_BUFFER_SIZE = 256;
	char name[TEXT_BUFFER_SIZE];
	char diff_map[TEXT_BUFFER_SIZE];
	char amb_map[TEXT_BUFFER_SIZE];
	bool32 has_diff_map;
	bool32 has_amb_map;
	hpm::Vector3 k_a;
	hpm::Vector3 k_d;
	hpm::Vector3 k_s;
	hpm::Vector3 k_e;
	float32 shininess;
};


uint32 ExtractStringFromToEnd(char* out_buf, uint32 out_size, const char* string) {
	const char* at = string;
	while (isspace((unsigned char)(*at))) at++;
	uint32 buf_at = 0;
	while ((*at != '\n' ) && (*at != '\0')) {
		out_buf[buf_at] = (*at);
		buf_at++;
		if (buf_at == out_size - 1) {
			break;
		}
		at++;
	}
	out_buf[buf_at] = '\0';
	buf_at++;
	assert(buf_at < out_size);
	return buf_at;
}

struct ParseKRet { hpm::Vector3 K; char* next; }
inline static ParseK(char* at) {  // after v
	char* next;
	float32 r = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 g = strtof(at, &next);
	assert(at != next);
	at = next;
	float32 b = strtof(at, &next);
	assert(at != next);

	return { { r, g, b }, next };
}

Mtl ParseMtl(char* _at, const char* obj_dir) {
	Mtl _mtl = {};
	char mtl_file_name[Mtl::TEXT_BUFFER_SIZE];
	uint32 mtl_file_name_at = 0;
	mtl_file_name_at = ExtractStringFromToEnd(mtl_file_name, Mtl::TEXT_BUFFER_SIZE, _at);

	char file_path[512];
	assert(strcpy_s(file_path, 256, obj_dir) == 0);
	strncat(file_path, mtl_file_name, mtl_file_name_at);
	auto[data, size] = ReadEntireFile(file_path);
	if (data) {
		bool32 material = false;
		for (uint64 i = 0; i < size;) {
			char* mtl_at = ((char*)data + i);
			switch (*mtl_at) {
			//case '': {} break;
			case '#': {
				while (*((char*)data + i) != '\n') i++;
				i++;
			} break;
			case 'n': {
				if (*(mtl_at + 1) == 'e' &&
					*(mtl_at + 2) == 'w' &&
					*(mtl_at + 3) == 'm' &&
					*(mtl_at + 4) == 't' &&
					*(mtl_at + 5) == 'l') 
				{
					mtl_at += 6;
					i += 6;
					ExtractStringFromToEnd(_mtl.name, Mtl::TEXT_BUFFER_SIZE, mtl_at);
					material = true;
					while (*((char*)data + i) != '\n') i++;
					i++;
				}
				
			} break;
			case 'm': {
				if (*(mtl_at + 1) == 'a' &&
					*(mtl_at + 2) == 'p' &&
					*(mtl_at + 3) == '_' &&
					*(mtl_at + 4) == 'K' &&
					*(mtl_at + 5) == 'd')
				{
					mtl_at += 6;
					i += 6;
					char tmp[Mtl::TEXT_BUFFER_SIZE];
					uint32 str_size = ExtractStringFromToEnd(tmp, Mtl::TEXT_BUFFER_SIZE, mtl_at);

					if (!_mtl.has_diff_map) {
						_mtl.has_diff_map = true;
						memcpy(_mtl.diff_map, tmp, Mtl::TEXT_BUFFER_SIZE);
					} else {
						printf("WARN: Diffuse map for material: %s already defined. Map: %s will be ignored.\n",
							_mtl.name, tmp);
					}
					while (*((char*)data + i) != '\n') i++;
					i++;
				} else
					if (*(mtl_at + 1) == 'a' &&
						*(mtl_at + 2) == 'p' &&
						*(mtl_at + 3) == '_' &&
						*(mtl_at + 4) == 'K' &&
						*(mtl_at + 5) == 'a')
					{
						mtl_at += 6;
						i += 6;
						char tmp[Mtl::TEXT_BUFFER_SIZE];
						uint32 str_size = ExtractStringFromToEnd(tmp, Mtl::TEXT_BUFFER_SIZE, mtl_at);

						if (!_mtl.has_amb_map) {
							_mtl.has_amb_map= true;
							memcpy(_mtl.amb_map, tmp, Mtl::TEXT_BUFFER_SIZE);
						}
						else {
							printf("WARN: Ambient map for material: %s already defined. Map: %s will be ignored.\n",
								_mtl.name, tmp);
						}
						while (*((char*)data + i) != '\n') i++;
						i++;
					}

			} break;
			case 'i': {
				if (*(mtl_at + 1) == 'l' &&
					*(mtl_at + 2) == 'l' &&
					*(mtl_at + 3) == 'u' &&
					*(mtl_at + 4) == 'm')
				{
					mtl_at += 5;
					i += 5;
					char* next = nullptr;
					uint32 illum = strtol(mtl_at, &next, 10);
					assert(mtl_at != next);
					assert(illum == 2); // Only Blinn-Phong supported now
					while (*((char*)data + i) != '\n') i++;
					i++;
				} else {
					while (*((char*)data + i) != '\n') i++;
					i++;
				}

			} break;
			case 'N': {
				if (*(mtl_at + 1) == 's') {
					char* next = nullptr;
					float32 shin = strtof(mtl_at + 2, &next);
					assert(mtl_at != next);
					_mtl.shininess = shin;
					while (*((char*)data + i) != '\n') i++;
					i++;
				} else {
					printf("MTL PARSER: Unknown value N%c in material: %s\n", *(mtl_at + 1), _mtl.name);
					while (*((char*)data + i) != '\n') i++;
					i++;
				}
			} break;
			case 'K': {
				assert(material); // Check is that related to any material. Only one material in file supported now!
				switch (*(mtl_at + 1)) {
				case 'a': {
					auto[amb_color, next] = ParseK(mtl_at + 2);
					i += next - mtl_at;
					_mtl.k_a = amb_color;
				} break;
				case 'd': {
					auto[dif_color, next] = ParseK(mtl_at + 2);
					i += next - mtl_at;
					_mtl.k_d = dif_color;
				} break;
				case 's': {
					auto[spc_color, next] = ParseK(mtl_at + 2);
					i += next - mtl_at;
					_mtl.k_s = spc_color;
				} break;
				case 'e': {
					auto[em_color, next] = ParseK(mtl_at + 2);
					i += next - mtl_at;
					_mtl.k_e = em_color;
				} break;
				default: {
					printf("MTL PARSER: Unknown value K%c in material: %s", *(mtl_at + 1), _mtl.name);
					while (*((char*)data + i) != '\n') i++;
					i++;
				}
				}
			} break;

			default: {
				i++;
			} break;
			}
		}
	} else {
		printf("Failed to open mtl file: %s", _mtl.name);
	}
	if (_mtl.has_amb_map && _mtl.has_diff_map) {
		assert(strcmp(_mtl.amb_map, _mtl.diff_map) == 0); // For now diff map and ambient map should be the same
	}
	return _mtl;
}

struct VertexData {
	Mtl material;
	bool32 has_material;
	hpm::Vector3* vertices; 
	hpm::Vector3* normals; 
	hpm::Vector2* uvs; 
	uint32* indices;
	uint32 vertices_count; 
	uint32 normals_count; 
	uint32 uvs_count;
	uint32 indices_count; 
};

VertexData ParseOBJ(char* file_data, uint64 size, const char* file_dir) {

	hpm::Vector3* tmp_vertices = nullptr;
	hpm::Vector3* tmp_normals = nullptr;
	hpm::Vector2* tmp_uvs = nullptr; 
	uint32* tmp_vertex_indices = nullptr;
	uint32* tmp_uv_indices = nullptr;
	uint32* tmp_normals_indices = nullptr;
	uint32 tmp_vertices_at = 0;
	uint32 tmp_normals_at = 0;
	uint32 tmp_uvs_at = 0;
	uint32 tmp_vertex_indices_at = 0;
	uint32 tmp_uv_indices_at = 0;
	uint32 tmp_normals_indices_at = 0;

	constexpr uint32 MAX_MATERIALS = 16;
	Mtl* materials = nullptr;
	uint32 matrials_at = 0;

	// Just allocating big enough memory for any case
	tmp_vertices = (hpm::Vector3*)malloc(sizeof(hpm::Vector3) * size);
	tmp_normals = (hpm::Vector3*)malloc(sizeof(hpm::Vector3) * size);
	tmp_uvs = (hpm::Vector2*)malloc(sizeof(hpm::Vector2) * size);
	assert(tmp_vertices);
	assert(tmp_normals);
	assert(tmp_uvs);

	FaceIndexType index_type = FaceIndexType::Undefined;
	bool32 mtl_found = false;
	int32 used_material_index = -1;

	for (uint64 i = 0; i < size;) {
		char* at = file_data + i;
		switch (*at) {
		case 'm': { // mtllib
			if (*(at + 1) == 't' &&
				*(at + 2) == 'l' &&
				*(at + 3) == 'l' &&
				*(at + 4) == 'i' &&
				*(at + 5) == 'b') 
			{
				if (!mtl_found) {
					mtl_found = true;
					at += 6;
					i += 6;
					Mtl mtl = {};
					mtl = ParseMtl(at, file_dir);
					if (!matrials_at) {
						materials = (Mtl*)malloc(sizeof(Mtl) * MAX_MATERIALS);
					}
					materials[matrials_at] = mtl;
					matrials_at++;

					while (*(file_data + i) != '\n') i++;
					i++;
				}
			} else {
				while (*(file_data + i) != '\n') i++;
				i++;
			}
		} break;
		case 'u': { // usemtl
			if (*(at + 1) == 's' &&
				*(at + 2) == 'e' &&
				*(at + 3) == 'm' &&
				*(at + 4) == 't' &&
				*(at + 5) == 'l') 
			{
				at += 6;
				i += 6;
				char buf[Mtl::TEXT_BUFFER_SIZE];
				uint32 size_buf = ExtractStringFromToEnd(buf, Mtl::TEXT_BUFFER_SIZE, at);
				for (uint32 i = 0; i < MAX_MATERIALS; i++) {
					if (strcmp(buf, materials[i].name) == 0) {
						used_material_index = i;
						break;
					}
				}
				if (used_material_index == -1) {
					printf("Error: usemtl: material with name: %s is not loaded", buf);
				}
			}
			while (*(file_data + i) != '\n') i++;
			i++;
		} break;
		case 'g': { // g - group 
			while (*(file_data + i) != '\n') i++;
			i++;
		} break;
		case '#': { // comment
			while (*(file_data + i) != '\n') i++;
			i++;
		} break;
		case 'o': { // object
			while (*(file_data + i) != '\n') i++;
			i++;
		} break;
		case 's': { // smoothing group
			while (*(file_data + i) != '\n') i++;
			i++;
		} break;
		case 'v': { // vertex (position)
			if (*(at + 1) == 't') {
				// uv
				auto[uv, next] = ParseUV(at + 2);
				i += next - at;
				tmp_uvs[tmp_uvs_at] = uv;
				tmp_uvs_at++;
			} else if (*(at + 1) == 'n') {
				//normal
				auto[normal, next] = ParseNormal(at + 2);
				i += next - at;
				tmp_normals[tmp_normals_at] = normal;
				tmp_normals_at++;
			} else {
				// vertex
				auto[v, next] = ParseVertex(at + 1);
				i += next - at;
				tmp_vertices[tmp_vertices_at] = v;
				tmp_vertices_at++;
			}
		} break;
		case 'f': { // face indices
			if (index_type == FaceIndexType::Undefined) {
				index_type = DefineFaceIndexType(at + 1);
				assert(index_type != FaceIndexType::Undefined); // Invalid face index format

				tmp_vertex_indices = (uint32*)malloc(sizeof(uint32) * size);
				switch (index_type) {
				case FaceIndexType::Vertex: {} break;
				case FaceIndexType::VertexUV: {
					tmp_uv_indices = (uint32*)malloc(sizeof(uint32) * size);
				} break;
				case FaceIndexType::VertexUVNormal: {
					tmp_uv_indices = (uint32*)malloc(sizeof(uint32) * size);
					tmp_normals_indices = (uint32*)malloc(sizeof(uint32) * size);
				} break;
				case FaceIndexType::VertexNormal: {
					tmp_normals_indices = (uint32*)malloc(sizeof(uint32) * size);
				} break;
				default: {} break;
				}
			} else {
				auto[v_ind, uv_ind, n_ind, next] = ParseFace(index_type, at + 1);
				i += next - at;

				tmp_vertex_indices[tmp_vertex_indices_at] = v_ind[0];
				tmp_vertex_indices[tmp_vertex_indices_at + 1] = v_ind[1];
				tmp_vertex_indices[tmp_vertex_indices_at + 2] = v_ind[2];
				tmp_vertex_indices_at += 3;

				if (tmp_normals_indices) {
					tmp_normals_indices[tmp_normals_indices_at] = n_ind[0];
					tmp_normals_indices[tmp_normals_indices_at + 1] = n_ind[1];
					tmp_normals_indices[tmp_normals_indices_at + 2] = n_ind[2];
					tmp_normals_indices_at += 3;
				}

				if (tmp_uv_indices) {
					tmp_uv_indices[tmp_uv_indices_at] = uv_ind[0];
					tmp_uv_indices[tmp_uv_indices_at + 1] = uv_ind[1];
					tmp_uv_indices[tmp_uv_indices_at + 2] = uv_ind[2];
					tmp_uv_indices_at += 3;
				}
			}
		} break;
		default: {
			i++;
		} break;
		}
	}

	assert(tmp_vertices_at != 0);
	assert(tmp_vertex_indices_at % 3 == 0); // Mesh has non triangle faces

	bool32 generated_normals = false;

	if (!tmp_normals_at) {
		printf("File does not contains normals. Generating normals...\n");
		tmp_normals_at = GenNormals(tmp_vertices, tmp_vertices_at, tmp_vertex_indices, tmp_vertex_indices_at, tmp_normals);
		generated_normals = true;
	}

	bool32 has_uv = tmp_uvs_at;

	hpm::Vector3* vertices = nullptr;
	hpm::Vector3* normals = nullptr;
	hpm::Vector2* uvs = nullptr;
	uint32* indices = nullptr;


	uint32 vertices_at = 0;
	uint32 normals_at = 0;
	uint32 uvs_at = 0;
	uint32 indices_at = 0;

	vertices = (hpm::Vector3*)malloc(tmp_vertex_indices_at * sizeof(hpm::Vector3));
	indices = (uint32*)malloc(tmp_vertex_indices_at * sizeof(uint32));
	if (has_uv) {
		uvs = (hpm::Vector2*)malloc(tmp_vertex_indices_at * sizeof(hpm::Vector2));
	}

	if (generated_normals) {
		normals = tmp_normals;
		normals_at = tmp_normals_at;
	} else {
		normals = (hpm::Vector3*)malloc(tmp_vertex_indices_at * sizeof(hpm::Vector3));
	}

	for (uint32 i = 0; i < tmp_vertex_indices_at; i += 3) {
		assert(tmp_vertex_indices[i + 2] < tmp_vertices_at); // Out of range
		if (tmp_uv_indices) {
			assert(tmp_uv_indices[i + 2] < tmp_uvs_at); // Out of range
		}
		if (tmp_normals_indices) {
			assert(tmp_normals_indices[i + 2] < tmp_normals_at); // Out of range
		}

		vertices[vertices_at] = tmp_vertices[tmp_vertex_indices[i]];
		vertices[vertices_at + 1] = tmp_vertices[tmp_vertex_indices[i + 1]];
		vertices[vertices_at + 2] = tmp_vertices[tmp_vertex_indices[i + 2]];
		vertices_at +=3;

		if (has_uv) {
			uvs[uvs_at] = tmp_uvs[tmp_uv_indices[i]];
			uvs[uvs_at + 1] = tmp_uvs[tmp_uv_indices[i + 1]];
			uvs[uvs_at + 2] = tmp_uvs[tmp_uv_indices[i + 2]];
			uvs_at += 3;
		}

		if (!generated_normals) {
			normals[normals_at] = tmp_normals[tmp_normals_indices[i]];
			normals[normals_at + 1] = tmp_normals[tmp_normals_indices[i + 1]];
			normals[normals_at + 2] = tmp_normals[tmp_normals_indices[i + 2]];
			normals_at += 3;
		}

		indices[indices_at] = i;
		indices[indices_at + 1] = i + 1;
		indices[indices_at + 2] = i + 2;
		indices_at += 3;
	}

	Mtl used_material = {};
	bool32 has_material = used_material_index != -1;
	if (used_material_index != -1) {
		used_material = materials[used_material_index];
	}

	free(tmp_vertex_indices);
	free(tmp_uv_indices);
	if (!generated_normals) {
		free(tmp_normals);
	}
	free(tmp_vertices);
	free(tmp_uvs);
	if (matrials_at) {
		free(materials);
	}

	return { used_material, has_material, vertices, normals, uvs, indices,  vertices_at, normals_at, uvs_at, indices_at };
}

void WriteAABMesh(VertexData vd) {
	uint64 material_name_size = 0;
	uint64 material_props_size = 0;
	uint64 material_diff_name_size = 0;
	char* material_diff_map_name = nullptr;
	if (vd.has_material) {
		material_name_size = strlen(vd.material.name) + 1;
		// TODO: For now its assumed that amb and diff map is always the same texture
		if (vd.material.has_amb_map) {
			material_diff_name_size = strlen(vd.material.amb_map) + 1;
			material_diff_map_name = vd.material.amb_map;
		}
		else if (vd.material.has_diff_map) {
			material_diff_name_size = strlen(vd.material.diff_map) + 1;
			material_diff_map_name = vd.material.diff_map;
		}
	}
	material_props_size = sizeof(AB::AABMeshMaterialProperties);

	uint64 data_size = sizeof(hpm::Vector3) * vd.vertices_count 
						+ sizeof(hpm::Vector2) * vd.uvs_count 
						+ sizeof(hpm::Vector3) * vd.normals_count 
						+ sizeof(uint32) * vd.indices_count
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
	header_ptr->vertices_count = vd.vertices_count;
	header_ptr->normals_count = vd.normals_count;
	header_ptr->uvs_count = vd.uvs_count;
	header_ptr->indices_count = vd.indices_count;

	void* vertices_ptr = (void*)(header_ptr + 1);
	uint64 vertices_size = sizeof(hpm::Vector3) * vd.vertices_count;
	memcpy(vertices_ptr, vd.vertices, vertices_size);


	void* normals_ptr = (void*)((byte*)vertices_ptr + vertices_size);
	uint64 normals_size = sizeof(hpm::Vector3) * vd.normals_count;
	memcpy(normals_ptr, vd.normals, normals_size);

	void* uvs_ptr = (void*)((byte*)normals_ptr + normals_size);
	uint64 uvs_size = sizeof(hpm::Vector2) * vd.uvs_count;
	if (uvs_size) {
		memcpy(uvs_ptr, vd.uvs, uvs_size);
	}

	void* indices_ptr = (void*)((byte*)uvs_ptr + uvs_size);
	uint64 indices_size = sizeof(uint32) * vd.indices_count;
	memcpy(indices_ptr, vd.indices, indices_size);

	if (vd.has_material) {
		// TODO: Strings in the middle of file can break aligment.
		// Should write strings at the end of file of introduce padding
		void* mat_ptr = (void*)((byte*)indices_ptr + indices_size);
		memcpy(mat_ptr, vd.material.name, material_name_size);
		mat_ptr = (void*)((byte*)mat_ptr + material_name_size);
		
		AB::AABMeshMaterialProperties aab_material = {};
		aab_material.k_a = vd.material.k_a;
		aab_material.k_d = vd.material.k_d;
		aab_material.k_s = vd.material.k_s;
		aab_material.k_e = vd.material.k_e;
		aab_material.shininess = vd.material.shininess;

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
	if (vd.has_material) {
		header_ptr->material_name_offset = header_ptr->indices_offset + indices_size;
		header_ptr->material_properties_offset = header_ptr->material_name_offset + material_name_size;
		header_ptr->material_diff_bitmap_name_offset = header_ptr->material_properties_offset + material_props_size;
	} else {
		header_ptr->material_name_offset = 0;
		header_ptr->material_properties_offset = 0;
		header_ptr->material_diff_bitmap_name_offset = 0;
	}

	assert(file_size < 0xffffffff); // Can`t write bigger than 4gb
	bool32 result = WriteFile("test.aab", file_buffer, (uint32)file_size);
	assert(result); // Failed to write file

	free(file_buffer);
}

int main(int argc, char** argv) {
	assert(argc > 1);
	auto[data, size] = ReadEntireFile(argv[1]);

	char separator;
#if defined(AB_PLATFORM_WINDOWS)
	separator = '\\';
#elif defined(AB_PLATFORM_LINUX)
	separator = '/';
#endif

	uint32 count = 0;
	char file_dir[256];
	char* at = argv[1];
	while (*(at + count) != '\0') count++;
	uint32 dir_offset = count;
	while (*(at + dir_offset) != separator) dir_offset--;
	memcpy(file_dir, argv[1], dir_offset);
	assert(dir_offset < 256);
	file_dir[dir_offset] = '\\';
	file_dir[dir_offset + 1] = '\0';

	VertexData vertex_data = ParseOBJ((char*)data, size, file_dir);
	FreeFileMemory(data);
	WriteAABMesh(vertex_data);

	return 0;
}