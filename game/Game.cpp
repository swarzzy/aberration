#define HYPERMATH_IMPL 
#include <hypermath.h>
#include "AB.h"
#include "Shared.h"
#include "Memory.h"
#include "OpenGL.h"
#include "InputManager.h"
#include "render/DebugRenderer.h"
#include "DebugTools.h"
#include "AssetManager.h"
#include "render/Renderer.h"

#include "Sandbox.h"

//using namespace AB;

namespace AB
{
	
	struct StaticStorage
	{
		MemoryArena* tempArena;
		struct InputMgr* inputManager;
		DebugRenderer* debugRenderer;
		DebugOverlay* debugOverlay;
		AssetManager* assetManager;
		Renderer* renderer;
		RenderGroup* renderGroup;
		Sandbox* gameVars;
	};

	static PlatformState* g_Platform;
	static StaticStorage* g_StaticStorage;
	static GLFuncTable* g_OpenGLFuncTable;

#define GlobalDeltaTime g_Platform->deltaTime

// NOTE: Why clang inserts the dot by itself
#if defined(_MSC_VER)
#define PLATFORM_FUNCTION(func) g_Platform->functions.##func
#else
#define PLATFORM_FUNCTION(func) g_Platform->functions. func
#endif
	

#define DebugReadFilePermanent PLATFORM_FUNCTION(DebugReadFilePermanent)
#define WindowGetSize PLATFORM_FUNCTION(WindowGetSize)
#define DebugGetFileSize PLATFORM_FUNCTION(DebugGetFileSize)
#define DebugReadFile PLATFORM_FUNCTION(DebugReadFile)
#define DebugReadTextFile PLATFORM_FUNCTION(DebugReadTextFile)
#define PlatformSetCursorPosition PLATFORM_FUNCTION(PlatformSetCursorPosition)
#define GetLocalTime PLATFORM_FUNCTION(GetLocalTime)
#define ConsoleSetColor PLATFORM_FUNCTION(ConsoleSetColor)
#define ConsolePrint PLATFORM_FUNCTION(ConsolePrint)

#define GL_FUNCTION(func) g_OpenGLFuncTable->_##func

#define glGenTextures GL_FUNCTION(glGenTextures)
#define glBindTexture GL_FUNCTION(glBindTexture)
#define glTexParameteri GL_FUNCTION(glTexParameteri)
#define glTexImage2D GL_FUNCTION(glTexImage2D)
#define glDeleteTextures GL_FUNCTION(glDeleteTextures)
#define glPolygonMode GL_FUNCTION(glPolygonMode)
#define glDisable GL_FUNCTION(glDisable)
#define glClearColor GL_FUNCTION(glClearColor)
#define glEnable GL_FUNCTION(glEnable)
#define glBindBuffer GL_FUNCTION(glBindBuffer)
#define glBufferData GL_FUNCTION(glBufferData)
#define glEnableVertexAttribArray GL_FUNCTION(glEnableVertexAttribArray)
#define glVertexAttribPointer GL_FUNCTION(glVertexAttribPointer)
#define glUseProgram GL_FUNCTION(glUseProgram)
#define glActiveTexture GL_FUNCTION(glActiveTexture)
#define glUniform1i GL_FUNCTION(glUniform1i)
#define glUniformSubroutinesuiv GL_FUNCTION(glUniformSubroutinesuiv)
#define glDrawElements GL_FUNCTION(glDrawElements)
#define glGenBuffers GL_FUNCTION(glGenBuffers)
#define glCreateShader GL_FUNCTION(glCreateShader)
#define glShaderSource GL_FUNCTION(glShaderSource)
#define glCompileShader GL_FUNCTION(glCompileShader)
#define glGetShaderiv GL_FUNCTION(glGetShaderiv)
#define glGetShaderInfoLog GL_FUNCTION(glGetShaderInfoLog)
#define glCreateProgram GL_FUNCTION(glCreateProgram)
#define glAttachShader GL_FUNCTION(glAttachShader)
#define glLinkProgram GL_FUNCTION(glLinkProgram)
#define glGetProgramiv GL_FUNCTION(glGetProgramiv)
#define glGetProgramInfoLog GL_FUNCTION(glGetProgramInfoLog)
#define glViewport GL_FUNCTION(glViewport)
#define glDeleteShader GL_FUNCTION(glDeleteShader)
#define glGetSubroutineIndex GL_FUNCTION(glGetSubroutineIndex)
#define glGetUniformLocation GL_FUNCTION(glGetUniformLocation)
#define glTexImage2DMultisample GL_FUNCTION(glTexImage2DMultisample)
#define glGenFramebuffers GL_FUNCTION(glGenFramebuffers)
#define glBindFramebuffer GL_FUNCTION(glBindFramebuffer)
#define glCheckFramebufferStatus GL_FUNCTION(glCheckFramebufferStatus)
#define glFramebufferTexture2D GL_FUNCTION(glFramebufferTexture2D)
#define glClear GL_FUNCTION(glClear)
#define glMapBuffer GL_FUNCTION(glMapBuffer)
#define glUnmapBuffer GL_FUNCTION(glUnmapBuffer)
#define glDepthMask GL_FUNCTION(glDepthMask)
#define glDepthFunc GL_FUNCTION(glDepthFunc)
#define glBlendEquation GL_FUNCTION(glBlendEquation)
#define glBlendFunc GL_FUNCTION(glBlendFunc)
#define glCullFace GL_FUNCTION(glCullFace)
#define glFrontface GL_FUNCTION(glFrontface)
#define glGenVertexArrays GL_FUNCTION(glGenVertexArrays)
#define glBindVertexArray GL_FUNCTION(glBindVertexArray)
#define glFrontFace GL_FUNCTION(glFrontFace)
#define glGetUniformBlockIndex GL_FUNCTION(glGetUniformBlockIndex)
#define glUniformBlockBinding GL_FUNCTION(glUniformBlockBinding)
#define glBindBufferRange GL_FUNCTION(glBindBufferRange)
#define glBindBufferBase GL_FUNCTION(glBindBufferBase)
#define glDrawArrays GL_FUNCTION(glDrawArrays)
#define glUniform1f GL_FUNCTION(glUniform1f)
#define glBufferSubData GL_FUNCTION(glBufferSubData)
#define glUniform3fv GL_FUNCTION(glUniform3fv)
#define glLineWidth GL_FUNCTION(glLineWidth)
#define glUniformMatrix4fv GL_FUNCTION(glUniformMatrix4fv)
#define glBlitFramebuffer GL_FUNCTION(glBlitFramebuffer)
#define glTexParameterfv GL_FUNCTION(glTexParameterfv)
		
	static constexpr u32 OPENGL_LOG_BUFFER_SIZE = 256;
	static char g_OpenGLLogBuffer[OPENGL_LOG_BUFFER_SIZE];

	void _OpenGLClearErrorQueue()
	{
		while (g_OpenGLFuncTable->_glGetError() != GL_NO_ERROR);
	}

	b32 _OpenGLPeekError()
	{
		GLenum errorCode = g_OpenGLFuncTable->_glGetError();
		if (errorCode == GL_NO_ERROR)
		{
			return false;
		}

		const char* error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM: { error = "GL_INVALID_ENUM"; } break;
		case GL_INVALID_VALUE: { error = "GL_INVALID_VALUE"; } break;
		case GL_INVALID_OPERATION: { error = "GL_INVALID_OPERATION"; } break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: { error = "GL_INVALID_FRAMEBUFFER_OPERATION"; } break;
		case GL_OUT_OF_MEMORY: { error = "GL_OUT_OF_MEMORY"; } break;
		default: {error = "UNKNOWN_ERROR"; } break;
		}

		FormatString(g_OpenGLLogBuffer, OPENGL_LOG_BUFFER_SIZE,
					 "A error caused by OpenGL call. Error: %s, code: %i32.",
					 error, errorCode);
		return true;
	}

	void _OpenGLGetLog(char* buffer, u64 size)
	{
		if (size >= OPENGL_LOG_BUFFER_SIZE)
		{
			memcpy(buffer, g_OpenGLLogBuffer, OPENGL_LOG_BUFFER_SIZE);
		}
		else
		{
			*buffer = '\0';
		}
	}

#define AB_DEBUG_OPENGL
#if defined(AB_DEBUG_OPENGL)
#define GLCall(call)								\
													\
	{												\
		AB::_OpenGLClearErrorQueue();				\
		call;										\
		while (AB::_OpenGLPeekError())				\
		{											\
			char buffer[256];						\
			AB::_OpenGLGetLog(buffer, 256);			\
			AB_CORE_ERROR(buffer, "Call: ", #call);	\
		}											\
	} while (false)
#else
#define GLCall(call) call
#endif

}

extern "C" GAME_CODE_ENTRY void GameInit(AB::MemoryArena* arena,
										 AB::PlatformState* platform)
{
	using namespace AB;
	g_StaticStorage = (AB::StaticStorage*)PushSize(arena, KILOBYTES(1), 1);
	g_Platform = platform;
	g_OpenGLFuncTable = platform->gl;

	g_StaticStorage->tempArena = AllocateSubArena(arena, arena->size / 2);
	AB_CORE_ASSERT(g_StaticStorage->tempArena, "Failed to allocate tempArena.");

	g_StaticStorage->inputManager = InputInitialize(arena, platform);
	g_StaticStorage->debugRenderer = Renderer2DInitialize(arena,
														  g_StaticStorage->tempArena,
														  1280, 720);
	InputConnectToPlatform(g_StaticStorage->inputManager, platform);
	DebugOverlay* debugOverlay = CreateDebugOverlay(arena,
													g_StaticStorage->debugRenderer,
													g_StaticStorage->inputManager,
													V2(1280, 720));
	DebugOverlayEnableMainPane(debugOverlay, true);
	g_StaticStorage->assetManager = AssetManagerInitialize(arena,
														   g_StaticStorage->tempArena);
	g_StaticStorage->debugOverlay = debugOverlay;

	RendererConfig config;
	config.numSamples = 4;
	config.renderResolutionW = 1280;
	config.renderResolutionH = 720;
	config.shadowMapRes = 2048;

	g_StaticStorage->renderer = AllocateRenderer(arena,
												 g_StaticStorage->tempArena,
												 config);
  
	AB_CORE_WARN("INIT!");

	g_StaticStorage->gameVars = (Sandbox*)PushSize(arena, sizeof(Sandbox),
												   alignof(Sandbox));
	AB_CORE_ASSERT(g_StaticStorage->gameVars);
	
	Init(arena, g_StaticStorage->tempArena,
		 g_StaticStorage->gameVars,
		 g_StaticStorage->assetManager,
		 g_StaticStorage->inputManager);
}

extern "C" GAME_CODE_ENTRY void GameReload(AB::MemoryArena* arena,
										   AB::PlatformState* platform)
{
	using namespace AB;
	g_Platform = platform;
	g_OpenGLFuncTable = platform->gl; 
	g_StaticStorage = (StaticStorage*)arena->begin;
	InputConnectToPlatform(g_StaticStorage->inputManager, platform);
}

extern "C" GAME_CODE_ENTRY void GameUpdate(AB::MemoryArena* arena,
										   AB::PlatformState* platform)
{
	using namespace AB;
	UpdateDebugOverlay(g_StaticStorage->debugOverlay, platform);
}

extern "C" GAME_CODE_ENTRY void GameRender(AB::MemoryArena* arena,
										   AB::PlatformState* platform)
{
	using namespace AB;
	platform->gl->_glClearColor(0.9, 0.9, 0.9, 1.0);
	platform->gl->_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawDebugOverlay(g_StaticStorage->debugOverlay);
	
	Render(g_StaticStorage->gameVars,
		   g_StaticStorage->assetManager,
		   g_StaticStorage->renderer,
		   g_StaticStorage->inputManager);
	
	Renderer2DFlush(g_StaticStorage->debugRenderer, g_Platform);
	InputEndFrame(g_StaticStorage->inputManager);
}

#include "Log.cpp"
#include "InputManager.cpp"
#include "ExtendedMath.cpp"
#include "ImageLoader.cpp"
#include "render/DebugRenderer.cpp"
#include "DebugTools.cpp"
#include "GraphicsAPI.cpp"
#include "AssetManager.cpp"
#include "GraphicsPipeline.cpp"
#include "render/RenderGroup.cpp"
#include "render/Renderer.cpp"
#include "Sandbox.cpp"
