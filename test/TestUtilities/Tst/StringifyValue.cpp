// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Approx.hpp"
#include "Tst/Detail/StringifyValue.hpp"

#include "SFML/System/Rect2.hpp"
#include "SFML/System/Vec2.hpp"
#include "SFML/System/Vec3.hpp"

#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtCString.hpp" // IWYU pragma: keep
#include "SFML/Base/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "SFML/Base/Fmt/FmtSink.hpp"
#include "SFML/Base/SizeT.hpp"


////////////////////////////////////////////////////////////
// The single TU that implements `stringifyValue`. Everything heavy --
// `<Fmt.hpp>`, the numeric/cstring formatters, and the full value-type
// headers -- is confined here; the public header stays at `SizeT` + a few
// forward declarations. Each overload wraps the caller-provided
// `char* + cap` sink into a `FmtSink` internally.
////////////////////////////////////////////////////////////


namespace tst::detail
{
////////////////////////////////////////////////////////////
sf::base::SizeT copyInto(char* buf, sf::base::SizeT cap, const char* data, sf::base::SizeT n) noexcept
{
    const sf::base::SizeT toCopy = n < cap ? n : cap;
    if (toCopy != 0u)
        SFML_BASE_MEMCPY(buf, data, toCopy);
    return toCopy;
}


////////////////////////////////////////////////////////////
#define SFML_TST_DEFINE_STRINGIFY_NUMERIC(T)                                     \
    sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, T v) noexcept \
    {                                                                            \
        sf::base::FmtSink sink{buf, cap};                                        \
        (void)sink.fmt("{}", v);                                                 \
        return sink.size();                                                      \
    }

SFML_TST_DEFINE_STRINGIFY_NUMERIC(short)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(int)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(long)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(long long)

SFML_TST_DEFINE_STRINGIFY_NUMERIC(unsigned short)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(unsigned int)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(unsigned long)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(unsigned long long)

SFML_TST_DEFINE_STRINGIFY_NUMERIC(float)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(double)
SFML_TST_DEFINE_STRINGIFY_NUMERIC(long double)

#undef SFML_TST_DEFINE_STRINGIFY_NUMERIC


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, bool v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.append(v ? "true" : "false", v ? 4u : 5u);
    return sink.size();
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, char v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.fmt("'{}'", v);
    return sink.size();
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const char* v) noexcept
{
    sf::base::FmtSink sink{buf, cap};

    if (v == nullptr)
        (void)sink.append("<null>", 6u);
    else
        (void)sink.fmt("\"{}\"", v);

    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Vec2<T>& v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.fmt("({}, {})", v.x, v.y);
    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Vec3<T>& v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.fmt("({}, {}, {})", v.x, v.y, v.z);
    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const sf::Rect2<T>& v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.fmt("[pos=({}, {}) size=({}, {})]", v.position.x, v.position.y, v.size.x, v.size.y);
    return sink.size();
}


////////////////////////////////////////////////////////////
sf::base::SizeT stringifyValue(char* buf, sf::base::SizeT cap, const tst::Approx& v) noexcept
{
    sf::base::FmtSink sink{buf, cap};
    (void)sink.fmt("Approx({})", v.value());
    return sink.size();
}


////////////////////////////////////////////////////////////
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec2<int>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec2<unsigned int>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec2<float>&) noexcept;

template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec3<int>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec3<unsigned int>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Vec3<float>&) noexcept;

template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Rect2<int>&) noexcept;
template sf::base::SizeT stringifyValue(char*, sf::base::SizeT, const sf::Rect2<float>&) noexcept;

} // namespace tst::detail
