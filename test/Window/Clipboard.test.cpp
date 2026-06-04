#include "Zancle/Window/Clipboard.hpp"
#include "Zancle/Window/WindowContext.hpp"

// Other 1st party headers
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"
#include "Zancle/System/Utf8String.hpp"


#ifndef ZA_SYSTEM_EMSCRIPTEN // TODO P1: clipboard not implemented for emscripten
TEST_CASE("[Window] za::Clipboard" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    // Capture current clipboard state
    const auto currentClipboard = za::Clipboard::getString();

    za::Utf8String string;

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
    CHECK(za::Clipboard::setString(string));
    CHECK(za::Clipboard::getString() == string);

    // Restore clipboard
    CHECK(za::Clipboard::setString(currentClipboard));

    // We rely on getString triggering clipboard event processing on X11 to make
    // setString work, but note that the way setString is guaranteed to work is
    // by having an open window for which events are being handled.
    CHECK(za::Clipboard::getString() == currentClipboard);
}
#endif
