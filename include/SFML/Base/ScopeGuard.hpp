#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/TokenPaste.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename F>
struct [[nodiscard]] ScopeGuard : F
{
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~ScopeGuard()
    {
        static_cast<F&>(*this)();
    }
};


////////////////////////////////////////////////////////////
template <typename F>
ScopeGuard(F) -> ScopeGuard<F>; // Needed by clang-cl


////////////////////////////////////////////////////////////
#define SFML_BASE_SCOPE_GUARD(...)                                      \
    ::sf::base::ScopeGuard SFML_BASE_TOKEN_PASTE(_scopeGuard, __LINE__) \
    {                                                                   \
        [&] __VA_ARGS__                                                 \
    }

} // namespace sf::base
