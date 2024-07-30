#pragma once

#include <Cango/CommonUtils/IntervalSleeper.hpp>
#include <Cango/CommonUtils/CounterX.hpp>

#include "ItemDelivery.hpp"
#include "ItemOwnership.hpp"
#include "TaskExecution.hpp"

namespace Cango :: inline TaskDesign {
	struct DeliveryTaskMonitor : virtual DoneSignal {
		virtual void HandleItemSourceError() noexcept = 0;
		virtual void HandleItemSourceSuccess() noexcept = 0;
	};

	template <typename TMonitor>
	concept IsDeliveryTaskMonitor = IsDoneSignal<TMonitor> && requires(TMonitor& monitor) {
		{ monitor.HandleItemSourceError() };
		{ monitor.HandleItemSourceSuccess() };
	};

	struct EasyDeliveryTaskMonitor final {
		std::function<void()> NormalHandler{};
		std::function<void()> ExceptionHandler{};
		std::atomic_bool Done{false};
		Counter16 Counter{0, 5};

		[[nodiscard]] bool IsDone() const noexcept;
		void Interrupt() noexcept;
		void Reset() noexcept;
		void HandleItemSourceError() noexcept;
		void HandleItemSourceSuccess() noexcept;
	};

	/// @brief 标准的传递任务。
	///		重复进行如下流程：从物品源获取物品，如果有异常就通知监视器，没有异常就交给目的地，最后休息一段时间。
	///		直到监视器发出终止信号。
	template <
		IsItemSource TItemSource,
		IsItemDestination TItemDestination,
		IsDeliveryTaskMonitor TTaskMonitor,
		std::default_initializable TItem = typename TItemSource::ItemType>
		requires std::same_as<TItem, typename TItemDestination::ItemType>
	class DeliveryTask final {
		Credential<TItemSource> ItemSource{};
		Credential<TItemDestination> ItemDestination{};
		Credential<TTaskMonitor> Monitor{};
		IntervalSleeper Sleeper{};

		struct Configurations {
			struct ActorsType {
				Credential<TItemSource>& ItemSource;
				Credential<TItemDestination>& ItemDestination;
				Credential<TTaskMonitor>& Monitor;
			} Actors;

			struct OptionsType {
				std::chrono::milliseconds& MinInterval;
			} Options;
		};

	public:
		[[nodiscard]] Configurations Configure() noexcept {
			return {
				.Actors = {ItemSource, ItemDestination, Monitor},
				.Options = {Sleeper.Interval}
			};
		}

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(ItemSource, ItemDestination, Monitor); }

		void Execute() noexcept {
			auto [source_user, source_object] = Acquire(ItemSource);
			auto [destination_user, destination_object] = Acquire(ItemDestination);
			auto [monitor_user, monitor_object] = Acquire(Monitor);

			TItem item{};
			while (!monitor_object.IsDone()) {
				Sleeper.Sleep();
				if (!source_object.GetItem(item)) monitor_object.HandleItemSourceError();
				else {
					destination_object.SetItem(item);
					monitor_object.HandleItemSourceSuccess();
				}
			}
		}
	};
}
