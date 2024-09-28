#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

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
    [[gnu::always_inline, gnu::flatten]] ~ScopeGuard()
    {
        static_cast<F&>(*this)();
    }
};


////////////////////////////////////////////////////////////
#define SFML_BASE_SCOPE_GUARD(...)                                      \
    ::sf::base::ScopeGuard SFML_BASE_TOKEN_PASTE(_scopeGuard, __LINE__) \
    {                                                                   \
        [&] __VA_ARGS__                                                 \
    }

} // namespace sf::base
