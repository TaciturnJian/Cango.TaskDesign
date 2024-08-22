#pragma once

#include <Cango/CommonUtils/CounterX.hpp>
#include <Cango/CommonUtils/IntervalSleeper.hpp>
#include <Cango/CommonUtils/ObjectOwnership.hpp>

#include "ItemDelivery.hpp"
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

	/// @brief 任务监视器的简单实现。
	///	默认行为是连续异常后终止任务，有正常后重置连续计数器，当提供了 NormalHandler 和 ExceptionHandler 后，将替代默认行为 
	struct EasyDeliveryTaskMonitor final {
		static void EmptyHandler() noexcept;

		std::function<void()> NormalHandler{};
		std::function<void()> ExceptionHandler{};
		std::atomic_bool Done{false};
		Counter16 Counter{0, 10};

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
		typename TItem = typename TItemSource::ItemType>
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
			ObjectUser<TItemSource> source_user = ItemSource.lock();
			ObjectUser<TItemDestination> destination_user = ItemDestination.lock();
			ObjectUser<TTaskMonitor> monitor_user = Monitor.lock();

			// 检查指针状态
			if (!source_user || !destination_user || !monitor_user) return;

			auto& source_object = *source_user;
			auto& destination_object = *destination_user;
			auto& monitor_object = *monitor_user;

			TItem item{};
			while (!monitor_object.IsDone()) {
				Sleeper.Sleep();
				if (source_object.GetItem(item)) {
					monitor_object.HandleItemSourceSuccess();
					destination_object.SetItem(item);
				}
				else monitor_object.HandleItemSourceError();
			}
		}

		void operator()() noexcept {
			Execute();
		}
	};
}
