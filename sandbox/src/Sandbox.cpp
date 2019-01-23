#include <Aberration.h>

// !!!!!!!!!!!!!!!!!!!!!!!!!!
// TODO: Rewrote whole log because not it have different instances in dll and exe
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	int main() {

		AB::utils::Log::Initialize(AB::utils::LogLevel::Info);
		AB::SystemMemoryInfo info = {};
		AB::GetSystemMemoryInfo(info);
		AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
			"\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
			"\n");

		AB::Window::Create("Aberration", 800, 600);

		//AB::Window::SetMouseMoveCallback([](uint32 x, uint32 y) {printf("Mouse moved: %d %d\n", x, y); });

		AB::Window::SetCloseCallback([]() {
			printf("Close callback called.\n");
		});

		AB::Window::SetResizeCallback([](uint32 w, uint32 h) {
			printf("Window resized: %d %d\n", w, h);
		});

		AB::Window::SetMouseButtonCallback([](AB::MouseButton b, bool s) {
			printf("Mouse button pressed: %u %u\n", static_cast<uint32>(b), static_cast<uint32>(s));
		});

		AB::Window::SetKeyCallback([](AB::KeyboardKey key, bool currState, bool prevState, uint32 repeatCount) {
			printf("Key pressed: %s %d %d %d\n", AB::ToString(key).c_str(), currState, prevState, repeatCount);
		});

		while (AB::Window::IsOpen()) {
			AB::Window::PollEvents();
		}

		AB::Window::Destroy();
	}