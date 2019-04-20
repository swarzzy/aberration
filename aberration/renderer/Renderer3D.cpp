#include "Renderer3D.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Common.h"
#include "utils/ImageLoader.h"
#include "platform/Memory.h"
#include "AssetManager.h"
#include "platform/Window.h"

namespace AB {

	// TODO: Inverse matrices in shaders might be too heavy.
	// Especially for skyboxes
	static constexpr char SKYBOX_VERTEX_PROGRAM[] = R"(
out Vector3 skyboxUV;
const Vector2 fullscreenQuad[6] = Vector2[6](Vector2(-1.0f, -1.0f),
								   Vector2(1.0f, -1.0f),
								   Vector2(1.0f, 1.0f),
								   Vector2(1.0f, 1.0f),
								   Vector2(-1.0f, 1.0f),
								   Vector2(-1.0f, -1.0f));
void main() {
Matrix4 invProj = inverse(sys_ProjectionMatrix);
Matrix3 invView = Matrix3(inverse(sys_ViewMatrix));
skyboxUV = invView * (invProj * Vector4(fullscreenQuad[gl_VertexID], 0.0f, 1.0f)).xyz;
out_Position = Vector4(fullscreenQuad[gl_VertexID], 0.0f, 1.0f);
out_Position = out_Position.xyww;
}
)";
	static constexpr char SKYBOX_FRAGMENT_PROGRAM[] = R"(
in Vector3 skyboxUV;
uniform samplerCube skybox;
void main() {
out_FragColor = texture(skybox, skyboxUV);
}
)";

	static constexpr char POSTFX_VERTEX_PROGRAM[] = R"(
out Vector2 UV;
const Vector2 fullscreenQuad[6] = Vector2[6](Vector2(-1.0f, -1.0f),
								   Vector2(1.0f, -1.0f),
								   Vector2(1.0f, 1.0f),
								   Vector2(1.0f, 1.0f),
								   Vector2(-1.0f, 1.0f),
								   Vector2(-1.0f, -1.0f));

const Vector2 quadUV[6] = Vector2[6](Vector2(0.0f, 0.0f),
					   Vector2(1.0f, 0.0f),
					   Vector2(1.0f, 1.0f),
					   Vector2(1.0f, 1.0f),
					   Vector2(0.0f, 1.0f),
					   Vector2(0.0f, 0.0f));
void main() {
UV = quadUV[gl_VertexID];
out_Position = Vector4(fullscreenQuad[gl_VertexID], 1.0f, 1.0f);
}
)";

	static constexpr char POSTFX_FRAGMENT_PROGRAM[] = R"(
in Vector2 UV;
uniform sampler2D colorBuffer;
uniform float32 u_Gamma = 2.2f;
uniform float32 u_Exposure = 1.0f;
void main() {
Vector4 sample = texture(colorBuffer, UV);
//Vector3 mappedColor = sample.xyz / (sample.xyz + (Vector3(1.0)));
float32 exposure = 1.0f;
Vector3 mappedColor = Vector3(1.0f) - exp(-sample.xyz * u_Exposure);
//mappedColor = sample.xyz;
out_FragColor = Vector4(pow(mappedColor, 1.0f / Vector3(u_Gamma)), sample.a);
//out_FragColor = Vector4(mappedColor, sample.a);
}
)";

	
	struct DrawCommand {
		int32 mesh_handle;
		hpm::Matrix4 transform;
		BlendMode blendMode;
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

	static constexpr uint32 POINT_LIGHTS_NUMBER = 4;
	static constexpr uint32 POINT_LIGHT_STRUCT_SIZE = sizeof(Vector4) * 4 + sizeof(float32) * 2;
	static constexpr uint32 POINT_LIGHT_STRUCT_ALIGMENT = sizeof(Vector4);
	static constexpr uint32 POINT_LIGHT_UBO_SIZE = POINT_LIGHTS_NUMBER * (POINT_LIGHT_STRUCT_SIZE + sizeof(float32) * 2); 

	struct Renderer {
		
		uint32 renderBufferSize;
		uint32 renderBufferFree;
		byte* renderBufferBegin;
		byte* renderBufferAt;

		uint32 commandQueueCapacity;
		RenderBucket* commandQueueBegin;
		RenderBucket* tmpCommandQueueBegin;
		uint32 commandQueueAt;
		
		RendererConfig config;
		RendererFlySettings flySettings;
		uint32 vertexSystemUBHandle;
		uint32 pointLightUBHandle;
		int32 skyboxHandle;
		int32 skyboxProgramHandle;
		int32 postFXProgramHandle;
		uint32 msColorTargetHandle;
		uint32 msDepthTargetHandle;
		uint32 postfxColorSourceHandle;
		uint32 postfxDepthSourceHandle;
		uint32 multisampledFBOHandle;
		uint32 postfxFBOHandle;
		int32 program_handle;
		uint32 draw_buffer_at;
		DrawCommand draw_buffer[DRAW_BUFFER_SIZE];
		Camera camera;
		hpm::Matrix4 projection;
		DirectionalLight dir_light;
		PointLight pointLights[POINT_LIGHTS_NUMBER];
		uint32 pointLightsAt;
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

#if 0
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
#endif

	static void _ReloadMultisampledFramebuffer(Renderer* renderer, bool32 recreateTextures, uint32 samples) {
		if (recreateTextures) {
			// Recreate textures
			GLCall(glDeleteTextures(1, &renderer->msColorTargetHandle));
			GLCall(glDeleteTextures(1, &renderer->msDepthTargetHandle));
			GLCall(glGenTextures(1, &renderer->msColorTargetHandle));
			GLCall(glGenTextures(1, &renderer->msDepthTargetHandle));
		}
		
		if (!samples) {
			GLCall(glBindTexture(GL_TEXTURE_2D, renderer->msColorTargetHandle));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
								renderer->config.renderResolutionW,
								renderer->config.renderResolutionH,
								0, GL_RGBA, GL_FLOAT, nullptr));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
			GLCall(glBindTexture(GL_TEXTURE_2D, renderer->msDepthTargetHandle));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
								renderer->config.renderResolutionW,
								renderer->config.renderResolutionH,
								0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr
								));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		   
		} else {
			GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
								 renderer->msColorTargetHandle
								 ));
			
			GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
										   samples, GL_RGBA16F,
										   renderer->config.renderResolutionW,
										   renderer->config.renderResolutionH,
										   GL_FALSE
										   ));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
			GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
								 renderer->msDepthTargetHandle
								 ));
			
			GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
										   samples, GL_DEPTH_COMPONENT32,
										   renderer->config.renderResolutionW,
										   renderer->config.renderResolutionH,
										   GL_FALSE
										   ));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}

		if (recreateTextures) {
			API::BindFramebuffer(renderer->multisampledFBOHandle, API::FB_TARGET_DRAW);
			if (!samples) {
				API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
											  API::FB_ATTACHMENT_COLOR0,
											  renderer->msColorTargetHandle
											  );

				API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
											  API::FB_ATTACHMENT_DEPTH,
											  renderer->msDepthTargetHandle
											  );
			} else {
				API::FramebufferAttachTextureMultisample(API::FB_TARGET_DRAW,
														 API::FB_ATTACHMENT_COLOR0,
														 renderer->msColorTargetHandle
														 );

				API::FramebufferAttachTextureMultisample(API::FB_TARGET_DRAW,
														 API::FB_ATTACHMENT_DEPTH,
														 renderer->msDepthTargetHandle
														 );			
			}

			bool32 framebuffer = API::ValidateFramebuffer(API::FB_TARGET_DRAW);
			AB_CORE_ASSERT(framebuffer, "Multisampled framebuffer is incomplete");

		}
	}

	static void _ReloadDownsampledFramebuffer(Renderer* renderer, bool32 recreateTextures) {
		if (recreateTextures) {
			GLCall(glDeleteTextures(1, &renderer->postfxColorSourceHandle));
			GLCall(glDeleteTextures(1, &renderer->postfxDepthSourceHandle));
			GLCall(glGenTextures(1, &renderer->postfxColorSourceHandle));
			GLCall(glGenTextures(1, &renderer->postfxDepthSourceHandle));
		}
		
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->postfxColorSourceHandle));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
							renderer->config.renderResolutionW,
							renderer->config.renderResolutionH,
							0, GL_RGBA, GL_FLOAT, nullptr
							));
		
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->postfxDepthSourceHandle));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
							renderer->config.renderResolutionW,
							renderer->config.renderResolutionH,
							0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr
							));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		if (recreateTextures) {
	 		API::BindFramebuffer(renderer->postfxFBOHandle, API::FB_TARGET_DRAW);
			API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
										  API::FB_ATTACHMENT_COLOR0,
										  renderer->postfxColorSourceHandle);
			API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
										  API::FB_ATTACHMENT_DEPTH,
										  renderer->postfxDepthSourceHandle);
		
			bool32 framebuffer = API::ValidateFramebuffer(API::FB_TARGET_DRAW);
			AB_CORE_ASSERT(framebuffer, "Downsampled framebuffer is incomplete");
		}
	}

	void RendererAllocateBuffers(Memory* memory, Renderer* renderer,
								 uint32 rbSize, uint32 commandQueueCap) {
		// TODO: Check for free space
		//AB_CORE_ASSERT(renderer->RenderBufferSize < memory->)
		if (!renderer->renderBufferBegin) {
			renderer->renderBufferSize = rbSize;
			renderer->renderBufferBegin = (byte*)SysAlloc(renderer->renderBufferSize);
			AB_CORE_ASSERT(renderer->renderBufferBegin, "Failed to allocate render buffer");
			renderer->renderBufferAt = renderer->renderBufferBegin;
			renderer->renderBufferFree = renderer->renderBufferSize;
		} else {
			AB_CORE_ASSERT(false);
		};

		if (!renderer->commandQueueBegin) {
			renderer->commandQueueCapacity = commandQueueCap;
			renderer->commandQueueAt = 0;
			uint64 _commandQueueSize = commandQueueCap * sizeof(RenderBucket);
			uint32 commandQueueSize = 0;
			AB_CORE_ASSERT(_commandQueueSize < 0xffffffff,
						   "Renderer command queue can't be bigger than 4gb.");
		    commandQueueSize = (uint32)_commandQueueSize;
			renderer->commandQueueBegin = (RenderBucket*)SysAllocAligned(commandQueueSize, alignof(RenderBucket));
			renderer->tmpCommandQueueBegin = (RenderBucket*)SysAllocAligned(commandQueueSize, alignof(RenderBucket));
			AB_CORE_ASSERT(renderer->commandQueueBegin,
						   "Failed to allocate command queue.");
			AB_CORE_ASSERT(renderer->tmpCommandQueueBegin,
						   "Failed to allocate command queue.");

		} else {
			AB_CORE_ASSERT(false);
		}
	};

	void RendererBeginFrame(Renderer* renderer) {
		renderer->renderBufferAt = renderer->renderBufferBegin;
		renderer->renderBufferFree = renderer->renderBufferSize;
		renderer->commandQueueAt = 0;
	}

	inline static void* _PushRenderData(Renderer* renderer, uint32 size, uint32 aligment, void* data) {
		uint32 padding = 0;
		uint32 useAligment = 0;
		byte* currentAt = renderer->renderBufferAt;

		if (aligment == 0) {
			AB_CORE_WARN("Zero aligment specified to render data. Assuming aligment = 1");
			useAligment = 1;
		} else {
			useAligment = aligment;
		}

		if ((uintptr)currentAt % useAligment != 0) {
			padding = (useAligment - (uintptr)currentAt % useAligment) % useAligment;
		}

		AB_CORE_ASSERT(size + padding < renderer->renderBufferFree,
					   "Not enough space in render buffer.");

		renderer->renderBufferAt += size + padding + 1;
		renderer->renderBufferFree -= size + padding + 1;
		byte* nextAt = currentAt + padding;

		memcpy(nextAt, data, size);

		AB_CORE_ASSERT((uintptr)nextAt % (uintptr)useAligment == 0, "Wrong aligment");

		return (void*)nextAt;
	}

#define PushRenderBucket(rendr, cmd)														\
	do{RenderBucket* _renderBucketDest = rendr->commandQueueBegin + rendr->commandQueueAt; 	\
	CopyScalar(RenderBucket, _renderBucketDest, &cmd);										\
	rendr->commandQueueAt++; }while(false)																
#define RENDER_KEY_INSERT_KIND(key, type) key |= (uint64)type << 63
#define RENDER_KEY_INSERT_BLEND_TYPE(key, type) key |= (uint64)type << 62
#define RENDER_KEY_INSERT_DEPTH(key, depth) 				\
	union {float32 flt; uint32 uint;} _pun_##type;			\
	_pun_##type.flt = depth;								\
	key |= (uint64)_pun_##type.uint << 30					

	
	void RendererPushCommand(Renderer* renderer, RenderCommandType rcType, void* rc) {
		void* renderDataPtr = nullptr;
		RenderBucket command = {};
		
		switch (rcType) {
		// TODO: Move repetetive parts out of cases
		case RENDER_COMMAND_DRAW_MESH: {
			RenderCommandDrawMesh* renderData = (RenderCommandDrawMesh*)rc;
			RENDER_KEY_INSERT_KIND(command.sortKey, KIND_BIT_DRAW_CALL);
			RenderKeyBlendTypeBit blendBit = {};
			if (renderData->blendMode == BLEND_MODE_OPAQUE) {
				blendBit = BLEND_TYPE_BIT_OPAQUE;
			} else {
				blendBit = BLEND_TYPE_BIT_TRANSPARENT;					
			}

			RENDER_KEY_INSERT_BLEND_TYPE(command.sortKey, blendBit);

			float32 distanceToCamSq;
			if (renderData->sortCriteria == RENDER_SORT_CRITERIA_MESH_ORIGIN) {
				Vector3 meshCenter = GetPosition(&renderData->transform.worldMatrix);
				distanceToCamSq = DistanceSq(meshCenter, renderer->camera.position);
				
			} else if (renderData->sortCriteria == RENDER_SORT_CRITERIA_NEAREST_VERTEX) {
				Mesh* mesh = AssetGetMeshData(PermStorage()->asset_manager,
											  renderData->meshHandle);
				AB_CORE_ASSERT(mesh, "Mesh pointer was nullptr");
				BBoxAligned bbox = mesh->aabb;

				v4 points[8];
				
				points[0] = V4(bbox.min, 1.0f);
				points[1] = V4(bbox.max.x, bbox.min.y, bbox.min.z, 1.0f);
				points[2] = V4(bbox.max.x, bbox.min.y, bbox.max.z, 1.0f);
				points[3] = V4(bbox.min.x, bbox.min.y, bbox.max.z, 1.0f);
				points[4] = V4(bbox.min.x, bbox.max.y, bbox.min.z, 1.0f);
				points[5] = V4(bbox.max.x, bbox.max.y, bbox.min.z, 1.0f);
				points[6] = V4(bbox.max.x, bbox.max.y, bbox.max.z, 1.0f);
				points[7] = V4(bbox.min.x, bbox.max.y, bbox.max.z, 1.0f);

				v4 point0 = MulM4V4(renderData->transform.worldMatrix,
									points[0]);
				
				float32 minDistanceSq = DistanceSq(V3(point0),
												   renderer->camera.position);
				
				for (int i = 1; i < 8; i++) {
					v4 pointInWorld = MulM4V4(renderData->transform.worldMatrix,
											  points[i]);
					float32 d = DistanceSq(V3(pointInWorld),
										   renderer->camera.position);
					if (d < minDistanceSq) {
						minDistanceSq = d;
					}
				}

				distanceToCamSq = minDistanceSq;				
			} else {
				AB_CORE_FATAL("Undefined RenderSortCriteria value.");
			}
			
			float32 sortValue;
			if (renderData->blendMode == BLEND_MODE_OPAQUE) {
				sortValue = distanceToCamSq;
			} else {
				// NOTE: Inverting distance in order to sort
				// transparent sprites from less significant to most
				if (distanceToCamSq < 0.001f) {
					sortValue = FLT_MAX;
				} else {
					sortValue = 1.0f / distanceToCamSq;
				}

				DEBUG_OVERLAY_PUSH_VAR("Distance to cam", distanceToCamSq);

			}
			RENDER_KEY_INSERT_DEPTH(command.sortKey, sortValue);
			command.commandType = RENDER_COMMAND_DRAW_MESH;

			renderDataPtr = _PushRenderData(renderer,
											sizeof(RenderCommandDrawMesh),
											alignof(RenderCommandDrawMesh),
											rc);
		} break;
		case RENDER_COMMAND_DRAW_MESH_WIREFRAME: {
			RenderCommandDrawMeshWireframe* renderData = (RenderCommandDrawMeshWireframe*)rc;
			RENDER_KEY_INSERT_KIND(command.sortKey, KIND_BIT_DRAW_CALL);
			RenderKeyBlendTypeBit blendBit = {};
			if (renderData->blendMode == BLEND_MODE_OPAQUE) {
				blendBit = BLEND_TYPE_BIT_OPAQUE;
			} else {
				blendBit = BLEND_TYPE_BIT_TRANSPARENT;					
			}
				
			RENDER_KEY_INSERT_BLEND_TYPE(command.sortKey, blendBit);
			RENDER_KEY_INSERT_DEPTH(command.sortKey, 0xffffffff);
			command.commandType = RENDER_COMMAND_DRAW_MESH_WIREFRAME;

			renderDataPtr = _PushRenderData(renderer,
											sizeof(RenderCommandDrawMeshWireframe),
											alignof(RenderCommandDrawMeshWireframe),
											rc);
		} break;
			default: { AB_CORE_ASSERT(FALSE);} break;
		}

		uintptr offset = (uintptr)renderDataPtr - (uintptr)renderer->renderBufferBegin;
		command.rbOffset = SafeCastUptrU32(offset);
		PushRenderBucket(renderer, command);
	}

	void RendererEndFrame(Renderer* renderer) {
	}
	
	Renderer* RendererInit(RendererConfig config) {
		Renderer* props = nullptr;
		if (!(PermStorage()->forward_renderer)) {
			props = (Renderer*)SysAlloc(sizeof(Renderer));
		} else {
			AB_CORE_WARN("Renderer already initialized.");
		}

		CopyScalar(RendererConfig, &props->config, &config);

		auto[vertexSource, vSize] = DebugReadTextFile("../assets/shaders/MeshVertex.glsl");
		auto[fragmentSource, fSize] = DebugReadTextFile("../assets/shaders/MeshFragment.glsl");

   		props->program_handle = RendererCreateProgram(vertexSource, fragmentSource);
		props->postFXProgramHandle = RendererCreateProgram(POSTFX_VERTEX_PROGRAM, POSTFX_FRAGMENT_PROGRAM);

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

		props->multisampledFBOHandle = API::CreateFramebuffer();
		props->postfxFBOHandle = API::CreateFramebuffer();

		_ReloadMultisampledFramebuffer(props, true, props->config.numSamples);
		_ReloadDownsampledFramebuffer(props, true);

		API::BindFramebuffer(API::DEFAULT_FB_HANDLE, API::FB_TARGET_DRAW);

		float32 aspectRatio = (float32)props->config.renderResolutionW /
								(float32)props->config.renderResolutionH;
							
		props->projection = hpm::PerspectiveRH(45.0f, aspectRatio, 0.1f, 100.0f);
		AB::GetMemory()->perm_storage.forward_renderer = props;

		return props;
	}

	RendererFlySettings* RendererGetFlySettings(Renderer* renderer) {
		return &renderer->flySettings;
	}

	RendererConfig RendererGetConfig(Renderer* renderer) {
		return renderer->config;
	}

	void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig) {
		if (newConfig->numSamples != renderer->config.numSamples) {
			bool32 multisamplingModeChanged = false;
			if (newConfig->numSamples == 0 || renderer->config.numSamples == 0) {
				multisamplingModeChanged = true;
			}
			_ReloadMultisampledFramebuffer(renderer,
										   multisamplingModeChanged,
										   newConfig->numSamples
										   );

		}
		if (newConfig->renderResolutionW != renderer->config.renderResolutionW ||
			newConfig->renderResolutionH != renderer->config.renderResolutionH) {
			renderer->config.renderResolutionW = newConfig->renderResolutionW;
			renderer->config.renderResolutionH = newConfig->renderResolutionH;
			
			_ReloadMultisampledFramebuffer(renderer, false, renderer->config.numSamples);
			_ReloadDownsampledFramebuffer(renderer, false);
			float32 aspectRatio = (float32)renderer->config.renderResolutionW /
								(float32)renderer->config.renderResolutionH;
			renderer->projection = hpm::PerspectiveRH(45.0f, aspectRatio, 0.1f, 100.0f);
		}
		CopyScalar(RendererConfig, &renderer->config, newConfig);
		
	}
	
	void RendererSetSkybox(Renderer* renderer, int32 cubemapHandle) {
		renderer->skyboxHandle = cubemapHandle;
	}

	void RendererSetDirectionalLight(Renderer* renderer, const DirectionalLight* light) {
		renderer->dir_light = *light;
	}

	AB_API void RendererSubmitPointLight(Renderer* renderer, PointLight* light) {
		CopyArray(PointLight, 1, renderer->pointLights + renderer->pointLightsAt, light);
		renderer->pointLightsAt++;
	}

	void RendererSetCamera(Renderer* renderer, hpm::Vector3 front, hpm::Vector3 position) {
		renderer->camera.front = hpm::Normalize(front);
		renderer->camera.position = position;
		renderer->camera.look_at = hpm::LookAtRH(renderer->camera.position, hpm::Add(renderer->camera.position, renderer->camera.front), {0.0f, 1.0f, 0.0f});
	}


	void RendererSubmit(Renderer* renderer, int32 mesh_handle, const hpm::Matrix4* transform, BlendMode blendMode) {
		if (renderer->draw_buffer_at < DRAW_BUFFER_SIZE) {
			renderer->draw_buffer[renderer->draw_buffer_at].mesh_handle = mesh_handle;
			renderer->draw_buffer[renderer->draw_buffer_at].transform = *transform;
			renderer->draw_buffer[renderer->draw_buffer_at].blendMode = blendMode;
			renderer->draw_buffer_at++;
		}
	}

	static void DrawSkybox(Renderer* renderer) {
		if (renderer->skyboxHandle) {
			GLCall(glEnable(GL_DEPTH_TEST));
			GLCall(glDepthMask(GL_FALSE));
			GLCall(glDepthFunc(GL_LEQUAL));
			GLCall(glUseProgram(renderer->skyboxProgramHandle));
			GLCall(glActiveTexture(GL_TEXTURE0));
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->skyboxHandle));
			GLCall(glUniform1i(glGetUniformLocation(renderer->skyboxProgramHandle,
													"skybox"), 0));
			BindSystemUniformBuffer(renderer, renderer->skyboxProgramHandle);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
			GLCall(glUseProgram(0));
			// TODO: If depth writing not being enabled here then there are
			// weirrd behavior
			GLCall(glDepthMask(GL_TRUE));
		}
	}

	static void PostFXPass(Renderer* renderer) {
		GLCall(glUseProgram(renderer->postFXProgramHandle));
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->postfxColorSourceHandle));
		GLCall(glUniform1i(glGetUniformLocation(renderer->postFXProgramHandle,
												"colorBuffer"), 0));
		GLCall(glUniform1f(glGetUniformLocation(renderer->postFXProgramHandle,
												"u_Gamma"), renderer->flySettings.gamma));
		GLCall(glUniform1f(glGetUniformLocation(renderer->postFXProgramHandle,
												"u_Exposure"), renderer->flySettings.exposure));
	
		BindSystemUniformBuffer(renderer, renderer->postFXProgramHandle);
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		GLCall(glUseProgram(0));			
	}

	typedef bool32(RenderSortPred)(uint64 a, uint64 b);

	static RenderBucket* SortBuckets(RenderBucket* bufferA, RenderBucket* bufferB, uint32 beg, uint32 end, RenderSortPred* pred) {
		RenderBucket* targetBuffer = bufferA;
		if (beg < end) {
			uint32 mid = beg + (end - beg) / 2;
			RenderBucket* left = SortBuckets(bufferA, bufferB, beg, mid, pred);
			RenderBucket* right = SortBuckets(bufferA, bufferB, mid + 1, end, pred);

			targetBuffer = left == bufferA ? bufferB : bufferA;

			uint32 leftAt = beg;
			uint32 rightAt = mid + 1;
			uint32 tempAt = beg;
			while (leftAt <= mid && rightAt <= end) {
				AB_CORE_ASSERT(tempAt <= end, "Merge sort error. Buffer overflow.");
				if (pred(left[leftAt].sortKey, right[rightAt].sortKey)) {
					targetBuffer[tempAt] = left[leftAt];
					tempAt++;
					leftAt++;
				}
				else {
					targetBuffer[tempAt] = right[rightAt];
					tempAt++;
					rightAt++;
					
				}
			}
			while (leftAt <= mid) {
				targetBuffer[tempAt] = left[leftAt];
				tempAt++;
				leftAt++;
			}
			while (rightAt <= end) {
				targetBuffer[tempAt] = right[rightAt];
				tempAt++;
				rightAt++;
			}
		}
		return targetBuffer;
	}

	inline static bool32 SortPred(uint64 a, uint64 b) {
		return a < b;
	}
	
	void RendererRender(Renderer* renderer) {
		CopyArray(RenderBucket,
				  renderer->commandQueueCapacity,
				  renderer->tmpCommandQueueBegin,
				  renderer->commandQueueBegin);

		RenderBucket* commandBuffer = SortBuckets(renderer->commandQueueBegin,
												  renderer->tmpCommandQueueBegin,
												  0,
												  renderer->commandQueueAt - 1,
												  SortPred);
		
		glViewport(0, 0, renderer->config.renderResolutionW, renderer->config.renderResolutionH);
		API::BindFramebuffer(renderer->multisampledFBOHandle, API::FB_TARGET_DRAW);

		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glDepthMask(GL_TRUE));
		GLCall(glDepthFunc(GL_LESS));
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		GLCall(glBlendEquation(GL_FUNC_ADD));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glCullFace(GL_BACK));
		GLCall(glFrontFace(GL_CCW));
	  
		API::ClearCurrentFramebuffer(API::CLEAR_COLOR | API::CLEAR_DEPTH);

		Matrix4 viewProj = MulM4M4(renderer->projection, renderer->camera.look_at);
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
		for (uint32 i = 0; i < POINT_LIGHTS_NUMBER; i++) {
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

		for (uint32 at = 0; at < renderer->commandQueueAt; at++) {
			RenderBucket* command = commandBuffer + at;

			Transform* transform = {};
			BlendMode blendMode = {};
			uint32 meshHandle = 0;
			switch (command->commandType) {
			case RENDER_COMMAND_DRAW_MESH: {
				auto* renderData = (RenderCommandDrawMesh*)(renderer->renderBufferBegin + command->rbOffset);
				transform = &renderData->transform;
				blendMode = renderData->blendMode;
				meshHandle = renderData->meshHandle;
				GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
				GLCall(glEnable(GL_CULL_FACE));
								
			} break;
			case RENDER_COMMAND_DRAW_MESH_WIREFRAME: {
				auto* renderData = (RenderCommandDrawMeshWireframe*)(renderer->renderBufferBegin + command->rbOffset);
				transform = &renderData->transform;
				blendMode = renderData->blendMode;
				meshHandle = renderData->meshHandle;
				GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
				GLCall(glLineWidth(renderData->lineWidth));
				GLCall(glDisable(GL_CULL_FACE));
			} break;
			default: {AB_CORE_ASSERT(false);} break;
			}
			
#if  1
			if (blendMode == BLEND_MODE_OPAQUE) {
				GLCall(glDepthMask(GL_TRUE));
			} else if (blendMode == BLEND_MODE_TRANSPARENT) {
				GLCall(glDepthMask(GL_FALSE));				
			} else {
				AB_CORE_FATAL("Undefined blend mode");
			}
#endif
			Mesh* mesh = AB::AssetGetMeshData(PermStorage()->asset_manager, meshHandle);

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

			GLCall(glUniformMatrix4fv(glGetUniformLocation(renderer->program_handle, "sys_ModelMatrix"), 1, GL_FALSE, transform->worldMatrix.data));


			Matrix4 inv = GetMatrix4(Inverse(GetMatrix3(transform->worldMatrix)));
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
		GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		//DrawSkybox(renderer);
		//GLCall(glDisable(GL_DEPTH_TEST));
		API::BindFramebuffer(renderer->multisampledFBOHandle, API::FB_TARGET_READ);
		API::BindFramebuffer(renderer->postfxFBOHandle, API::FB_TARGET_DRAW);
		// TODO: Framebuffers resolution, remove depth buffer frompostfx fbo?
		GLCall(glBlitFramebuffer(0, 0,
								 renderer->config.renderResolutionW,
								 renderer->config.renderResolutionH,
								 0, 0,
								 renderer->config.renderResolutionW,
								 renderer->config.renderResolutionH,
								 GL_COLOR_BUFFER_BIT, GL_LINEAR
								 ));

		API::BindFramebuffer(API::DEFAULT_FB_HANDLE, API::FB_TARGET_DRAW);
		//API::ClearCurrentFramebuffer(API::CLEAR_DEPTH | API::CLEAR_COLOR);
		uint32 w, h;
		WindowGetSize(&w, &h);
		glViewport(0, 0, w, h);
		// TODO: Cleanup this pipeline state mess!
		glDisable(GL_DEPTH_TEST);
		PostFXPass(renderer);

		renderer->pointLightsAt = 0;
		renderer->draw_buffer_at = 0;
	}
}
