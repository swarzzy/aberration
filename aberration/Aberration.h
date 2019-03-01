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

	typedef void(PlatformCloseCallback)(void);
	typedef void(PlatformResizeCallback)(uint32 width, uint32 height);
	typedef void(PlatformKeyCallback)(KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount);
	typedef void(PlatformMouseButtonCallback)(MouseButton btn, bool32 state);
	typedef void(PlatformMouseMoveCallback)(uint32 xPos, uint32 yPos, void* userData);
	typedef void(PlatformFocusCallback)(bool32 focus, void* userData);


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
	typedef void(ABWindowSetKeyCallbackFn)(PlatformKeyCallback* func);
	typedef void(ABWindowSetMouseMoveCallbackFn)(PlatformMouseMoveCallback* func, void* userData);
	typedef void(ABWindowGetMousePositionFn)(uint32* xPos, uint32* yPos);

	typedef ABGLProcs*(ABGLGetFunctions)();

	typedef int32(ABFormatStringFn)(char* buffer, uint32 bufferSize, const char* format, ...);
	typedef void(ABPrintStringFn)(const char* format, ...);
	typedef void(ABLogFn)(LogLevel level, const char* file, const char* func, uint32 line, const char* fmt, ...);
	typedef void(ABLogAssertFn)(LogLevel level, const char* file, const char* func, uint32 line, const char* assertStr, const char* fmt, ...);

	typedef Image(ABLoadBMPFn)(const char* filename);
	typedef void(ABDeleteBitmapFn)(void* ptr);

	typedef bool32(ABWindowGetKeyboardStateFn)(byte* buffer, uint32 bufferSize);

	struct PlatformKeyInfo;
	struct PlatformMouseButtonInfo;

	typedef PlatformKeyInfo(ABGetKeyStateFn)(KeyboardKey key);
	typedef PlatformMouseButtonInfo(ABGetMouseButtonStateFn)(MouseButton button);
	typedef void(ABWindowGetSizeFn)(uint32* width, uint32* height);
	typedef void(ABSetMousePositionFn)(uint32 x, uint32 y);

	typedef void(PlatformWindowSetFocusCallbackFn)(void* userData, PlatformFocusCallback* func);
	typedef bool32(PlatformWindowWindowActiveFn)();
	typedef void(PlatformWindowsShowCursorFn)(bool32 show);

	// DebugOverlay
	typedef void(PlatformDebugOverlayPushStringFn)(const char* string);
	typedef void(PlatformDebugOverlayPushV2Fn)(const char* title, hpm::Vector2 vec);
	typedef void(PlatformDebugOverlayPushV3Fn)(const char* title, hpm::Vector3 vec);
	typedef void(PlatformDebugOverlayPushV4Fn)(const char* title, hpm::Vector4 vec);
	typedef void(PlatformDebugOverlayPushF32Fn)(const char* title, float32 var);
	typedef void(PlatformDebugOverlayPushI32Fn)(const char* title, int32 var);
	typedef void(PlatformDebugOverlayPushU32Fn)(const char* title, uint32 var);

	struct Engine {
		ABRendererFillRectangleColorFn* fillRectangleColor;
		ABRendererFillRectangleTextureFn* fillRectangleTexture;
		ABDebugDrawStringWFn* debugDrawStringW;
		ABDebugDrawStringFn* debugDrawString;
		ABGetStringBoundingRectFn* getStringBoundingRect;
		ABRendererLoadTextureFn* loadTexture;
		ABRendererFreeTextureFn* freeTexture;
		ABWindowSetKeyCallbackFn* windowSetKeyCallback;
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
		ABWindowGetKeyboardStateFn* GetKeyboardState;
		ABGetKeyStateFn* GetKeyState;
		ABGetMouseButtonStateFn* GetMouseButtonState;
		ABWindowSetMouseMoveCallbackFn* SetMouseMoveCallback;
		ABWindowGetSizeFn* GetWindowSize;
		ABSetMousePositionFn* SetMousePosition;
		PlatformWindowSetFocusCallbackFn* SetFocusCallback;
		PlatformWindowWindowActiveFn* WindowActive;
		PlatformWindowsShowCursorFn* ShowCursor;

		// Debug Overlay
		PlatformDebugOverlayPushStringFn* DebugOverlayPushStr;
		PlatformDebugOverlayPushV2Fn* DebugOverlayPushV2;
		PlatformDebugOverlayPushV3Fn* DebugOverlayPushV3;
		PlatformDebugOverlayPushV4Fn* DebugOverlayPushV4;
		PlatformDebugOverlayPushF32Fn* DebugOverlayPushF32;
		PlatformDebugOverlayPushI32Fn* DebugOverlayPushI32;
		PlatformDebugOverlayPushU32Fn* DebugOverlayPushU32;
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
		float32 xLastMouse;
		float32 yLastMouse;
	};

}