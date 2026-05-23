// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Fmt/FmtNumeric.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtResult.hpp"
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/Fmt/FmtSpec.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/ToCharsRadix.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
    requires isIntegral<T>
FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec)
{
    SFML_BASE_ASSERT(spec.precision < 0 && spec.type != 'f');

    // 64 covers the worst case: 64-bit binary (`{:b}` on `unsigned long long`).
    // Decimal and signed cases (max 20 chars + sign) and hex/oct also fit, so
    // the helper returns a non-null pointer on every code path below.
    char buf[64];

    const char* end = nullptr;

    // `bool` is formatted as a string ("true" / "false") by default, matching
    // `std::format` and `fmt::format`. The radix specs (`{:b}`, `{:x}`, etc.)
    // and `MakeUnsigned<bool>` would be ill-formed anyway, so the path is
    // bypassed entirely. Callers who want numeric output can cast to `int`.
    if constexpr (SFML_BASE_IS_SAME(T, bool))
    {
        return arg ? sink.append("true", 4u) : sink.append("false", 5u);
    }
    else
    {
        switch (spec.type)
        {
            case 'x':
                end = toCharsRadix(buf, buf + sizeof(buf), arg, Radix::Hex, /* upperHex */ false);
                break;
            case 'X':
                end = toCharsRadix(buf, buf + sizeof(buf), arg, Radix::Hex, /* upperHex */ true);
                break;
            case 'o':
                end = toCharsRadix(buf, buf + sizeof(buf), arg, Radix::Oct);
                break;
            case 'b':
                end = toCharsRadix(buf, buf + sizeof(buf), arg, Radix::Bin);
                break;
            default: // '\0' or 'd' -> signed/unsigned decimal
                end = toChars(buf, buf + sizeof(buf), arg);
                break;
        }
    }

    SFML_BASE_ASSERT(end != nullptr && "Numeric `fmtArg` ran out of buffer -- buf[64] should always fit");
    return end == nullptr ? FmtResult::failed : sink.append(buf, static_cast<SizeT>(end - buf));
}


////////////////////////////////////////////////////////////
template <typename T>
    requires isFloatingPoint<T>
FmtResult fmtArg(FmtSink& sink, const T& arg, const FmtSpec& spec)
{
    SFML_BASE_ASSERT(spec.type == '\0' || spec.type == 'f');

    const int prec = spec.precision >= 0 ? spec.precision : defaultFloatPrecision;

    // 40 covers sign + ~20-digit integral part + '.' + up to 10 fractional digits + slack.
    char buf[40];

    char* const end = toChars(buf, buf + sizeof(buf), arg, prec);
    if (end == nullptr)
    {
        SFML_BASE_ASSERT(false && "Floating-point `fmtArg` could not represent the value in its fixed buffer");
        return FmtResult::failed;
    }

    return sink.append(buf, static_cast<SizeT>(end - buf));
}


////////////////////////////////////////////////////////////
// Explicit instantiations -- match the `extern template` decls in the header.
// Each row emits three symbols: `fmtArg<T>`, `priv::dispatchFmtArg<T>`,
// `priv::dispatchFmtArgErased<T>`. The last is the one `fmtAssembleImpl`'s
// dispatcher table takes the address of; pre-emitting them avoids per-TU
// weak symbols.
////////////////////////////////////////////////////////////
#define SFML_BASE_FMT_INSTANTIATE(T)                                                \
    template FmtResult fmtArg<T>(FmtSink&, const T&, const FmtSpec&);               \
    template FmtResult priv::dispatchFmtArg<T>(FmtSink&, const T&, const FmtSpec&); \
    template FmtResult priv::dispatchFmtArgErased<T>(FmtSink&, const void*, const FmtSpec&)

SFML_BASE_FMT_INSTANTIATE(bool);
SFML_BASE_FMT_INSTANTIATE(char);
SFML_BASE_FMT_INSTANTIATE(signed char);
SFML_BASE_FMT_INSTANTIATE(unsigned char);
SFML_BASE_FMT_INSTANTIATE(short);
SFML_BASE_FMT_INSTANTIATE(unsigned short);
SFML_BASE_FMT_INSTANTIATE(int);
SFML_BASE_FMT_INSTANTIATE(unsigned int);
SFML_BASE_FMT_INSTANTIATE(long);
SFML_BASE_FMT_INSTANTIATE(unsigned long);
SFML_BASE_FMT_INSTANTIATE(long long);
SFML_BASE_FMT_INSTANTIATE(unsigned long long);
SFML_BASE_FMT_INSTANTIATE(float);
SFML_BASE_FMT_INSTANTIATE(double);

#undef SFML_BASE_FMT_INSTANTIATE

} // namespace sf::base
