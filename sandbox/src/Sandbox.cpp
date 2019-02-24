#include <hypermath.h>
#include <Aberration.h>
extern "C" {
	ABERRATION_ENTRY void GameInitialize(AB::Engine* engine, AB::GameContext* gameContext) {
		gameContext->texHandle = engine->loadTexture("../../../assets/test.bmp");
		engine->freeTexture(gameContext->texHandle);
		gameContext->texHandle = engine->loadTexture("../../../assets/test.bmp");
		gameContext->regHandle = engine->textureCreateRegion(gameContext->texHandle, { 0.5, 0.5 }, { 0.8, 0.8 });
		gameContext->texHandle1 = engine->loadTexture("../../../assets/test_op.bmp");
		gameContext->texHandle2 = engine->loadTexture("../../../assets/art.bmp");
		// TODO: Game context in lambda
		engine->windowSetKeyCallback([](AB::KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount){
			if (key == AB::KeyboardKey::W) {
				//gameContext->x += 1;
			}
		});
	}
	ABERRATION_ENTRY void GameRender(AB::Engine* engine, AB::GameContext* gameContext) {
		//engine->fillRectangleColor({ 25, 560 }, 10, 0, 0, { 225, 40 }, 0xee2e271e);
		//engine->fillRectangleColor({ 0, 560 }, 10, 0, 0, { 25, 40 }, 0xff01a8ff);
		//hpm::Rectangle strr = engine->getStringBoundingRect(20.0, L"0 fps");
		//float32 h = (40 + strr.max.y) / 2;
		//engine->debugDrawString({35, 600 - h}, 20.0, 0xffffffff, L"0 fps");


		uint16 reg = engine->textureCreateRegion(gameContext->texHandle2, { 0.0, 0.0 }, { 0.8, 0.8 });
		uint16 reg1 = engine->textureCreateRegion(gameContext->texHandle2, { 0.0, 0.0 }, { 0.2, 0.2 });
#if 0
		const wchar_t* str = LR"(�� �������� ��������� ��� ������� � ���������� �� � ������ ����������
��������� �����, ��������� �� ���� � ��� �������������� ����������� ������ �����.
���� ����������� ������������ �������� ������ ������ ��� ���� ��������...
����� ����� ������������� �������� �������� ���������� ������ � �������������� � ����
������������ ������� � ����������������� ������������ ������, �������
�� ���� �� ��� �������� �����������, �.�.
������ ������, ��� ����� �� �������� ����������� ��� ��������������...
����� ��������� ������� �� ����������� �����, ��������, ���� ���
���������������� � ����� ��������.)";
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
		hpm::Vector2 p{ 10, 500 };
		hpm::Rectangle rect;
		engine->debugDrawString(p, 20.0, 0xff55ff44, str);
		rect = engine->getStringBoundingRect(p, 20, str);
		engine->fillRectangleColor(rect.min, 10, 0, 0, hpm::Subtract(rect.max, rect.min), 0xaaa33b8c);
		engine->fillRectangleColor({ 0, 0 }, 1, 0, 0, { 100, 100 }, 0x22ff0000);
		engine->fillRectangleTexture({ 50, 50 }, 0, 0, 0, { 100, 100 }, gameContext->texHandle1);
		engine->fillRectangleTexture({ 100, 100 }, 1,  0, 0, { 100, 100}, gameContext->texHandle);
		engine->fillRectangleTexture({ 150, 150 }, 2, 0, 0, { 100, 100 }, gameContext->texHandle2);
		engine->fillRectangleColor({ 200, 200 }, 3, 0, 0, { 100, 100 }, 0xaa0000ff);

		engine->fillRectangleTexture({ 200, 200 }, 7, 0, 0, { 100, 100 }, reg);
		engine->fillRectangleColor({ 250, 250 }, 6, 0, 0, { 100, 100 }, 0xaa0000ff);
		engine->fillRectangleColor({300, 300}, 8, 0, 0, { 200, 100 }, 0xffff0000);
		//// TODO: Here is a byg with sorting
		engine->fillRectangleColor({ 350, 350 },10, 0, 0, { 100, 30 }, 0xff1e272e);
		int i = 5;
		engine->fillRectangleColor({ 400, 400 }, 2, 0, 0, { 100, 100 }, 0xffff0000);
		engine->fillRectangleColor({ 450, 450 }, 10, 0, 0, { 100, 30 }, 0xff1e272e);
		engine->fillRectangleColor({ 500, 500 }, 3, 0, 0, { 100, 100 }, 0xff11f497);
		engine->fillRectangleTexture({550, 550 }, 5, 0, 0, { 100, 100}, reg1);

		engine->freeTexture(reg);
		engine->freeTexture(reg1);
	}
	ABERRATION_ENTRY void GameUpdate(AB::Engine* engine, AB::GameContext* gameContext) {}
}
