#include "StringifyOptionalUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"

#include "SFML/System/FileInputStream.hpp"

#include "SFML/System/Path.hpp"

#include "SFML/Base/Macros.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Trait/IsCopyAssignable.hpp"
#include "SFML/Base/Trait/IsCopyConstructible.hpp"
#include "SFML/Base/Trait/IsDefaultConstructible.hpp"
#include "SFML/Base/Trait/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Trait/IsNothrowMoveConstructible.hpp"


using sf::testing::TemporaryFile;

TEST_CASE("[System] sf::FileInputStream")
{
    using namespace sf::base::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_DEFAULT_CONSTRUCTIBLE(sf::FileInputStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::FileInputStream));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::FileInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::FileInputStream));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::FileInputStream));
    }

    const TemporaryFile temporaryFile("Hello world");
    char                buffer[32];

    SECTION("Move semantics")
    {
        SECTION("Move constructor")
        {
            auto                movedFileInputStream = sf::FileInputStream::open(temporaryFile.getPath()).value();
            sf::FileInputStream fileInputStream      = SFML_BASE_MOVE(movedFileInputStream);
            CHECK(fileInputStream.read(buffer, 6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
            CHECK(fileInputStream.getSize().value() == 11);
            CHECK(sf::base::StringView(buffer, 6) == "Hello "_sv);
        }

        SECTION("Move assignment")
        {
            auto                movedFileInputStream = sf::FileInputStream::open(temporaryFile.getPath()).value();
            const TemporaryFile temporaryFile2("Hello world the sequel");
            auto                fileInputStream = sf::FileInputStream::open(temporaryFile2.getPath()).value();
            fileInputStream                     = SFML_BASE_MOVE(movedFileInputStream);
            CHECK(fileInputStream.read(buffer, 6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
            CHECK(fileInputStream.getSize().value() == 11);
            CHECK(sf::base::StringView(buffer, 6) == "Hello "_sv);
        }
    }

    SECTION("Temporary file stream")
    {
        auto fileInputStream = sf::FileInputStream::open(temporaryFile.getPath()).value();
        CHECK(fileInputStream.read(buffer, 5).value() == 5);
        CHECK(fileInputStream.tell().value() == 5);
        CHECK(fileInputStream.getSize().value() == 11);
        CHECK(sf::base::StringView(buffer, 5) == "Hello"_sv);
        CHECK(fileInputStream.seek(6).value() == 6);
        CHECK(fileInputStream.tell().value() == 6);
    }

#ifndef SFML_SYSTEM_EMSCRIPTEN // TODO P1: throws an exception on Emscripten
    SECTION("open()")
    {
        const sf::Path filenameSuffixes[] = {U"", U"-ń", U"-🐌"};
        for (const auto& filenameSuffix : filenameSuffixes)
        {
            const sf::Path filename = U"test" + filenameSuffix + U".txt";
            INFO("Filename: " << filename.to<sf::base::String>().cStr());

            auto fileInputStream = sf::FileInputStream::open(filename).value();
            CHECK(fileInputStream.read(buffer, 5).value() == 5);
            CHECK(fileInputStream.tell().value() == 5);
            CHECK(fileInputStream.getSize().value() == 12);
            CHECK(sf::base::StringView(buffer, 5) == "Hello"_sv);
            CHECK(fileInputStream.seek(6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
        }
    }
#endif
}
