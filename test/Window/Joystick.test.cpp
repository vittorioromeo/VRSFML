#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/Window/Joystick.hpp"
#include "Zancle/Window/WindowContext.hpp"

TEST_CASE("[Window] za::Joystick")
{
    SECTION("Constants")
    {
        STATIC_CHECK(za::Joystick::MaxCount == 8);
        STATIC_CHECK(za::Joystick::ButtonCount == 32);
        STATIC_CHECK(za::Joystick::AxisCount == 8);
    }

    // By avoiding calling za::Joystick::update() we can guarantee that
    // no joysticks will be detected. This is how we can ensure these
    // tests are portable and reliable.

    auto windowContext = za::WindowContext::create().value();

    for (unsigned int joystickId = 0u; joystickId < za::Joystick::MaxCount; ++joystickId)
    {
        SECTION("query()")
        {
            CHECK(!za::Joystick::query(joystickId).hasValue());
        }
    }
}
