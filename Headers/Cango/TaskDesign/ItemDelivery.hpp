#pragma once

#include "TaskExecution.hpp"

namespace Cango :: inline TaskDesign {
	/// @brief 宿节点，表明此对象能够接收物品
	///	@tparam TItem 节点接收物品的类型，需要能够使用常量引用传递
	///	@note 接口的指针变量不拥有对象的所有权，故不负责销毁对象
	template <typename TItem>
	struct ItemDestination {
		using ItemType = TItem;

		/// @brief 设置物品，目的地将会接收物品并开始处理
		///	@param item 使用常量引用传递的物品
		virtual void SetItem(const TItem& item) noexcept = 0;
	};

	/// @brief @c ItemDestination 的概念
	template <typename TObject, typename TItem = typename TObject::ItemType>
	concept IsItemDestination = requires(TObject destination, const TItem& item) {
		{ destination.SetItem(item) };
	};

	template <typename TObject>
	concept IsFunctionalItemDestination = IsItemDestination<TObject> && IsFunctionalObject<TObject>;

	/// @brief 空目的地节点，不对接收的物品做任何操作
	template <typename TItem>
	struct EmptyItemDestination final : ItemDestination<TItem> {
		void SetItem(const TItem& item) noexcept override { (void)item; }
	};

	/// @brief 物品源节点，表明此对象可以提供物品
	template <typename TItem>
	struct ItemSource {
		using ItemType = TItem;

		/// @brief 从源中获取物品
		///	@param item 通过修改此参数将源中的物品呈递到外部
		///	@return 是否成功获取
		[[nodiscard]] virtual bool GetItem(TItem& item) noexcept = 0;
	};

	/// @brief 约束物品源
	/// 要求 @c TObject 类型中提供物品类型 @c TObject::ItemType 和 @c TObject::GetItem 函数
	template <typename TObject, typename TItem = typename TObject::ItemType>
	concept IsItemSource = requires(TObject& node, TItem& item) {
		{ node.GetItem(item) } -> std::convertible_to<bool>;
	};

	/// @brief 约束可判断是否能够工作的物品源
	template <typename TObject>
	concept IsFunctionalItemSource = IsItemSource<TObject> && IsFunctionalObject<TObject>;

	/// @brief 空的物品源节点，不提供物品，只用于测试
	template <typename TItem>
	struct EmptyItemSource final : ItemSource<TItem> {
		[[nodiscard]] bool GetItem(TItem& item) noexcept override { return false; }
	};

	/// @brief 简单的物品源节点，只提供内部设置的物品
	template <std::default_initializable TItem>
	struct SimpleItemSource final : ItemSource<TItem> {
		TItem Item{};

		[[nodiscard]] bool GetItem(TItem& item) noexcept override {
			item = Item;
			return true;
		}
	};

	template <
		IsFunctionalItemDestination TFunctionalConsumer,
		std::default_initializable TItem = typename TFunctionalConsumer::ItemType>
	class NonBlockFunctionalConsumer {
		struct ConsumerWithFlag {
			std::atomic_bool IsConsuming{false};
			TFunctionalConsumer Consumer{};
		};

		TFunctionalConsumer PrototypeConsumer{};
		std::list<ConsumerWithFlag> ConsumerList{};

		ConsumerWithFlag& AcquireIdleConsumer() noexcept {
			for (auto& consumer : ConsumerList) if (!consumer.IsConsuming) return consumer;
			return ConsumerList.emplace_back();
		}

	public:
		auto Configure() noexcept { return PrototypeConsumer.Configure(); }

		[[nodiscard]] bool IsFunctional() noexcept { return PrototypeConsumer.IsFunctional(); }

		void SetItem(const TItem& item) {
			auto& consumer_with_flag = AcquireIdleConsumer();
			consumer_with_flag.IsConsuming = true;
			consumer_with_flag.Consumer = PrototypeConsumer;
			std::thread{
				[item, &consumer_with_flag] {
					consumer_with_flag.Consumer.SetItem(item);
					consumer_with_flag.IsConsuming = false;
				}
			}.detach();
		}
	};
}
