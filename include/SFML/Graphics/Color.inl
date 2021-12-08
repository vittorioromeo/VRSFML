////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2021 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
constexpr inline Color::Color() :
r(0),
g(0),
b(0),
a(255)
{

}


////////////////////////////////////////////////////////////
constexpr inline Color::Color(Uint8 red, Uint8 green, Uint8 blue, Uint8 alpha) :
r(red),
g(green),
b(blue),
a(alpha)
{

}


////////////////////////////////////////////////////////////
constexpr inline Color::Color(Uint32 color) :
r(static_cast<Uint8>((color & 0xff000000) >> 24)),
g(static_cast<Uint8>((color & 0x00ff0000) >> 16)),
b((color & 0x0000ff00) >> 8 ),
a((color & 0x000000ff) >> 0 )
{

}


////////////////////////////////////////////////////////////
constexpr inline Uint32 Color::toInteger() const
{
    return static_cast<Uint32>((r << 24) | (g << 16) | (b << 8) | a);
}


////////////////////////////////////////////////////////////
constexpr inline bool operator ==(const Color& left, const Color& right)
{
    return (left.r == right.r) &&
           (left.g == right.g) &&
           (left.b == right.b) &&
           (left.a == right.a);
}


////////////////////////////////////////////////////////////
constexpr inline bool operator !=(const Color& left, const Color& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
constexpr inline Color operator +(const Color& left, const Color& right)
{
    return Color(Uint8(priv::min(int(left.r) + right.r, 255)),
                 Uint8(priv::min(int(left.g) + right.g, 255)),
                 Uint8(priv::min(int(left.b) + right.b, 255)),
                 Uint8(priv::min(int(left.a) + right.a, 255)));
}


////////////////////////////////////////////////////////////
constexpr inline Color operator -(const Color& left, const Color& right)
{
    return Color(Uint8(priv::max(int(left.r) - right.r, 0)),
                 Uint8(priv::max(int(left.g) - right.g, 0)),
                 Uint8(priv::max(int(left.b) - right.b, 0)),
                 Uint8(priv::max(int(left.a) - right.a, 0)));
}


////////////////////////////////////////////////////////////
constexpr inline Color operator *(const Color& left, const Color& right)
{
    return Color(Uint8(int(left.r) * right.r / 255),
                 Uint8(int(left.g) * right.g / 255),
                 Uint8(int(left.b) * right.b / 255),
                 Uint8(int(left.a) * right.a / 255));
}


////////////////////////////////////////////////////////////
constexpr inline Color& operator +=(Color& left, const Color& right)
{
    return left = left + right;
}


////////////////////////////////////////////////////////////
constexpr inline Color& operator -=(Color& left, const Color& right)
{
    return left = left - right;
}


////////////////////////////////////////////////////////////
constexpr inline Color& operator *=(Color& left, const Color& right)
{
    return left = left * right;
}

constexpr inline Color::ColorInit::operator Color() const
{
    return Color(r, g, b, a);
}

// Sanity check:
static_assert(Color::Black == Color{0, 0, 0, 255});
static_assert(Color{0, 0, 0, 255} == Color::Black);