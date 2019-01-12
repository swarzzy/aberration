#include <Aberration.h>

class A {
public:
	std::string ToString() { return "dfg"; }
};

class B {
	
};
int main() {
	AB_CORE_INFO("Macro info ", 43);
	AB_CORE_WARN("Macro Warn ", 43);
	AB_CORE_ERROR("Macro error ", 43);
	//AB_CORE_FATAL("Macro fatal ", 43);
	ab::DateTime t;
	ab::get_local_time(t);
	printf(t.ToString().c_str());



	system("pause");
	return 0;
}
