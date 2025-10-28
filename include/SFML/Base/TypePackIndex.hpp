#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsSame.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
enum : SizeT
{
    badTypePackIndex = static_cast<SizeT>(-1)
};


////////////////////////////////////////////////////////////
template <typename T, typename... Ts>
[[nodiscard]] consteval SizeT getTypePackIndex() noexcept
{
    constexpr bool matches[]{SFML_BASE_IS_SAME(T, Ts)...};

    for (SizeT i = 0u; i < sizeof...(Ts); ++i)
        if (matches[i])
            return i;

    return badTypePackIndex;
}


} // namespace sf::base
