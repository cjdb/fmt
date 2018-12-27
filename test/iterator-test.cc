// Formatting library for C++ - std::ostream support tests
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.

#include "fmt/iterator.h"

#include <algorithm>
#include "gtest-extra.h"
#include <numeric>
#include <sstream>
#include "util.h"

static_assert(__cplusplus > 201703L, "Test expects something C++20-compatible.");
// TODO(@cjdb): Uncomment when Standard iterator concepts become available.
// static_assert(std::OutputIterator<fmt::print_iterator<int>, int>);
// static_assert(not std::Writable<fmt::print_iterator<int>, short>);
// static_assert(not std::Writable<fmt::print_iterator<int>, long>);


TEST(PrintIteratorTest, DefaultConstructible) {
	auto i = fmt::print_iterator<int>{};
	EXPECT_EQ(i.format(), "{}");
	EXPECT_WRITE(stdout, *i = 0, "0");
}

TEST(PrintIteratorTest, FormatStringConstructible) {
	constexpr auto format_string = "${}";
	auto i = fmt::print_iterator<int>{format_string};
	EXPECT_EQ(i.format(), format_string);
	EXPECT_WRITE(stdout, *i = 100, "$100");
}

TEST(PrintIteratorTest, FILEConstructible) {
	constexpr auto format_string = "${:.2f}";
	auto i = fmt::print_iterator<double>{format_string, stderr};
	EXPECT_EQ(i.format(), format_string);
	EXPECT_WRITE(stderr, *i = 10.0, "$10.00");
}

TEST(PrintIteratorTest, OstreamConstructible) {
	auto const times = []{
		auto result = std::vector<int>(12);
		std::iota(std::begin(result), std::end(result), 1);
		return result;
	}();

	constexpr auto format_string = "{:02d}:00 UTC\n";
	auto formatted_times = std::ostringstream{};
	auto output = fmt::print_iterator<int>{format_string, formatted_times};
	EXPECT_EQ(output.format(), format_string);

	std::copy(std::begin(times), std::end(times), output);
	EXPECT_EQ(formatted_times.str(), "01:00 UTC\n"
												"02:00 UTC\n"
												"03:00 UTC\n"
												"04:00 UTC\n"
												"05:00 UTC\n"
												"06:00 UTC\n"
												"07:00 UTC\n"
												"08:00 UTC\n"
												"09:00 UTC\n"
												"10:00 UTC\n"
												"11:00 UTC\n"
												"12:00 UTC\n");
}
