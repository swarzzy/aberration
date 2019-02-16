#include <hypermath.h>
#include <Aberration.h>
extern "C" {
	ABERRATION_ENTRY void GameInitialize(AB::Engine* engine, AB::GameContext* gameContext) {
		gameContext->texHandle = engine->loadTexture("A:\\dev\\ab_nonrepo\\test.bmp");
		engine->freeTexture(gameContext->texHandle);
		gameContext->texHandle = engine->loadTexture("A:\\dev\\ab_nonrepo\\test.bmp");
		gameContext->regHandle = engine->textureCreateRegion(gameContext->texHandle, { 0.5, 0.5 }, { 0.8, 0.8 });
		gameContext->texHandle1 = engine->loadTexture("A:\\dev\\ab_nonrepo\\art.bmp");
		gameContext->texHandle2 = engine->loadTexture("A:\\dev\\ab_nonrepo\\test_op.bmp");
		// TODO: Game context in lambda
		engine->windowSetKeyCallback([](AB::KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount){
			if (key == AB::KeyboardKey::W) {
				//gameContext->x += 1;
			}
		});
	}

	ABERRATION_ENTRY void GameUpdate(AB::Engine* engine, AB::GameContext* gameContext) {
		uint16 reg = engine->textureCreateRegion(gameContext->texHandle2, { 0.0, 0.0 }, { 0.8, 0.8 });
		uint16 reg1 = engine->textureCreateRegion(gameContext->texHandle2, { 0.0, 0.0 }, { 0.2, 0.2 });
		const char* str = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed\ndo eiusmod tempor incididunt ut labore et dolore magna\nnaliqua.Ut enim ad minim veniam, quis nostrud exercitation ullamco\nlaboris nisi ut aliquip ex ea commodo consequat.Duis aute irure\ndolor in reprehenderit in voluptate velit esse cillum dolore\neu fugiat nulla pariatur.Excepteur sint occaecat cupidatat\nnon proident, sunt in culpa qui officia deserunt mollit\nnim id est laborum.";
		hpm::Vector2 p(10, 500);
		engine->debugDrawString(p, 20.0, str);
		hpm::Rectangle rect;
		rect = engine->getStringBoundingRect(20, str);
		engine->fillRectangleColor(hpm::Add(rect.min, p), 10, 0, 0, rect.max, 0x55a33b8c);
		engine->fillRectangleTexture({ 150, 150 }, 5, 0, 0, { 100, 100 }, gameContext->texHandle1);
		engine->fillRectangleTexture({ 0, 0 }, 0,  0, 0, { 100, 100}, gameContext->texHandle);
		engine->fillRectangleColor({ 50, 50 }, 0, 0, 0, { 100, 100 }, 0xaa0000ff);
		engine->fillRectangleColor({ 300, 500 }, 1, 0, 0, { 100, 100 }, 0x44ff0000);
		engine->fillRectangleTexture({ 150, 170 }, 1, 0, 0, { 100, 100 }, gameContext->texHandle2);
		engine->fillRectangleTexture({ 200, 200 }, 7, 0, 0, { 100, 100 }, reg);
		engine->fillRectangleColor({350, 350}, 1, 0, 0, { 100, 100 }, 0xffff0000);
		engine->fillRectangleColor({ 250, 250 }, 1, 0, 0, { 100, 100 }, 0xff11f497);
		engine->fillRectangleTexture({ 70, 70 }, 1, 0, 0, { 100, 100}, reg1);
		engine->fillRectangleColor({ 400, 400 }, 1, 0, 0, { 100, 100 }, 0xffff0000);
		engine->freeTexture(reg);
		engine->freeTexture(reg1);
	}
	ABERRATION_ENTRY void GameRender(AB::Engine* engine, AB::GameContext* gameContext) {}
}