#include <Aberration.h>

class A {
public:
	std::string ToString() { return "dfg"; }
};

class B {
	
};
int main() {
	ab::SystemMemoryInfo info = {};
	ab::get_system_memory_info(info);
	AB_CORE_INFO("Memory load: ", info.memoryLoad, "\nTotal phys: ", info.totalPhys / 1024 / 1024, "\nAvail phys: ", info.availablePhys / 1024 / 1024,
				 "\nTotal swap: ", info.totalSwap / (1024 * 1024), "\nAvail swap: ", info.availableSwap / 1024 / 1024,
				 "\n");

	int* i = AB_NEW int[999];
	AB_DELA i;
	//std::unique_ptr<int> a = std::make_unique<int>(4);
	ab::AppMemoryInfo mem = {};
	ab::get_app_memory_info(mem);
	AB_CORE_INFO("\nCurr: ", mem.currentUsed, "\nCurr alloc: ", mem.currentAllocations, "\nTotal: ", mem.totalUsed, "\nTotal alloc: ", mem.totalAllocations, "\n");
	std::vector<int, ab::Allocator<int>> list;
	system("pause");
	return 0;
}
