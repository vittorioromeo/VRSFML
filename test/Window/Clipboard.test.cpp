#include "SFML/Window/Clipboard.hpp"

#include "SFML/Window/WindowContext.hpp"

// Other 1st party headers
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "SFML/System/Utf8String.hpp"


#ifndef SFML_SYSTEM_EMSCRIPTEN // TODO P1: clipboard not implemented for emscripten
TEST_CASE("[Window] sf::Clipboard" * tst::skip(skipDisplayTests))
{
    auto windowContext = sf::WindowContext::create().value();

    // Capture current clipboard state
    const auto currentClipboard = sf::Clipboard::getString();

    sf::Utf8String string;

    SECTION("ASCII")
    {
        string = u8"Snail";
    }

    SECTION("Latin1")
    {
        string = u8"Limacé";
    }

    SECTION("Basic Multilingual Plane")
    {
        string = u8"カタツムリ";
    }

    SECTION("Emoji")
    {
        string = u8"🐌";
    }

    INFO("String: " << string.cStr());
    CHECK(sf::Clipboard::setString(string));
    CHECK(sf::Clipboard::getString() == string);

    // Restore clipboard
    CHECK(sf::Clipboard::setString(currentClipboard));

    // We rely on getString triggering clipboard event processing on X11 to make
    // setString work, but note that the way setString is guaranteed to work is
    // by having an open window for which events are being handled.
    CHECK(sf::Clipboard::getString() == currentClipboard);
}
#endif
