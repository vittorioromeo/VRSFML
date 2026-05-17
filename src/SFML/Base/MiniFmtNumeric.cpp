// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/MiniFmtNumeric.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/ToChars.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
    requires isIntegral<T>
void formatArg(FormatSink& sink, const T& arg, const FormatSpec&)
{
    // 24 covers signed/unsigned up to 64 bits (worst case "-9223372036854775808" = 20 chars).
    char buf[24];

    char* const end = toChars(buf, buf + sizeof(buf), arg);
    sink.append(buf, static_cast<SizeT>(end - buf));
}


////////////////////////////////////////////////////////////
template <typename T>
    requires isFloatingPoint<T>
void formatArg(FormatSink& sink, const T& arg, const FormatSpec& spec)
{
    const int prec = spec.precision >= 0 ? spec.precision : defaultFloatPrecision;

    // 40 covers sign + ~20-digit integral part + '.' + up to 10 fractional digits + slack.
    char buf[40];

    char* const end = toChars(buf, buf + sizeof(buf), arg, prec);
    sink.append(buf, static_cast<SizeT>(end - buf));
}


////////////////////////////////////////////////////////////
// Explicit instantiations -- match the `extern template` decls in the header.
// Each row emits three symbols: `formatArg<T>`, `priv::dispatchFormatArg<T>`,
// `priv::erasedDispatch<T>`. The last is the one assembleImpl's dispatcher
// table takes the address of; pre-emitting them avoids per-TU weak symbols.
////////////////////////////////////////////////////////////
#define SFML_BASE_MINIFMT_INSTANTIATE(T)                                                \
    template void formatArg<T>(FormatSink&, const T&, const FormatSpec&);               \
    template void priv::dispatchFormatArg<T>(FormatSink&, const T&, const FormatSpec&); \
    template void priv::erasedDispatch<T>(FormatSink&, const void*, const FormatSpec&)

SFML_BASE_MINIFMT_INSTANTIATE(bool);
SFML_BASE_MINIFMT_INSTANTIATE(char);
SFML_BASE_MINIFMT_INSTANTIATE(signed char);
SFML_BASE_MINIFMT_INSTANTIATE(unsigned char);
SFML_BASE_MINIFMT_INSTANTIATE(short);
SFML_BASE_MINIFMT_INSTANTIATE(unsigned short);
SFML_BASE_MINIFMT_INSTANTIATE(int);
SFML_BASE_MINIFMT_INSTANTIATE(unsigned int);
SFML_BASE_MINIFMT_INSTANTIATE(long);
SFML_BASE_MINIFMT_INSTANTIATE(unsigned long);
SFML_BASE_MINIFMT_INSTANTIATE(long long);
SFML_BASE_MINIFMT_INSTANTIATE(unsigned long long);
SFML_BASE_MINIFMT_INSTANTIATE(float);
SFML_BASE_MINIFMT_INSTANTIATE(double);

#undef SFML_BASE_MINIFMT_INSTANTIATE

} // namespace sf::base
