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
/// \brief RAII helper that runs an arbitrary callable on scope exit
///
/// Inherits from the callable so that no extra storage is required for
/// stateless lambdas (empty base optimization). The destructor invokes
/// the captured callable, making this a building block for ad-hoc
/// cleanup code without requiring a custom RAII type.
///
/// Use the `SFML_BASE_SCOPE_GUARD` macro for the canonical
/// `[&] { ... }` pattern.
///
////////////////////////////////////////////////////////////
template <typename F>
struct [[nodiscard]] ScopeGuard : F
{
    ////////////////////////////////////////////////////////////
    /// \brief Destructor, invokes the wrapped callable
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] ~ScopeGuard()
    {
        static_cast<F&>(*this)();
    }
};


////////////////////////////////////////////////////////////
/// \brief Deduction guide allowing `ScopeGuard{lambda}` syntax (required by clang-cl)
///
////////////////////////////////////////////////////////////
template <typename F>
ScopeGuard(F) -> ScopeGuard<F>; // Needed by clang-cl


////////////////////////////////////////////////////////////
/// \brief Convenience macro that declares a uniquely-named scope guard from a lambda body
///
/// Usage: `SFML_BASE_SCOPE_GUARD({ cleanup(); });`
///
////////////////////////////////////////////////////////////
#define SFML_BASE_SCOPE_GUARD(...)                                            \
    const ::sf::base::ScopeGuard SFML_BASE_TOKEN_PASTE(_scopeGuard, __LINE__) \
    {                                                                         \
        [&] __VA_ARGS__                                                       \
    }

} // namespace sf::base
