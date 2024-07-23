#include <iostream>
#include <Cango/TaskDesign.hpp>

namespace {
	struct IntSource final{
		using ItemType = int;
		std::chrono::steady_clock::time_point LastLogTime{};
		std::uint32_t Count{};

		int Value{};
		bool GetItem(int& value) noexcept {
			++Count;

			const auto now = std::chrono::steady_clock::now();
			const auto diff = now - LastLogTime;
			if (constexpr auto interval = std::chrono::milliseconds{1000}; diff > interval) {
				std::cout << "[Rate]: " << Count << '\n';
				LastLogTime = now;
				Count = 0;
			}

			value = Value++;
			return value % 7 != 0;
		}
	};

	struct SimpleMonitor final {
		std::atomic_bool Done{false};

		bool IsDone() const noexcept { return Done.load(std::memory_order_relaxed); }
		void Interrupt() noexcept { Done.store(true, std::memory_order_relaxed); }
		void Reset() noexcept { Done.store(false, std::memory_order_relaxed); }
		void HandleItemSourceError() noexcept { }
	};
}

using namespace Cango;

int main() {
	const auto source_owner = std::make_shared<IntSource>();
	const auto destination_owner = std::make_shared<EmptyItemDestination<int>>();
	const auto monitor_owner = std::make_shared<SimpleMonitor>();
	DeliveryTask<IntSource, EmptyItemDestination<int>, SimpleMonitor> task{};
	{
		auto&& config = task.Configure();
		config.Actors.ItemSource = source_owner;
		config.Actors.ItemDestination = destination_owner;
		config.Actors.Monitor = monitor_owner;
		config.Options.MinInterval = std::chrono::milliseconds{1};
	}
	task.Execute();

	return 0;
}
