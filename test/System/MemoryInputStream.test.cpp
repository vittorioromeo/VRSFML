#include "SFML/System/MemoryInputStream.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

#include <string_view>

TEST_CASE("[System] sf::MemoryInputStream")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::MemoryInputStream));
    }

    using namespace std::literals::string_view_literals;

    SECTION("open()")
    {
        static constexpr auto input = "hello world"sv;

        SECTION("Zero length")
        {
            sf::MemoryInputStream memoryInputStream(input.data(), 0);
            CHECK(memoryInputStream.tell().value() == 0);
            CHECK(memoryInputStream.getSize().value() == 0);
        }

        SECTION("Full length")
        {
            sf::MemoryInputStream memoryInputStream(input.data(), input.size());
            CHECK(memoryInputStream.tell().value() == 0);
            CHECK(memoryInputStream.getSize().value() == input.size());
        }
    }

    SECTION("read()")
    {
        static constexpr auto input = "hello world"sv;
        sf::MemoryInputStream memoryInputStream(input.data(), input.size());
        CHECK(memoryInputStream.tell().value() == 0);
        CHECK(memoryInputStream.getSize().value() == input.size());

        // Read within input
        char output[32]{};
        CHECK(memoryInputStream.read(output, 5).value() == 5);
        CHECK(std::string_view(output, 5) == "hello"sv);
        CHECK(memoryInputStream.tell().value() == 5);
        CHECK(memoryInputStream.getSize().value() == input.size());

        // Read beyond input
        CHECK(memoryInputStream.read(output, 100).value() == 6);
        CHECK(std::string_view(output, 6) == " world"sv);
        CHECK(memoryInputStream.tell().value() == 11);
        CHECK(memoryInputStream.getSize().value() == input.size());
    }

    SECTION("seek()")
    {
        static constexpr auto input = "We Love SFML!"sv;
        sf::MemoryInputStream memoryInputStream(input.data(), input.size());
        CHECK(memoryInputStream.tell().value() == 0);
        CHECK(memoryInputStream.getSize().value() == input.size());

        SECTION("Seek within input")
        {
            CHECK(memoryInputStream.seek(0).value() == 0);
            CHECK(memoryInputStream.tell().value() == 0);

            CHECK(memoryInputStream.seek(5).value() == 5);
            CHECK(memoryInputStream.tell().value() == 5);
        }

        SECTION("Seek beyond input")
        {
            CHECK(memoryInputStream.seek(1'000).value() == input.size());
            CHECK(memoryInputStream.tell().value() == input.size());
        }
    }
}
