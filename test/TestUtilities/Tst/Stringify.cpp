// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


#include "Tst/Detail/Stringify.hpp"

#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/ToChars.hpp"
#include "SFML/Base/ToString.hpp"


////////////////////////////////////////////////////////////
// Out-of-line stringification primitives. All `StringMaker<T>::convert`
// specializations declared in `Stringify.hpp` forward to one of these
// functions so the actual conversion code lives once per program.
////////////////////////////////////////////////////////////


namespace tst
{
////////////////////////////////////////////////////////////
// Built-in `StringMaker` bodies. Out-of-line so `Stringify.hpp` can
// stay free of `<SFML/Base/String.hpp>`.
////////////////////////////////////////////////////////////
sf::base::String StringMaker<bool>::convert(bool v)
{
    return detail::stringifyBool(v);
}
sf::base::String StringMaker<char>::convert(char v)
{
    return detail::stringifyChar(v);
}
sf::base::String StringMaker<signed char>::convert(signed char v)
{
    return detail::stringifySignedLongLong(static_cast<long long>(v));
}
sf::base::String StringMaker<unsigned char>::convert(unsigned char v)
{
    return detail::stringifyUnsignedLongLong(static_cast<unsigned long long>(v));
}
sf::base::String StringMaker<short>::convert(short v)
{
    return detail::stringifySignedLongLong(static_cast<long long>(v));
}
sf::base::String StringMaker<unsigned short>::convert(unsigned short v)
{
    return detail::stringifyUnsignedLongLong(static_cast<unsigned long long>(v));
}
sf::base::String StringMaker<int>::convert(int v)
{
    return detail::stringifySignedLongLong(static_cast<long long>(v));
}
sf::base::String StringMaker<unsigned int>::convert(unsigned int v)
{
    return detail::stringifyUnsignedLongLong(static_cast<unsigned long long>(v));
}
sf::base::String StringMaker<long>::convert(long v)
{
    return detail::stringifySignedLongLong(static_cast<long long>(v));
}
sf::base::String StringMaker<unsigned long>::convert(unsigned long v)
{
    return detail::stringifyUnsignedLongLong(static_cast<unsigned long long>(v));
}
sf::base::String StringMaker<long long>::convert(long long v)
{
    return detail::stringifySignedLongLong(v);
}
sf::base::String StringMaker<unsigned long long>::convert(unsigned long long v)
{
    return detail::stringifyUnsignedLongLong(v);
}
sf::base::String StringMaker<float>::convert(float v)
{
    return detail::stringifyDouble(static_cast<double>(v));
}
sf::base::String StringMaker<double>::convert(double v)
{
    return detail::stringifyDouble(v);
}
sf::base::String StringMaker<const char*>::convert(const char* v)
{
    return detail::stringifyCString(v);
}
sf::base::String StringMaker<char*>::convert(char* v)
{
    return detail::stringifyCString(v);
}
} // namespace tst


namespace tst::detail
{
////////////////////////////////////////////////////////////
sf::base::String defaultStringification()
{
    return sf::base::String{"<?>"};
}


////////////////////////////////////////////////////////////
sf::base::String stringifyBool(bool v)
{
    return v ? sf::base::String{"true"} : sf::base::String{"false"};
}


////////////////////////////////////////////////////////////
sf::base::String stringifyChar(char v)
{
    char buf[3]{'\'', v, '\''};
    return sf::base::String{buf, 3u};
}


////////////////////////////////////////////////////////////
sf::base::String stringifySignedLongLong(long long v)
{
    return sf::base::toString(v);
}


////////////////////////////////////////////////////////////
sf::base::String stringifyUnsignedLongLong(unsigned long long v)
{
    return sf::base::toString(v);
}


////////////////////////////////////////////////////////////
sf::base::String stringifyDouble(double v)
{
    return sf::base::toString(v);
}


////////////////////////////////////////////////////////////
sf::base::String stringifyCString(const char* v)
{
    sf::base::String out;

    if (v == nullptr)
    {
        out.append("<null>", 6u);
    }
    else
    {
        out.append("\"", 1u);
        out.append(v);
        out.append("\"", 1u);
    }

    return out;
}


////////////////////////////////////////////////////////////
sf::base::String stringifyStringView(sf::base::StringView v)
{
    sf::base::String out;
    out.append("\"", 1u);
    out.append(v);
    out.append("\"", 1u);
    return out;
}


////////////////////////////////////////////////////////////
sf::base::String stringifyPtr(const void* v)
{
    sf::base::String out;

    if (v == nullptr)
    {
        out.append("nullptr", 7u);
        return out;
    }

    char        buf[32];
    const char* end = sf::base::toChars(buf, buf + 32, reinterpret_cast<sf::base::SizeT>(v));
    if (end == nullptr)
    {
        out.append("<ptr?>", 6u);
        return out;
    }

    out.append("0x", 2u);
    out.append(buf, static_cast<sf::base::SizeT>(end - buf));
    return out;
}

} // namespace tst::detail
