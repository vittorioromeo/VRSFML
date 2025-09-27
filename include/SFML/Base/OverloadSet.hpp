#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename... Fs>
struct [[nodiscard]] OverloadSet : Fs...
{
    template <typename... FFwds>
    [[nodiscard, gnu::always_inline]] explicit OverloadSet(FFwds&&... fs) noexcept : Fs{static_cast<FFwds&&>(fs)}...
    {
    }

    using Fs::operator()...;
};


////////////////////////////////////////////////////////////
template <typename... Fs>
OverloadSet(Fs...) -> OverloadSet<Fs...>;

} // namespace sf::base
