#include <iostream>
#include <vector>
#include <spdlog/spdlog.h>
#include <Cango/CommonUtils/FormatExtension.hpp>


namespace {
	struct TestObject {
		int V1;
		float V2;
	};

	struct TestObject2 {
		double V1;
		float V2;
		int V3;
	};
}

namespace fmt {
	// {:1} -> V1
	// {:2} -> V2
	// {} -> V1, V2
	template <>
	struct formatter<TestObject> : char_status_parser {
		auto format(const TestObject& obj, auto& context) const {
			switch (Status) {
			case 'a':
				return format_to(context.out(), "{}", obj.V1);
			case 'b':
				return format_to(context.out(), "{}", obj.V2);
			default:
				return format_to(context.out(), "{},{}", obj.V1, obj.V2);
			}
		}
	};

	template <>
	struct formatter<TestObject2> : empty_parser {
		auto format(const TestObject2& obj, auto& context) {
			return format_to(context.out(), "{},{},{}", obj.V1, obj.V2, obj.V3);
		}
	};
}

int main() {
	const TestObject object{1, 2};
	fmt::print("[[{0}]] {1} {2}", fmt::join(std::vector{object, object}, "], ["), TestObject2{});
}
