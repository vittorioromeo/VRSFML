#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Export.hpp"

#include "SFML/System/UnicodeString.hpp"


////////////////////////////////////////////////////////////
/// \brief Give access to the system clipboard
///
////////////////////////////////////////////////////////////
namespace sf::Clipboard
{
////////////////////////////////////////////////////////////
/// \brief Get the content of the clipboard as string data
///
/// This function returns the content of the clipboard
/// as a string. If the clipboard does not contain string
/// it returns an empty `sf::UnicodeString` object.
///
/// \return Clipboard contents as `sf::UnicodeString` object
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API UnicodeString getString();

////////////////////////////////////////////////////////////
/// \brief Set the content of the clipboard as string data
///
/// This function sets the content of the clipboard as a
/// string.
///
/// \warning Due to limitations on some operating systems,
///          setting the clipboard contents is only
///          guaranteed to work if there is currently an
///          open window for which events are being handled.
///
/// \param text `sf::UnicodeString` containing the data to be sent
/// to the clipboard
///
/// \return `true` if the clipboard was successfully set
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_WINDOW_API bool setString(const UnicodeString& text);

} // namespace sf::Clipboard

////////////////////////////////////////////////////////////
/// \namespace sf::Clipboard
/// \ingroup window
///
/// `sf::Clipboard` provides an interface for getting and
/// setting the contents of the system clipboard.
///
/// It is important to note that due to limitations on some
/// operating systems, setting the clipboard contents is
/// only guaranteed to work if there is currently an open
/// window for which events are being handled.
///
/// Usage example:
/// \code
/// // get the clipboard content as a string
/// sf::UnicodeString string = sf::Clipboard::getString();
///
/// // or use it in the event loop
/// while (const sf::base::Optional event = window.pollEvent())
/// {
///     if (event->is<sf::Event::Closed>())
///         return 0; // break out of both event and main loops
///
///     if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
///     {
///         // Using Ctrl + V to paste a string into SFML
///         if (keyPressed->control && keyPressed->code == sf::Keyboard::Key::V)
///             string = sf::Clipboard::getString();
///
///         // Using Ctrl + C to copy a string out of SFML
///         if (keyPressed->control && keyPressed->code == sf::Keyboard::Key::C)
///             sf::Clipboard::setString("Hello World!");
///     }
/// }
/// \endcode
///
/// \see `sf::UnicodeString`, `sf::Event`
///
////////////////////////////////////////////////////////////
