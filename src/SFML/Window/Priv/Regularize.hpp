#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename F>
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr auto regularize(F&& f)
{
    if constexpr (SFML_BASE_IS_SAME(decltype(f()), void))
    {
        f();
        return nullptr;
    }
    else
    {
        return f();
    }
}

} // namespace sf::priv
