#include <Aberration.h>
#include "src/platform/API/OpenGL/ABOpenGL.h"
#include "Windows.h"
#include <hypermath.h>
#include "src/renderer/Renderer2D.h"

// !!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: Rewrote whole log because it have different instances in dll and exe
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// TODO: Window::Close

void showMemInfo() {
	AB::utils::Log::Initialize(AB::utils::LogLevel::Info);
	AB::SystemMemoryInfo info = {};
	AB::GetSystemMemoryInfo(info);
	AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
		"\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
		"\n");
}

uint32 frameCounter = 0;

int main() {
	showMemInfo();

	AB::Window::Create("sds d", 800, 600);
	AB::Window::EnableVSync(true);

	AB::Window::SetKeyCallback([](AB::KeyboardKey key, bool32 currState, bool32 prevState, uint32 repeatCount) {
		if (key == AB::KeyboardKey::Escape)
			AB::Window::Close();
	});

	AB::Renderer2D::Initialize(800, 600);
	AB::Renderer2D::LoadTexture("A:\\dev\\ab_nonrepo\\test.bmp");

	LARGE_INTEGER lastTime;
	LARGE_INTEGER performanceFeq;
	QueryPerformanceCounter(&lastTime);
	QueryPerformanceFrequency(&performanceFeq);
	while (AB::Window::IsOpen())
	{
		Color c;
		c.r = 0;
		c.g = 255;
		c.b = 0;
		c.a = 255;
		static float32 angle = 0;
		AB::Renderer2D::DrawRectangle({ 400, 400 }, angle, -0.5,  { 200, 200 }, c.hex);
		AB::Renderer2D::DrawRectangle({ 100, 100 }, angle * 2, -0.5,  { 50, 50 }, 0xff34f497);
		AB::Renderer2D::DrawRectangle({ 200, 200 }, angle / 2 , -0.5, { 100, 100 }, 0xffa6c8f3 );

		AB::Renderer2D::Flush();

		//angle += 1;
		AB::Window::SwapBuffers();
		AB::Window::PollEvents();
		LARGE_INTEGER currTime;
		QueryPerformanceCounter(&currTime);
		uint64 elapsed = currTime.QuadPart - lastTime.QuadPart;
		uint64 elapsedMS = (elapsed * 1000 * 1000) / performanceFeq.QuadPart;
		lastTime = currTime;
		frameCounter++;
		if (frameCounter > 1000) {
			printf("Time: %lluns\n", elapsedMS);
			frameCounter = 0;
		}
	}
	return 0;
}