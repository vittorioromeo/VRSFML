#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once


namespace sf
{
class String;

namespace priv
{
////////////////////////////////////////////////////////////
/// \brief Stub implementation of clipboard
///
////////////////////////////////////////////////////////////
class StubClipboardImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Get the content of the clipboard as string data
    ///
    /// This function returns the content of the clipboard
    /// as a string. If the clipboard does not contain string
    /// it returns an empty sf::String object.
    ///
    /// \return Current content of the clipboard
    ///
    ////////////////////////////////////////////////////////////
    static String getString();

    ////////////////////////////////////////////////////////////
    /// \brief Set the content of the clipboard as string data
    ///
    /// This function sets the content of the clipboard as a
    /// string.
    ///
    /// \param text sf::String object containing the data to be sent
    /// to the clipboard
    ///
    ////////////////////////////////////////////////////////////
    static void setString(const String& text);
};

} // namespace priv

} // namespace sf
