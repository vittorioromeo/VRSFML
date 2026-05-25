#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/TstFwd.hpp"


////////////////////////////////////////////////////////////
// Test-case registration glue. Test cases are registered via
// file-scope static initializers; the actual storage lives in
// `Registry.cpp`.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
using TestFn = void (*)();


////////////////////////////////////////////////////////////
/// \brief Registers a test case, returns the index for decorator chaining.
[[nodiscard]] int registerTestCase(TestFn fn, const char* file, int line, const char* name = nullptr) noexcept;


////////////////////////////////////////////////////////////
/// \brief Registers a templated test case (different name suffix per type).
[[nodiscard]] int registerTemplatedTestCase(TestFn fn, const char* file, int line, const char* name, const char* typeStr) noexcept;


////////////////////////////////////////////////////////////
void setTestCaseName(int index, const char* name) noexcept;
void setTestCaseSkip(int index, bool skip) noexcept;


////////////////////////////////////////////////////////////
/// \brief Trivial chain-builder so `TEST_CASE("name" * skip(...))`
/// can compose decorators left-to-right.
////////////////////////////////////////////////////////////
struct TestCaseHandle
{
    int index;

    [[gnu::always_inline]] TestCaseHandle& operator*(const char* name) noexcept
    {
        setTestCaseName(index, name);
        return *this;
    }

    [[gnu::always_inline]] TestCaseHandle& operator*(const SkipDecorator& dec) noexcept;
};


////////////////////////////////////////////////////////////
struct SkipDecorator
{
    bool value;
};


////////////////////////////////////////////////////////////
inline TestCaseHandle& TestCaseHandle::operator*(const SkipDecorator& dec) noexcept
{
    setTestCaseSkip(index, dec.value);
    return *this;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline TestCaseHandle makeHandle(TestFn fn, const char* file, int line) noexcept
{
    return {registerTestCase(fn, file, line)};
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline TestCaseHandle makeTemplatedHandle(
    TestFn      fn,
    const char* file,
    int         line,
    const char* name,
    const char* typeStr) noexcept
{
    return {registerTemplatedTestCase(fn, file, line, name, typeStr)};
}


////////////////////////////////////////////////////////////
/// \brief Tag used so `[[maybe_unused]] static int = ...` registrations
/// can be discarded by the linker dead-code pass without warnings.
///
/// Variadic so the macro's trailing extra arguments (Catch2-style
/// `TEST_CASE("name", "[tag]")` where `[tag]` falls through as a
/// second swallow argument) are silently dropped.
////////////////////////////////////////////////////////////
template <typename... Ts>
[[gnu::always_inline]] inline int swallow(const TestCaseHandle&, const Ts&...) noexcept
{
    return 0;
}


} // namespace tst::detail


namespace tst
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::always_inline]] inline detail::SkipDecorator skip(bool value = true) noexcept
{
    return detail::SkipDecorator{value};
}
} // namespace tst
