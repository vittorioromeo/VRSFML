#include "SFML/Audio/SoundFileReader.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>

TEST_CASE("[Audio] sf::SoundFileReader")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::SoundFileReader));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::SoundFileReader));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::SoundFileReader));
        STATIC_CHECK(!SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::SoundFileReader));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::SoundFileReader));
        STATIC_CHECK(SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::SoundFileReader));
    }

    SECTION("Info")
    {
        const sf::SoundFileReader::Info info;
        CHECK(info.sampleCount == 0);
        CHECK(info.channelMap.getSize() == 0);
        CHECK(info.sampleRate == 0);
    }
}
