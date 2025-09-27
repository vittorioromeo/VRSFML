// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md

namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Print a stack trace to stdout
///
/// Only has effect if `SFML_ENABLE_STACK_TRACES` is defined
///
////////////////////////////////////////////////////////////
[[gnu::cold, gnu::noinline]] void printStackTrace();

} // namespace sf::base::priv
