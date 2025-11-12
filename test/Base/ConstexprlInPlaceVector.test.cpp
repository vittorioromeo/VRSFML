#if 0 // TODO P1: revisit in C++26

#include "SFML/Base/InPlaceVector.hpp"

#include <Doctest.hpp>


namespace
{
TEST_CASE("[Base] Base/InPlaceVector.hpp (Constexpr)")
{
    SECTION("Constexpr Support")
    {
        constexpr auto result = []
        {
            sf::base::InPlaceVector<int, 8> vec;

            for (int i = 0; i < 8; ++i)
                vec.pushBack(i);

            int acc = 0;

            for (const auto& v : vec)
                acc += v;

            return acc;
        }();

        STATIC_CHECK(result == 28);
    }
}

} // namespace

#endif
