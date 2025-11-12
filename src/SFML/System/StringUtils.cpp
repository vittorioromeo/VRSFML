// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/StringUtils.hpp"

#include "SFML/Base/String.hpp"

#include <string>

#include <cctype>


namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
T toLower(T str)
{
    for (char& c : str)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return str;
}


////////////////////////////////////////////////////////////
template std::string  toLower<std::string>(std::string);
template base::String toLower<base::String>(base::String);

} // namespace sf::priv
