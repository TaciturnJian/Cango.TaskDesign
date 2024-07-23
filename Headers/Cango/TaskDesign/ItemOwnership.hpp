#pragma once

#include <memory>
#include <ostream>

namespace Cango :: inline TaskDesign {
	/// @brief 对象所有者，表明此实例管理对象的生命周期，并且其声明之处就是创建对象的地方
	///	也就是既负责配置资源对象，又确保对象存在
	template <typename TObject>
	using ObjectOwner = std::shared_ptr<TObject>;

	/// @brief 对象的使用者，表明此实例管理对象的生命周期，但其声明之处不是创建对象的地方
	///	也就是不负责配置资源对象，只负责确保使用时对象存在
	template <typename TObject>
	using ObjectUser = std::shared_ptr<TObject>;

	/// @brief 对象的凭据，此实例不管理对象的生命周期，使用 @c Acquire 函数获取对象
	///	也就是不负责配置资源对象，也不负责确保对象存在，但是可以通过凭据获取 @c ObjectUser<TObject>
	template <typename TObject>
	using Credential = std::weak_ptr<TObject>;

	/// @brief 包含一个使用者和对象本身，在上下文中，使用者维持对象的生命周期，对象发挥作用，配合 @c Acquire 函数使用
	template <typename TObject>
	struct UserAndObject {
		ObjectUser<TObject> User;
		TObject& Object;
	};

	/// @brief 通过凭据获取对象的使用权
	///	@param credential 对象的凭据
	///	@param user 函数返回的对象使用者，指向对象的同时维持对象的生命周期
	///	@return 是否成功获取对象的使用权
	template <typename T>
	[[nodiscard]] bool Acquire(const Credential<T>& credential, ObjectUser<T>& user) noexcept {
		user = credential.lock();
		return user != nullptr;
	}

	/// @brief
	///		通过凭据获取对象的使用权和对象（使用指针访问内存）
	///		使用结构化绑定简化函数调用：
	///		@code
	///	auto [object_user, object] = Acquire(credential);
	///		@endcode
	///		在这个上下文中，object_user 存在，那么 object 对象就能保证不会被 std::shared_ptr 回收
	///	@param credential 对象的凭据
	///	@return 对象的使用者和对象，获取对象的同时维持生命周期
	///	@warning 此函数不检查凭据是否有效，当无法获取的对象时会导致段错误
	template <typename TObject>
	[[nodiscard]] UserAndObject<TObject> Acquire(const Credential<TObject>& credential) noexcept {
		auto user = credential.lock();
		return {user, *user};
	}

	/// @brief 在所有凭据中寻找是否有失效的
	///	@tparam TWeakPtrList 一系列 @c std::weak_ptr 或者 @c Credential
	template <typename... TWeakPtrList>
	bool FindAnyExpired(const TWeakPtrList&... list) { return (list.expired() || ...); }

	/// @brief 验证所有凭据是否都有效
	///	@tparam TWeakPtrList 一系列 @c std::weak_ptr 或者 @c Credential
	template <typename... TWeakPtrList>
	bool ValidateAll(const TWeakPtrList&... list) { return !FindAnyExpired(list...); }

	/// @brief 如果凭据有效则使用 @c std::ostream 格式化对象，否则向流中传入 @c "nullptr"
	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const Credential<T>& credential) {
		if (const auto pointer = credential.lock()) return stream << *pointer;
		return stream << "nullptr";
	}
}
