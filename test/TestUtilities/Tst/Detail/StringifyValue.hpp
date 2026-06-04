#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/SizeT.hpp"


////////////////////////////////////////////////////////////
// Forward declarations of the value types Tst knows how to render.
// Tags (`struct`/`class`) match the rest of the test tree to avoid
// `-Wmismatched-tags`.
////////////////////////////////////////////////////////////
namespace za
{
template <typename>
struct Vec2;

template <typename>
struct Vec3;

template <typename>
class Rect2;
} // namespace za


namespace tst
{
class Approx;
} // namespace tst


namespace tst::detail
{
////////////////////////////////////////////////////////////
// (1) Catch-all -- inline, dependency-free. Any type without a more
//     specialized overload below lands here and renders "<?>".
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const T&) noexcept
{
    const char* src = "<?>";
    zb::SizeT   n   = 0u;

    while (src[n] != '\0' && n < cap)
    {
        buf[n] = src[n];
        ++n;
    }

    return n;
}


////////////////////////////////////////////////////////////
// (1b) String literals / char arrays decay to the `const char*` path so
//      `CHECK("foo" == s)` shows `"foo"` rather than "<?>".
////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const char* v) noexcept;

template <zb::SizeT N>
[[gnu::always_inline]] inline zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const char (&v)[N]) noexcept
{
    return stringifyValue(buf, cap, static_cast<const char*>(v));
}


////////////////////////////////////////////////////////////
// (2) Scalars -- declared here, defined out-of-line.
////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, bool v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, char v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, short v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, unsigned short v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, int v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, unsigned int v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, long v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, unsigned long v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, long long v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, unsigned long long v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, float v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, double v) noexcept;
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, long double v) noexcept;


////////////////////////////////////////////////////////////
// (3) Library / composite types -- declared here, DEFINED and explicitly
//     instantiated in `StringifyValue.cpp`. Comparing an instantiation
//     that the `.cpp` does not list is a (localized) link error -- add it
//     to the instantiation block there.
////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Vec2<T>& v) noexcept;

template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Vec3<T>& v) noexcept;

template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Rect2<T>& v) noexcept;

zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const tst::Approx& v) noexcept;


////////////////////////////////////////////////////////////
/// \brief Copies up to `cap` bytes of `[data, data + n)` into `buf` and
/// returns the number of bytes copied. The bridge used by out-of-line
/// formatters that build an `zb::String` internally (in their own
/// `.cpp`) and hand the bytes back without exposing `String` here.
///
////////////////////////////////////////////////////////////
zb::SizeT copyInto(char* buf, zb::SizeT cap, const char* data, zb::SizeT n) noexcept;


////////////////////////////////////////////////////////////
/// \brief Type-erased thunk captured by the expression decomposer. This
/// is the ONLY per-operand-type code instantiated in a test TU; it names
/// only `stringifyValue` + `SizeT`, so it drags in neither Fmt nor String.
///
////////////////////////////////////////////////////////////
using StringifyFn = zb::SizeT (*)(char* buf, zb::SizeT cap, const void* value);


////////////////////////////////////////////////////////////
/// \brief Non-erased dispatch. Resolves `stringifyValue` by ordinary
/// lookup (the overloads above) plus ADL (overloads a value type declares
/// in its own namespace, e.g. `za::stringifyValue(const za::Color&)`).
/// Wrappers such as `Approx<T>` call this to render their nested value.
///
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline zb::SizeT renderValue(char* buf, zb::SizeT cap, const T& v) noexcept
{
    return stringifyValue(buf, cap, v);
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline zb::SizeT stringifyThunk(char* buf, zb::SizeT cap, const void* value) noexcept
{
    return renderValue(buf, cap, *static_cast<const T*>(value));
}

} // namespace tst::detail


////////////////////////////////////////////////////////////
/// \file
/// Value stringification for assertion diagnostics.
///
/// The customization point is a plain free-function overload set named
/// `stringifyValue`, resolved by ordinary lookup + partial ordering:
///
///   * the generic catch-all renders "<?>" for any unhandled type. It is
///     defined inline and pulls in NEITHER `<Fmt.hpp>` NOR `<String.hpp>`;
///   * the scalar overloads and the `za::Vec2<T>` / `za::Rect2<T>` /
///     `tst::Approx` overloads are DECLARED here and DEFINED ONCE in
///     `StringifyValue.cpp`, where the heavy Fmt machinery and the full
///     value-type headers live. A using TU never sees Fmt or String.
///
/// The sink is a raw `char*` + capacity (Fmt is wrapped internally in the
/// `.cpp`), so Fmt does not leak into this header's API at all. Each
/// overload returns the number of bytes written, clamped to `cap`; no NUL
/// terminator is written.
////////////////////////////////////////////////////////////
