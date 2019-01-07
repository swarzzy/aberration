#include <Aberration.h>

class A {
public:
	std::string ToString() { return "dfg"; }
};

class B {
	
};
int main() {
	int arr[] = { 96 };
	int32 a = ab::console_print(arr, 1);
	ab::console_set_color(ab::ConsoleColor::Red, ab::ConsoleColor::DarkWhite);
	ab::console_print("\n", 1);
	ab::console_set_color(ab::ConsoleColor::DarkWhite, ab::ConsoleColor::Black);
	ab::console_print(std::to_string(a).c_str(), 2);
	printf("\n%llu", std::size("12345"));
	const char* str = "12345";
	B b;
	ab::utils::Log::Initialize(ab::utils::LogLevel::Warn);
	ab::utils::Log::GetInstance()->Message(ab::utils::LogLevel::Warn, str, "6789asdffg", "zxcvbn" );
	//ab::utils::Log::GetInstance()->HandleArg("6789asdf");
	//ab::utils::Log::GetInstance()->HandleArg("zxcvbn");

	//ab::utils::Log::GetInstance()->HandleArg("zxcv");



	system("pause");
	return 0;
}
