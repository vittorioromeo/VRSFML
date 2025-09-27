#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Traits/IsSame.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
struct RegularizeVoidDummy
{
};


////////////////////////////////////////////////////////////
template <typename F>
[[nodiscard, gnu::always_inline, gnu::flatten]] inline constexpr auto regularizeVoid(F&& f)
{
    if constexpr (SFML_BASE_IS_SAME(decltype(f()), void))
    {
        f();
        return RegularizeVoidDummy{};
    }
    else
    {
        return f();
    }
}

} // namespace sf
