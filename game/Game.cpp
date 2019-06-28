#define HYPERMATH_IMPL 
#include <hypermath.h>
#include "Shared.h"
#include "Memory.h"
#include "OpenGL.h"
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
		GameState* gameVars;
	};

	static PlatformState* g_Platform;
	static StaticStorage* g_StaticStorage;
	static GLFuncTable* g_OpenGLFuncTable;
// NOTE: Actual frame tile
#define GlobalAbsDeltaTime g_Platform->absDeltaTime
// NOTE: Frame time corrected by game speed
#define GlobalGameDeltaTime g_Platform->gameDeltaTime
// NOTE: Why clang inserts the dot by itself
#define GlobalInput g_Platform->input
#define PlatformGlobals (*g_Platform)
#if defined(_MSC_VER)
#define PLATFORM_FUNCTION(func) g_Platform->functions.##func
#else
#define PLATFORM_FUNCTION(func) g_Platform->functions. func
#endif
	

#define DebugReadFilePermanent PLATFORM_FUNCTION(DebugReadFilePermanent)
//#define WindowGetSize PLATFORM_FUNCTION(WindowGetSize)
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
#define glVertexAttribDivisor GL_FUNCTION(glVertexAttribDivisor)
#define glDrawElementsInstanced GL_FUNCTION(glDrawElementsInstanced)
#define glDrawArraysInstanced GL_FUNCTION(glDrawArraysInstanced)
#define glClearDepth GL_FUNCTION(glClearDepth)
#define glTexImage3D GL_FUNCTION(glTexImage3D)
#define glTexSubImage3D GL_FUNCTION(glTexSubImage3D)
#define glTexStorage3D GL_FUNCTION(glTexStorage3D)
#define glGenerateMipmap GL_FUNCTION(glGenerateMipmap)
#define glTexParameterf GL_FUNCTION(glTexParameterf)
		
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

void GameInit(AB::MemoryArena* arena,  AB::PlatformState* platform);
void GameReload(AB::MemoryArena* arena,  AB::PlatformState* platform);
void GameUpdate(AB::MemoryArena* arena,  AB::PlatformState* platform);
void GameRender(AB::MemoryArena* arena,  AB::PlatformState* platform);

extern "C" GAME_CODE_ENTRY void
GameUpdateAndRender(AB::MemoryArena* arena,
					AB::PlatformState* platform,
					AB::GameUpdateAndRenderReason reason)
{
	using namespace AB;
	switch (reason)
	{
	case GUR_REASON_INIT:
	{
		GameInit(arena, platform);
	} break;
	case GUR_REASON_RELOAD:
	{
		GameReload(arena, platform);
	} break;
	case GUR_REASON_UPDATE:
	{
		GameUpdate(arena, platform);
	} break;
	case GUR_REASON_RENDER:
	{
		GameRender(arena, platform);
	} break;
	INVALID_DEFAULT_CASE();
	}
}

void GameInit(AB::MemoryArena* arena,
			  AB::PlatformState* platform)
{
	using namespace AB;
	g_StaticStorage = (AB::StaticStorage*)PushSize(arena, KILOBYTES(1), 1);
	g_Platform = platform;
	g_OpenGLFuncTable = platform->gl;

	g_Platform->gameSpeed = 1.0f;

	g_StaticStorage->tempArena = AllocateSubArena(arena, arena->size / 2);
	AB_CORE_ASSERT(g_StaticStorage->tempArena, "Failed to allocate tempArena.");

	g_StaticStorage->debugRenderer = Renderer2DInitialize(arena,
														  g_StaticStorage->tempArena,
														  1366, 768);
	DebugOverlay* debugOverlay = CreateDebugOverlay(arena,
													g_StaticStorage->debugRenderer,
													V2(1366, 768));
	DebugOverlayEnableMainPane(debugOverlay, true);
	g_StaticStorage->assetManager = AssetManagerInitialize(arena,
														   g_StaticStorage->tempArena);
	g_StaticStorage->debugOverlay = debugOverlay;

	RendererConfig config;
	config.numSamples = 4;
	config.renderResolutionW = 1366;
	config.renderResolutionH = 768;
	config.shadowMapRes = 2048;

	g_StaticStorage->renderer = AllocateRenderer(arena,
												 g_StaticStorage->tempArena,
												 config);
  
	AB_CORE_WARN("INIT!");

	g_StaticStorage->gameVars = (GameState*)PushSize(arena, sizeof(GameState),
												   alignof(GameState));
	AB_CORE_ASSERT(g_StaticStorage->gameVars);
	
	Init(arena, g_StaticStorage->tempArena,
		 g_StaticStorage->gameVars,
		 g_StaticStorage->assetManager);
}

void GameReload(AB::MemoryArena* arena, AB::PlatformState* platform)
{
	using namespace AB;
	g_Platform = platform;
	g_OpenGLFuncTable = platform->gl; 
	g_StaticStorage = (StaticStorage*)arena->begin;
}

void GameUpdate(AB::MemoryArena* arena, AB::PlatformState* platform)
{
	using namespace AB;
	UpdateDebugOverlay(g_StaticStorage->debugOverlay, platform);
}

void GameRender(AB::MemoryArena* arena, AB::PlatformState* platform)
{
	using namespace AB;
	platform->gl->_glClearColor(0.9, 0.9, 0.9, 1.0);
	platform->gl->_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawDebugOverlay(g_StaticStorage->debugOverlay);
	
	Render(arena, g_StaticStorage->tempArena,
		   g_StaticStorage->gameVars,
		   g_StaticStorage->assetManager,
		   g_StaticStorage->renderer);
	
	Renderer2DFlush(g_StaticStorage->debugRenderer, g_Platform);
}

#include "Log.cpp"
#include "ExtendedMath.cpp"
#include "ImageLoader.cpp"
#include "render/DebugRenderer.cpp"
#include "DebugTools.cpp"
#include "GraphicsAPI.cpp"
#include "AssetManager.cpp"
//#include "GraphicsPipeline.cpp"
#include "render/RenderGroup.cpp"
#include "render/Renderer.cpp"
#include "render/ChunkMesher.cpp"
#include "Sandbox.cpp"
