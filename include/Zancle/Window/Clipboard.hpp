#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Export.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Utf8String;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Give access to the system clipboard
///
////////////////////////////////////////////////////////////
struct ZA_WINDOW_API Clipboard
{
    ////////////////////////////////////////////////////////////
    /// \brief Get the content of the clipboard as string data
    ///
    /// This function returns the content of the clipboard
    /// as a UTF-8 string. If the clipboard does not contain
    /// string data, an empty `za::Utf8String` is returned.
    ///
    /// \return Clipboard contents as a UTF-8 `za::Utf8String`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static Utf8String getString();

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
    /// \param text `za::Utf8String` containing the data to be sent
    /// to the clipboard
    ///
    /// \return `true` if the clipboard was successfully set
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool setString(const Utf8String& text);
};

} // namespace za

////////////////////////////////////////////////////////////
/// \struct za::Clipboard
/// \ingroup window
///
/// `za::Clipboard` provides an interface for getting and
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
/// za::Utf8String string = za::Clipboard::getString();
///
/// // or use it in the event loop
/// while (const zb::Optional event = window.pollEvent())
/// {
///     if (event->is<za::Event::Closed>())
///         return 0; // break out of both event and main loops
///
///     if (const auto* keyPressed = event->getIf<za::Event::KeyPressed>())
///     {
///         // Using Ctrl + V to paste a string into Zancle
///         if (keyPressed->control && keyPressed->code == za::Keyboard::Key::V)
///             string = za::Clipboard::getString();
///
///         // Using Ctrl + C to copy a string out of Zancle
///         if (keyPressed->control && keyPressed->code == za::Keyboard::Key::C)
///             za::Clipboard::setString("Hello World!");
///     }
/// }
/// \endcode
///
/// \see `za::Utf8String`, `za::Event`
///
////////////////////////////////////////////////////////////
