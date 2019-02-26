#pragma once
#include <src/platform/Input.h>
#include <hypermath.h>

// TODO: Temporary
#include "platform/API/OpenGL/ABOpenGL.h"
#include "utils/ImageLoader.h"

#if defined(AB_PLATFORM_WINDOWS)
#define ABERRATION_ENTRY __declspec(dllexport)
#elif defined(AB_PLATFORM_LINUX)
#define ABERRATION_ENTRY
#else
#error Unsupported OS
#endif

namespace AB {

	typedef void(CloseCallback)(void);
	typedef void(ResizeCallback)(uint32 width, uint32 height);
	typedef void(KeyCallback)(KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount);
	typedef void(MouseButtonCallback)(MouseButton btn, bool32 state);
	typedef void(MouseMoveCallback)(uint32 xPos, uint32 yPos);
	typedef void(GamepadButtonCallback)(uint8 gpNumber, GamepadButton btn, bool32 currState, bool32 prevState);
	typedef void(GamepadStickCallback)(uint8 gpNumber, int16 xLs, int16 yLs, int16 xRs, int16 yRs);
	typedef void(GamepadTriggerCallback)(uint8 gpNumber, byte lt, byte rt);

	typedef void(ABRendererFillRectangleColorFn)(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, color32 color);
	typedef void(ABRendererFillRectangleTextureFn)(hpm::Vector2 position, uint16 depth, float32 angle, float32 anchor, hpm::Vector2 size, uint16 textureHandle);
	typedef void(ABDebugDrawStringWFn)(hpm::Vector2 position, float32 scale, color32 color, const wchar_t* string);
	typedef void(ABDebugDrawStringFn)(hpm::Vector2 position, float32 scale, color32 color, const char* string);
	typedef hpm::Rectangle(ABGetStringBoundingRectFn)(hpm::Vector2 position, float32 height, const wchar_t* string);
	typedef uint16(ABRendererLoadTextureFn)(const char* filepath);
	typedef void(ABRendererFreeTextureFn)(uint16 handle);
	typedef uint16(ABRendererTextureCreateRegion)(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
	typedef void(ABWindowSetKeyCallbackFn)(KeyCallback* func);
	typedef void(ABWindowSetMouseMoveCallbackFn)(MouseMoveCallback* func);
	typedef void(ABWindowGetMousePositionFn)(uint32* xPos, uint32* yPos);

	typedef ABGLProcs*(ABGLGetFunctions)();

	typedef int32(ABFormatStringFn)(char* buffer, uint32 bufferSize, const char* format, ...);
	typedef void(ABPrintStringFn)(const char* format, ...);
	typedef void(ABLogFn)(LogLevel level, const char* file, const char* func, uint32 line, const char* fmt, ...);
	typedef void(ABLogAssertFn)(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr, const char* fmt, ...);

	typedef Image(ABLoadBMPFn)(const char* filename);
	typedef void(ABDeleteBitmapFn)(void* ptr);

	struct Engine {
		ABRendererFillRectangleColorFn* fillRectangleColor;
		ABRendererFillRectangleTextureFn* fillRectangleTexture;
		ABDebugDrawStringWFn* debugDrawStringW;
		ABDebugDrawStringFn* debugDrawString;
		ABGetStringBoundingRectFn* getStringBoundingRect;
		ABRendererLoadTextureFn* loadTexture;
		ABRendererFreeTextureFn* freeTexture;
		ABWindowSetKeyCallbackFn* windowSetKeyCallback;
		ABWindowSetMouseMoveCallbackFn* windowSetMouseMoveCallback;
		ABWindowGetMousePositionFn* windowGetMousePositionCallback;
		ABRendererTextureCreateRegion* textureCreateRegion;
		ABGLGetFunctions* glGetFunctions;
		ABLogFn* log;
		ABLogAssertFn* logAssert;
		ABFormatStringFn* formatString;
		ABPrintStringFn* printString;
		ABLoadBMPFn* loadBMP;
		ABDeleteBitmapFn* deleteBitmap;
		//struct Renderer2D {
		//	static void FreeTexture(uint16 handle);
		//};
	};

	struct GameContext {
		int32 shaderHandle;
		uint32 vbo;
		uint32 texture;
		hpm::Vector3 camPos;
		hpm::Vector3 camFront;
		hpm::Vector3 camUp;
		float32 pitch;
		float32 yaw;
		uint32 xLastMouse;
		uint32 yLastMouse;
	};

}

// TODO: This is depends on gameContext var now
#if defined(AB_CONFIG_DISTRIB)

#define AB_INFO(format, ...)	do{}while(false)
#define AB_WARN(format, ...)	do{}while(false)
#define AB_ERROR(format, ...)	do{}while(false)
#define AB_FATAL(format, ...)	do{}while(false)
#define AB_ASSERT(format, expr, ...)	do{}while(false)

#else

#if defined (__clang__)
#define AB_INFO(format, ...) engine->log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_WARN(format, ...) engine->log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_ERROR(format, ...) engine->log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_FATAL(format, ...) do { engine->log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();} while(false)
// TODO: print assertions
#define AB_ASSERT(expr, format, ...) do { if (!(expr)) {engine->logAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#else
#define AB_INFO(format, ...) engine->log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_WARN(format, ...) engine->log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_ERROR(format, ...) engine->log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_FATAL(format, ...) do { engine->log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, __VA_ARGS__); AB_DEBUG_BREAK();} while(false)
// TODO: print assertions
#define AB_ASSERT(expr, format, ...) do { if (!(expr)) {engine->logAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, #expr ,format, __VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#endif
#endif