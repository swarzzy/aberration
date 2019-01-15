#include <Aberration.h>
#include "src/platform/Window.h"
#include "src/platform/windows/Win32Window.h"

int main() {

	AB::SystemMemoryInfo info = {};
	AB::get_system_memory_info(info);
	AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
				 "\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
				 "\n");

	AB::WindowCreate("Aberration", 400, 300);
	AB::WindowSetResizeCallback([](uint32 w, uint32 h) {printf("%u %u\n", w, h); });
	AB::InputSetGamepadButtonCallback([](uint8 g, AB::GamepadButton b, bool c, bool p) {printf("Button: %d c: %d p: %d\n", b, c, p); });
	AB::InputSetGamepadStickCallback([](uint8 g, int16 lx, int16 ly, int16 rx, int16 ry) {system("cls"); printf("%d %d %d %d", lx, ly, rx, ry); });


	while(AB::WindowIsOpen()) {

		AB::WindowPollEvents();
		system("cls");
		
		if (AB::InputGamepadButtonPressed(0, AB::GamepadButton::RightStick))
			printf("lol\n");
	}

	AB::WindowDestroy();
	AB::AppMemoryInfo mem = {};
	AB::get_app_memory_info(mem);
	AB_CORE_INFO("\nCurr: ", mem.currentUsed, "\nCurr alloc: ", mem.currentAllocations, "\nTotal: ", mem.totalUsed, "\nTotal alloc: ", mem.totalAllocations, "\n");
	std::vector<int, AB::Allocator<int>> list;
	return 0;
}