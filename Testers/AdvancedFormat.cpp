#include <algorithm>
#include <format>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <matchit.h>

constexpr auto constexpr_return(auto value) {
	return [value] constexpr {return value;};
}

constexpr std::int32_t factorial(std::int32_t n)
{
	using namespace matchit;

	if (n < 0) throw std::invalid_argument("n must be non-negative");

	return match(n)(
		pattern | 0 = constexpr_return(1),
		pattern | 1 = constexpr_return(1),
		pattern | 2 = constexpr_return(2),
		pattern | _ = [n] { return n * factorial(n - 1); }
	);
}

int main() {
	std::cout << std::format("Factorial of 5 is {}\n", factorial(5));

	return 0;
}
