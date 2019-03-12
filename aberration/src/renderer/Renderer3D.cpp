#include "Renderer3D.h"
#include "platform/API/OpenGL/ABOpenGL.h"
#include "platform/Platform.h"
#include "utils/ImageLoader.h"
#include "platform/Memory.h"
#include "AssetManager.h"
#include "Application.h"
//#include <cstddef>

namespace AB {

	struct DrawCommand {
		int32 mesh_handle;
		int32 material_handle;
		hpm::Matrix4 transform;
	};

	struct Material {
		bool8 used;
		uint32 diffuse_map_handle;
		uint32 specular_map_handle;
		float32 shininess;
	};

	struct Camera {
		hpm::Vector3 position;
		hpm::Vector3 front;
		hpm::Matrix4 look_at;
	};

	static constexpr int32 DRAW_BUFFER_SIZE = 256;
	static constexpr int32 MAX_MATERIALS = 128;

	struct Renderer3D {
		int32 program_handle;
		Material materials[MAX_MATERIALS];
		uint32 draw_buffer_at;
		DrawCommand draw_buffer[DRAW_BUFFER_SIZE];
		Camera camera;
		hpm::Matrix4 projection;
		DirectionalLight dir_light;
	};

	//static Renderer3D* g_Renderer = nullptr;

	static int32 LoadShader(const char* vertex_path, const char* fragment_path) {
		uint32 bytes_read = 0;

		GLchar* vertex_src_data = (GLchar*)DebugReadFile(vertex_path, &bytes_read);
		AB_CORE_ASSERT(vertex_src_data, "Failed to read vertex source.");
		vertex_src_data[bytes_read] = '\0';
		GLchar* fragment_src_data = (GLchar*)DebugReadFile(fragment_path, &bytes_read);
		AB_CORE_ASSERT(fragment_src_data, "Failed to read vertex source.");
		fragment_src_data[bytes_read] = '\0';


		GLint vertex_shader;
		GLCall(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
		GLCall(glShaderSource(vertex_shader, 1, &vertex_src_data, nullptr));
		GLCall(glCompileShader(vertex_shader));

		GLint fragment_shader;
		GLCall(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
		GLCall(glShaderSource(fragment_shader, 1, &fragment_src_data, nullptr));
		GLCall(glCompileShader(fragment_shader));

		GLint vertex_result = 0;
		GLCall(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_result));
		if (!vertex_result) {
			GLint log_len;
			GLCall(glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocators
			GLchar* message = (GLchar*)malloc(log_len);
			GLCall(glGetShaderInfoLog(vertex_shader, log_len, nullptr, message));
			AB_CORE_FATAL("Vertex shader compilation error:\n%s", message);
		}

		GLint fragment_result = 0;
		GLCall(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &fragment_result));
		if (!fragment_result) {
			GLint log_len;
			GLCall(glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocators
			GLchar* message = (GLchar*)malloc(log_len);
			GLCall(glGetShaderInfoLog(vertex_shader, log_len, nullptr, message));
			AB_CORE_FATAL("Frgament shader compilation error:\n%s", message);
		}

		GLint program_handle;
		GLCall(program_handle = glCreateProgram());
		GLCall(glAttachShader(program_handle, vertex_shader));
		GLCall(glAttachShader(program_handle, fragment_shader));
		GLCall(glLinkProgram(program_handle));

		GLint link_result = 0;
		GLCall(glGetProgramiv(program_handle, GL_LINK_STATUS, &link_result));
		if (!link_result) {
			int32 log_len;
			GLCall(glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocator
			char* message = (char*)malloc(log_len);
			GLCall(glGetProgramInfoLog(program_handle, log_len, 0, message));
			AB_CORE_FATAL("Shader program linking error:\n%s", message);
		}

		GLCall(glDeleteShader(vertex_shader));
		GLCall(glDeleteShader(fragment_shader));
		return program_handle;
	}

	static uint32 LoadTexture(const char* filepath) {
		Image image = LoadBMP(filepath);
		GLuint texHandle = 0;
		if (image.bitmap) {
			uint32 format = GL_RED;
			uint32 inFormat = GL_RED;
			switch (image.format) {
			case PixelFormat::RGB: {
				format = GL_RGB;
				inFormat = GL_RGB8;
			} break;
			case PixelFormat::RGBA: {
				format = GL_RGBA;
				inFormat = GL_RGBA8;
			} break;
			default: {
				// TODO: FIX THIS
				AB_CORE_ERROR("Wrong image format");
			} break;
			}
			GLCall(glGenTextures(1, &texHandle));
			GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.bitmap));

			GLCall(glBindTexture(GL_TEXTURE_2D, 0));

			DeleteBitmap(image.bitmap);
		}
		else {
			AB_CORE_ERROR("Failed to load texture. Cannot load image: %s", filepath);
		}
		return texHandle;
	}

	void Renderer3DInit() {
		Renderer3D* props = nullptr;
		//auto** ptr = &GetMemory()->perm_storage.forward_renderer;
		if (!(PermStorage()->forward_renderer)) {
			props = (Renderer3D*)SysAlloc(sizeof(Renderer3D));
		} else {
			AB_CORE_WARN("Renderer already initialized.");
		}
		props->program_handle = LoadShader("../../../assets/shaders/MeshVertex.glsl", "../../../assets/shaders/MeshFragment.glsl");
		props->projection = hpm::PerspectiveRH(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
		AB::GetMemory()->perm_storage.forward_renderer = props;
	}

	void SetDirectionalLight(const DirectionalLight* light) {
		PermStorage()->forward_renderer->dir_light = *light;
	}

	int32 CreateMaterial(const char* diff_path, const char* spec_path, float32 shininess) {
		auto renderer = PermStorage()->forward_renderer;

		int32 free_index = -1;
		for (int32 i = 0; i < MAX_MATERIALS; i++) {
			if (!renderer->materials[i].used) {
				free_index = i;
				renderer->materials[i].used = true;
				break;
			}
		}

		AB_CORE_ASSERT(free_index != -1, "Materials storage is full.");

		renderer->materials[free_index].diffuse_map_handle = LoadTexture(diff_path);
		renderer->materials[free_index].specular_map_handle = LoadTexture(spec_path);
		renderer->materials[free_index].shininess = shininess;
		
		return  free_index;
	}

	void SetCamera(hpm::Vector3 front, hpm::Vector3 position) {
		auto renderer = PermStorage()->forward_renderer;

		renderer->camera.front = hpm::Normalize(front);
		renderer->camera.position = position;
		renderer->camera.look_at = hpm::LookAtRH(renderer->camera.position, hpm::Add(renderer->camera.position, renderer->camera.front), {0.0f, 1.0f, 0.0f});
	}


	void Submit(int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform) {
		auto renderer = PermStorage()->forward_renderer;

		if (renderer->draw_buffer_at < DRAW_BUFFER_SIZE) {
			renderer->draw_buffer[renderer->draw_buffer_at].mesh_handle = mesh_handle;
			renderer->draw_buffer[renderer->draw_buffer_at].material_handle = material_handle;
			renderer->draw_buffer[renderer->draw_buffer_at].transform = *transform;
			renderer->draw_buffer_at++;
		}
	}

	void Render() {

		auto renderer = PermStorage()->forward_renderer;

		GLCall(glUseProgram(renderer->program_handle));
		GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "projection"), 1, GL_FALSE, renderer->projection.data));
		GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "view"), 1, GL_FALSE, renderer->camera.look_at.data));

		hpm::Vector3* view_pos = &renderer->camera.position;
		GLCall(glUniform3f(glGetUniformLocation(renderer->program_handle, "view_pos"), view_pos->x, view_pos->y, view_pos->z));

		GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.diffuse_map"), 0));
		GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.spec_map"), 1));

		GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "dir_light.direction"), 1, renderer->dir_light.direction.data));
		GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "dir_light.ambient"), 1, renderer->dir_light.ambient.data));
		GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "dir_light.diffuse"), 1, renderer->dir_light.diffuse.data));
		GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "dir_light.specular"), 1, renderer->dir_light.specular.data));

			

		for (uint32 i = 0; i < renderer->draw_buffer_at; i++) {
			DrawCommand* command = &renderer->draw_buffer[i];
			Mesh* mesh = AB::AssetGetMeshData(PermStorage()->asset_manager, command->mesh_handle);

			GLCall(glBindBuffer(GL_ARRAY_BUFFER, mesh->api_vb_handle));
#if 1
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			if (mesh->uvs) {
				GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)((byte*)(mesh->uvs) - (byte*)(mesh->positions))));
				GLCall(glEnableVertexAttribArray(1));
			}
			if (mesh->normals) {
				GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)((byte*)mesh->normals - (byte*)mesh->positions)));
				GLCall(glEnableVertexAttribArray(2));
			}

			if (mesh->api_ib_handle != 0) {
				GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->api_ib_handle));
			}

#else

			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, (sizeof(hpm::Vector3) * 2 + sizeof(hpm::Vector2)), (void*)0));
			GLCall(glEnableVertexAttribArray(0));
			GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (sizeof(hpm::Vector3) * 2 + sizeof(hpm::Vector2)), (void*)(sizeof(hpm::Vector3))));
			GLCall(glEnableVertexAttribArray(1));
			GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, (sizeof(hpm::Vector3) * 2 + sizeof(hpm::Vector2)), (void*)(sizeof(hpm::Vector2) + sizeof(hpm::Vector3))));
			GLCall(glEnableVertexAttribArray(2));
			GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->api_ib_handle));

#endif
			GLCall(glEnable(GL_DEPTH_TEST));

			GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "model"), 1, GL_FALSE, command->transform.data));

			Material* material = &renderer->materials[command->material_handle];

			GLCall(glUniform1f(glGetUniformLocation(renderer->program_handle, "material.shininess"), material->shininess));

			GLCall(glActiveTexture(GL_TEXTURE0));
			GLCall(glBindTexture(GL_TEXTURE_2D, material->diffuse_map_handle));
			GLCall(glActiveTexture(GL_TEXTURE1));
			GLCall(glBindTexture(GL_TEXTURE_2D, material->specular_map_handle));


			if (mesh->api_ib_handle != 0) {
				GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)mesh->num_indices, GL_UNSIGNED_INT, 0));
			} else {
				GLCall(glDrawArrays(GL_TRIANGLES, 0, mesh->num_vertices));
			}
		}


		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
}



#if 0
#include "Renderer3D.h"
#include "platform/API/OpenGL/ABOpenGL.h"
#include "utils/ImageLoader.h"
#include <hypermath.h>
#include "platform/Platform.h"

float32 vertices[288] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, -1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, -1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f,  0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f, -1.0f,  0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f, -1.0f,  0.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f,  0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f,  0.0f
};

namespace AB::Renderer3D {

	struct Material {
		MaterialProperties properties;
		GLint program_handle;
		GLuint textures[32];
	};

	GLint LoadShader(const char* vertex_path, const char* fragment_path) {
		uint32 bytes_read = 0;

		GLchar* vertex_src_data = (GLchar*)DebugReadFile(vertex_path, &bytes_read);
		AB_CORE_ASSERT(vertex_src_data, "Failed to read vertex source.");
		vertex_src_data[bytes_read] = '\0';
		GLchar* fragment_src_data = (GLchar*)DebugReadFile(fragment_path, &bytes_read);
		AB_CORE_ASSERT(fragment_src_data, "Failed to read vertex source.");
		fragment_src_data[bytes_read] = '\0';


		GLint vertex_shader;
		GLCall(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
		GLCall(glShaderSource(vertex_shader, 1, &vertex_src_data, nullptr));
		GLCall(glCompileShader(vertex_shader));

		GLint fragment_shader;
		GLCall(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
		GLCall(glShaderSource(fragment_shader, 1, &fragment_src_data, nullptr));
		GLCall(glCompileShader(fragment_shader));

		GLint vertex_result = 0;
		GLCall(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vertex_result));
		if (!vertex_result) {
			GLint log_len;
			GLCall(glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocators
			GLchar* message = (GLchar*)malloc(log_len);
			GLCall(glGetShaderInfoLog(vertex_shader, log_len, nullptr, message));
			AB_CORE_FATAL("Vertex shader compilation error:\n%s", message);
		}

		GLint fragment_result = 0;
		GLCall(glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &fragment_result));
		if (!fragment_result) {
			GLint log_len;
			GLCall(glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocators
			GLchar* message = (GLchar*)malloc(log_len);
			GLCall(glGetShaderInfoLog(vertex_shader, log_len, nullptr, message));
			AB_CORE_FATAL("Frgament shader compilation error:\n%s", message);
		}

		GLint program_handle;
		GLCall(program_handle = glCreateProgram());
		GLCall(glAttachShader(program_handle, vertex_shader));
		GLCall(glAttachShader(program_handle, fragment_shader));
		GLCall(glLinkProgram(program_handle));

		GLint link_result = 0;
		GLCall(glGetProgramiv(program_handle, GL_LINK_STATUS, &link_result));
		if (!link_result) {
			int32 log_len;
			GLCall(glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &log_len));
			// TODO: Allocator
			char* message = (char*)malloc(log_len);
			GLCall(glGetProgramInfoLog(program_handle, log_len, 0, message));
			AB_CORE_FATAL("Shader program linking error:\n%s", message);
		}

		GLCall(glDeleteShader(vertex_shader));
		GLCall(glDeleteShader(fragment_shader));
		return program_handle;
	}

GLuint LoadTexture(const char* filepath) {
	Image image = LoadBMP(filepath);
	GLuint texHandle = 0;
	if (image.bitmap) {
		uint32 format = GL_RED;
		uint32 inFormat = GL_RED;
		switch (image.format) {
		case PixelFormat::RGB: {
			format = GL_RGB;
			inFormat = GL_RGB8;
		} break;
		case PixelFormat::RGBA: {
			format = GL_RGBA;
			inFormat = GL_RGBA8;
		} break;
		default: {
			// TODO: FIX THIS
			AB_CORE_ERROR("Wrong image format");
		} break;
		}
		GLCall(glGenTextures(1, &texHandle));
		GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, inFormat, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.bitmap));

		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		DeleteBitmap(image.bitmap);
	}
	else {
		AB_CORE_ERROR("Failed to load texture. Cannot load image: %s", filepath);
	}
	return texHandle;
}

	MaterialProperties* GetMaterialProperties(Material* material) {
		return &material->properties;
	}

	Material* CreateMaterial(MaterialProperties* properties) {
		Material* material = nullptr;
		material = (Material*)std::malloc(sizeof(Material));
		memset(material, 0, sizeof(Material));
		return material;
	}

	void MaterialAddTexture(Material* mt, uint32 slot, const char* texture_path) {
		auto tex_handle = LoadTexture(texture_path);
		AB_CORE_ASSERT(texture_path, "Failed to load texture");
		mt->textures[slot] = tex_handle;
	}

	void MaterialSetUniform(Material* mt, const char* name, float val) {
		GLCall(glUniform1f(glGetUniformLocation(mt->program_handle, name), val));
	}

	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector2 val) {
		GLCall(glUniform2f(glGetUniformLocation(mt->program_handle, name), val.x, val.y));
	}
	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector3 val) {
		GLCall(glUniform3f(glGetUniformLocation(mt->program_handle, name), val.x, val.y, val.z));
	}
	void MaterialSetUniform(Material* mt, const char* name, hpm::Vector4 val) {
		GLCall(glUniform4f(glGetUniformLocation(mt->program_handle, name), val.x, val.y, val.z, val.w));
	}
	void MaterialSetUniform(Material* mt, const char* name, hpm::Matrix4 val) {
		GLCall(glUniformMatrix4fv(glGetUniformLocation(mt->program_handle, name), 1, GL_FALSE, val.data));
	}
	void MaterialSetUniform(Material* mt, const char* name, int32 val) {
		GLCall(glUniform1i(glGetUniformLocation(mt->program_handle, name), val));
	}

	struct Renderer3D {
		GLint program_handle;
		GLint light_program_handle;
		GLuint vertex_buffer;
		GLuint light_vertex_buffer;
		GLuint diff_map;
		GLuint spec_map;
		hpm::Vector3 cam_front;
		hpm::Vector3 cam_pos;
		hpm::Vector3 cam_up;
		float32 yaw;
		float32 pitch;
		float32 last_mouse_x;
		float32 last_mouse_y;
	};

	Renderer3D* Initialize() {
		Renderer3D* properties = nullptr;
		properties = (Renderer3D*)malloc(sizeof(Renderer3D));
		memset(properties, 0, sizeof(Renderer3D));
		AB_CORE_ASSERT(properties, "Failed to allocate Renderer3D data.");

		properties->program_handle = LoadShader("../../../assets/shaders/MeshVertex.glsl", "../../../assets/shaders/MeshFragment.glsl");
		properties->light_program_handle = LoadShader("../../../assets/shaders/LightVertex.glsl", "../../../assets/shaders/LightFragment.glsl");

		glGenBuffers(1, &properties->vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, properties->vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 288, vertices, GL_STATIC_DRAW);
		glBindVertexArray(0);

		glGenBuffers(1, &properties->light_vertex_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, properties->light_vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * 288, vertices, GL_STATIC_DRAW);
		glBindVertexArray(0);

		glGenTextures(1, &properties->diff_map);
		glBindTexture(GL_TEXTURE_2D, properties->diff_map);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Image img = LoadBMP("../../../assets/test.bmp");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bitmap);
		DeleteBitmap(img.bitmap);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenTextures(1, &properties->spec_map);
		glBindTexture(GL_TEXTURE_2D, properties->spec_map);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Image img_spec = LoadBMP("../../../assets/spec.bmp");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, img_spec.width, img_spec.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_spec.bitmap);
		DeleteBitmap(img_spec.bitmap);
		glBindTexture(GL_TEXTURE_2D, 0);

		properties->cam_front = { 0, 0, -1 };
		properties->cam_pos = { 0, 0, 3 };
		properties->cam_up = { 0, 1, 0 };

		return  properties;
	}


	void Submit(uint32 meshHandle, uint32 materialHandle) {
		
	}

	void Render(Renderer3D* props) {
		hpm::Vector3 cubePositions[] = {
			hpm::Vector3{ 0.0f,  0.0f,  0.0f },
			hpm::Vector3{ 2.0f,  5.0f, -15.0f },
			hpm::Vector3{ -1.5f, -2.2f, -2.5f },
			hpm::Vector3{ -3.8f, -2.0f, -12.3f },
			hpm::Vector3{ 2.4f, -0.4f, -3.5f },
			hpm::Vector3{ -1.7f,  3.0f, -7.5f },
			hpm::Vector3{ 1.3f, -2.0f, -2.5f },
			hpm::Vector3{ 1.5f,  2.0f, -2.5f },
			hpm::Vector3{ 1.5f,  0.2f, -1.5f },
			hpm::Vector3{ -1.3f,  1.0f, -1.5f }
		};

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, props->diff_map);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, props->spec_map);
		glUseProgram(props->program_handle);

		props->cam_front.x = hpm::Cos(hpm::ToRadians(props->pitch)) * hpm::Cos(hpm::ToRadians(props->yaw));
		props->cam_front.y = hpm::Sin(hpm::ToRadians(props->pitch));
		props->cam_front.z = hpm::Cos(hpm::ToRadians(props->pitch)) * hpm::Sin(hpm::ToRadians(props->yaw));
		props->cam_front = hpm::Normalize(props->cam_front);
		hpm::Matrix4 view = hpm::LookAtRH(props->cam_pos, hpm::Add(props->cam_pos, props->cam_front), props->cam_up);

		hpm::Matrix4 proj = hpm::PerspectiveRH(90.0f, 800.0f / 600.0f, 0.1f, 100.0f);


		int32 view_pos_loc = glGetUniformLocation(props->program_handle, "view_pos");
		int32 light_pos_loc = glGetUniformLocation(props->program_handle, "lights[0].position");
		int32 light_amb_loc = glGetUniformLocation(props->program_handle, "lights[0].ambient");
		int32 light_diff_loc = glGetUniformLocation(props->program_handle, "lights[0].diffuse");
		int32 light_spec_loc = glGetUniformLocation(props->program_handle, "lights[0].specular");
		int32 light_ln_loc = glGetUniformLocation(props->program_handle, "lights[0].linear");
		int32 light_qd_loc = glGetUniformLocation(props->program_handle, "lights[0].quadratic");

		int32 light1_pos_loc = glGetUniformLocation(props->program_handle, "lights[1].position");
		int32 light1_amb_loc = glGetUniformLocation(props->program_handle, "lights[1].ambient");
		int32 light1_diff_loc = glGetUniformLocation(props->program_handle, "lights[1].diffuse");
		int32 light1_spec_loc = glGetUniformLocation(props->program_handle, "lights[1].specular");
		int32 light1_ln_loc = glGetUniformLocation(props->program_handle, "lights[1].linear");
		int32 light1_qd_loc = glGetUniformLocation(props->program_handle, "lights[1].quadratic");

		int32 material_shin_loc = glGetUniformLocation(props->program_handle, "material.shininess");

		int32 dl_dir_loc = glGetUniformLocation(props->program_handle, "dir_light.direction");
		int32 dl_amb_loc = glGetUniformLocation(props->program_handle, "dir_light.ambient");
		int32 dl_dif_loc = glGetUniformLocation(props->program_handle, "dir_light.diffuse");
		int32 dl_spc_loc = glGetUniformLocation(props->program_handle, "dir_light.specular");

		int32 modelLoc = glGetUniformLocation(props->program_handle, "model");
		int32 viewLoc = glGetUniformLocation(props->program_handle, "view");
		int32 projLoc = glGetUniformLocation(props->program_handle, "projection");

		glUniform1i(glGetUniformLocation(props->program_handle, "material.diffuse_map"), 0);
		glUniform1i(glGetUniformLocation(props->program_handle, "material.spec_map"), 1);

		glUniform3f(dl_dir_loc, 0, -1, 0);
		glUniform3f(dl_amb_loc, 0.1, 0.1, 0.1);
		glUniform3f(dl_dif_loc, 0.5, 0.5 ,0.5);
		glUniform3f(dl_spc_loc, 0.7, 0.7, 0.7);


		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, proj.data);
		glUniform3f(view_pos_loc, props->cam_pos.x, props->cam_pos.y, props->cam_pos.z);
		glUniform3f(light_pos_loc, 1, 0, -1);
		glUniform3f(light_amb_loc, 0.2, 0.0, 0.0);
		glUniform3f(light_diff_loc, 0.4, 0.0, 0.0);
		glUniform3f(light_spec_loc, 0.8, 0.0, 0.0);
		glUniform1f(light_ln_loc, 0.35);
		glUniform1f(light_qd_loc, 0.44);

		glUniform3f(light1_pos_loc, -1, 0, -1);
		glUniform3f(light1_amb_loc, 0.0, 0.3, 0.0);
		glUniform3f(light1_diff_loc, 0.0, 0.7, 0.0);
		glUniform3f(light1_spec_loc, 0.0, 1.0, 0.0);
		glUniform1f(light1_ln_loc, 0.35);
		glUniform1f(light1_qd_loc, 0.44);

		glUniform1f(material_shin_loc, 128.0f);


		glBindBuffer(GL_ARRAY_BUFFER, props->vertex_buffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glEnable(GL_DEPTH_TEST);
		for (GLuint i = 0; i < 10; i++) {
			hpm::Matrix4 tr = hpm::Translation(cubePositions[i]);
			GLfloat angle = 20.0f * i;
			hpm::Matrix4 rt = hpm::Identity4();//hpm::Rotate(tr, a, { 1.0f, 0.3f, 0.5f });
			hpm::Matrix4 sc = hpm::Identity4();//hpm::Scaling({1, 1, 1});
											   //rt = hpm::Scale(rt, { 2, 3, 0.2 });

			hpm::Matrix4 model = hpm::Multiply(tr, sc);
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glBindBuffer(GL_ARRAY_BUFFER, props->vertex_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glUseProgram(props->light_program_handle);
		int32 LModelLoc = glGetUniformLocation(props->light_program_handle, "model");
		int32 LViewLoc = glGetUniformLocation(props->light_program_handle, "view");
		int32 LProjLoc = glGetUniformLocation(props->light_program_handle, "projection");
		int32 LColorLoc = glGetUniformLocation(props->light_program_handle, "lightColor");

		glUniform3f(LColorLoc, 1, 1, 1);
		glUniformMatrix4fv(LViewLoc, 1, GL_FALSE, view.data);
		glUniformMatrix4fv(LProjLoc, 1, GL_FALSE, proj.data);
		glUniformMatrix4fv(LModelLoc, 1, GL_FALSE, hpm::Translation({1, 0, -1}).data);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glUniformMatrix4fv(LModelLoc, 1, GL_FALSE, hpm::Translation({-1, 0, -1}).data);

		glDrawArrays(GL_TRIANGLES, 0, 36);


		glDisable(GL_DEPTH_TEST);
	}

	void SetCameraAngles(Renderer3D* props, float32 pitch, float32 yaw) {
		props->pitch = pitch;
		props->yaw = yaw;
	}
}
#endif