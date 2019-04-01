#include "Renderer3D.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Common.h"
#include "utils/ImageLoader.h"
#include "platform/Memory.h"
#include "AssetManager.h"

namespace AB {

	// TODO: Inverse matrices in shaders might be too heavy.
	// Especially for skyboxes
	static constexpr char SKYBOX_VERTEX_PROGRAM[] = R"(
out Vector3 skyboxUV;
void main() {
Matrix4 invProj = inverse(sys_ProjectionMatrix);
Matrix3 invView = Matrix3(inverse(sys_ViewMatrix));
skyboxUV = invView * (invProj * Vector4(v_Position, 1.0f)).xyz;
out_Position = Vector4(v_Position.x, v_Position.y, 1.0f, 1.0f);
}
)";
	static constexpr char SKYBOX_FRAGMENT_PROGRAM[] = R"(
in Vector3 skyboxUV;
uniform samplerCube skybox;
void main() {
out_FragColor = texture(skybox, skyboxUV);
}
)";

	
	struct DrawCommand {
		int32 mesh_handle;
		int32 material_handle;
		hpm::Matrix4 transform;
	};

	struct Camera {
		hpm::Vector3 position;
		hpm::Vector3 front;
		hpm::Matrix4 look_at;
	};

//#pragma pack(push, 1)
	struct _PointLightStd140 {
		Vector3 position;
		float32 _pad1;
		Vector3 ambient;
		float32 _pad2;
		Vector3 diffuse;
		float32 _pad3;
		Vector3 specular;
		float32 _pad4;
		float32 linear;
		float32 quadratic;
		float32 _pad5;
		float32 _pad6;
	};
//#pragma pack(pop)

	static constexpr int32 DRAW_BUFFER_SIZE = 256;
	static constexpr uint32 SYSTEM_UBO_SIZE = sizeof(Matrix4) * 4 + sizeof(Vector4);
	static constexpr uint32 SYSTEM_UBO_VERTEX_OFFSET = 0;
	static constexpr uint32 SYSTEM_UBO_VERTEX_SIZE = sizeof(Matrix4) * 4;
	static constexpr uint32 SYSTEM_UBO_VERTEX_VIEWPROJ_OFFSET = 0;
	static constexpr uint32 SYSTEM_UBO_VERTEX_VIEW_OFFSET = sizeof(Matrix4) * 1;
	static constexpr uint32 SYSTEM_UBO_VERTEX_PROJ_OFFSET = sizeof(Matrix4) * 2;
	static constexpr uint32 SYSTEM_UBO_VERTEX_NORMAL_OFFSET = sizeof(Matrix4) * 3;
	static constexpr uint32 SYSTEM_UBO_FRAGMENT_OFFSET = sizeof(Matrix4) * 4;
	static constexpr uint32 SYSTEM_UBO_FRAGMENT_SIZE= sizeof(Vector4);

	static constexpr uint32 POINT_LIGHTS_NUMBER = 2;
	static constexpr uint32 POINT_LIGHT_STRUCT_SIZE = sizeof(Vector4) * 4 + sizeof(float32) * 2;
	static constexpr uint32 POINT_LIGHT_STRUCT_ALIGMENT = sizeof(Vector4);
	static constexpr uint32 POINT_LIGHT_UBO_SIZE = POINT_LIGHTS_NUMBER * (POINT_LIGHT_STRUCT_SIZE + sizeof(float32) * 2); 

	struct Renderer {
		uint32 vertexSystemUBHandle;
		uint32 pointLightUBHandle;
		int32 skyboxHandle;
		int32 skyboxProgramHandle;
		uint32 skyboxVB;
		int32 program_handle;
		uint32 draw_buffer_at;
		DrawCommand draw_buffer[DRAW_BUFFER_SIZE];
		Camera camera;
		hpm::Matrix4 projection;
		DirectionalLight dir_light;
		PointLight pointLights[POINT_LIGHTS_NUMBER];
	};

	static uint32 RendererCreateProgram(const char* vertexSource, const char* fragmentSource) 
	{

		const char* commonShaderHeader = R"(
#version 330 core
#define Vector2 vec2
#define Vector3 vec3
#define Vector4 vec4
#define Matrix4 mat4
#define Matrix3 mat3
#define float32 float
)";

		const char* vertexShaderHeader = R"(
#define out_Position gl_Position
layout (location = 0) in Vector3 v_Position;
layout (location = 1) in Vector2 v_UV;
layout (location = 2) in Vector3 v_Normal;

layout (std140) uniform _vertexSystemUniformBlock {
Matrix4 sys_ViewProjMatrix;
Matrix4 sys_ViewMatrix;
Matrix4 sys_ProjectionMatrix;
Matrix4 sys_NormalMatrix;
};
uniform Matrix4 sys_ModelMatrix;
)";

		const char* fragmentShaderHeader = R"(
out vec4 out_FragColor;
layout(std140) uniform _fragmentSystemUniformBlock {
Vector3 sys_ViewPos;};
)";

		uint64 commonHeaderLength = strlen(commonShaderHeader);
		uint64 vertexHeaderLength = strlen(vertexShaderHeader);
		uint64 fragmentHeaderLength = strlen(fragmentShaderHeader);
		uint64 fragmentSourceLength = strlen(fragmentSource);
		uint64 vertexSourceLength = strlen(vertexSource);

		// TODO: allocation
		char* fullVertexSource = (char*)malloc(commonHeaderLength + vertexHeaderLength + vertexSourceLength + 1);
		AB_CORE_ASSERT(fullVertexSource);
		CopyArray(char, commonHeaderLength, fullVertexSource, commonShaderHeader);
		CopyArray(char, vertexHeaderLength, fullVertexSource + commonHeaderLength, vertexShaderHeader);
		CopyArray(char, vertexSourceLength + 1, fullVertexSource + commonHeaderLength + vertexHeaderLength, vertexSource);

		char* fullFragmentSource = (char*)malloc(commonHeaderLength + fragmentHeaderLength + fragmentSourceLength + 1);
		AB_CORE_ASSERT(fullFragmentSource);
		CopyArray(char, commonHeaderLength, fullFragmentSource, commonShaderHeader);
		CopyArray(char, fragmentHeaderLength, fullFragmentSource + commonHeaderLength, fragmentShaderHeader);
		CopyArray(char, fragmentSourceLength + 1, fullFragmentSource + commonHeaderLength + fragmentHeaderLength, fragmentSource);

		uint32 resultHandle = 0;
		GLint vertexHandle;
		GLCall(vertexHandle = glCreateShader(GL_VERTEX_SHADER));
		if (vertexHandle) 
		{
			GLCall(glShaderSource(vertexHandle, 1, &fullVertexSource, nullptr));
			GLCall(glCompileShader(vertexHandle));

			GLint vertexResult = 0;
			GLCall(glGetShaderiv(vertexHandle, GL_COMPILE_STATUS, &vertexResult));
			if (vertexResult)
			{
				GLint fragmentHandle;
				GLCall(fragmentHandle = glCreateShader(GL_FRAGMENT_SHADER));
				if (fragmentHandle)
				{
					GLCall(glShaderSource(fragmentHandle, 1, &fullFragmentSource, nullptr));
					GLCall(glCompileShader(fragmentHandle));

					GLint fragmentResult = 0;
					GLCall(glGetShaderiv(fragmentHandle, GL_COMPILE_STATUS, &fragmentResult));
					if (fragmentResult)
					{
						GLint programHandle;
						GLCall(programHandle = glCreateProgram());
						if (programHandle) 
						{
							GLCall(glAttachShader(programHandle, vertexHandle));
							GLCall(glAttachShader(programHandle, fragmentHandle));
							GLCall(glLinkProgram(programHandle));

							GLint linkResult = 0;
							GLCall(glGetProgramiv(programHandle, GL_LINK_STATUS, &linkResult));
							if (linkResult) 
							{
								GLCall(glDeleteShader(vertexHandle));
								GLCall(glDeleteShader(fragmentHandle));
								resultHandle = programHandle;
							} 
							else 
							{
								int32 logLength;
								GLCall(glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength));
								// TODO: Allocator
								char* message = (char*)malloc(logLength);
								AB_CORE_ASSERT(message);
								GLCall(glGetProgramInfoLog(programHandle, logLength, 0, message));
								AB_CORE_ERROR("Shader program linking error:\n%s", message);
								free(message);
							}
						} 
						else 
						{
							AB_CORE_ERROR("Falled to create shader program");
						}
					}
					else
					{
						GLint logLength;
						GLCall(glGetShaderiv(fragmentHandle, GL_INFO_LOG_LENGTH, &logLength));
						// TODO: Allocators
						GLchar* message = (GLchar*)malloc(logLength);
						AB_CORE_ASSERT(message);
						GLCall(glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message));
						AB_CORE_ERROR("Frgament shader compilation error:\n%s", message);
						free(message);
					}
				}
				else 
				{
					AB_CORE_ERROR("Falled to create fragment shader");
				}
			}
			else 
			{
				GLint logLength;
				GLCall(glGetShaderiv(vertexHandle, GL_INFO_LOG_LENGTH, &logLength));
				// TODO: Allocators
				GLchar* message = (GLchar*)malloc(logLength);
				AB_CORE_ASSERT(message);
				GLCall(glGetShaderInfoLog(vertexHandle, logLength, nullptr, message));
				AB_CORE_ERROR("Vertex shader compilation error:\n%s", message);
				free(message);
			}
		}
		else 
		{
			AB_CORE_ERROR("Falled to create vertex shader");
		}

		free(fullVertexSource);
		free(fullFragmentSource);

		return resultHandle;
	}

	
	static void BindSystemUniformBuffer(Renderer* renderer, int32 programHandle) {
		uint32 sysUBOVertexIndex;
		GLCall(sysUBOVertexIndex = glGetUniformBlockIndex(programHandle,
														  "_vertexSystemUniformBlock"));
		GLCall(glUniformBlockBinding(programHandle, sysUBOVertexIndex, 0));
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->vertexSystemUBHandle,
								 SYSTEM_UBO_VERTEX_OFFSET, SYSTEM_UBO_VERTEX_SIZE));

		uint32 sysUBOFragIndex;
		GLCall(sysUBOFragIndex = glGetUniformBlockIndex(programHandle,
														"_fragmentSystemUniformBlock"));
		GLCall(glUniformBlockBinding(programHandle, sysUBOFragIndex, 1));
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 1, renderer->vertexSystemUBHandle,
								 SYSTEM_UBO_FRAGMENT_OFFSET, SYSTEM_UBO_FRAGMENT_SIZE));
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

	Renderer* RendererInit() {
		Renderer* props = nullptr;
		if (!(PermStorage()->forward_renderer)) {
			props = (Renderer*)SysAlloc(sizeof(Renderer));
		} else {
			AB_CORE_WARN("Renderer already initialized.");
		}

		auto[vertexSource, vSize] = DebugReadTextFile("../assets/shaders/MeshVertex.glsl");
		auto[fragmentSource, fSize] = DebugReadTextFile("../assets/shaders/MeshFragment.glsl");

		props->program_handle = RendererCreateProgram(vertexSource, fragmentSource);

		uint32 sysVertexUB;
		GLCall(glGenBuffers(1, &sysVertexUB));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sysVertexUB));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, SYSTEM_UBO_SIZE, NULL, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		props->vertexSystemUBHandle = sysVertexUB;

		uint32 pointLightUB;
		GLCall(glGenBuffers(1, &pointLightUB));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, pointLightUB));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, POINT_LIGHT_UBO_SIZE, NULL, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		props->pointLightUBHandle = pointLightUB;
		
		DebugFreeFileMemory(vertexSource);
		DebugFreeFileMemory(fragmentSource);

		props->skyboxProgramHandle = RendererCreateProgram(SKYBOX_VERTEX_PROGRAM,
														   SKYBOX_FRAGMENT_PROGRAM);
		float32 fullscreenQuadVertices[18] = {
			-1.0f,  -1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f,
			1.0f,  -1.0f, 0.0f,
			-1.0f, -1.0f, 0.0f
		};
			
		GLCall(glGenBuffers(1, &props->skyboxVB));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, props->skyboxVB));
		GLCall(glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(float32), fullscreenQuadVertices, GL_STATIC_DRAW));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		
		props->projection = hpm::PerspectiveRH(45.0f, 16.0f / 9.0f, 0.1f, 100.0f);
		AB::GetMemory()->perm_storage.forward_renderer = props;

		return props;
	}

	void RendererSetSkybox(Renderer* renderer, int32 cubemapHandle) {
		renderer->skyboxHandle = cubemapHandle;
	}

	void RendererSetDirectionalLight(Renderer* renderer, const DirectionalLight* light) {
		renderer->dir_light = *light;
	}

	AB_API void RendererSetPointLight(Renderer* renderer, uint32 index, PointLight* light) {
		CopyArray(PointLight, 1, renderer->pointLights + index, light);
	}

	void RendererSetCamera(Renderer* renderer, hpm::Vector3 front, hpm::Vector3 position) {
		renderer->camera.front = hpm::Normalize(front);
		renderer->camera.position = position;
		renderer->camera.look_at = hpm::LookAtRH(renderer->camera.position, hpm::Add(renderer->camera.position, renderer->camera.front), {0.0f, 1.0f, 0.0f});
	}


	void RendererSubmit(Renderer* renderer, int32 mesh_handle, int32 material_handle, const hpm::Matrix4* transform) {
		if (renderer->draw_buffer_at < DRAW_BUFFER_SIZE) {
			renderer->draw_buffer[renderer->draw_buffer_at].mesh_handle = mesh_handle;
			renderer->draw_buffer[renderer->draw_buffer_at].material_handle = material_handle;
			renderer->draw_buffer[renderer->draw_buffer_at].transform = *transform;
			renderer->draw_buffer_at++;
		}
	}

	static void DrawSkybox(Renderer* renderer) {
		if (renderer->skyboxHandle) {
			GLCall(glEnable(GL_DEPTH_TEST));
			GLCall(glDepthMask(GL_FALSE));
			AB_GLCALL(glDepthFunc(GL_LEQUAL));
			GLCall(glUseProgram(renderer->skyboxProgramHandle));
			GLCall(glActiveTexture(GL_TEXTURE0));
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->skyboxHandle));
			GLCall(glUniform1i(glGetUniformLocation(renderer->skyboxProgramHandle,
													"skybox"), 0));
			BindSystemUniformBuffer(renderer, renderer->skyboxProgramHandle);
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, renderer->skyboxVB));
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(hpm::Vector3), (void*)0));
			GLCall(glEnableVertexAttribArray(0));
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
			GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
			GLCall(glUseProgram(0));
		}
	}
	
	void RendererRender(Renderer* renderer) {

		// TODO: Temporary setting culling here
		GLCall(glDisable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
		GLCall(glFrontFace(GL_CCW));

		Matrix4 viewProj = Multiply(renderer->projection, renderer->camera.look_at);
		hpm::Vector3* view_pos = &renderer->camera.position;

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, renderer->vertexSystemUBHandle));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_VIEWPROJ_OFFSET, sizeof(Matrix4), viewProj.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_VIEW_OFFSET, sizeof(Matrix4), renderer->camera.look_at.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_PROJ_OFFSET, sizeof(Matrix4), renderer->projection.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_FRAGMENT_OFFSET, SYSTEM_UBO_FRAGMENT_SIZE, view_pos->data));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, renderer->pointLightUBHandle));
		_PointLightStd140* buffer;
		GLCall(buffer = (_PointLightStd140*)glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
		for (uint32 i = 0; i < POINT_LIGHTS_NUMBER; i++)
		{
			buffer[i].position = renderer->pointLights[i].position;
			buffer[i].ambient = renderer->pointLights[i].ambient;
			buffer[i].diffuse = renderer->pointLights[i].diffuse;
			buffer[i].specular = renderer->pointLights[i].specular;
			buffer[i].linear = renderer->pointLights[i].linear;
			buffer[i].quadratic = renderer->pointLights[i].quadratic;
		}
		GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "sys_ViewProjMatrix"), 1, GL_FALSE, viewProj.data));
		
		DrawSkybox(renderer);
		
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDepthMask(GL_TRUE));
		GLCall(glDepthFunc(GL_LESS));
		
		GLCall(glUseProgram(renderer->program_handle));

		BindSystemUniformBuffer(renderer, renderer->program_handle);
		
		uint32 pointLightsUBOIndex;
		GLCall(pointLightsUBOIndex = glGetUniformBlockIndex(renderer->program_handle, "pointLightsData"));
		GLCall(glUniformBlockBinding(renderer->program_handle, pointLightsUBOIndex, 2));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2, renderer->pointLightUBHandle));

		//GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "projection"), 1, GL_FALSE, renderer->projection.data));
		//GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "view"), 1, GL_FALSE, renderer->camera.look_at.data));

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
			GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "material.ambinet"), 1,  mesh->material->ambient.data));
			GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "material.diffuse"), 1, mesh->material->diffuse.data));
			GLCall(glUniform3fv(glGetUniformLocation(renderer->program_handle, "material.specular"), 1, mesh->material->specular.data));
			GLCall(glUniform1f(glGetUniformLocation(renderer->program_handle, "material.shininess"), mesh->material->shininess));

			GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "sys_ModelMatrix"), 1, GL_FALSE, command->transform.data));


			Matrix4 inv = Inverse(command->transform);
			Matrix4 normalMatrix = Transpose(inv);
		
			GLCall(glBindBuffer(GL_UNIFORM_BUFFER, renderer->vertexSystemUBHandle));
			GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_NORMAL_OFFSET, sizeof(Matrix4), normalMatrix.data));
			GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			
			GLCall(glActiveTexture(GL_TEXTURE0));
			Texture* diff_texture = AssetGetTextureData(PermStorage()->asset_manager, mesh->material->diff_map_handle);
			if (diff_texture) {
				GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.use_diff_map"), 1));
				GLCall(glBindTexture(GL_TEXTURE_2D, diff_texture->api_handle));
			} else {
				GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.use_diff_map"), 0));
			}
			//GLCall(glBindTexture(GL_TEXTURE_2D, mesh->material->diffuse_map_handle));
			GLCall(glActiveTexture(GL_TEXTURE1));
			Texture* spec_texture = AssetGetTextureData(PermStorage()->asset_manager, mesh->material->spec_map_handle);
			if (spec_texture) {
				GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.use_spec_map"), 1));
				GLCall(glBindTexture(GL_TEXTURE_2D, spec_texture->api_handle));
			}
			else {
				GLCall(glUniform1i(glGetUniformLocation(renderer->program_handle, "material.use_spec_map"), 0));
			}
			//GLCall(glBindTexture(GL_TEXTURE_2D, mesh->material->specular_map_handle));


			if (mesh->api_ib_handle != 0) {
				GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)mesh->num_indices, GL_UNSIGNED_INT, 0));
			} else {
				GLCall(glDrawArrays(GL_TRIANGLES, 0, mesh->num_vertices));
			}
		}


		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}
}
