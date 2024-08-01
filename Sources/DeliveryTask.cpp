#include <Cango/TaskDesign/DeliveryTask.hpp>

namespace Cango :: inline TaskDesign {
	void EasyDeliveryTaskMonitor::EmptyHandler() noexcept {}
	bool EasyDeliveryTaskMonitor::IsDone() const noexcept { return Done.load(std::memory_order_relaxed); }
	void EasyDeliveryTaskMonitor::Interrupt() noexcept { Done.store(true, std::memory_order_relaxed); }

	void EasyDeliveryTaskMonitor::Reset() noexcept {
		Done.store(false, std::memory_order_relaxed);
		Counter.Reset();
	}

	void EasyDeliveryTaskMonitor::HandleItemSourceError() noexcept {
		if (ExceptionHandler) {
			ExceptionHandler();
			return;
		}
		if (Counter.Count()) Interrupt();
	}

	void EasyDeliveryTaskMonitor::HandleItemSourceSuccess() noexcept {
		if (NormalHandler) {
			NormalHandler();
			return;
		}
		Counter.Reset();
	}
}
