#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Combine multiple callables into a single overloaded callable
///
/// Standard "overloaded" idiom: inherits from each `Fs` and pulls in
/// their `operator()` so that the resulting object exposes one
/// overload per base. Typically used to write pattern-matching style
/// visitors for `Variant`.
///
/// Example:
/// \code
/// variant.recursiveMatch(
///     [](int i)         { ... },
///     [](const String&) { ... });
/// \endcode
///
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
/// \brief Deduction guide allowing brace-init from a list of callables
///
////////////////////////////////////////////////////////////
template <typename... Fs>
OverloadSet(Fs...) -> OverloadSet<Fs...>;

} // namespace sf::base
