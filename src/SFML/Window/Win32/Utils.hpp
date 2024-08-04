#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#if __has_include(<bits/stringfwd.h>)
#include <bits/stringfwd.h>
#else
#include <string>
#endif

using DWORD = unsigned long;

namespace sf::priv
{
////////////////////////////////////////////////////////////
std::string getErrorString(DWORD error);

} // namespace sf::priv
