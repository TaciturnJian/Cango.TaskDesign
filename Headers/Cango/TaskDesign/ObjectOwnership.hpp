#pragma once

#include <memory>
#include <concepts>
#include <ostream>

namespace Cango :: inline TaskDesign {
	template <typename TObject>
	concept IsValidatable = requires(const TObject& obj) {
		{ obj.IsValid() } -> std::convertible_to<bool>;
	};

	bool Validate(const IsValidatable auto&... objects) { return (objects.IsValid() && ...); }

	/// @brief 对象所有者，表明此实例管理对象的生命周期，并且其声明之处就是创建对象的地方
	///	也就是既负责配置资源对象，又确保对象存在
	///	可以通过 @c std::move 转移 @c ObjectOwner 所在的位置，不可复制
	template <typename T>
	class ObjectOwner;

	/// @brief 对象的使用者，表明此实例管理对象的生命周期，但其声明之处不是创建对象的地方
	///	也就是不负责配置资源对象，只负责确保使用时对象存在
	///	可以复制和转移 @c ObjectUser
	template <typename T>
	class ObjectUser;

	/// @brief 对象的凭据，此实例不管理对象的生命周期，使用 @c Credential::Acquire 函数获取对象
	///	也就是不负责配置资源对象，也不负责确保对象存在，但是可以通过凭据获取 @c ObjectUser<TObject>
	///	可以复制和转移 @c Credential
	template <typename T>
	class Credential;

	template <typename T>
	class Credential {
		std::weak_ptr<T> WeakPointer;

	public:
		using element_type = T;
		using ObjectType = element_type;
		using ItemType = element_type;

		/// @brief 创建一个空凭据
		Credential() noexcept = default;

		/// @brief 使用 @c std::weak_ptr<T> 构建凭据，有效性取决于传入的指针
		explicit Credential(std::weak_ptr<T> pointer) noexcept : WeakPointer(std::move(pointer)) {}

		/// @brief 判断凭据是否有效
		[[nodiscard]] bool IsValid() const noexcept { return !WeakPointer.expired(); }

		/// @brief 从凭据获取使用权
		bool Acquire(ObjectUser<T>& user) const noexcept;

		[[nodiscard]] ObjectUser<T> AcquireUser() const noexcept;
	};

	template <typename T>
	class ObjectUser {
		std::shared_ptr<T> Pointer;

	public:
		using element_type = T;
		using ObjectType = element_type;
		using ItemType = element_type;

		/// @brief 构造一个无效的使用者
		ObjectUser() = default;

		/// @brief 从 @c std::shared_ptr 构造一个使用者
		explicit ObjectUser(std::shared_ptr<T> other) noexcept : Pointer(std::move(other)) {}

		/// @brief 判断此使用者是否有效
		[[nodiscard]] bool IsValid() const noexcept { return Pointer != nullptr; }

		/// @brief 为凭据赋予当前使用者的有效性
		bool Authorize(Credential<T>& credential) const noexcept;

		/// @brief 从使用者获取具体对象
		///	@warning 如果使用者无效，此方法会导致错误的指针调用而引发段错误
		T& operator*() const noexcept { return *Pointer; }

		/// @brief 从使用者调用对象的成员
		///	@warning 如果使用者无效，此方法会导致错误的指针调用而引发段错误
		T* operator->() const noexcept { return Pointer.get(); }
	};

	template <typename T>
	class ObjectOwner {
		std::unique_ptr<std::shared_ptr<T>> Pointer;

	public:
		using element_type = T;
		using ObjectType = element_type;
		using ItemType = element_type;

		/// @brief 构造并占有对象
		explicit ObjectOwner(auto&&... args) noexcept : Pointer(
			std::make_unique<std::shared_ptr<T>>(
				std::move(
					std::make_shared<T>(args...)
				)
			)
		) {}

		/// @brief 如果对象不支持默认构造，则不占有对象，否则占有一个对象
		ObjectOwner() noexcept {
			if constexpr (std::default_initializable<T>) {
				Pointer = std::move(std::make_unique<std::shared_ptr<T>>(std::make_shared<T>()));
			}
		}

		/// @brief 移动所有权到此对象
		ObjectOwner(ObjectOwner&& other) noexcept : Pointer(std::move(other.Pointer)) {}

		/// @brief 移动所有权到此对象
		ObjectOwner& operator=(ObjectOwner&& other) noexcept {
			if (this != &other) { this->Pointer = std::move(other.Pointer); }
			return *this;
		}

		/// @brief 返回所有者是否有效
		[[nodiscard]] bool IsValid() const noexcept { return Pointer != nullptr; }

		/// @brief 获取使用者
		[[nodiscard]] ObjectUser<T> AcquireUser() const noexcept;

		bool Acquire(ObjectUser<T>& user) const noexcept;

		/// @brief 认证凭据，当所有者有效时能保证执行后，参数中的凭据有效
		bool Authorize(Credential<T>& credential) const noexcept;

		/// @brief 获取具体对象
		T& operator*() const noexcept { return **Pointer; }

		/// @brief 调用具体对象的成员
		T* operator->() const noexcept { return (*Pointer).get(); }
	};

	template <typename T>
	bool Credential<T>::Acquire(ObjectUser<T>& user) const noexcept {
		const auto pointer = WeakPointer.lock();
		if (pointer == nullptr) return false;
		user = std::move(ObjectUser<T>{pointer});
		return true;
	}

	template <typename T>
	ObjectUser<T> Credential<T>::AcquireUser() const noexcept { return ObjectUser<T>{WeakPointer.lock()}; }

	template <typename T>
	bool ObjectUser<T>::Authorize(Credential<T>& credential) const noexcept {
		if (!IsValid()) return false;
		credential = Credential<T>{std::weak_ptr<T>{Pointer}};
		return true;
	}

	template <typename T>
	ObjectUser<T> ObjectOwner<T>::AcquireUser() const noexcept { return ObjectUser<T>{*Pointer}; }

	template <typename T>
	bool ObjectOwner<T>::Acquire(ObjectUser<T>& user) const noexcept {
		if (!IsValid()) return false;
		user = ObjectUser<T>{*Pointer};
		return true;
	}

	template <typename T>
	bool ObjectOwner<T>::Authorize(Credential<T>& credential) const noexcept {
		if (!IsValid()) return false;
		credential = Credential<T>{std::weak_ptr<T>{*Pointer}};
		return true;
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const ObjectUser<T>& user) noexcept {
		return stream << (user.IsValid() ? *user : "nullptr");
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const Credential<T>& credential) noexcept {
		ObjectUser<T> user{};
		return stream << (credential.Acquire(user) ? *user : "nullptr");
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& stream, const ObjectOwner<T>& owner) noexcept {
		return stream << (owner.IsValid() ? *owner : "nullptr");
	}
}
