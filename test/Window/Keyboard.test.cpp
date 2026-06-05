#include "Zancle/Window/Keyboard.hpp"

#include "Zancle/Window/WindowContext.hpp"

// Other 1st party headers
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/String/Utf8String.hpp" // IWYU pragma: keep

// We're limited on what can be tested. Without control over the hardware and the
// configuration of the operating system, certain things cannot be tested. In
// general, the mapping between keys and scancodes is a user configuration. Our
// tests cannot assume any particular configuration.
//
// Regardless this test case represents a best faith effort to cover some of this
// code in a way that is hopefully not prone to fail on different machines.

TEST_CASE("[Window] za::Keyboard" * tst::skip(skipDisplayTests))
{
    auto windowContext = za::WindowContext::create().value();

    SECTION("isKeyPressed(Key)")
    {
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Key::W));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Key::A));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Key::S));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Key::D));
    }

    SECTION("isKeyPressed(Scancode)")
    {
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Scan::W));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Scan::A));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Scan::S));
        CHECK(!za::Keyboard::isKeyPressed(za::Keyboard::Scan::D));
    }

    SECTION("localize(Scancode)")
    {
        CHECK(za::Keyboard::localize(za::Keyboard::Scan::Space) == za::Keyboard::Key::Space);
    }

    SECTION("delocalize(Key)")
    {
        CHECK(za::Keyboard::delocalize(za::Keyboard::Key::Space) == za::Keyboard::Scan::Space);
    }

    SECTION("getDescription(Scancode)")
    {
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F1) == "F1");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F2) == "F2");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F3) == "F3");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F4) == "F4");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F5) == "F5");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F6) == "F6");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F7) == "F7");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F8) == "F8");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F9) == "F9");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F10) == "F10");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F11) == "F11");
        CHECK(za::Keyboard::getDescription(za::Keyboard::Scan::F12) == "F12");
    }
}
