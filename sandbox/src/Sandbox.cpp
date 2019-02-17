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
#if 0
		const wchar_t* str = LR"(Но немногие захватили эту горстку и превратили ее в орудие присвоения
продуктов труда, постоянно из года в год возобновляемых подавляющей массой людей.
Этим объясняется чрезвычайная важность такого орудия для этих немногих...
Около трети национального годового продукта отнимается теперь у производителей в виде
общественных налогов и непроизводительно потребляется людьми, которые
не дают за это никакого эквивалента, т.е.
ничего такого, что имело бы значение эквивалента для производителей...
Толпа изумленно смотрит на накопленные массы, особенно, если они
сконцентрированы в руках немногих.)";
#else
		const wchar_t* str = LR"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed
do eiusmod tempor incididunt ut labore et dolore magna
naliqua.Ut enim ad minim veniam, quis nostrud exercitation ullamco
laboris nisi ut aliquip ex ea commodo consequat.Duis aute irure
dolor in reprehenderit in voluptate velit esse cillum dolore
eu fugiat nulla pariatur.Excepteur sint occaecat cupidatat
non proident, sunt in culpa qui officia deserunt mollit
nim id est laborum.)";
#endif
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