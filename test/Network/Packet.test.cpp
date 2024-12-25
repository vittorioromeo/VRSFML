#include "SFML/Network/Packet.hpp"

// Other 1st party headers
#include "SFML/System/String.hpp"

#include "SFML/Base/Builtins/Strlen.hpp"
#include "SFML/Base/SizeT.hpp"

#include <Doctest.hpp>

#include <CommonTraits.hpp>
#include <StringifyVectorUtil.hpp>
#include <SystemUtil.hpp>

#include <limits>
#include <string>
#include <vector>

#include <cwchar>

#define CHECK_PACKET_STREAM_OPERATORS(expected)              \
    do                                                       \
    {                                                        \
        sf::Packet packet;                                   \
        packet << (expected);                                \
        CHECK(packet.getReadPosition() == 0);                \
        CHECK(packet.getData() != nullptr);                  \
        CHECK(packet.getDataSize() == sizeof(expected));     \
        CHECK(!packet.endOfPacket());                        \
        CHECK(bool{packet});                                 \
                                                             \
        decltype(expected) received;                         \
        packet >> received;                                  \
        CHECK(packet.getReadPosition() == sizeof(expected)); \
        CHECK(packet.getData() != nullptr);                  \
        CHECK(packet.getDataSize() == sizeof(expected));     \
        CHECK(packet.endOfPacket());                         \
        CHECK(bool{packet});                                 \
        CHECK((expected) == received);                       \
    } while (false)

#define CHECK_PACKET_STRING_STREAM_OPERATORS(expected, size) \
    do                                                       \
    {                                                        \
        sf::Packet packet;                                   \
        packet << (expected);                                \
        CHECK(packet.getReadPosition() == 0);                \
        CHECK(packet.getData() != nullptr);                  \
        CHECK(packet.getDataSize() == (size));               \
        CHECK(!packet.endOfPacket());                        \
        CHECK(bool{packet});                                 \
                                                             \
        std::remove_const_t<decltype(expected)> received;    \
        packet >> received;                                  \
        CHECK(packet.getReadPosition() == (size));           \
        CHECK(packet.getData() != nullptr);                  \
        CHECK(packet.getDataSize() == (size));               \
        CHECK(packet.endOfPacket());                         \
        CHECK(bool{packet});                                 \
        CHECK(sf::String(expected) == sf::String(received)); \
    } while (false)

struct Packet : sf::Packet
{
    using sf::Packet::onReceive;
    using sf::Packet::onSend;
};

TEST_CASE("[Network] sf::Packet")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Packet));
        STATIC_CHECK(SFML_BASE_IS_COPY_ASSIGNABLE(sf::Packet));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Packet));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Packet));
    }

    SECTION("Default constructor")
    {
        const sf::Packet packet;
        CHECK(packet.getReadPosition() == 0);
        CHECK(packet.getData() == nullptr);
        CHECK(packet.getDataSize() == 0);
        CHECK(packet.endOfPacket());
        CHECK(bool{packet});
    }

    static constexpr int data[]{1, 2, 3, 4, 5, 6};

    SECTION("Append and clear")
    {
        sf::Packet packet;
        packet.append(data, 6);
        CHECK(packet.getReadPosition() == 0);
        CHECK(packet.getData() != nullptr);
        CHECK(packet.getDataSize() == 6);
        CHECK(!packet.endOfPacket());
        CHECK(bool{packet});

        packet.clear();
        CHECK(packet.getReadPosition() == 0);
        CHECK(packet.getData() == nullptr);
        CHECK(packet.getDataSize() == 0);
        CHECK(packet.endOfPacket());
        CHECK(bool{packet});
    }

    SECTION("Network ordering")
    {
        sf::Packet packet;

        SECTION("16 bit int")
        {
            packet << sf::base::U16{12'345};
            const auto*       dataPtr = static_cast<const std::byte*>(packet.getData());
            const std::vector bytes(dataPtr, dataPtr + packet.getDataSize());
            const std::vector expectedBytes{std::byte{0x30}, std::byte{0x39}};
            CHECK(bytes == expectedBytes);
        }

        SECTION("32 bit int")
        {
            packet << sf::base::U32{1'234'567'890};
            const auto*       dataPtr = static_cast<const std::byte*>(packet.getData());
            const std::vector bytes(dataPtr, dataPtr + packet.getDataSize());
            const std::vector expectedBytes{std::byte{0x49}, std::byte{0x96}, std::byte{0x02}, std::byte{0xD2}};
            CHECK(bytes == expectedBytes);
        }

        SECTION("float")
        {
            packet << 123.456f;
            const auto*       dataPtr = static_cast<const std::byte*>(packet.getData());
            const std::vector bytes(dataPtr, dataPtr + packet.getDataSize());
            const std::vector expectedBytes{std::byte{0x79}, std::byte{0xe9}, std::byte{0xf6}, std::byte{0x42}};
            CHECK(bytes == expectedBytes);
        }

        SECTION("double")
        {
            packet << 789.123;
            const auto*       dataPtr = static_cast<const std::byte*>(packet.getData());
            const std::vector bytes(dataPtr, dataPtr + packet.getDataSize());
            const std::vector expectedBytes{std::byte{0x44},
                                            std::byte{0x8b},
                                            std::byte{0x6c},
                                            std::byte{0xe7},
                                            std::byte{0xfb},
                                            std::byte{0xa8},
                                            std::byte{0x88},
                                            std::byte{0x40}};
            CHECK(bytes == expectedBytes);
        }
    }

    SECTION("Stream operators")
    {
        SECTION("bool")
        {
            CHECK_PACKET_STREAM_OPERATORS(true);
            CHECK_PACKET_STREAM_OPERATORS(false);
        }

        SECTION("sf::base::I8")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I8(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I8(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I8>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I8>::max());
        }

        SECTION("sf::base::U8")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U8(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U8(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U8>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U8>::max());
        }

        SECTION("sf::base::I16")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I16(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I16(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I16>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I16>::max());
        }

        SECTION("sf::base::U16")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U16(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U16(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U16>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U16>::max());
        }

        SECTION("sf::base::I32")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I32(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I32(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I32>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I32>::max());
        }

        SECTION("sf::base::U32")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U32(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U32(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U32>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U32>::max());
        }

        SECTION("sf::base::I64")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I64(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::I64(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I64>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::I64>::max());
        }

        SECTION("sf::base::U64")
        {
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U64(0));
            CHECK_PACKET_STREAM_OPERATORS(sf::base::U64(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U64>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<sf::base::U64>::max());
        }

        SECTION("float")
        {
            CHECK_PACKET_STREAM_OPERATORS(0.f);
            CHECK_PACKET_STREAM_OPERATORS(1.f);
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<float>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<float>::max());
        }

        SECTION("double")
        {
            CHECK_PACKET_STREAM_OPERATORS(0.);
            CHECK_PACKET_STREAM_OPERATORS(1.);
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<double>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<double>::max());
        }

        SECTION("char*")
        {
            const char string[] = "testing";
            CHECK_PACKET_STRING_STREAM_OPERATORS(string, SFML_BASE_STRLEN(string) + 4);
        }

        SECTION("std::string")
        {
            const std::string string = "testing";
            CHECK_PACKET_STRING_STREAM_OPERATORS(string, string.size() + 4);
        }

        SECTION("wchar_t*")
        {
            const wchar_t string[] = L"testing";
            CHECK_PACKET_STRING_STREAM_OPERATORS(string, 4 * std::wcslen(string) + 4);
        }

        SECTION("std::wstring")
        {
            const std::wstring string = L"testing";
            CHECK_PACKET_STRING_STREAM_OPERATORS(string, 4 * string.size() + 4);
        }

        SECTION("sf::String")
        {
            const sf::String string = "testing";
            CHECK_PACKET_STRING_STREAM_OPERATORS(string, 4 * string.getSize() + 4);
        }
    }

    SECTION("onSend")
    {
        Packet          packet;
        sf::base::SizeT size = 0;
        CHECK(packet.onSend(size) == nullptr);
        CHECK(size == 0);

        packet.append(data, 6);
        CHECK(packet.onSend(size) != nullptr);
        CHECK(size == 6);
    }

    SECTION("onReceive")
    {
        Packet packet;
        packet.onReceive(data, 6);
        CHECK(packet.getReadPosition() == 0);
        CHECK(packet.getData() != nullptr);
        CHECK(packet.getDataSize() == 6);
    }
}
