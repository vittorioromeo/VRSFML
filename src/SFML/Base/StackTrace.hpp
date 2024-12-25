#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
/// \brief Print a stack trace to stdout
///
/// Only has effect if `SFML_ENABLE_STACK_TRACES` is defined
///
////////////////////////////////////////////////////////////
[[gnu::cold]] void printStackTrace();

} // namespace sf::base::priv
