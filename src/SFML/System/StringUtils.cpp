#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/StringUtils.hpp"

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
template std::string toLower<std::string>(std::string);

} // namespace sf::priv
