#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Minimal forward declarations for the bespoke testing library.
// No standard-library includes anywhere in this header tree.
////////////////////////////////////////////////////////////


namespace sf::base
{
class String;
class StringView;
class FmtSink;
} // namespace sf::base


namespace tst
{
class Approx;

namespace detail
{
struct SkipDecorator;
struct TestCaseHandle;
class SubcaseScope;
struct Result;
class ExpressionDecomposer;
template <typename L>
struct ExpressionLhs;

using TestFn = void (*)();

////////////////////////////////////////////////////////////
enum class AssertKind : unsigned char
{
    Check        = 0u,
    CheckFalse   = 1u,
    Require      = 2u,
    RequireFalse = 3u,
    Warn         = 4u,
};

} // namespace detail
} // namespace tst
