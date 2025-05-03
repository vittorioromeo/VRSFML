#include "SFML/Window/Clipboard.hpp"

// Other 1st party headers
#include "SFML/System/String.hpp"

#include <Doctest.hpp>

#include <SystemUtil.hpp>
#include <WindowUtil.hpp>

#include <string>


#ifndef SFML_SYSTEM_EMSCRIPTEN // TODO P1: clipboard not implemented for emscripten
TEST_CASE("[Window] sf::Clipboard" * doctest::skip(skipDisplayTests))
{
    // Capture current clipboard state
    const auto currentClipboard = sf::Clipboard::getString();

    sf::String string;

    SECTION("ASCII")
    {
        string = "Snail";
    }

    SECTION("Latin1")
    {
        string = U"Limacé";
    }

    SECTION("Basic Multilingual Plane")
    {
        string = U"カタツムリ";
    }

    SECTION("Emoji")
    {
        string = U"🐌";
    }

    INFO("String: " << reinterpret_cast<const char*>(string.toUtf8<std::u8string>().c_str()));
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
