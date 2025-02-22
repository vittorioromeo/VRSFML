#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Win32/Utils.hpp"

#include "SFML/System/String.hpp"
#include "SFML/System/Win32/WindowsHeader.hpp"

#include <string>


namespace sf::priv
{
////////////////////////////////////////////////////////////
std::string getErrorString(DWORD error)
{
    PTCHAR buffer = nullptr;
    if (FormatMessage(FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      nullptr,
                      error,
                      0,
                      reinterpret_cast<PTCHAR>(&buffer),
                      0,
                      nullptr) == 0)
    {
        return "Unknown error.";
    }

    const sf::String message = buffer;
    LocalFree(buffer);
    return message.toAnsiString<std::string>();
}

} // namespace sf::priv
