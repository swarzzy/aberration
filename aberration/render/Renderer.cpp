#include "Renderer.h"
#include "platform/API/OpenGL/OpenGL.h"
#include "platform/Common.h"
#include "utils/ImageLoader.h"
#include "platform/Memory.h"
#include "AssetManager.h"
#include "platform/Window.h"
#include "platform/API/GraphicsPipeline.h"

namespace AB
{

	// TODO: Inverse matrices in shaders might be too heavy.
	// Especially for skyboxes
	static constexpr char SKYBOX_VERTEX_PROGRAM[] = R"(
out v3 skyboxUV;
const v2 fullscreenQuad[6] = v2[6](v2(-1.0f, -1.0f),
								   v2(1.0f, -1.0f),
								   v2(1.0f, 1.0f),
								   v2(1.0f, 1.0f),
								   v2(-1.0f, 1.0f),
								   v2(-1.0f, -1.0f));
void main() {
m4x4 invProj = inverse(sys_ProjectionMatrix);
m3x3 invView = m3x3(inverse(sys_ViewMatrix));
skyboxUV = invView * (invProj * v4(fullscreenQuad[gl_VertexID], 0.0f, 1.0f)).xyz;
out_Position = v4(fullscreenQuad[gl_VertexID], 0.0f, 1.0f);
out_Position = out_Position.xyww;
}
)";
	static constexpr char SKYBOX_FRAGMENT_PROGRAM[] = R"(
in v3 skyboxUV;
uniform samplerCube skybox;
void main() {
out_FragColor = texture(skybox, skyboxUV);
}
)";

	static constexpr char POSTFX_VERTEX_PROGRAM[] = R"(
out v2 UV;
const v2 fullscreenQuad[6] = v2[6](v2(-1.0f, -1.0f),
								   v2(1.0f, -1.0f),
								   v2(1.0f, 1.0f),
								   v2(1.0f, 1.0f),
								   v2(-1.0f, 1.0f),
								   v2(-1.0f, -1.0f));

const v2 quadUV[6] = v2[6](v2(0.0f, 0.0f),
					   v2(1.0f, 0.0f),
					   v2(1.0f, 1.0f),
					   v2(1.0f, 1.0f),
					   v2(0.0f, 1.0f),
					   v2(0.0f, 0.0f));
void main() {
UV = quadUV[gl_VertexID];
out_Position = v4(fullscreenQuad[gl_VertexID], 1.0f, 1.0f);
}
)";

	static constexpr char POSTFX_FRAGMENT_PROGRAM[] = R"(
in v2 UV;
uniform sampler2D colorBuffer;
uniform f32 u_Gamma = 2.2f;
uniform f32 u_Exposure = 1.0f;
void main() {
v4 sample = texture(colorBuffer, UV);
//v3 mappedColor = sample.xyz / (sample.xyz + (v3(1.0)));
f32 exposure = 1.0f;
v3 mappedColor = v3(1.0f) - exp(-sample.xyz * u_Exposure);
//mappedColor = sample.xyz;
// Setting alpha to 1, otherwise in could ocassionally blend with screen
// buffer in the wrong way
out_FragColor = v4(pow(mappedColor, 1.0f / v3(u_Gamma)), 1.0f);
//out_FragColor = v4(mappedColor, 1.0f);

}
)";

	
	struct _PointLightStd140
	{
		v3 position;
		f32 _pad1;
		v3 ambient;
		f32 _pad2;
		v3 diffuse;
		f32 _pad3;
		v3 specular;
		f32 _pad4;
		f32 linear;
		f32 quadratic;
		f32 _pad5;
		f32 _pad6;
	};

	static constexpr u32 SYSTEM_UBO_SIZE = sizeof(m4x4) * 4 + sizeof(v4);
	static constexpr u32 SYSTEM_UBO_VERTEX_OFFSET = 0;
	static constexpr u32 SYSTEM_UBO_VERTEX_SIZE = sizeof(m4x4) * 4;
	static constexpr u32 SYSTEM_UBO_VERTEX_VIEWPROJ_OFFSET = 0;
	static constexpr u32 SYSTEM_UBO_VERTEX_VIEW_OFFSET = sizeof(m4x4) * 1;
	static constexpr u32 SYSTEM_UBO_VERTEX_PROJ_OFFSET = sizeof(m4x4) * 2;
	static constexpr u32 SYSTEM_UBO_VERTEX_NORMAL_OFFSET = sizeof(m4x4) * 3;
	static constexpr u32 SYSTEM_UBO_FRAGMENT_OFFSET = sizeof(m4x4) * 4;
	static constexpr u32 SYSTEM_UBO_FRAGMENT_SIZE= sizeof(v4);

	static constexpr u32 POINT_LIGHTS_NUMBER = 4;
	static constexpr u32 POINT_LIGHT_STRUCT_SIZE = sizeof(v4) * 4 + sizeof(f32) * 2;
	static constexpr u32 POINT_LIGHT_STRUCT_ALIGMENT = sizeof(v4);
	static constexpr u32 POINT_LIGHT_UBO_SIZE = POINT_LIGHTS_NUMBER * (POINT_LIGHT_STRUCT_SIZE + sizeof(f32) * 2); 

	struct RendererImpl
	{
		u32 vertexSystemUBHandle;
		u32 pointLightUBHandle;
		i32 skyboxHandle;
		i32 skyboxProgramHandle;
		i32 postFXProgramHandle;
		u32 msColorTargetHandle;
		u32 msDepthTargetHandle;
		u32 postfxColorSourceHandle;
		u32 postfxDepthSourceHandle;
		u32 multisampledFBOHandle;
		u32 postfxFBOHandle;
		i32 programHandle;		
	};

	static u32 RendererCreateProgram(const char* vertexSource, const char* fragmentSource) 
	{

		const char* commonShaderHeader = R"(
#version 330 core
#define v2 vec2
#define v3 vec3
#define v4 vec4
#define m4x4 mat4
#define m3x3 mat3
#define f32 float
)";

		const char* vertexShaderHeader = R"(
#define out_Position gl_Position
layout (location = 0) in v3 v_Position;
layout (location = 1) in v2 v_UV;
layout (location = 2) in v3 v_Normal;

layout (std140) uniform _vertexSystemUniformBlock {
m4x4 sys_ViewProjMatrix;
m4x4 sys_ViewMatrix;
m4x4 sys_ProjectionMatrix;
m4x4 sys_NormalMatrix;
};
uniform m4x4 sys_ModelMatrix;
)";

		const char* fragmentShaderHeader = R"(
out vec4 out_FragColor;
layout(std140) uniform _fragmentSystemUniformBlock {
v3 sys_ViewPos;};
)";

		u64 commonHeaderLength = strlen(commonShaderHeader);
		u64 vertexHeaderLength = strlen(vertexShaderHeader);
		u64 fragmentHeaderLength = strlen(fragmentShaderHeader);
		u64 fragmentSourceLength = strlen(fragmentSource);
		u64 vertexSourceLength = strlen(vertexSource);

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

		u32 resultHandle = 0;
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
								i32 logLength;
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

	
	static void BindSystemUniformBuffer(Renderer* renderer, i32 programHandle)
	{
		u32 sysUBOVertexIndex;
		GLCall(sysUBOVertexIndex = glGetUniformBlockIndex(programHandle,
														  "_vertexSystemUniformBlock"));
		GLCall(glUniformBlockBinding(programHandle, sysUBOVertexIndex, 0));
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, renderer->impl->vertexSystemUBHandle,
								 SYSTEM_UBO_VERTEX_OFFSET, SYSTEM_UBO_VERTEX_SIZE));

		u32 sysUBOFragIndex;
		GLCall(sysUBOFragIndex = glGetUniformBlockIndex(programHandle,
														"_fragmentSystemUniformBlock"));
		GLCall(glUniformBlockBinding(programHandle, sysUBOFragIndex, 1));
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 1, renderer->impl->vertexSystemUBHandle,
								 SYSTEM_UBO_FRAGMENT_OFFSET, SYSTEM_UBO_FRAGMENT_SIZE));

		u32 pointLightsUBOIndex;
		GLCall(pointLightsUBOIndex = glGetUniformBlockIndex(renderer->impl->programHandle,
															"pointLightsData"));
		GLCall(glUniformBlockBinding(renderer->impl->programHandle,
									 pointLightsUBOIndex, 2));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2, renderer->impl->pointLightUBHandle));

	}  	

	static void _ReloadMultisampledFramebuffer(Renderer* renderer,
											   b32 recreateTextures, u32 samples)
	{
		if (recreateTextures)
		{
			GLCall(glDeleteTextures(1, &renderer->impl->msColorTargetHandle));
			GLCall(glDeleteTextures(1, &renderer->impl->msDepthTargetHandle));
			GLCall(glGenTextures(1, &renderer->impl->msColorTargetHandle));
			GLCall(glGenTextures(1, &renderer->impl->msDepthTargetHandle));
		}
		
		if (!samples)
		{
			GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->msColorTargetHandle));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
								renderer->config.renderResolutionW,
								renderer->config.renderResolutionH,
								0, GL_RGBA, GL_FLOAT, nullptr));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
			GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->msDepthTargetHandle));
			GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
								renderer->config.renderResolutionW,
								renderer->config.renderResolutionH,
								0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		   
		}
		else
		{
			GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
								 renderer->impl->msColorTargetHandle));
			
			GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
										   samples, GL_RGBA16F,
										   renderer->config.renderResolutionW,
										   renderer->config.renderResolutionH,
										   GL_FALSE));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
			GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE,
								 renderer->impl->msDepthTargetHandle));
			
			GLCall(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
										   samples, GL_DEPTH_COMPONENT32,
										   renderer->config.renderResolutionW,
										   renderer->config.renderResolutionH,
										   GL_FALSE));

			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		}

		if (recreateTextures)
		{
			API::BindFramebuffer(renderer->impl->multisampledFBOHandle, API::FB_TARGET_DRAW);
			if (!samples)
			{
				API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
											  API::FB_ATTACHMENT_COLOR0,
											  renderer->impl->msColorTargetHandle);

				API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
											  API::FB_ATTACHMENT_DEPTH,
											  renderer->impl->msDepthTargetHandle);
			}
			else
			{
				API::FramebufferAttachTextureMultisample(API::FB_TARGET_DRAW,
														 API::FB_ATTACHMENT_COLOR0,
														 renderer->impl->msColorTargetHandle);

				API::FramebufferAttachTextureMultisample(API::FB_TARGET_DRAW,
														 API::FB_ATTACHMENT_DEPTH,
														 renderer->impl->msDepthTargetHandle);			
			}

			b32 framebuffer = API::ValidateFramebuffer(API::FB_TARGET_DRAW);
			AB_CORE_ASSERT(framebuffer, "Multisampled framebuffer is incomplete");

		}
	}

	static void _ReloadDownsampledFramebuffer(Renderer* renderer, b32 recreateTextures)
	{
		if (recreateTextures)
		{
			GLCall(glDeleteTextures(1, &renderer->impl->postfxColorSourceHandle));
			GLCall(glDeleteTextures(1, &renderer->impl->postfxDepthSourceHandle));
			GLCall(glGenTextures(1, &renderer->impl->postfxColorSourceHandle));
			GLCall(glGenTextures(1, &renderer->impl->postfxDepthSourceHandle));
		}
		
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->postfxColorSourceHandle));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F,
							renderer->config.renderResolutionW,
							renderer->config.renderResolutionH,
							0, GL_RGBA, GL_FLOAT, nullptr));
		
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->postfxDepthSourceHandle));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
							renderer->config.renderResolutionW,
							renderer->config.renderResolutionH,
							0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr));

		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
		
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		if (recreateTextures)
		{
	 		API::BindFramebuffer(renderer->impl->postfxFBOHandle, API::FB_TARGET_DRAW);
			API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
										  API::FB_ATTACHMENT_COLOR0,
										  renderer->impl->postfxColorSourceHandle);
			API::FramebufferAttachTexture(API::FB_TARGET_DRAW,
										  API::FB_ATTACHMENT_DEPTH,
										  renderer->impl->postfxDepthSourceHandle);
		
			b32 framebuffer = API::ValidateFramebuffer(API::FB_TARGET_DRAW);
			AB_CORE_ASSERT(framebuffer, "Downsampled framebuffer is incomplete");
		}
	}
		
	Renderer* RendererInit(RendererConfig config, API::Pipeline* pipeline)
	{
		// TODO: Move allocation out of here to some more appropriate place
		// Stop making renderer singleton
		Renderer* renderer = nullptr;
		if (!(PermStorage()->forward_renderer))
		{
			renderer = (Renderer*)SysAlloc(sizeof(Renderer));
			renderer->impl = (RendererImpl*)SysAlloc(sizeof(RendererImpl));
			AB_CORE_ASSERT(renderer, "Failed to allocate renderer");
			AB_CORE_ASSERT(renderer->impl, "Failed to allocate renderer");
		}
		else
		{
			AB_CORE_WARN("Renderer already initialized.");
		}

		CopyScalar(RendererConfig, &renderer->config, &config);

		renderer->pipeline = pipeline;

		// TODO: Temporary arena moltithreading and all that stuff
		auto[vertexSource, vSize] = DebugReadTextFile("../assets/shaders/MeshVertex.glsl");
		auto[fragmentSource, fSize] = DebugReadTextFile("../assets/shaders/MeshFragment.glsl");

   		renderer->impl->programHandle = RendererCreateProgram(vertexSource,
															  fragmentSource);
		renderer->impl->postFXProgramHandle = RendererCreateProgram(POSTFX_VERTEX_PROGRAM,
																	POSTFX_FRAGMENT_PROGRAM);
		
		DebugFreeFileMemory(vertexSource);
		DebugFreeFileMemory(fragmentSource);
		
		u32 sysVertexUB;
		GLCall(glGenBuffers(1, &sysVertexUB));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sysVertexUB));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, SYSTEM_UBO_SIZE, NULL, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		renderer->impl->vertexSystemUBHandle = sysVertexUB;

		u32 pointLightUB;
		GLCall(glGenBuffers(1, &pointLightUB));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, pointLightUB));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, POINT_LIGHT_UBO_SIZE, NULL, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		renderer->impl->pointLightUBHandle = pointLightUB;
	   
		renderer->impl->skyboxProgramHandle = RendererCreateProgram(SKYBOX_VERTEX_PROGRAM,
																	SKYBOX_FRAGMENT_PROGRAM);

		renderer->impl->multisampledFBOHandle = API::CreateFramebuffer();
		renderer->impl->postfxFBOHandle = API::CreateFramebuffer();

		_ReloadMultisampledFramebuffer(renderer, true, renderer->config.numSamples);
		_ReloadDownsampledFramebuffer(renderer, true);

		API::BindFramebuffer(API::DEFAULT_FB_HANDLE, API::FB_TARGET_DRAW);

		AB::GetMemory()->perm_storage.forward_renderer = renderer;

		return renderer;
	}

	RendererConfig RendererGetConfig(Renderer* renderer)
	{
		return renderer->config;
	}

	void RendererApplyConfig(Renderer* renderer, RendererConfig* newConfig)
	{
		if (newConfig->numSamples != renderer->config.numSamples)
		{
			b32 multisamplingModeChanged = false;
			if (newConfig->numSamples == 0 || renderer->config.numSamples == 0)
			{
				multisamplingModeChanged = true;
			}
			_ReloadMultisampledFramebuffer(renderer,
										   multisamplingModeChanged,
										   newConfig->numSamples);

		}
		if (newConfig->renderResolutionW != renderer->config.renderResolutionW ||
			newConfig->renderResolutionH != renderer->config.renderResolutionH)
		{
			renderer->config.renderResolutionW = newConfig->renderResolutionW;
			renderer->config.renderResolutionH = newConfig->renderResolutionH;
			
			_ReloadMultisampledFramebuffer(renderer, false, renderer->config.numSamples);
			_ReloadDownsampledFramebuffer(renderer, false);
		}
		CopyScalar(RendererConfig, &renderer->config, newConfig);		
	}
	
	void RendererSetSkybox(Renderer* renderer, i32 cubemapHandle)
	{
		renderer->impl->skyboxHandle = cubemapHandle;
	}

	static void DrawSkybox(Renderer* renderer)
	{
		if (renderer->impl->skyboxHandle)
		{
			API::PipelineCommitState(renderer->pipeline);
			API::EnableDepthTest(renderer->pipeline, false);
			API::WriteDepth(renderer->pipeline, false);
			
			GLCall(glUseProgram(renderer->impl->skyboxProgramHandle));
			GLCall(glActiveTexture(GL_TEXTURE0));
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP
								 , renderer->impl->skyboxHandle));
			GLuint skyboxLoc = glGetUniformLocation(renderer->impl->skyboxProgramHandle,
													"skybox");
			GLCall(glUniform1i(skyboxLoc, 0));
			BindSystemUniformBuffer(renderer, renderer->impl->skyboxProgramHandle);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
			GLCall(glUseProgram(0));
			API::PipelineResetState(renderer->pipeline);
		}
	}

	static void PostFXPass(Renderer* renderer)
	{		
		GLCall(glUseProgram(renderer->impl->postFXProgramHandle));
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->postfxColorSourceHandle));
		
		GLCall(glUniform1i(glGetUniformLocation(renderer->impl->postFXProgramHandle,
												"colorBuffer"), 0));
		GLCall(glUniform1f(glGetUniformLocation(renderer->impl->postFXProgramHandle,
												"u_Gamma"), renderer->cc.gamma));
		GLCall(glUniform1f(glGetUniformLocation(renderer->impl->postFXProgramHandle,
												"u_Exposure"), renderer->cc.exposure));
	
		BindSystemUniformBuffer(renderer, renderer->impl->postFXProgramHandle);
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		GLCall(glUseProgram(0));
	}

	inline static b32 RendererSortPred(u64 a, u64 b)
	{
		return a < b;
	}

	inline static void _FillSystemUniformBuffer(Renderer* renderer, RenderGroup* renderGroup)
	{
		m4x4 viewProj = MulM4M4(renderGroup->projectionMatrix,
								renderGroup->camera.lookAt);
		v3* viewPos = &renderGroup->camera.position;

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, renderer->impl->vertexSystemUBHandle));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_VIEWPROJ_OFFSET,
							   sizeof(m4x4), viewProj.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_VIEW_OFFSET,
							   sizeof(m4x4), renderGroup->camera.lookAt.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_VERTEX_PROJ_OFFSET,
							   sizeof(m4x4), renderGroup->projectionMatrix.data));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, SYSTEM_UBO_FRAGMENT_OFFSET,
							   SYSTEM_UBO_FRAGMENT_SIZE, viewPos->data));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, renderer->impl->pointLightUBHandle));
		_PointLightStd140* buffer;
		GLCall(buffer = (_PointLightStd140*)glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
		// TODO: Number of point lights
		for (u32 i = 0; i < POINT_LIGHTS_NUMBER && i < renderGroup->pointLightsAt; i++)
		{
			buffer[i].position = renderGroup->pointLights[i].position;
			buffer[i].ambient = renderGroup->pointLights[i].ambient;
			buffer[i].diffuse = renderGroup->pointLights[i].diffuse;
			buffer[i].specular = renderGroup->pointLights[i].specular;
			buffer[i].linear = renderGroup->pointLights[i].linear;
			buffer[i].quadratic = renderGroup->pointLights[i].quadratic;
		}
		
		GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	inline static void _FillSystemUniforms(Renderer* renderer, RenderGroup* group)
	{
		GLCall(glUseProgram(renderer->impl->programHandle));
		auto diffMapLoc = glGetUniformLocation(renderer->impl->programHandle,
											   "material.diffuse_map");

		auto specMapLoc = glGetUniformLocation(renderer->impl->programHandle,
											   "material.spec_map");
		GLCall(glUniform1i(diffMapLoc, 0));
		GLCall(glUniform1i(specMapLoc, 1));

		auto dirLoc = glGetUniformLocation(renderer->impl->programHandle,
										   "dir_light.direction");
		auto ambLoc = glGetUniformLocation(renderer->impl->programHandle,
										   "dir_light.ambient");
		auto difLoc = glGetUniformLocation(renderer->impl->programHandle,
										   "dir_light.diffuse");
		auto spcLoc = glGetUniformLocation(renderer->impl->programHandle,
										   "dir_light.specular");
	   
		if (group->dirLightEnabled)
		{
			GLCall(glUniform3fv(dirLoc, 1, group->dirLight.direction.data));
			GLCall(glUniform3fv(ambLoc, 1, group->dirLight.ambient.data));
			GLCall(glUniform3fv(difLoc, 1, group->dirLight.diffuse.data));
			GLCall(glUniform3fv(spcLoc, 1, group->dirLight.specular.data));
		}
		else
		{
			v3 null = V3(0.0f);
			GLCall(glUniform3fv(dirLoc, 1, null.data));
			GLCall(glUniform3fv(ambLoc, 1, null.data));
			GLCall(glUniform3fv(difLoc, 1, null.data));
			GLCall(glUniform3fv(spcLoc, 1, null.data));			
		}		
	}
	
	void RendererRender(Renderer* renderer, RenderGroup* renderGroup)
	{
		if (renderGroup->commandQueueAt == 0)
		{
			return;
		}

		CopyArray(CommandQueueEntry,
				  renderGroup->commandQueueCapacity,
				  renderGroup->tmpCommandQueue,
				  renderGroup->commandQueue);

#if 1
				
		CommandQueueEntry* commandBuffer = RenderGroupSortCommandQueue(renderGroup->commandQueue,
																	   renderGroup->tmpCommandQueue,
																	   0,
																	   renderGroup->commandQueueAt - 1,
																	   RendererSortPred);
#else
		CommandQueueEntry* commandBuffer= renderGroup->commandQueue;
#endif

		_FillSystemUniformBuffer(renderer, renderGroup);
		_FillSystemUniforms(renderer, renderGroup);

		GLCall(glUseProgram(renderer->impl->programHandle));
		BindSystemUniformBuffer(renderer, renderer->impl->programHandle);		

		
		API::BindFramebuffer(renderer->impl->multisampledFBOHandle, API::FB_TARGET_DRAW);
		GLCall(glViewport(0, 0, renderer->config.renderResolutionW, renderer->config.renderResolutionH));

		API::PipelineResetBackend(renderer->pipeline);
		API::EnableDepthTest(renderer->pipeline, true);
		API::WriteDepth(renderer->pipeline, true);
		API::SetDepthFunc(renderer->pipeline, API::DEPTH_FUNC_LESS);
		API::EnableBlending(renderer->pipeline, false);
		//API::SetBlendFunction(renderer->pipeline, API::BLEND_FUNC_ADD);
		//API::SetBlendFactor(renderer->pipeline,
		//					API::BLEND_FACTOR_SRC_ALPHA,
		//					API::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
		API::EnableFaceCulling(renderer->pipeline, true);
		API::SetFaceCullingMode(renderer->pipeline,
								API::FACE_CULL_MODE_BACK);
		API::SetDrawOrder(renderer->pipeline, API::DRAW_ORDER_CCW);
		
		API::ClearCurrentFramebuffer(API::CLEAR_COLOR | API::CLEAR_DEPTH);

		DrawSkybox(renderer);
		GLCall(glUseProgram(renderer->impl->programHandle));

		API::PipelineCommitState(renderer->pipeline);
		
		for (u32 at = 0; at < renderGroup->commandQueueAt; at++)
		{
			API::PipelineResetState(renderer->pipeline);
			CommandQueueEntry* command = commandBuffer + at;

			Transform* transform = {};
			BlendMode blendMode = {};
			u32 meshHandle = 0;
			switch (command->commandType)
			{
			case RENDER_COMMAND_DRAW_MESH:
			{
				auto* renderData = (RenderCommandDrawMesh*)(renderGroup->renderBuffer + command->rbOffset);
				transform = &renderData->transform;
				blendMode = renderData->blendMode;
				meshHandle = renderData->meshHandle;
				API::SetPolygonFillMode(renderer->pipeline,
										API::POLYGON_FILL_MODE_FILL);
				API::EnableFaceCulling(renderer->pipeline, true);
								
			} break;
			case RENDER_COMMAND_DRAW_MESH_WIREFRAME:
			{
				auto* renderData = (RenderCommandDrawMeshWireframe*)(renderGroup->renderBuffer + command->rbOffset);
				transform = &renderData->transform;
				blendMode = renderData->blendMode;
				meshHandle = renderData->meshHandle;
				API::SetPolygonFillMode(renderer->pipeline,
										API::POLYGON_FILL_MODE_LINE);
				GLCall(glLineWidth(renderData->lineWidth));
				API::EnableFaceCulling(renderer->pipeline, false);
			} break;
			case RENDER_COMMAND_SET_DIR_LIGHT: { continue; } break;
			case RENDER_COMMAND_SET_POINT_LIGHT: { continue; } break;
			INVALID_DEFAULT_CASE();
			}
			
			if (blendMode == BLEND_MODE_OPAQUE)
			{
				//API::WriteDepth(renderer->pipeline, true);
				//API::EnableBlending(renderer->pipeline, false);
				GLCall(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
				GLCall(glDisable(GL_SAMPLE_COVERAGE));

			}
			else if (blendMode == BLEND_MODE_TRANSPARENT)
			{
				//API::WriteDepth(renderer->pipeline, true);
				//API::EnableBlending(renderer->pipeline, false);
				//API::SetBlendFunction(renderer->pipeline, API::BLEND_FUNC_ADD);
				//API::SetBlendFactor(renderer->pipeline,
				//					API::BLEND_FACTOR_SRC_ALPHA,
				//					API::BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
				
				GLCall(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
				GLCall(glEnable(GL_SAMPLE_COVERAGE));

			} else
			{
				INVALID_CODE_PATH();
			}

			Mesh* mesh = AB::AssetGetMeshData(PermStorage()->asset_manager, meshHandle);

			GLCall(glBindBuffer(GL_ARRAY_BUFFER, mesh->api_vb_handle));

#define OPENGL_PUSH_SOA_LAYOUT
#if defined(OPENGL_PUSH_SOA_LAYOUT)
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			if (mesh->uvs)
			{
				GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0,
											 (void*)((byte*)(mesh->uvs) - (byte*)(mesh->positions))));
				GLCall(glEnableVertexAttribArray(1));
			}
			if (mesh->normals)
			{
				GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0,
											 (void*)((byte*)mesh->normals - (byte*)mesh->positions)));
				GLCall(glEnableVertexAttribArray(2));
			}

			if (mesh->api_ib_handle != 0)
			{
				GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->api_ib_handle));
			}
#else
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
										 (sizeof(hpm::v3) * 2 + sizeof(hpm::v2)),
										 (void*)0));
			GLCall(glEnableVertexAttribArray(0));
			GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
										 (sizeof(hpm::v3) * 2 + sizeof(hpm::v2)),
										 (void*)(sizeof(hpm::v3))));
			GLCall(glEnableVertexAttribArray(1));
			GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
										 (sizeof(hpm::v3) * 2 + sizeof(hpm::v2)),
										 (void*)(sizeof(hpm::v2) + sizeof(hpm::v3))));
			GLCall(glEnableVertexAttribArray(2));
			GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->api_ib_handle));

#endif
			GLuint ambLoc = glGetUniformLocation(renderer->impl->programHandle,
												 "material.ambinet");
			GLuint diffLoc = glGetUniformLocation(renderer->impl->programHandle,
												  "material.diffuse");
			GLuint specLoc = glGetUniformLocation(renderer->impl->programHandle,
												  "material.specular");
			GLuint shinLoc = glGetUniformLocation(renderer->impl->programHandle,
												  "material.shininess");
			
			GLCall(glUniform3fv(ambLoc, 1,  mesh->material->ambient.data));
			GLCall(glUniform3fv(diffLoc, 1, mesh->material->diffuse.data));
			GLCall(glUniform3fv(specLoc, 1, mesh->material->specular.data));
			GLCall(glUniform1f(shinLoc, mesh->material->shininess));

			GLuint modelLoc = glGetUniformLocation(renderer->impl->programHandle,
												   "sys_ModelMatrix");
			GLCall(glUniformMatrix4fv(modelLoc, 1, GL_FALSE, transform->worldMatrix.data));

			m4x4 inv = M4x4(Inverse(M3x3(transform->worldMatrix)));
			m4x4 normalMatrix = Transpose(inv);
		
			GLCall(glBindBuffer(GL_UNIFORM_BUFFER,
								renderer->impl->vertexSystemUBHandle));
			GLCall(glBufferSubData(GL_UNIFORM_BUFFER,
								   SYSTEM_UBO_VERTEX_NORMAL_OFFSET,
								   sizeof(m4x4), normalMatrix.data));
			GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
			
			GLCall(glActiveTexture(GL_TEXTURE0));			
			Texture* diffTexture = AssetGetTextureData(PermStorage()->asset_manager,
													   mesh->material->diff_map_handle);

			GLint useDiffMap = (GLint)(diffTexture != 0);
			GLuint useDiffMapLoc = glGetUniformLocation(renderer->impl->programHandle,
														"material.use_diff_map");
			GLCall(glUniform1i(useDiffMapLoc, useDiffMap));

			if (useDiffMap)
			{
				GLCall(glBindTexture(GL_TEXTURE_2D, diffTexture->api_handle));
			}
			
			GLCall(glActiveTexture(GL_TEXTURE1));
			Texture* specTexture = AssetGetTextureData(PermStorage()->asset_manager,
													   mesh->material->spec_map_handle);
			GLint useSpecMap = (GLint)(specTexture != 0);
			GLuint useSpecMapLoc = glGetUniformLocation(renderer->impl->programHandle,
														"material.use_spec_map");
			GLCall(glUniform1i(useSpecMapLoc, useSpecMap));

			if (useSpecMap)
			{
				GLCall(glBindTexture(GL_TEXTURE_2D, specTexture->api_handle));
			}

			if (mesh->api_ib_handle != 0)
			{
				GLCall(glDrawElements(GL_TRIANGLES, (GLsizei)mesh->num_indices,
									  GL_UNSIGNED_INT, 0));
			}
			else
			{
				GLCall(glDrawArrays(GL_TRIANGLES, 0, mesh->num_vertices));
			}
		}

		API::PipelineResetState(renderer->pipeline);
		API::EnableDepthTest(renderer->pipeline, false);
		
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		
		API::BindFramebuffer(renderer->impl->multisampledFBOHandle, API::FB_TARGET_READ);
		API::BindFramebuffer(renderer->impl->postfxFBOHandle, API::FB_TARGET_DRAW);
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
		u32 w, h;
		WindowGetSize(&w, &h);
		glViewport(0, 0, w, h);
		PostFXPass(renderer);
	}
}
