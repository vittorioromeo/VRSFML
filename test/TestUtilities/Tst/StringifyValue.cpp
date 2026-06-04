// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Tst/Approx.hpp"
#include "Tst/Detail/StringifyValue.hpp"

#include "Zancle/System/Rect2.hpp"
#include "Zancle/System/Vec2.hpp"
#include "Zancle/System/Vec3.hpp"

#include "ZancleBase/Builtin/Memcpy.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtCString.hpp" // IWYU pragma: keep
#include "ZancleBase/Fmt/FmtNumeric.hpp" // IWYU pragma: keep
#include "ZancleBase/Fmt/FmtSink.hpp"
#include "ZancleBase/SizeT.hpp"


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
zb::SizeT copyInto(char* buf, zb::SizeT cap, const char* data, zb::SizeT n) noexcept
{
    const zb::SizeT toCopy = n < cap ? n : cap;
    if (toCopy != 0u)
        ZB_MEMCPY(buf, data, toCopy);
    return toCopy;
}


////////////////////////////////////////////////////////////
#define ZA_TST_DEFINE_STRINGIFY_NUMERIC(T)                                     \
    zb::SizeT stringifyValue(char* buf, zb::SizeT cap, T v) noexcept \
    {                                                                            \
        zb::FmtSink sink{buf, cap};                                        \
        (void)sink.fmt("{}", v);                                                 \
        return sink.size();                                                      \
    }

ZA_TST_DEFINE_STRINGIFY_NUMERIC(short)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(int)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(long)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(long long)

ZA_TST_DEFINE_STRINGIFY_NUMERIC(unsigned short)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(unsigned int)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(unsigned long)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(unsigned long long)

ZA_TST_DEFINE_STRINGIFY_NUMERIC(float)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(double)
ZA_TST_DEFINE_STRINGIFY_NUMERIC(long double)

#undef ZA_TST_DEFINE_STRINGIFY_NUMERIC


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, bool v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.append(v ? "true" : "false", v ? 4u : 5u);
    return sink.size();
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, char v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.fmt("'{}'", v);
    return sink.size();
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const char* v) noexcept
{
    zb::FmtSink sink{buf, cap};

    if (v == nullptr)
        (void)sink.append("<null>", 6u);
    else
        (void)sink.fmt("\"{}\"", v);

    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Vec2<T>& v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.fmt("({}, {})", v.x, v.y);
    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Vec3<T>& v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.fmt("({}, {}, {})", v.x, v.y, v.z);
    return sink.size();
}


////////////////////////////////////////////////////////////
template <typename T>
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const za::Rect2<T>& v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.fmt("[pos=({}, {}) size=({}, {})]", v.position.x, v.position.y, v.size.x, v.size.y);
    return sink.size();
}


////////////////////////////////////////////////////////////
zb::SizeT stringifyValue(char* buf, zb::SizeT cap, const tst::Approx& v) noexcept
{
    zb::FmtSink sink{buf, cap};
    (void)sink.fmt("Approx({})", v.value());
    return sink.size();
}


////////////////////////////////////////////////////////////
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec2<int>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec2<unsigned int>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec2<float>&) noexcept;

template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec3<int>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec3<unsigned int>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Vec3<float>&) noexcept;

template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Rect2<int>&) noexcept;
template zb::SizeT stringifyValue(char*, zb::SizeT, const za::Rect2<float>&) noexcept;

} // namespace tst::detail
