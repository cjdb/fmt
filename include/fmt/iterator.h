// Formatting library for C++ - the core API
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.
//
// Copyright (c) 2018 - present, Remotion (Igor Schulz)
// All Rights Reserved
// {fmt} support for ranges, containers and types tuple interface.

#ifndef FMT_ITERATOR_H_
#define FMT_ITERATOR_H_

#if __cplusplus <= 201703L
	#error "{fmt} iterators are only supported in post-C++17 code."
#endif // __cplusplus < C++20

#include "fmt/format.h"
#include "fmt/ostream.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <iterator>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

FMT_BEGIN_NAMESPACE

namespace detail {
	// Detail base class to take advantage of the Empty Base class Optimisation (EBO). All iterator
	// types should privately inherit from this type and expose the public members.
	//
	// \tparam I The type that is derived from format_base_iterator. We use I to take advantage of
	//           the Curiously
	//
	template<class I, class T, class CharT>
	class format_iterator_base {
	public:
		using difference_type = std::intmax_t;
		using iterator_category = std::output_iterator_tag;

		/// \brief Constructs an object of type format_iterator_base.
		///
		/// By default, a format_iterator_base uses the output "{}". It is therefore safe to use a
		/// default-constructed format_iterator_base.
		///
		format_iterator_base() = default;

		/// \brief Constructs an object of type format_iterator_base.
		/// \param s A {fmt}-compatible format string. Must contain exactly one non-escaped "{}".
		///
		explicit format_iterator_base(std::basic_string_view<CharT> s) noexcept
			: format_{s}
		{}

		/// \brief A no-op, required only so that format_iterator_base models concept Iterator.
		/// \returns *this
		///
		I& operator*() noexcept
		{ return no_op(*this); }

		/// \brief A no-op, required only so that format_iterator_base models concept Iterator.
		/// \returns *this
		///
		I& operator++() noexcept
		{ return no_op(*this); }

		/// \brief A no-op, required only so that format_iterator_base models concept Iterator.
		/// \returns *this
		///
		I& operator++(int) noexcept
		{ return no_op(*this); }

		/// \returns The format string that the format_iterator_base is constructed with.
		///
		std::basic_string_view<CharT> format() const noexcept
		{ return format_; }
	private:
		std::basic_string_view<CharT> format_ = "{}";
		// Helps document that something is intentionally a no-op.
		//
		static I& no_op(format_iterator_base& self) noexcept
		{ return static_cast<I&>(self); }
	};

	template<class... Ts>
	struct overloaded : Ts... {
		using Ts::operator()...;
	};

	template<class... Ts>
	overloaded(Ts&&...) -> overloaded<Ts...>;
} // namespace detail

template<class T, class CharT = char, class Traits = std::char_traits<CharT>>
// requires Formattable<T>
class print_iterator
: private detail::format_iterator_base<print_iterator<T, CharT, Traits>, T, CharT> {
	using base = detail::format_iterator_base<print_iterator<T, CharT, Traits>, T, CharT>;
	friend base;
public:
	using base::difference_type;
	using base::iterator_category;

	using base::base;
	using base::operator*;
	using base::operator++;
	using base::format;

	explicit print_iterator(std::string_view s, FILE* file)
		: base{s}
		, out_{file}
	{}

	explicit print_iterator(std::string_view s, std::basic_ostream<CharT, Traits>& file)
		: base{s}
		, out_{std::addressof(file)}
	{}

	print_iterator(print_iterator&&) = default;
	print_iterator(print_iterator const&) = default;

	~print_iterator() = default;

	print_iterator& operator=(print_iterator&&) = default;
	print_iterator& operator=(print_iterator const&) = default;

	/// \brief Writes a value to file using the prescribed format string.
	///
	/// Let `out` be the `FILE*` or ostream object that the print_iterator was constructed
	/// with, and let `format` be the string_view object that the print_iterator was constructed
	/// with. Then, this operation is equivalent to `fmt::print(out, format, value)`.
	///
	/// \param value The value to be written to file.
	/// \note This operator only participates in overload resolution if U is the same type as T
	///       (ignoring cv and reference qualifiers).
	/// \returns *this
	///
	template<class U = T>
	auto operator=(U const& value)
		-> std::enable_if_t<std::is_same_v<
				std::remove_cv_t<std::remove_reference_t<T>>,
				std::remove_cv_t<std::remove_reference_t<U>>>, print_iterator&>
	{
		std::visit(detail::overloaded{
			[this, &value](FILE* file){ ::fmt::print(file, format(), value); },
			[this, &value](std::basic_ostream<CharT, Traits>* os){ ::fmt::print(*os, format(), value); },
		}, out_);
		
		return *this;
	}
private:
	std::variant<FILE*, std::basic_ostream<CharT, Traits>*> out_ = stdout;
};

FMT_END_NAMESPACE

namespace std {
	template<class T, class CharT>
	struct iterator_traits<fmt::print_iterator<T, CharT>> {
		using value_type = void;
		using difference_type = typename fmt::print_iterator<T, CharT>::difference_type;
		using pointer = void;
		using reference = void;
		using iterator_category = typename fmt::print_iterator<T, CharT>::iterator_category;
	};
} // namespace std

#endif // FMT_ITERATOR_H_