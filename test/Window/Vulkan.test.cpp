#include "Tst/Tst.hpp"
#include "Zancle/Window/Vulkan.hpp"

TEST_CASE("[Window] za::Vulkan")
{
    SECTION("getFunction()")
    {
        CHECK(za::Vulkan::getFunction("", {}) == nullptr);
        CHECK(za::Vulkan::getFunction(" ", {}) == nullptr);
        CHECK(za::Vulkan::getFunction("a string that will never resolve to a Vulkan function", {}) == nullptr);

        if (za::Vulkan::isAvailable())
        {
            CHECK(za::Vulkan::getFunction("vkCreateInstance", {}) != nullptr);
        }
    }

    SECTION("getGraphicsRequiredInstanceExtensions()")
    {
        // If Vulkan is not available this function may or may not return a non-empty vector
        // If Vulkan is available then it will always return a non-empty vector
        if (za::Vulkan::isAvailable())
        {
            CHECK(!za::Vulkan::getGraphicsRequiredInstanceExtensions().empty());
        }
    }
}
