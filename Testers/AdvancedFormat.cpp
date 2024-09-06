#include <algorithm>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>

struct TestObject2 {
	int V3;
	float V2;
	double V1;
};

template <>
struct std::formatter<TestObject2, char> {
	constexpr auto parse(std::format_parse_context& context) {
		const auto it = context.begin();
		if (*it != '}') throw std::format_error("invalid format");
		return it;
	}

	auto format(const TestObject2& obj, std::format_context& context) const {
		return std::format_to(context.out(), "{} {} {}", obj.V1, obj.V2, obj.V3);
	}
};

int main() {
	TestObject2 object2{};
	std::cout << std::format("{} {}\n", 1, object2);
	return 0;
}
