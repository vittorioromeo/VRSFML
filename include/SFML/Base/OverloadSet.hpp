#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Fs>
struct [[nodiscard]] OverloadSet : Fs...
{
    [[nodiscard, gnu::always_inline]] explicit OverloadSet(Fs&&... fs) noexcept : Fs{static_cast<Fs&&>(fs)}...
    {
    }

    using Fs::operator()...;
};


////////////////////////////////////////////////////////////
template <typename... Fs>
OverloadSet(Fs...) -> OverloadSet<Fs...>;

} // namespace sf::base
