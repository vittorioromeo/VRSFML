#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Trait/IsSame.hpp"


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
        static_cast<F&&>(f)();
        return RegularizeVoidDummy{};
    }
    else
    {
        return static_cast<F&&>(f)();
    }
}

} // namespace sf
