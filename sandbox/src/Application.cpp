#include "Application.h"
#include "Aberration.h"

namespace AB {
	static AB::Engine* g_PlatformContext = nullptr;
	static AB::GameContext* g_GameContext = nullptr;

	const Engine* Platform() {
		return g_PlatformContext;
	}
}

extern "C" inline  ABERRATION_ENTRY void GameReconnect(AB::Engine* engine, AB::GameContext* gameContext) {
	AB::g_PlatformContext = engine;
	AB::g_GameContext = gameContext;
}