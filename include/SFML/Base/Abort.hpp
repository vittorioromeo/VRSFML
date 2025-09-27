#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Immediately terminate the current process abnormally
///
/// This function calls `std::abort()`. It is used internally
/// by SFML for irrecoverable errors, such as failed assertions
/// in debug mode.
///
////////////////////////////////////////////////////////////
[[noreturn, gnu::cold, gnu::noinline]] void abort() noexcept;

} // namespace sf::base
