#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/WindowEnums.hpp> // Prevent conflict with macro None from Xlib

#include <SFML/System/String.hpp>

#include <X11/Xlib.h>

#include <deque>
#include <memory>


namespace sf::priv
{

////////////////////////////////////////////////////////////
/// \brief Give access to the system clipboard
///
////////////////////////////////////////////////////////////
class ClipboardImpl
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

    ////////////////////////////////////////////////////////////
    /// \brief Process pending events for the hidden clipboard window
    ///
    /// This function has to be called as part of normal window
    /// event processing in order for our application to respond
    /// to selection requests from other applications.
    ///
    ////////////////////////////////////////////////////////////
    static void processEvents();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Constructor
    ///
    ////////////////////////////////////////////////////////////
    ClipboardImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~ClipboardImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Get singleton instance
    ///
    /// \return Singleton instance
    ///
    ////////////////////////////////////////////////////////////
    static ClipboardImpl& getInstance();

    ////////////////////////////////////////////////////////////
    /// \brief getString implementation
    ///
    /// \return Current content of the clipboard
    ///
    ////////////////////////////////////////////////////////////
    String getStringImpl();

    ////////////////////////////////////////////////////////////
    /// \brief setString implementation
    ///
    /// \param text sf::String object containing the data to be sent to the clipboard
    ///
    ////////////////////////////////////////////////////////////
    void setStringImpl(const String& text);

    ////////////////////////////////////////////////////////////
    /// \brief processEvents implementation
    ///
    ////////////////////////////////////////////////////////////
    void processEventsImpl();

    ////////////////////////////////////////////////////////////
    /// \brief Process an incoming event from the window
    ///
    /// \param windowEvent Event which has been received
    ///
    ////////////////////////////////////////////////////////////
    void processEvent(XEvent& windowEvent);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    ::Window                   m_window{};          ///< X identifier defining our window
    std::shared_ptr<::Display> m_display;           ///< Pointer to the display
    Atom                       m_clipboard;         ///< X Atom identifying the CLIPBOARD selection
    Atom                       m_targets;           ///< X Atom identifying TARGETS
    Atom                       m_text;              ///< X Atom identifying TEXT
    Atom                       m_utf8String;        ///< X Atom identifying UTF8_STRING
    Atom                       m_targetProperty;    ///< X Atom identifying our destination window property
    String                     m_clipboardContents; ///< Our clipboard contents
    std::deque<XEvent>         m_events;            ///< Queue we use to store pending events for this window
    bool m_requestResponded{}; ///< Holds whether our selection request has been responded to or not
};

} // namespace sf::priv
