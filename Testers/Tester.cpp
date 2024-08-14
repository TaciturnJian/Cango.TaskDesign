#include <Cango/TaskDesign/ObjectOwnership.hpp>
#include <iostream>

using namespace Cango;

namespace {
	struct A {
		int A1{10};
	};
}

int main() {
	ObjectOwner<A> a_owner{};
	std::cout << a_owner->A1;
	return 0;
}