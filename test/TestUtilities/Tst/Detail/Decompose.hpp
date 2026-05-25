#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/TstFwd.hpp"

#include "SFML/Base/Builtin/Pragma.hpp"
#include "SFML/Base/Trait/IsArray.hpp"


namespace tst::detail
{
////////////////////////////////////////////////////////////
struct Result
{
    bool passed;

    // Implicit converting ctor so the SFINAE-fallback path (where the
    // user's `lhs op rhs` returns plain `bool` after our templated
    // operator drops out) still feeds into `const Result =` cleanly.
    [[gnu::always_inline]] constexpr /*implicit*/ Result(bool p) noexcept :
        passed{p} // NOLINT(google-explicit-constructor)
    {
    }
};


////////////////////////////////////////////////////////////
template <typename L>
struct ExpressionLhs
{
    const L&   lhs;
    AssertKind kind;

    [[gnu::always_inline]] ExpressionLhs(const L& in, AssertKind k) noexcept : lhs{in}, kind{k}
    {
    }


    // SFINAE-guarded so the operator drops out of overload resolution when
    // `lhs op rhs` is ill-formed or ambiguous in the calling context.
    // Matches doctest's `SFINAE_OP` trick: when the templated member
    // operator is removed, the compiler falls through to the implicit
    // conversion `operator L()` below.

// NOLINTBEGIN(bugprone-macro-parentheses)
// (clang-format would otherwise concatenate adjacent string literals
// inside `_Pragma(...)` arguments and split them across lines, which
// produces ill-formed code -- `_Pragma` requires a single string literal
// token, not a concatenation expression.)
#define SFML_TST_DECL_CMP_OP(op)                                                                         \
                                                                                                         \
    SFML_BASE_PRAGMA(GCC diagnostic push);                                                               \
    /* Suppress the unknown-pragma warning before mentioning Clang-only options below. */                \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wpragmas");                                                \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wunknown-warning-option");                                 \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wsign-compare");                                           \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wsign-conversion");                                        \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wimplicit-int-float-conversion");                          \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wdouble-promotion");                                       \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wfloat-equal");                                            \
    SFML_BASE_PRAGMA(GCC diagnostic ignored "-Wconversion");                                             \
                                                                                                         \
    template <typename R>                                                                                \
    [[gnu::always_inline]] auto operator op(const R& rhs)                                                \
        const->decltype((void)(static_cast<const L&>(lhs) op static_cast<const R&>(rhs)), Result{false}) \
    {                                                                                                    \
        bool res = static_cast<bool>(lhs op rhs);                                                        \
        if (kind == AssertKind::CheckFalse || kind == AssertKind::RequireFalse)                          \
            res = !res;                                                                                  \
        return Result{res};                                                                              \
    }                                                                                                    \
                                                                                                         \
    SFML_BASE_PRAGMA(GCC diagnostic pop);

    SFML_TST_DECL_CMP_OP(==)
    SFML_TST_DECL_CMP_OP(!=)
    SFML_TST_DECL_CMP_OP(>)
    SFML_TST_DECL_CMP_OP(<)
    SFML_TST_DECL_CMP_OP(>=)
    SFML_TST_DECL_CMP_OP(<=)

#undef SFML_TST_DECL_CMP_OP
    // NOLINTEND(bugprone-macro-parentheses)


    // Conversion to Result for unary expressions like CHECK(b)
    [[gnu::always_inline]] operator Result() const // NOLINT(google-explicit-constructor)
    {
        bool res = static_cast<bool>(lhs);
        if (kind == AssertKind::CheckFalse || kind == AssertKind::RequireFalse)
            res = !res;
        return Result{res};
    }


    // Fallback implicit conversion to L. Lets the compiler retry the
    // ambient `lhs op rhs` overload set when our templated operator
    // is excluded by SFINAE -- gives the user a useful diagnostic
    // instead of a "no operator" error. Returning an array type is
    // ill-formed, so SFINAE-guard the function for non-array L only.
    template <typename U = L>
    operator U() const noexcept // NOLINT(google-explicit-constructor)
        requires(!::sf::base::isArray<U>)
    {
        return lhs;
    }
};


////////////////////////////////////////////////////////////
class ExpressionDecomposer
{
public:
    AssertKind kind;

    [[gnu::always_inline]] explicit ExpressionDecomposer(AssertKind k) noexcept : kind{k}
    {
    }

    template <typename L>
    [[gnu::always_inline]] ExpressionLhs<L> operator<<(const L& operand) const noexcept
    {
        return ExpressionLhs<L>(operand, kind);
    }
};


////////////////////////////////////////////////////////////
/// \brief Out-of-line glue. Records the assertion result with the runner,
/// reports a failure (with just the `#expr` string -- no value
/// decomposition; see header comment) if needed, and returns `true` for
/// "passed". Body lives once in `Decompose.cpp`.
////////////////////////////////////////////////////////////
bool handleAssertion(Result res, AssertKind kind, const char* file, int line, const char* exprStr);

} // namespace tst::detail


////////////////////////////////////////////////////////////
/// \file
///
/// Expression decomposition for CHECK / REQUIRE.
///
/// Same Catch/doctest trick: the `CHECK(a == b)` expression is wrapped
/// in `Decomposer{} << (a == b)` -- `operator<<` grabs the LHS via
/// `ExpressionLhs<L>`, then `ExpressionLhs::operator==` (and friends)
/// capture the RHS, evaluate the comparison, and return a tiny
/// `Result` carrying just the pass/fail bit.
///
/// Stringification of the captured operands is intentionally NOT
/// performed here (the per-type StringMaker specializations under
/// `Stringify.hpp` overwhelmingly return a placeholder). The failure
/// report just echoes the textual `#expr` from the macro -- which is
/// already where doctest's own "expansion" line lands when no
/// stringification is registered. The saving: this header doesn't pull
/// `<SFML/Base/String.hpp>`, which shaves a healthy chunk off every
/// test TU's parse time.
////////////////////////////////////////////////////////////
