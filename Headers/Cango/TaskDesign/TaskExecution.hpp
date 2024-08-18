#pragma once

#include <concepts>
#include <functional>
#include <list>
#include <thread>
#include <vector>

namespace Cango :: inline TaskDesign {
	template <typename TObject>
	concept IsFunctionalObject = requires(TObject& object) {
		{ object.IsFunctional() } -> std::convertible_to<bool>;
	};

	struct FunctionalObject {
		[[nodiscard]] virtual bool IsFunctional() const noexcept = 0;
	};

	template <typename TObject>
	concept IsExecutableTask = IsFunctionalObject<TObject> && requires(TObject& object) {
		{ object.Execute() };
	};

	/// @brief 代表可以被执行的任务
	struct ExecutableTask {
		/// @brief 检查是否可以执行任务
		virtual bool IsFunctional() noexcept = 0;

		/// @brief 执行任务
		virtual void Execute() noexcept = 0;
	};

	template <typename TSignal>
	concept IsDoneSignal = requires(const TSignal& readonlyObject, TSignal& mutableObject) {
		{ readonlyObject.IsDone() } -> std::convertible_to<bool>;
		{ mutableObject.Interrupt() };
		{ mutableObject.Reset() };
	};

	/// @brief 代表任务是否应该从任务中退出的信号
	struct DoneSignal {
		/// @brief 获取任务是否应该从状态中退出
		[[nodiscard]] virtual bool IsDone() const noexcept = 0;

		/// @brief 发送中断信号，通知任务应该退出
		virtual void Interrupt() noexcept = 0;

		/// @brief 重置任务状态，取消中断信号
		virtual void Reset() noexcept = 0;
	};

	using ThreadList = std::list<std::thread>;

	inline ThreadList& operator<<(ThreadList& group, std::thread&& thread) noexcept {
		group.push_back(std::move(thread));
		return group;
	}

	inline ThreadList& operator<<(ThreadList& group, const std::function<void()>& function) noexcept {
		group.emplace_back(function);
		return group;
	}

	ThreadList& operator<<(ThreadList& group, IsExecutableTask auto& task) noexcept {
		return group << [&task] { task.Execute(); };
	}

	using ThreadVector = std::vector<std::thread>;

	inline ThreadVector& operator<<(ThreadVector& group, std::thread&& thread) noexcept {
		group.push_back(std::move(thread));
		return group;
	}

	inline ThreadVector& operator<<(ThreadVector& group, const std::function<void()>& function) noexcept {
		group.emplace_back(function);
		return group;
	}

	ThreadVector& operator<<(ThreadVector& group, IsExecutableTask auto& task) noexcept {
		return group << [&task] { task.Execute(); };
	}

	void JoinThreads(auto& threads) { for (auto& thread : threads) thread.join(); }
}
