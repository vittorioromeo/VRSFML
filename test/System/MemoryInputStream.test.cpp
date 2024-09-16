#include "SFML/System/MemoryInputStream.hpp"

#include "SFML/Base/StringView.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

#include <StringifyStringViewUtil.hpp>

TEST_CASE("[System] sf::MemoryInputStream")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::MemoryInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::MemoryInputStream));
    }

    using namespace sf::base::literals;

    SECTION("open()")
    {
        static constexpr auto input = "hello world"_sv;

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
        static constexpr auto input = "hello world"_sv;
        sf::MemoryInputStream memoryInputStream(input.data(), input.size());
        CHECK(memoryInputStream.tell().value() == 0);
        CHECK(memoryInputStream.getSize().value() == input.size());

        // Read within input
        char output[32]{};
        CHECK(memoryInputStream.read(output, 5).value() == 5);
        CHECK(sf::base::StringView(output, 5) == "hello"_sv);
        CHECK(memoryInputStream.tell().value() == 5);
        CHECK(memoryInputStream.getSize().value() == input.size());

        // Read beyond input
        CHECK(memoryInputStream.read(output, 100).value() == 6);
        CHECK(sf::base::StringView(output, 6) == " world"_sv);
        CHECK(memoryInputStream.tell().value() == 11);
        CHECK(memoryInputStream.getSize().value() == input.size());
    }

    SECTION("seek()")
    {
        static constexpr auto input = "We Love SFML!"_sv;
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
