#include <Aberration.h>
#include <conio.h>

int main() {
	AB::utils::Log::Initialize(AB::utils::LogLevel::Info);
	AB::SystemMemoryInfo info = {};
	AB::GetSystemMemoryInfo(info);
	AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
				 "\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
				 "\n");

	AB::Window::Create("Aberration", 800, 600);
	AB::Window::SetResizeCallback([](uint32 w, uint32 h) {printf("Window resized: %u %u\n", w, h); });
	AB::Window::SetGamepadButtonCallback([](uint8 g, AB::GamepadButton b, bool c, bool p) {printf("Button: %d c: %d p: %d\n", b, c, p); });
	AB::Window::SetKeyCallback([](AB::KeyboardKey k, bool c, bool p, uint32 rc) {printf("Key pressed: %d %d %d %d\n", static_cast<uint32>(k), c, p, rc); });
	AB::Window::SetMouseButtonCallback([](AB::MouseButton b, bool s) {printf("Mouse pressed: %d %d\n", static_cast<uint8>(b), s); });
	//AB::Window::SetMouseMoveCallback([](uint32 x, uint32 y) {printf("%d %d\n", x, y); });
	
	while(AB::Window::IsOpen()) {
		
		AB::Window::PollEvents();
	}

	AB::Window::Destroy();

	AB::AppMemoryInfo mem = {};
	AB::GetAppMemoryInfo(mem);
	AB_CORE_INFO("\nCurr: ", mem.currentUsed, "\nCurr alloc: ", mem.currentAllocations, "\nTotal: ", mem.totalUsed, "\nTotal alloc: ", mem.totalAllocations, "\n");
	std::vector<int, AB::Allocator<int>> list;
	_getch();
	return 0;
}