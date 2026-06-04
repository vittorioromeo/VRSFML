#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Approx.hpp"                // IWYU pragma: export
#include "Tst/Detail/Decompose.hpp"      // IWYU pragma: export
#include "Tst/Detail/Registry.hpp"       // IWYU pragma: export
#include "Tst/Detail/StringifyValue.hpp" // IWYU pragma: export
#include "Tst/Detail/Subcase.hpp"        // IWYU pragma: export
#include "Tst/TstFwd.hpp"                // IWYU pragma: export

#include "ZancleBase/Builtin/Pragma.hpp"
#include "ZancleBase/SizeT.hpp"


namespace tst::detail
{
////////////////////////////////////////////////////////////
// Messages
////////////////////////////////////////////////////////////
void emitInfoPushRaw(const char* file, int line, const char* data, zb::SizeT size) noexcept;
void emitInfoPop() noexcept;
void emitMessageRaw(AssertKind kind, const char* file, int line, const char* data, zb::SizeT size);


////////////////////////////////////////////////////////////
/// \brief Stream-chain accumulator for `INFO`/`MESSAGE`/`CAPTURE`.
///
/// Uses a fixed 1 KiB stack buffer instead of a `zb::String`,
/// keeping this header free of `<ZancleBase/String.hpp>` -- a measurable
/// per-TU win across the ~130 test files that include `Tst.hpp`.
/// Messages longer than 1 KiB are silently truncated; in practice INFO
/// payloads are short (file paths, ints, a few words).
///
/// The macro wraps the user expression as `Builder{} * <expr>`; `*` has
/// higher precedence than `<<`, so the first sub-expression anchors a
/// `Builder&` left-fold that `<<` then continues. Non-template
/// overloads cover every type used by the test suite (char*,
/// `StringView`, all integer types, `bool`, `float`, `double`); bodies
/// live in `Messages.cpp` so no per-TU template instantiation happens.
////////////////////////////////////////////////////////////
struct MessageBuilder // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    static constexpr zb::SizeT capacity = 1024u;

    char      buf[capacity]; // intentionally uninitialized -- avoid zeroing 1 KiB on every CHECK
    zb::SizeT len = 0u;

    void appendRaw(const char* data, zb::SizeT n) noexcept;

#define ZA_TST_MSGB_DECL_PAIR(T)                 \
    MessageBuilder&  operator*(T v) & noexcept;  \
    MessageBuilder&  operator<<(T v) & noexcept; \
    MessageBuilder&& operator*(T v) && noexcept; \
    MessageBuilder&& operator<<(T v) && noexcept

    ZA_TST_MSGB_DECL_PAIR(const char*);
    ZA_TST_MSGB_DECL_PAIR(zb::StringView);
    ZA_TST_MSGB_DECL_PAIR(bool);
    ZA_TST_MSGB_DECL_PAIR(char);
    ZA_TST_MSGB_DECL_PAIR(short);
    ZA_TST_MSGB_DECL_PAIR(unsigned short);
    ZA_TST_MSGB_DECL_PAIR(int);
    ZA_TST_MSGB_DECL_PAIR(unsigned int);
    ZA_TST_MSGB_DECL_PAIR(long);
    ZA_TST_MSGB_DECL_PAIR(unsigned long);
    ZA_TST_MSGB_DECL_PAIR(long long);
    ZA_TST_MSGB_DECL_PAIR(unsigned long long);
    ZA_TST_MSGB_DECL_PAIR(float);
    ZA_TST_MSGB_DECL_PAIR(double);

#undef ZA_TST_MSGB_DECL_PAIR
};


////////////////////////////////////////////////////////////
// Lets `MESSAGE("a", b)` work the same as `MESSAGE("a" << b)` -- the
// comma at the top of the parenthesized macro expression is parsed as
// the comma operator. We provide a non-template `operator,` that
// works on the rvalue builder + a generic right-hand-side via the
// builder's existing overloads. Defined out-of-line in `Messages.cpp`.
////////////////////////////////////////////////////////////
#define ZA_TST_MSGB_COMMA(T) MessageBuilder&& operator,(MessageBuilder&& mb, T v) noexcept

ZA_TST_MSGB_COMMA(const char*);
ZA_TST_MSGB_COMMA(zb::StringView);
ZA_TST_MSGB_COMMA(bool);
ZA_TST_MSGB_COMMA(char);
ZA_TST_MSGB_COMMA(short);
ZA_TST_MSGB_COMMA(unsigned short);
ZA_TST_MSGB_COMMA(int);
ZA_TST_MSGB_COMMA(unsigned int);
ZA_TST_MSGB_COMMA(long);
ZA_TST_MSGB_COMMA(unsigned long);
ZA_TST_MSGB_COMMA(long long);
ZA_TST_MSGB_COMMA(unsigned long long);
ZA_TST_MSGB_COMMA(float);
ZA_TST_MSGB_COMMA(double);

#undef ZA_TST_MSGB_COMMA


////////////////////////////////////////////////////////////
class ScopedMessage
{
public:
    [[gnu::always_inline]] ScopedMessage(const char* file, int line, MessageBuilder&& mb) noexcept
    {
        emitInfoPushRaw(file, line, mb.buf, mb.len);
    }

    [[gnu::always_inline]] ~ScopedMessage()
    {
        emitInfoPop();
    }

    ScopedMessage(const ScopedMessage&)            = delete;
    ScopedMessage(ScopedMessage&&)                 = delete;
    ScopedMessage& operator=(const ScopedMessage&) = delete;
    ScopedMessage& operator=(ScopedMessage&&)      = delete;
};


////////////////////////////////////////////////////////////
[[gnu::always_inline]] inline void messageImplFromBuilder(AssertKind kind, const char* file, int line, MessageBuilder&& mb)
{
    emitMessageRaw(kind, file, line, mb.buf, mb.len);
}


////////////////////////////////////////////////////////////
// Try-catch wrappers for CHECK_NOTHROW.
////////////////////////////////////////////////////////////
bool checkNothrowImpl(const char* file, int line, const char* exprStr) noexcept;
void noteNothrowSuccess() noexcept;

} // namespace tst::detail


////////////////////////////////////////////////////////////
// Implementation macro helpers
////////////////////////////////////////////////////////////
#define ZA_TST_CAT_INNER(a, b) a##b
#define ZA_TST_CAT(a, b)       ZA_TST_CAT_INNER(a, b)
// Use `__COUNTER__` (not `__LINE__`) so identifiers stay unique under
// unity builds, where multiple test files are concatenated into one TU
// and would otherwise clash on identical line numbers.
#define ZA_TST_ANON(prefix) ZA_TST_CAT(prefix, __COUNTER__)


////////////////////////////////////////////////////////////
// TEST_CASE / SUBCASE
////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
// `decorators` is everything passed to `TEST_CASE(...)`. The chain
// `handle * "name" * skip(...)` builds left-to-right; the macro
// intentionally does NOT parenthesize so the user can chain Catch2-style
// `TEST_CASE("name", "[tag]")` -- the second comma-separated arg
// reaches `operator*` only as a trailing comma operand. We catch that
// in `TestCaseHandle::operator*(const char*)` (it just sets the name);
// any subsequent comma-separated args are dropped by the discarded
// comma-operator expression.
#define ZA_TST_TEST_CASE_IMPL(func, ...)                                                            \
    /* NOLINTNEXTLINE(misc-use-anonymous-namespace) */                                              \
    static void func();                                                                             \
    namespace                                                                                       \
    {                                                                                               \
    [[maybe_unused]] static const int ZA_TST_ANON(zancleTstReg_) =                                  \
        ::tst::detail::swallow(::tst::detail::makeHandle(&func, __FILE__, __LINE__) * __VA_ARGS__); \
    }                                                                                               \
    /* NOLINTNEXTLINE(misc-use-anonymous-namespace) */                                              \
    static void func()
// NOLINTEND(bugprone-macro-parentheses)

#define TEST_CASE(...) ZA_TST_TEST_CASE_IMPL(ZA_TST_ANON(zancleTstFunc_), __VA_ARGS__)

// NOLINTBEGIN(bugprone-macro-parentheses)
#define ZA_TST_SUBCASE_IMPL(var, name)                                         \
    if (const ::tst::detail::SubcaseScope var(name, __FILE__, __LINE__); !var) \
    {                                                                          \
    }                                                                          \
    else
// NOLINTEND(bugprone-macro-parentheses)

#define SUBCASE(name) ZA_TST_SUBCASE_IMPL(ZA_TST_ANON(zancleTstSubcase_), name)
#define SECTION(name) SUBCASE(name)


////////////////////////////////////////////////////////////
// TEMPLATE_TEST_CASE
////////////////////////////////////////////////////////////
namespace tst::detail
{
template <typename T>
[[nodiscard]] inline const char* typeNameOf() noexcept
{
    return "<?>";
}
} // namespace tst::detail

#define ZA_TST_DECLARE_TYPE_NAME(type, str)                      \
    namespace tst::detail                                        \
    {                                                            \
    template <>                                                  \
    [[nodiscard]] inline const char* typeNameOf<type>() noexcept \
    {                                                            \
        return str;                                              \
    }                                                            \
    }


ZA_TST_DECLARE_TYPE_NAME(int, "int")
ZA_TST_DECLARE_TYPE_NAME(unsigned int, "unsigned int")
ZA_TST_DECLARE_TYPE_NAME(long, "long")
ZA_TST_DECLARE_TYPE_NAME(unsigned long, "unsigned long")
ZA_TST_DECLARE_TYPE_NAME(long long, "long long")
ZA_TST_DECLARE_TYPE_NAME(unsigned long long, "unsigned long long")
ZA_TST_DECLARE_TYPE_NAME(float, "float")
ZA_TST_DECLARE_TYPE_NAME(double, "double")
ZA_TST_DECLARE_TYPE_NAME(bool, "bool")
ZA_TST_DECLARE_TYPE_NAME(char, "char")


// NOLINTBEGIN(bugprone-macro-parentheses)
#define ZA_TST_TEMPLATE_TEST_CASE_IMPL(func, regfunc, name, T, ...)                                                      \
    template <typename T>                                                                                                \
    static void func();                                                                                                  \
    namespace                                                                                                            \
    {                                                                                                                    \
    template <typename... Ts>                                                                                            \
    [[gnu::always_inline]] inline int regfunc(const char* file, int line, const char* tcName) noexcept                   \
    {                                                                                                                    \
        ((void)::tst::detail::makeTemplatedHandle(&func<Ts>, file, line, tcName, ::tst::detail::typeNameOf<Ts>()), ...); \
        return 0;                                                                                                        \
    }                                                                                                                    \
    [[maybe_unused]] static const int ZA_TST_ANON(zancleTstTplReg_) = regfunc<__VA_ARGS__>(__FILE__, __LINE__, name);    \
    }                                                                                                                    \
    template <typename T>                                                                                                \
    static void func()
// NOLINTEND(bugprone-macro-parentheses)


#define TEMPLATE_TEST_CASE(name, tag, ...) \
    ZA_TST_TEMPLATE_TEST_CASE_IMPL(ZA_TST_ANON(zancleTstTplFunc_), ZA_TST_ANON(zancleTstTplRegFn_), name, TestType, __VA_ARGS__)


////////////////////////////////////////////////////////////
// CHECK / REQUIRE family
////////////////////////////////////////////////////////////
// NOLINTBEGIN(bugprone-macro-parentheses)
#define ZA_TST_ASSERT_IMPL(kind, ...)                                                                                          \
    do                                                                                                                         \
    {                                                                                                                          \
        ZB_PRAGMA(GCC diagnostic push);                                                                                        \
        /* Suppress the unknown-pragma warning before mentioning Clang-only options below. */                                  \
        ZB_PRAGMA(GCC diagnostic ignored "-Wpragmas");                                                                         \
        ZB_PRAGMA(GCC diagnostic ignored "-Wunknown-warning-option");                                                          \
        ZB_PRAGMA(GCC diagnostic ignored "-Woverloaded-shift-op-parentheses");                                                 \
        ZB_PRAGMA(GCC diagnostic ignored "-Wsign-compare");                                                                    \
        ZB_PRAGMA(GCC diagnostic ignored "-Wsign-conversion");                                                                 \
                                                                                                                               \
        const ::tst::detail::Result zancleTstRes = ::tst::detail::ExpressionDecomposer(::tst::detail::AssertKind::kind)        \
                                                   << __VA_ARGS__;                                                             \
                                                                                                                               \
        (void)::tst::detail::handleAssertion(zancleTstRes, ::tst::detail::AssertKind::kind, __FILE__, __LINE__, #__VA_ARGS__); \
                                                                                                                               \
        ZB_PRAGMA(GCC diagnostic pop);                                                                                         \
    } while (false)
// NOLINTEND(bugprone-macro-parentheses)


#define CHECK(...)         ZA_TST_ASSERT_IMPL(Check, __VA_ARGS__)
#define CHECK_FALSE(...)   ZA_TST_ASSERT_IMPL(CheckFalse, __VA_ARGS__)
#define REQUIRE(...)       ZA_TST_ASSERT_IMPL(Require, __VA_ARGS__)
#define REQUIRE_FALSE(...) ZA_TST_ASSERT_IMPL(RequireFalse, __VA_ARGS__)
#define WARN(...)          ZA_TST_ASSERT_IMPL(Warn, __VA_ARGS__)

#define CHECK_UNARY(...)       CHECK(__VA_ARGS__)
#define CHECK_UNARY_FALSE(...) CHECK_FALSE(__VA_ARGS__)
#define REQUIRE_UNARY(...)     REQUIRE(__VA_ARGS__)


////////////////////////////////////////////////////////////
// CHECK_THAT (matcher-style; we route through `operator==(value, matcher)`)
////////////////////////////////////////////////////////////
#define CHECK_THAT(value, ...) CHECK((value) == (__VA_ARGS__))


////////////////////////////////////////////////////////////
// CHECK_NOTHROW
////////////////////////////////////////////////////////////
#define CHECK_NOTHROW(...)                                                           \
    do                                                                               \
    {                                                                                \
        bool zancleTstNothrowOk = true;                                              \
        try                                                                          \
        {                                                                            \
            (void)(__VA_ARGS__);                                                     \
        } catch (...) /* NOLINT(bugprone-empty-catch) */                             \
        {                                                                            \
            zancleTstNothrowOk = false;                                              \
        }                                                                            \
        if (zancleTstNothrowOk)                                                      \
            ::tst::detail::noteNothrowSuccess();                                     \
        else                                                                         \
            (void)::tst::detail::checkNothrowImpl(__FILE__, __LINE__, #__VA_ARGS__); \
    } while (false)


////////////////////////////////////////////////////////////
// STATIC_CHECK family -- compile-time asserts
////////////////////////////////////////////////////////////
#define STATIC_CHECK(...)       static_assert(__VA_ARGS__)
#define STATIC_CHECK_FALSE(...) static_assert((__VA_ARGS__) == false)


////////////////////////////////////////////////////////////
// INFO / CAPTURE / MESSAGE / FAIL
//
// The `Builder{} * __VA_ARGS__` trick lets users write either
// `INFO("text")`, `INFO("text" << v)`, or `INFO("text" << v << w)`.
// `*` has higher precedence than `<<`, so the chain folds left
// starting at `Builder{} * <first sub-expr>`.
////////////////////////////////////////////////////////////
#define INFO(...)                                   \
    const ::tst::detail::ScopedMessage ZA_TST_ANON( \
        zancleTstInfo_)(__FILE__, __LINE__, (::tst::detail::MessageBuilder{} * __VA_ARGS__))

#define CAPTURE(x) INFO(#x " := " << (x))

#define MESSAGE(...)                                                       \
    ::tst::detail::messageImplFromBuilder(::tst::detail::AssertKind::Warn, \
                                          __FILE__,                        \
                                          __LINE__,                        \
                                          (::tst::detail::MessageBuilder{} * __VA_ARGS__))

#define FAIL_CHECK(...)                                                     \
    ::tst::detail::messageImplFromBuilder(::tst::detail::AssertKind::Check, \
                                          __FILE__,                         \
                                          __LINE__,                         \
                                          (::tst::detail::MessageBuilder{} * __VA_ARGS__))

#define FAIL(...)                                                             \
    ::tst::detail::messageImplFromBuilder(::tst::detail::AssertKind::Require, \
                                          __FILE__,                           \
                                          __LINE__,                           \
                                          (::tst::detail::MessageBuilder{} * __VA_ARGS__))


////////////////////////////////////////////////////////////
// Runner entry point. Returns the conventional exit code (0 on success).
////////////////////////////////////////////////////////////
namespace tst
{
[[nodiscard]] int run(int argc, char** argv);
} // namespace tst


////////////////////////////////////////////////////////////
/// \file
/// Bespoke testing library -- minimal, header-light, no standard-library
/// dependencies in the public surface. The macros mirror doctest/Catch2.
///
/// The expression decomposition machinery uses the classic
/// `Decomposer << expr` trick; everything heavy compiles once in
/// `Tst/*.cpp`.
////////////////////////////////////////////////////////////
