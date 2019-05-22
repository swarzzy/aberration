#include "Renderer.h"
#include "OpenGL.h"
#include "../ImageLoader.h"
#include "Memory.h"
#include "../AssetManager.h"
#include "../GraphicsPipeline.h"

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
uniform m4x4 u_ProjectionMatrix;
uniform m4x4 u_ViewMatrix;
void main() {
m4x4 invProj = inverse(u_ProjectionMatrix);
m3x3 invView = m3x3(inverse(u_ViewMatrix));
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
f32 exposure = 1.0f;
v3 mappedColor = v3(1.0f) - exp(-sample.xyz * u_Exposure);
// Setting alpha to 1, otherwise in could ocassionally blend with screen
// buffer in the wrong way
out_FragColor = v4(pow(mappedColor, 1.0f / v3(u_Gamma)), 1.0f);

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

	static constexpr u32 POINT_LIGHTS_NUMBER = 4;
	static constexpr u32 POINT_LIGHT_STRUCT_SIZE = sizeof(v4) * 4 + sizeof(f32) * 2;
	static constexpr u32 POINT_LIGHT_STRUCT_ALIGMENT = sizeof(v4);
	static constexpr u32 POINT_LIGHT_UBO_SIZE = POINT_LIGHTS_NUMBER * (POINT_LIGHT_STRUCT_SIZE + sizeof(f32) * 2); 

	struct RendererImpl
	{
		u32 pointLightUBHandle;
		i32 skyboxProgramHandle;
		i32 postFXProgramHandle;
		u32 msColorTargetHandle;
		u32 msDepthTargetHandle;
		u32 postfxColorSourceHandle;
		u32 postfxDepthSourceHandle;
		u32 multisampledFBOHandle;
		u32 postfxFBOHandle;
		i32 programHandle;
		u32 shadowMapHandle;
		u32 shadowPassFBHandle;
		i32 shadowPassShaderHandle;
		i32 debugInstancingShaderHandle;
		u32 debugInstancingVBHandle;
	};
	// NOTE: Does not setting temp arena point and does not flushes at the end.
	// TODO: Implement memory stack propperly
	static u32 RendererCreateProgram(MemoryArena* tempArena,
									 const char* vertexSource,
									 const char* fragmentSource) 
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
)";

		const char* fragmentShaderHeader = R"(
out vec4 out_FragColor;
)";

		u64 commonHeaderLength = strlen(commonShaderHeader);
		u64 vertexHeaderLength = strlen(vertexShaderHeader);
		u64 fragmentHeaderLength = strlen(fragmentShaderHeader);
		u64 fragmentSourceLength = strlen(fragmentSource);
		u64 vertexSourceLength = strlen(vertexSource);

		uptr vertexSourceSize = commonHeaderLength +
			vertexHeaderLength + vertexSourceLength + 1;
		char* fullVertexSource = (char*)PushSize(tempArena, vertexSourceSize, 0);
		AB_CORE_ASSERT(fullVertexSource);
		CopyArray(char, commonHeaderLength, fullVertexSource, commonShaderHeader);
		CopyArray(char, vertexHeaderLength, fullVertexSource + commonHeaderLength,
				  vertexShaderHeader);
		CopyArray(char, vertexSourceLength + 1,
				  fullVertexSource + commonHeaderLength + vertexHeaderLength,
				  vertexSource);
		
		uptr fragSourceSize = commonHeaderLength + fragmentHeaderLength +
			fragmentSourceLength + 1;
		char* fullFragmentSource = (char*)PushSize(tempArena, fragSourceSize, 0);
		AB_CORE_ASSERT(fullFragmentSource);
		CopyArray(char, commonHeaderLength, fullFragmentSource,
				  commonShaderHeader);
		CopyArray(char, fragmentHeaderLength,
				  fullFragmentSource + commonHeaderLength, fragmentShaderHeader);
		CopyArray(char, fragmentSourceLength + 1,
				  fullFragmentSource + commonHeaderLength + fragmentHeaderLength,
				  fragmentSource);

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
								char* message = (char*)PushSize(tempArena,
																logLength, 0);
								AB_CORE_ASSERT(message);
								GLCall(glGetProgramInfoLog(programHandle, logLength, 0, message));
								AB_CORE_ERROR("Shader program linking error:\n%s", message);
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
						GLchar* message = (GLchar*)PushSize(tempArena,
															logLength, 0);
						AB_CORE_ASSERT(message);
						GLCall(glGetShaderInfoLog(fragmentHandle, logLength, nullptr, message));
						AB_CORE_ERROR("Frgament shader compilation error:\n%s", message);
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
				GLchar* message = (GLchar*)PushSize(tempArena, logLength, 0);
				AB_CORE_ASSERT(message);
				GLCall(glGetShaderInfoLog(vertexHandle, logLength, nullptr, message));
				AB_CORE_ERROR("Vertex shader compilation error:\n%s", message);
			}
		}
		else 
		{
			AB_CORE_ERROR("Falled to create vertex shader");
		}

		return resultHandle;
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
			BindFramebuffer(renderer->impl->multisampledFBOHandle, FB_TARGET_DRAW);
			if (!samples)
			{
				FramebufferAttachTexture(FB_TARGET_DRAW,
										 FB_ATTACHMENT_COLOR0,
										 renderer->impl->msColorTargetHandle);

				FramebufferAttachTexture(FB_TARGET_DRAW,
										 FB_ATTACHMENT_DEPTH,
										 renderer->impl->msDepthTargetHandle);
			}
			else
			{
				FramebufferAttachTextureMultisample(FB_TARGET_DRAW,
													FB_ATTACHMENT_COLOR0,
													renderer->impl->msColorTargetHandle);

				FramebufferAttachTextureMultisample(FB_TARGET_DRAW,
													FB_ATTACHMENT_DEPTH,
													renderer->impl->msDepthTargetHandle);			
			}

			b32 framebuffer = ValidateFramebuffer(FB_TARGET_DRAW);
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
	 		BindFramebuffer(renderer->impl->postfxFBOHandle, FB_TARGET_DRAW);
			FramebufferAttachTexture(FB_TARGET_DRAW,
									 FB_ATTACHMENT_COLOR0,
									 renderer->impl->postfxColorSourceHandle);
			FramebufferAttachTexture(FB_TARGET_DRAW,
									 FB_ATTACHMENT_DEPTH,
									 renderer->impl->postfxDepthSourceHandle);
		
			b32 framebuffer = ValidateFramebuffer(FB_TARGET_DRAW);
			AB_CORE_ASSERT(framebuffer, "Downsampled framebuffer is incomplete");
		}
	}
		
	Renderer* AllocateRenderer(MemoryArena* memory, MemoryArena* tempArena,
							   RendererConfig config)
	{
		Renderer* renderer = nullptr;
		renderer = (Renderer*)PushSize(memory, sizeof(Renderer),
									   alignof(Renderer));
		renderer->impl = (RendererImpl*)PushSize(memory, sizeof(RendererImpl),
												 alignof(Renderer));
		AB_CORE_ASSERT(renderer, "Failed to allocate renderer");
		AB_CORE_ASSERT(renderer->impl, "Failed to allocate renderer");
		renderer->pipeline = AllocatePipeline(memory);

		CopyScalar(RendererConfig, &renderer->config, &config);

		// TODO: Temporary arena multithreading and all that stuff
		BeginTemporaryMemory(tempArena);
		u32 vSize = DebugGetFileSize("../assets/shaders/MeshVertex.glsl");
		u32 fSize = DebugGetFileSize("../assets/shaders/MeshFragment.glsl");
		void* vertexSource = PushSize(tempArena, vSize + 1, 0);
		void* fragSource = PushSize(tempArena, fSize + 1, 0);
		u32 vRead = DebugReadTextFile(vertexSource, vSize + 1,
									  "../assets/shaders/MeshVertex.glsl");
		u32 fRead = DebugReadTextFile(fragSource, fSize + 1,
									  "../assets/shaders/MeshFragment.glsl");
		AB_CORE_ASSERT(vRead == vSize + 1);
		AB_CORE_ASSERT(fRead == fSize + 1);

   		renderer->impl->programHandle =
			RendererCreateProgram(tempArena, (const char*)vertexSource,
								  (const char*)fragSource);
		renderer->impl->postFXProgramHandle =
			RendererCreateProgram(tempArena, POSTFX_VERTEX_PROGRAM,
								  POSTFX_FRAGMENT_PROGRAM);
				
		renderer->impl->skyboxProgramHandle =
			RendererCreateProgram(tempArena,
								  SKYBOX_VERTEX_PROGRAM, SKYBOX_FRAGMENT_PROGRAM);

		u32 pointLightUB;
		GLCall(glGenBuffers(1, &pointLightUB));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, pointLightUB));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, POINT_LIGHT_UBO_SIZE,
							NULL, GL_DYNAMIC_DRAW));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		renderer->impl->pointLightUBHandle = pointLightUB;

		renderer->impl->multisampledFBOHandle = CreateFramebuffer();
		renderer->impl->postfxFBOHandle = CreateFramebuffer();

		_ReloadMultisampledFramebuffer(renderer, true,
									   renderer->config.numSamples);
		_ReloadDownsampledFramebuffer(renderer, true);

		u32 shadowMap;
		GLCall(glGenTextures(1, & shadowMap));
		GLCall(glBindTexture(GL_TEXTURE_2D, shadowMap));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
							renderer->config.shadowMapRes,
							renderer->config.shadowMapRes,
							0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
		GLCall(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
								V4(1.0f, 0.0f, 0.0f, 0.0f).data));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
							   GL_CLAMP_TO_BORDER));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
							   GL_CLAMP_TO_BORDER));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		u32 shadowFB;
		GLCall(glGenFramebuffers(1, &shadowFB));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, shadowFB));
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
									  GL_TEXTURE_2D, shadowMap, 0));
		u32 shadowFBStatus = 0;
		GLCall(shadowFBStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
		AB_ASSERT(shadowFBStatus == GL_FRAMEBUFFER_COMPLETE);
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));		
		renderer->impl->shadowPassFBHandle = shadowFB;
		renderer->impl->shadowMapHandle = shadowMap;
		BindFramebuffer(DEFAULT_FB_HANDLE, FB_TARGET_DRAW);

		u32 shadowVertSz =
			DebugGetFileSize("../assets/shaders/ShadowPassVert.glsl");
		u32 shadowFragSz =
			DebugGetFileSize("../assets/shaders/ShadowPassFrag.glsl");
		void* shadowVertSrc = PushSize(tempArena, shadowVertSz + 1, 0);
		void* shadowFragSrc = PushSize(tempArena, shadowFragSz + 1, 0);
		u32 sVRd = DebugReadTextFile(shadowVertSrc, shadowVertSz + 1,
									 "../assets/shaders/ShadowPassVert.glsl");
		u32 sFRd = DebugReadTextFile(shadowFragSrc, shadowFragSz + 1,
									 "../assets/shaders/ShadowPassFrag.glsl");
		AB_CORE_ASSERT(sVRd == shadowVertSz + 1);
		AB_CORE_ASSERT(sFRd == shadowFragSz + 1);

		renderer->impl->shadowPassShaderHandle =
			RendererCreateProgram(tempArena, (const char*)shadowVertSrc,
								  (const char*)shadowFragSrc);

		u32 dbgInstVertSz =
			DebugGetFileSize("../assets/shaders/DebugInstancingVert.glsl");
		u32 dbgInstFragSz =
			DebugGetFileSize("../assets/shaders/DebugInstancingFrag.glsl");
		void* dbgInstVertSrc = PushSize(tempArena, dbgInstVertSz + 1, 0);
		void* dbgInstFragSrc = PushSize(tempArena, dbgInstFragSz + 1, 0);
		u32 diVRd = DebugReadTextFile(dbgInstVertSrc, dbgInstVertSz + 1,
									 "../assets/shaders/DebugInstancingVert.glsl");
		u32 diFRd = DebugReadTextFile(dbgInstFragSrc, dbgInstFragSz + 1,
									 "../assets/shaders/DebugInstancingFrag.glsl");
		AB_CORE_ASSERT(diVRd == dbgInstVertSz + 1);
		AB_CORE_ASSERT(diFRd == dbgInstFragSz + 1);

		renderer->impl->debugInstancingShaderHandle =
			RendererCreateProgram(tempArena, (const char*)dbgInstVertSrc,
								  (const char*)dbgInstFragSrc);

		u32 dbgInsVBO = 0;
		GLCall(glGenBuffers(1, &dbgInsVBO));
		renderer->impl->debugInstancingVBHandle = dbgInsVBO;

		EndTemporaryMemory(tempArena);
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
	
	static void DrawSkybox(Renderer* renderer, RenderGroup* renderGroup)
	{
		// TODO: IMPORTANT: Zero isn't actually null handle
		// ASSET_INVALID_HANDLE = -1
		// It's better to use 0 as invalid handle in asset manager
		if (renderGroup->skyboxHandle)
		{
			PipelineCommitState(renderer->pipeline);
			EnableDepthTest(renderer->pipeline, false);
			WriteDepth(renderer->pipeline, false);
			
			GLCall(glUseProgram(renderer->impl->skyboxProgramHandle));
			GLCall(glActiveTexture(GL_TEXTURE0));
			GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP,
								 renderGroup->skyboxHandle));
			GLint shaderHandle = renderer->impl->skyboxProgramHandle;
			GLuint skyboxLoc;
			GLuint projLoc;
			GLuint viewLoc;
			GLCall(projLoc = glGetUniformLocation(shaderHandle,
												  "u_ProjectionMatrix"));
			GLCall(viewLoc = glGetUniformLocation(shaderHandle,
												  "u_ViewMatrix"));
			GLCall(skyboxLoc = glGetUniformLocation(shaderHandle,
													"skybox"));
			GLCall(glUniform1i(skyboxLoc, 0));
			GLCall(glUniformMatrix4fv(projLoc, 1 ,GL_FALSE,
									  renderGroup->projectionMatrix.data));
			GLCall(glUniformMatrix4fv(viewLoc, 1 ,GL_FALSE,
									  renderGroup->camera.lookAt.data));

			//BindSystemUniformBuffer(renderer, renderer->impl->skyboxProgramHandle);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
			GLCall(glUseProgram(0));
			PipelineResetState(renderer->pipeline);
		}
	}

	static void PostFXPass(Renderer* renderer)
	{		
		GLCall(glUseProgram(renderer->impl->postFXProgramHandle));
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->postfxColorSourceHandle));
		GLuint colorLoc;
		GLuint gammaLoc;
		GLuint expLoc;
		GLCall(colorLoc = glGetUniformLocation(renderer->impl->postFXProgramHandle,
											   "colorBuffer"));
		GLCall(gammaLoc = glGetUniformLocation(renderer->impl->postFXProgramHandle,
											   "u_Gamma"));
		GLCall(expLoc = glGetUniformLocation(renderer->impl->postFXProgramHandle,
											 "u_Exposure"));
		
		GLCall(glUniform1i(colorLoc, 0));
		GLCall(glUniform1f(gammaLoc, renderer->cc.gamma));
		GLCall(glUniform1f(expLoc, renderer->cc.exposure));
	
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));
		GLCall(glUseProgram(0));
	}

	inline static b32 RendererSortPred(u64 a, u64 b)
	{
		return a < b;
	}
	
	static void FillLightUniforms(Renderer* renderer, RenderGroup* group, i32 shaderHandle)
	{
		GLCall(glUseProgram(shaderHandle));
		auto diffMapLoc = glGetUniformLocation(shaderHandle,
											   "material.diffuse_map");

		auto specMapLoc = glGetUniformLocation(shaderHandle,
											   "material.spec_map");
		GLCall(glUniform1i(diffMapLoc, 0));
		GLCall(glUniform1i(specMapLoc, 1));

		auto dirLoc = glGetUniformLocation(shaderHandle,
										   "dir_light.direction");
		auto ambLoc = glGetUniformLocation(shaderHandle,
										   "dir_light.ambient");
		auto difLoc = glGetUniformLocation(shaderHandle,
										   "dir_light.diffuse");
		auto spcLoc = glGetUniformLocation(shaderHandle,
										   "dir_light.specular");
	   
		if (group->dirLightEnabled)
		{
			v3 dir = Normalize(
				SubV3V3(group->dirLight.target, group->dirLight.from));
			GLCall(glUniform3fv(dirLoc, 1, dir.data));
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

	static void BindPointLightUniformBuffer(Renderer* renderer)
	{
		u32 pointLightsUBOIndex;
		GLCall(pointLightsUBOIndex =
			   glGetUniformBlockIndex(renderer->impl->programHandle,
									  "pointLightsData"));
		GLCall(glUniformBlockBinding(renderer->impl->programHandle,
									 pointLightsUBOIndex, 2));
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 2,
								renderer->impl->pointLightUBHandle));
	}

	static void FillPointLightUnformBuffer(Renderer* renderer,
										   RenderGroup* renderGroup)
	{
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER,
							renderer->impl->pointLightUBHandle));
		_PointLightStd140* buffer;
		GLCall(buffer = (_PointLightStd140*)glMapBuffer(GL_UNIFORM_BUFFER,
														GL_READ_WRITE));
		// TODO: Number of point lights
		for (u32 i = 0;
			 i < POINT_LIGHTS_NUMBER && i < renderGroup->pointLightsAt;
			 i++)
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
	
	struct DrawCallData    
	{
		Transform* transform;
		BlendMode blendMode;
		i32 meshHandle;
		v3 debugColor;
		b32 useDebugColor;		
	};
	
	static DrawCallData FetchDrawCallDataAndSetState(Renderer* renderer,
													 RenderGroup* renderGroup,
													 CommandQueueEntry* command)
	{
		AB_ASSERT(command);
		DrawCallData result = {};
		switch (command->commandType)
		{
		case RENDER_COMMAND_DRAW_MESH:
		{
			auto* renderData = (RenderCommandDrawMesh*)(renderGroup->renderBuffer +
														command->rbOffset);
			result.transform = &renderData->transform;
			result.blendMode = renderData->blendMode;
			result.meshHandle = renderData->meshHandle;
			SetPolygonFillMode(renderer->pipeline,
							   POLYGON_FILL_MODE_FILL);
			EnableFaceCulling(renderer->pipeline, true);
								
		} break;
		case RENDER_COMMAND_DRAW_MESH_WIREFRAME:
		{
			auto* renderData =
				(RenderCommandDrawMeshWireframe*)(renderGroup->renderBuffer
												  + command->rbOffset);
			result.transform = &renderData->transform;
			result.blendMode = renderData->blendMode;
			result.meshHandle = renderData->meshHandle;
			SetPolygonFillMode(renderer->pipeline,
							   POLYGON_FILL_MODE_LINE);
			GLCall(glLineWidth(renderData->lineWidth));
			EnableFaceCulling(renderer->pipeline, false);
		} break;
		case RENDER_COMMAND_DRAW_DEBUG_CUBE:
		{
			auto* renderData =
				(RenderCommandDrawDebugCube*)(renderGroup->renderBuffer +
											  command->rbOffset);
			result.transform = &renderData->transform;
			result.blendMode = BLEND_MODE_OPAQUE;
			result.meshHandle = renderData->_meshHandle;
			result.debugColor = renderData->color;
			result.useDebugColor = true;
			SetPolygonFillMode(renderer->pipeline,
							   POLYGON_FILL_MODE_FILL);
			EnableFaceCulling(renderer->pipeline, true);
					
		} break;
		
		INVALID_DEFAULT_CASE
			}
			
		if (result.blendMode == BLEND_MODE_OPAQUE)
		{
			GLCall(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			GLCall(glDisable(GL_SAMPLE_COVERAGE));

		}
		else if (result.blendMode == BLEND_MODE_TRANSPARENT)
		{
			GLCall(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
			GLCall(glEnable(GL_SAMPLE_COVERAGE));

		} else
		{
			INVALID_CODE_PATH
				}

		return result;
	}

	static void VertexBufferToDraw(Mesh* mesh)
	{
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, mesh->api_vb_handle));

#define OPENGL_PUSH_SOA_LAYOUT
#if defined(OPENGL_PUSH_SOA_LAYOUT)
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));
		if (mesh->uvs)
		{
			void* pointer = (void*)((byte*)(mesh->uvs) - (byte*)(mesh->positions));
			GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, pointer ));
			GLCall(glEnableVertexAttribArray(1));
		}
		if (mesh->normals)
		{
			void* pointer = (void*)((byte*)mesh->normals - (byte*)mesh->positions);
			GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, pointer));
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
	}

	static void ShadowPassSetPerMeshMatices(Renderer* renderer,
											RenderGroup* renderGroup,
											DrawCallData dcData)
	{
		GLuint modelLoc;
		GLCall(modelLoc =
			   glGetUniformLocation(renderer->impl->shadowPassShaderHandle,
									"modelMatrix")); 
		GLCall(glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
								  dcData.transform->worldMatrix.data));
	}

	static void SetPerMeshUniformsAndTextures(Renderer* renderer,
											  AssetManager* assetManager,
											  Mesh* mesh, DrawCallData dcData)
	{
		GLuint ambLoc = glGetUniformLocation(renderer->impl->programHandle,
											 "material.ambinet");
		GLuint diffLoc = glGetUniformLocation(renderer->impl->programHandle,
											  "material.diffuse");
		GLuint specLoc = glGetUniformLocation(renderer->impl->programHandle,
											  "material.specular");
		GLuint shinLoc = glGetUniformLocation(renderer->impl->programHandle,
											  "material.shininess");

		v3 ambColor = mesh->material->ambient;
		v3 diffColor = mesh->material->diffuse;
		v3 specColor = mesh->material->specular;
			
		if (dcData.useDebugColor) 
		{
			ambColor = dcData.debugColor;
			diffColor = dcData.debugColor;
			specColor = dcData.debugColor;	
		}

		GLCall(glUniform3fv(ambLoc, 1,  ambColor.data));
		GLCall(glUniform3fv(diffLoc, 1, diffColor.data));
		GLCall(glUniform3fv(specLoc, 1, specColor.data));
		GLCall(glUniform1f(shinLoc, mesh->material->shininess));


		GLuint modelLoc = glGetUniformLocation(renderer->impl->programHandle,
											   "modelMatrix"); 
		GLCall(glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
								  dcData.transform->worldMatrix.data));

		m3x3 invWorldMtx = M3x3(dcData.transform->worldMatrix);
		Inverse(&invWorldMtx);
		m4x4 inv = M4x4(invWorldMtx);
		m4x4 normalMatrix = Transpose(inv);

		GLuint normalLoc = glGetUniformLocation(renderer->impl->programHandle,
												"normalMatrix"); 
		GLCall(glUniformMatrix4fv(normalLoc, 1, GL_FALSE,
								  normalMatrix.data));

		GLCall(glActiveTexture(GL_TEXTURE0));			
		Texture* diffTexture =
			AssetGetTextureData(assetManager,
								mesh->material->diff_map_handle);

		GLint useDiffMap = (GLint)(diffTexture != 0);
		GLuint useDiffMapLoc =
			glGetUniformLocation(renderer->impl->programHandle,
								 "material.use_diff_map");
		GLCall(glUniform1i(useDiffMapLoc, useDiffMap));

		if (useDiffMap)
		{
			GLCall(glBindTexture(GL_TEXTURE_2D, diffTexture->api_handle));
		}
			
		GLCall(glActiveTexture(GL_TEXTURE1));
		Texture* specTexture = AssetGetTextureData(assetManager,
												   mesh->material->spec_map_handle);
		GLint useSpecMap = (GLint)(specTexture != 0);
		GLuint useSpecMapLoc = glGetUniformLocation(renderer->impl->programHandle,
													"material.use_spec_map");
		GLCall(glUniform1i(useSpecMapLoc, useSpecMap));

		
		if (useSpecMap)
		{
			GLCall(glBindTexture(GL_TEXTURE_2D, specTexture->api_handle));
		}

	}
// TODO: Move it to some better place
#define SHADOW_MAP_TEXTURE_SLOT GL_TEXTURE4
#define SHADOW_MAP_TEXTURE_SLOT_NUMBER 4
	static void MainPass(Renderer* renderer, RenderGroup* renderGroup,
						 AssetManager* assetManager,
						 CommandQueueEntry* commandBuffer,
						 m4x4* lightSpaceMtx)
	{
		GLCall(glUseProgram(renderer->impl->programHandle));

		m4x4 viewProj = MulM4M4(renderGroup->projectionMatrix,
								renderGroup->camera.lookAt);
		v3* viewPos = &renderGroup->camera.position;
		
		GLuint viewProjLoc;
		GLuint viewPosLoc;
		GLuint lightSpaceLoc;
		GLuint shadowMapLoc;
		GLCall(viewProjLoc = glGetUniformLocation(renderer->impl->programHandle,
												  "viewProjMatrix"));
		GLCall(viewPosLoc = glGetUniformLocation(renderer->impl->programHandle,
												 "u_ViewPos"));
		GLCall(lightSpaceLoc = glGetUniformLocation(renderer->impl->programHandle,
													"lightSpaceMatrix"));
		GLCall(shadowMapLoc = glGetUniformLocation(renderer->impl->programHandle,
												   "shadowMap"));


		GLCall(glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE,
								  viewProj.data));

		GLCall(glUniform3fv(viewPosLoc, 1, viewPos->data));

		GLCall(glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE,
								  lightSpaceMtx->data));
		GLCall(glUniform1i(shadowMapLoc, SHADOW_MAP_TEXTURE_SLOT_NUMBER));

		GLCall(glActiveTexture(SHADOW_MAP_TEXTURE_SLOT));
		GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->shadowMapHandle));

		FillLightUniforms(renderer, renderGroup, renderer->impl->programHandle);
		FillPointLightUnformBuffer(renderer, renderGroup);
		BindPointLightUniformBuffer(renderer);

		PipelineResetBackend(renderer->pipeline);
		EnableDepthTest(renderer->pipeline, true);
		WriteDepth(renderer->pipeline, true);
		SetDepthFunc(renderer->pipeline, DEPTH_FUNC_LESS);
		EnableBlending(renderer->pipeline, false);
		EnableFaceCulling(renderer->pipeline, true);
		SetFaceCullingMode(renderer->pipeline,
						   FACE_CULL_MODE_BACK);
		SetDrawOrder(renderer->pipeline, DRAW_ORDER_CCW);
		
		PipelineCommitState(renderer->pipeline);

		BindFramebuffer(renderer->impl->multisampledFBOHandle, FB_TARGET_DRAW);
		GLCall(glViewport(0, 0, renderer->config.renderResolutionW,
						  renderer->config.renderResolutionH));
		ClearCurrentFramebuffer(CLEAR_COLOR | CLEAR_DEPTH);
		DrawSkybox(renderer, renderGroup);
		GLCall(glUseProgram(renderer->impl->programHandle));
		
		for (u32 at = 0; at < renderGroup->commandQueueAt; at++)
		{
			PipelineResetState(renderer->pipeline);
			CommandQueueEntry* command = commandBuffer + at;
			// NOTE: All instancing here is for now
			if (command->commandType == RENDER_COMMAND_BEGIN_DEBUG_CUBE_INSTANCING)
			{
				auto* renderData =
					(RenderCommandBeginDebugCubeInctancing*)
					(renderGroup->renderBuffer + command->rbOffset);
				
				BlendMode blendMode = renderData->blendMode;
				i32 meshHandle = renderData->_meshHandle;
				u16 instanceCount = command->instanceCount;
				SetPolygonFillMode(renderer->pipeline,
								   POLYGON_FILL_MODE_FILL);
				EnableFaceCulling(renderer->pipeline, true);

				if (blendMode == BLEND_MODE_OPAQUE)
				{
					GLCall(glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE));
					GLCall(glDisable(GL_SAMPLE_COVERAGE));

				}
				else if (blendMode == BLEND_MODE_TRANSPARENT)
				{
					GLCall(glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE));
					GLCall(glEnable(GL_SAMPLE_COVERAGE));

				} else
				{
					INVALID_CODE_PATH
						}

				Mesh* mesh = AB::AssetGetMeshData(assetManager, meshHandle);

				
				VertexBufferToDraw(mesh);
				//SetPerMeshUniformsAndTextures(renderer, assetManager,
				//							  mesh, dcData);
				GLCall(glUseProgram(renderer->impl->debugInstancingShaderHandle));

				m4x4 viewProj = MulM4M4(renderGroup->projectionMatrix,
										renderGroup->camera.lookAt);
				v3* viewPos = &renderGroup->camera.position;
		
				GLuint viewProjLoc;
				GLuint viewPosLoc;
				GLuint lightSpaceLoc;
				GLuint shadowMapLoc;
				GLCall(viewProjLoc = glGetUniformLocation(renderer->impl->debugInstancingShaderHandle,
														  "viewProjMatrix"));
				GLCall(viewPosLoc = glGetUniformLocation(renderer->impl->debugInstancingShaderHandle,
														 "u_ViewPos"));
				GLCall(lightSpaceLoc = glGetUniformLocation(renderer->impl->debugInstancingShaderHandle,
															"lightSpaceMatrix"));
				GLCall(shadowMapLoc = glGetUniformLocation(renderer->impl->debugInstancingShaderHandle,
														   "shadowMap"));


				GLCall(glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE,
										  viewProj.data));

				GLCall(glUniform3fv(viewPosLoc, 1, viewPos->data));

				GLCall(glUniformMatrix4fv(lightSpaceLoc, 1, GL_FALSE,
										  lightSpaceMtx->data));
				GLCall(glUniform1i(shadowMapLoc, SHADOW_MAP_TEXTURE_SLOT_NUMBER));

				GLCall(glActiveTexture(SHADOW_MAP_TEXTURE_SLOT));
				GLCall(glBindTexture(GL_TEXTURE_2D, renderer->impl->shadowMapHandle));

				FillLightUniforms(renderer, renderGroup, renderer->impl->debugInstancingShaderHandle);
				// TODO: Point lights
				//FillPointLightUnformBuffer(renderer, renderGroup);
				//BindPointLightUniformBuffer(renderer);

				
				u32 instancingVBO = renderer->impl->debugInstancingVBHandle;
				GLCall(glBindBuffer(GL_ARRAY_BUFFER, instancingVBO));
				u32 bufferSize =
					instanceCount * sizeof(RenderCommandPushDebugCubeInstance);
				byte* instanceData = (byte*)renderData +
					sizeof(RenderCommandBeginDebugCubeInctancing);
				GLCall(glBufferData(GL_ARRAY_BUFFER, bufferSize,
									instanceData, GL_STATIC_DRAW));
				GLCall(glEnableVertexAttribArray(3));
				GLCall(glEnableVertexAttribArray(4));
				GLCall(glEnableVertexAttribArray(5));
				GLCall(glEnableVertexAttribArray(6));
				u32 stride = sizeof(m4x4) + sizeof(v3);
				GLCall(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride,
											 nullptr));
				GLCall(glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride,
											 (void*)(sizeof(v4))));
				GLCall(glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride,
											 (void*)(sizeof(v4) * 2)));
				GLCall(glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride,
											 (void*)(sizeof(v4) * 3)));

				GLCall(glEnableVertexAttribArray(7));
				GLCall(glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, stride,
											 (void*)(sizeof(v4) * 4)));

				GLCall(glVertexAttribDivisor(3, 1));
				GLCall(glVertexAttribDivisor(4, 1));
				GLCall(glVertexAttribDivisor(5, 1));
				GLCall(glVertexAttribDivisor(6, 1));
				GLCall(glVertexAttribDivisor(7, 1));

				VertexBufferToDraw(mesh);
				
				if (mesh->api_ib_handle != 0)
				{
					GLCall(glDrawElementsInstanced(GL_TRIANGLES,
												   (GLsizei)mesh->num_indices,
												   GL_UNSIGNED_INT,
												   0, instanceCount));
				}
				else
				{
					GLCall(glDrawArraysInstanced(GL_TRIANGLES,
												 0, mesh->num_vertices,
												 instanceCount));
				}

			}
			else
			{
				if (!(command->commandType == RENDER_COMMAND_SET_DIR_LIGHT ||
					  command->commandType == RENDER_COMMAND_SET_POINT_LIGHT))
				{
					DrawCallData dcData = {};			
					dcData = FetchDrawCallDataAndSetState(renderer,
														  renderGroup, command);

					Mesh* mesh = AB::AssetGetMeshData(assetManager, dcData.meshHandle);

					VertexBufferToDraw(mesh);
					GLCall(glUseProgram(renderer->impl->programHandle));
					SetPerMeshUniformsAndTextures(renderer, assetManager,
												  mesh, dcData);

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
			}
		}
		PipelineResetState(renderer->pipeline);
		BindFramebuffer(0, FB_TARGET_DRAW);
		
	}

	static void ShadowPass(Renderer* renderer, RenderGroup* renderGroup,
						   AssetManager* assetManager,
						   CommandQueueEntry* commandBuffer,
						   m4x4* lightSpaceMtx)
	{
		GLCall(glUseProgram(renderer->impl->shadowPassShaderHandle));

		GLuint viewProjLoc;
		GLCall(viewProjLoc =
			   glGetUniformLocation(renderer->impl->shadowPassShaderHandle,
									"viewProjMatrix"));

		GLCall(glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE,
								  lightSpaceMtx->data));

		PipelineResetBackend(renderer->pipeline);
		EnableDepthTest(renderer->pipeline, true);
		glEnable(GL_DEPTH_TEST);
		WriteDepth(renderer->pipeline, true);
		SetDepthFunc(renderer->pipeline, DEPTH_FUNC_LESS);
		EnableBlending(renderer->pipeline, false);
		EnableFaceCulling(renderer->pipeline, true);
		SetFaceCullingMode(renderer->pipeline,
						   FACE_CULL_MODE_BACK);
		SetDrawOrder(renderer->pipeline, DRAW_ORDER_CCW);
		
		PipelineCommitState(renderer->pipeline);

		BindFramebuffer(renderer->impl->shadowPassFBHandle, FB_TARGET_DRAW);
		GLCall(glViewport(0, 0, renderer->config.shadowMapRes,
						  renderer->config.shadowMapRes));
		ClearCurrentFramebuffer(CLEAR_DEPTH);
		
		for (u32 at = 0; at < renderGroup->commandQueueAt; at++)
		{
			PipelineResetState(renderer->pipeline);
			CommandQueueEntry* command = commandBuffer + at;
			
			if (!(command->commandType == RENDER_COMMAND_SET_DIR_LIGHT ||
				  command->commandType == RENDER_COMMAND_SET_POINT_LIGHT))
			{
				DrawCallData dcData = {};			
				dcData = FetchDrawCallDataAndSetState(renderer,
													  renderGroup, command);

				Mesh* mesh = AB::AssetGetMeshData(assetManager, dcData.meshHandle);

				VertexBufferToDraw(mesh);
				ShadowPassSetPerMeshMatices(renderer, renderGroup, dcData);

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
		}
		PipelineResetState(renderer->pipeline);
		BindFramebuffer(0, FB_TARGET_DRAW);
	}

	void RendererRender(Renderer* renderer, AssetManager* assetManager,
						RenderGroup* renderGroup)
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
				
		CommandQueueEntry* commandBuffer =
			RenderGroupSortCommandQueue(renderGroup->commandQueue,
										renderGroup->tmpCommandQueue,
										0,
										renderGroup->commandQueueAt - 1,
										RendererSortPred);
#else
		CommandQueueEntry* commandBuffer= renderGroup->commandQueue;
#endif
		v3 casterDir = renderGroup->dirLight.target - renderGroup->dirLight.from;
		v3 casterPos = renderGroup->dirLight.from;
						
			m4x4 lightProjection =
			OrthogonalRH(-10, 10, -10, 10, 0.1f, 50);
		
		m4x4 lookat = LookAtRH(renderGroup->dirLight.from,
							   renderGroup->dirLight.target, V3(0, 1, 0));
		m4x4 lightSpace = MulM4M4(lightProjection, lookat);
		//m4x4 lightSpace = renderGroup->dirLightMatrix;
		//ShadowPass(renderer, renderGroup, assetManager, commandBuffer, &lightSpace);
		MainPass(renderer, renderGroup, assetManager, commandBuffer, &lightSpace);
		EnableDepthTest(renderer->pipeline, false);
		
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		BindFramebuffer(renderer->impl->multisampledFBOHandle, FB_TARGET_READ);
		BindFramebuffer(renderer->impl->postfxFBOHandle, FB_TARGET_DRAW);
		// TODO: Framebuffers resolution, remove depth buffer frompostfx fbo?
		GLCall(glBlitFramebuffer(0, 0,
								 renderer->config.renderResolutionW,
								 renderer->config.renderResolutionH,
								 0, 0,
								 renderer->config.renderResolutionW,
								 renderer->config.renderResolutionH,
								 GL_COLOR_BUFFER_BIT, GL_LINEAR));
		BindFramebuffer(DEFAULT_FB_HANDLE, FB_TARGET_DRAW);
		u32 w, h;
		WindowGetSize(&w, &h);
		glViewport(0, 0, w, h);
		PostFXPass(renderer);
	}
}
