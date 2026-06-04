#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


namespace zb
{
class FmtSink;
class String;
class StringView;
} // namespace zb


namespace tst
{
class Approx;

namespace detail
{
////////////////////////////////////////////////////////////
class ExpressionDecomposer;
class SubcaseScope;
struct Result;
struct SkipDecorator;
struct TestCaseHandle;

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


////////////////////////////////////////////////////////////
/// \file
/// Minimal forward declarations for the bespoke testing library.
///
////////////////////////////////////////////////////////////
