#pragma once
#include "src/ABHeader.h"
#include "src/platform/Input.h"
#include <hypermath.h>

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
	typedef void(ABDebugDrawStringFn)(hpm::Vector2 position, float32 scale, const char* string);
	typedef hpm::Rectangle(ABGetStringBoundingRectFn)(float32 height, const char* string);
	typedef uint16(ABRendererLoadTextureFn)(const char* filepath);
	typedef void(ABRendererFreeTextureFn)(uint16 handle);
	typedef uint16(ABRendererTextureCreateRegion)(uint16 handle, hpm::Vector2 min, hpm::Vector2 max);
	typedef void(ABWindowSetKeyCallbackFn)(KeyCallback* func);

	struct Engine {
		ABRendererFillRectangleColorFn* fillRectangleColor;
		ABRendererFillRectangleTextureFn* fillRectangleTexture;
		ABDebugDrawStringFn* debugDrawString;
		ABGetStringBoundingRectFn* getStringBoundingRect;
		ABRendererLoadTextureFn* loadTexture;
		ABRendererFreeTextureFn* freeTexture;
		ABWindowSetKeyCallbackFn* windowSetKeyCallback;
		ABRendererTextureCreateRegion* textureCreateRegion;
		struct Renderer2D {
			static void FreeTexture(uint16 handle);
		};
	};

	struct GameContext {
		uint32 x;
		uint32 y;
		uint16 texHandle;
		uint16 texHandle1;
		uint16 texHandle2;
		uint16 regHandle;
	};

}