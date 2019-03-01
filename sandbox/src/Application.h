#pragma once
#include "Aberration.h"

namespace AB {
	const Engine* Platform();

#define DEBUG_OVERLAY_PUSH_STRING AB::Platform()->DebugOverlayPushStr
#define DEBUG_OVERLAY_PUSH_V2 AB::Platform()->DebugOverlayPushV2
#define DEBUG_OVERLAY_PUSH_V3 AB::Platform()->DebugOverlayPushV3
#define DEBUG_OVERLAY_PUSH_V4 AB::Platform()->DebugOverlayPushV4
#define DEBUG_OVERLAY_PUSH_F32 AB::Platform()->DebugOverlayPushF32
#define DEBUG_OVERLAY_PUSH_I32 AB::Platform()->DebugOverlayPushI32
#define DEBUG_OVERLAY_PUSH_U32 AB::Platform()->DebugOverlayPushU32

#if defined(AB_CONFIG_DISTRIB)

#define AB_INFO(format, ...)	do{}while(false)
#define AB_WARN(format, ...)	do{}while(false)
#define AB_ERROR(format, ...)	do{}while(false)
#define AB_FATAL(format, ...)	do{}while(false)
#define AB_ASSERT(format, expr, ...)	do{}while(false)

#else

#if defined (__clang__)
#define AB_INFO(format, ...) AB::Platform()->log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_WARN(format, ...) AB::Platform()->log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_ERROR(format, ...) AB::Platform()->log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__)
#define AB_FATAL(format, ...) do { AB::Platform()->log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();} while(false)
	// TODO: print assertions
#define AB_ASSERT(expr, format, ...) do { if (!(expr)) {engine->logAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#else
#define AB_INFO(format, ...) AB::Platform()->log(AB::LogLevel::Info, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_WARN(format, ...) AB::Platform()->log(AB::LogLevel::Warn, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_ERROR(format, ...) AB::Platform()->log(AB::LogLevel::Error, __FILE__, __func__, __LINE__, format, __VA_ARGS__)
#define AB_FATAL(format, ...) do { AB::Platform()->log(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, format, __VA_ARGS__); AB_DEBUG_BREAK();} while(false)
	// TODO: print assertions
#define AB_ASSERT(expr, format, ...) do { if (!(expr)) {AB::Platform()->logAssert(AB::LogLevel::Fatal, __FILE__, __func__, __LINE__, #expr ,format, __VA_ARGS__); AB_DEBUG_BREAK();}} while(false)
#endif
#endif
}
