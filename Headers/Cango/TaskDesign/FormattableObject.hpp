#pragma once

#include <sstream>
#include <span>

namespace Cango:: inline TaskDesign {
	/// @brief FormattableObject 的概念，要求实例提供 Format 函数格式化对象到流
	template <typename TObject>
	concept IsFormattableObject = requires(const TObject& obj, std::ostream& stream) {
		{ obj.Format(stream) } -> std::convertible_to<std::ostream&>;
	};

	/// @brief 代表可以格式化的对象，提供 Format 函数，将对象格式化输出到流
	///	@note 如果你的结构体对内存布局有要求，不要继承此结构体，使用 @c IsFormattableObject 概念会更好
	struct FormattableObject {
		/// @brief 格式化对象到流
		/// @param stream 输出的流
		/// @return 参数中的流，以便连续调用
		virtual std::ostream& Format(std::ostream& stream) const noexcept = 0;
	};

	/// @brief 调用对象的 @c Format 方法格式化对象到流
	std::ostream& Format(std::ostream& stream, const IsFormattableObject auto& obj) noexcept {
		return obj.Format(stream);
	}

	/// @brief 调用对象的 @c Format 方法格式化对象到流
	std::ostream& operator <<(std::ostream& stream, const IsFormattableObject auto& obj) noexcept {
		return Format(stream, obj);
	}

	/// @brief
	///		调用 @c std::span 内部元素的 @c FormattableObject::Format 方法格式化对象到流
	///		如果 @c objects 为空，则输出 "[]"
	///		如果只有一个元素，则输出 "[element]"
	///		其他则输出 "[element1, element2, ...]"
	template <IsFormattableObject TObject>
	std::ostream& Format(std::ostream& stream, const std::span<const TObject> objects) noexcept {
		const auto size = objects.size();
		if (size == 0) return stream << "[]";
		if (size == 1) return stream << '[' << objects[0] << ']';
		stream << '[';
		for (const auto& object : std::span<const TObject>{objects.begin(), size - 1})
			object.Format(stream) << ", ";
		return stream << objects.back() << ']';
	}

	/// @brief 简化 @c Format 的调用
	template <IsFormattableObject TObject>
	std::ostream& operator <<(std::ostream& stream, const std::span<const TObject> objects) noexcept {
		return Format(stream, objects);
	}

	/// @brief 调用对象的 @c Format 方法格式化对象到 @c std::ostringstream 并返回字符串
	std::string Format(const IsFormattableObject auto& obj) noexcept {
		std::ostringstream stream{};
		obj.Format(stream);
		return stream.str();
	}
}
