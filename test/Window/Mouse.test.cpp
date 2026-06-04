#include "Tst/Tst.hpp"
#include "WindowUtil.hpp"

#include "Zancle/Window/Mouse.hpp"

TEST_CASE("[Window] za::Mouse" * tst::skip(skipDisplayTests))
{
    SECTION("isButtonPressed()")
    {
        CHECK(!za::Mouse::isButtonPressed(za::Mouse::Button::Left));
        CHECK(!za::Mouse::isButtonPressed(za::Mouse::Button::Right));
        CHECK(!za::Mouse::isButtonPressed(za::Mouse::Button::Middle));
        CHECK(!za::Mouse::isButtonPressed(za::Mouse::Button::Extra1));
        CHECK(!za::Mouse::isButtonPressed(za::Mouse::Button::Extra2));
    }
}
