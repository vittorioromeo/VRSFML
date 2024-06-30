#pragma once

#include <doctest/parts/doctest_fwd.h>


#define SECTION(name) DOCTEST_SUBCASE(name)

#undef TEST_CASE
#define TEST_CASE(name, ...) DOCTEST_TEST_CASE(name __VA_ARGS__)

#define STATIC_CHECK(...)       static_assert(__VA_ARGS__)
#define STATIC_CHECK_FALSE(...) static_assert((__VA_ARGS__) == false)

#define STATIC_REQUIRE(...)       static_assert(__VA_ARGS__)
#define STATIC_REQUIRE_FALSE(...) static_assert((__VA_ARGS__) == false)

#define TEMPLATE_TEST_CASE(name, tag, ...) DOCTEST_TEST_CASE_TEMPLATE(name, TestType, __VA_ARGS__)

#define SUCCEED(...) MESSAGE((std::string{} + __VA_ARGS__))

#define CHECK_THAT(value, ...) CHECK(value == (__VA_ARGS__))

#define CHECKED_IF if

namespace doctest::Matchers
{
struct WithinRel
{
    double target;
    double epsilon;
};

struct WithinAbs
{
    double target;
    double epsilon;
};

bool operator==(float value, const WithinRel& withinRel);
bool operator==(float value, const WithinAbs& withinAbs);

} // namespace doctest::Matchers

namespace Catch
{

struct Approx : doctest::Approx
{
    using doctest::Approx::Approx;
    Approx& margin(double newMargin);
};

// NOLINTNEXTLINE(misc-unused-alias-decls)
namespace Matchers = doctest::Matchers;

} // namespace Catch


namespace doctest
{

template <>
struct StringMaker<char32_t>
{
    static doctest::String convert(const char32_t&);
};

template <>
struct StringMaker<Matchers::WithinRel>
{
    static doctest::String convert(const Matchers::WithinRel&);
};

template <>
struct StringMaker<Matchers::WithinAbs>
{
    static doctest::String convert(const Matchers::WithinAbs&);
};

template <>
struct StringMaker<Catch::Approx>
{
    static doctest::String convert(const Catch::Approx&);
};

} // namespace doctest
