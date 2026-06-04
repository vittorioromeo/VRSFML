#include "StringifyOptionalUtil.hpp"
#include "StringifyStringViewUtil.hpp"
#include "TemporaryFile.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/System/FileInputStream.hpp"
#include "Zancle/System/Path.hpp"
#include "ZancleBase/Macros.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsDefaultConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


using za::testing::TemporaryFile;

TEST_CASE("[System] za::FileInputStream")
{
    using namespace zb::literals;

    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_DEFAULT_CONSTRUCTIBLE(za::FileInputStream));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::FileInputStream));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::FileInputStream));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::FileInputStream));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::FileInputStream));
    }

    const TemporaryFile temporaryFile("Hello world");
    char                buffer[32];

    SECTION("Move semantics")
    {
        SECTION("Move constructor")
        {
            auto                movedFileInputStream = za::FileInputStream::open(temporaryFile.getPath()).value();
            za::FileInputStream fileInputStream      = ZB_MOVE(movedFileInputStream);
            CHECK(fileInputStream.read(buffer, 6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
            CHECK(fileInputStream.getSize().value() == 11);
            CHECK(zb::StringView(buffer, 6) == "Hello "_sv);
        }

        SECTION("Move assignment")
        {
            auto                movedFileInputStream = za::FileInputStream::open(temporaryFile.getPath()).value();
            const TemporaryFile temporaryFile2("Hello world the sequel");
            auto                fileInputStream = za::FileInputStream::open(temporaryFile2.getPath()).value();
            fileInputStream                     = ZB_MOVE(movedFileInputStream);
            CHECK(fileInputStream.read(buffer, 6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
            CHECK(fileInputStream.getSize().value() == 11);
            CHECK(zb::StringView(buffer, 6) == "Hello "_sv);
        }
    }

    SECTION("Temporary file stream")
    {
        auto fileInputStream = za::FileInputStream::open(temporaryFile.getPath()).value();
        CHECK(fileInputStream.read(buffer, 5).value() == 5);
        CHECK(fileInputStream.tell().value() == 5);
        CHECK(fileInputStream.getSize().value() == 11);
        CHECK(zb::StringView(buffer, 5) == "Hello"_sv);
        CHECK(fileInputStream.seek(6).value() == 6);
        CHECK(fileInputStream.tell().value() == 6);
    }

#ifndef ZA_SYSTEM_EMSCRIPTEN // TODO P1: throws an exception on Emscripten
    SECTION("open()")
    {
        const za::Path filenameSuffixes[] = {U"", U"-ń", U"-🐌"};
        for (const auto& filenameSuffix : filenameSuffixes)
        {
            const za::Path filename = U"test" + filenameSuffix + U".txt";
            INFO("Filename: " << filename.to<zb::String>().cStr());

            auto fileInputStream = za::FileInputStream::open(filename).value();
            CHECK(fileInputStream.read(buffer, 5).value() == 5);
            CHECK(fileInputStream.tell().value() == 5);
            CHECK(fileInputStream.getSize().value() == 12);
            CHECK(zb::StringView(buffer, 5) == "Hello"_sv);
            CHECK(fileInputStream.seek(6).value() == 6);
            CHECK(fileInputStream.tell().value() == 6);
        }
    }
#endif
}
