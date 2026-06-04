#include "Zancle/Network/Packet.hpp"

// Other 1st party headers
#include "SystemUtil.hpp"
#include "Tst/Tst.hpp"
#include "Zancle/System/Utf8String.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"
#include "ZancleBase/Trait/RemoveConst.hpp"
#include "ZancleBase/Vector.hpp"

#include <limits>
#include <string>

#include <cstddef>


#define CHECK_PACKET_STREAM_OPERATORS(expected)              \
    do                                                       \
    {                                                        \
        za::Packet packet;                                   \
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

#define CHECK_PACKET_NARROW_STRING_STREAM_OPERATORS(expected, size) \
    do                                                              \
    {                                                               \
        za::Packet packet;                                          \
        packet << (expected);                                       \
        CHECK(packet.getReadPosition() == 0);                       \
        CHECK(packet.getData() != nullptr);                         \
        CHECK(packet.getDataSize() == (size));                      \
        CHECK(!packet.endOfPacket());                               \
        CHECK(bool{packet});                                        \
                                                                    \
        ZB_REMOVE_CONST(decltype(expected)) received;               \
        packet >> received;                                         \
        CHECK(packet.getReadPosition() == (size));                  \
        CHECK(packet.getData() != nullptr);                         \
        CHECK(packet.getDataSize() == (size));                      \
        CHECK(packet.endOfPacket());                                \
        CHECK(bool{packet});                                        \
        CHECK(zb::String{expected} == zb::String{received});        \
    } while (false)

#define CHECK_PACKET_WIDE_STRING_STREAM_OPERATORS(expected, size)                               \
    do                                                                                          \
    {                                                                                           \
        za::Packet packet;                                                                      \
        packet << (expected);                                                                   \
        CHECK(packet.getReadPosition() == 0);                                                   \
        CHECK(packet.getData() != nullptr);                                                     \
        CHECK(packet.getDataSize() == (size));                                                  \
        CHECK(!packet.endOfPacket());                                                           \
        CHECK(bool{packet});                                                                    \
                                                                                                \
        ZB_REMOVE_CONST(decltype(expected)) received;                                           \
        packet >> received;                                                                     \
        CHECK(packet.getReadPosition() == (size));                                              \
        CHECK(packet.getData() != nullptr);                                                     \
        CHECK(packet.getDataSize() == (size));                                                  \
        CHECK(packet.endOfPacket());                                                            \
        CHECK(bool{packet});                                                                    \
        /* `std::wstring` is not stringifiable by the test framework, so compare via `bool`. */ \
        const bool roundTripEqual = std::wstring{expected} == std::wstring{received};           \
        CHECK(roundTripEqual);                                                                  \
    } while (false)

struct Packet : za::Packet
{
    using za::Packet::onReceive;
    using za::Packet::onSend;
};

TEST_CASE("[Network] za::Packet")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(ZB_IS_COPY_CONSTRUCTIBLE(za::Packet));
        STATIC_CHECK(ZB_IS_COPY_ASSIGNABLE(za::Packet));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Packet));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Packet));
    }

    SECTION("Default constructor")
    {
        const za::Packet packet;
        CHECK(packet.getReadPosition() == 0);
        CHECK(packet.getData() == nullptr);
        CHECK(packet.getDataSize() == 0);
        CHECK(packet.endOfPacket());
        CHECK(bool{packet});
    }

    static constexpr int data[]{1, 2, 3, 4, 5, 6};

    SECTION("Append and clear")
    {
        za::Packet packet;
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

    SECTION("Wire byte order (little-endian, host order)")
    {
        za::Packet packet;

        SECTION("16 bit int")
        {
            packet << zb::U16{12'345};
            const auto*                 dataPtr = static_cast<const std::byte*>(packet.getData());
            const zb::Vector<std::byte> bytes(dataPtr, dataPtr + packet.getDataSize());
            const zb::Vector<std::byte> expectedBytes{std::byte{0x39}, std::byte{0x30}};
            CHECK((bytes == expectedBytes));
        }

        SECTION("32 bit int")
        {
            packet << zb::U32{1'234'567'890};
            const auto*                 dataPtr = static_cast<const std::byte*>(packet.getData());
            const zb::Vector<std::byte> bytes(dataPtr, dataPtr + packet.getDataSize());
            const zb::Vector<std::byte> expectedBytes{std::byte{0xD2}, std::byte{0x02}, std::byte{0x96}, std::byte{0x49}};
            CHECK((bytes == expectedBytes));
        }

        SECTION("float")
        {
            packet << 123.456f;
            const auto*                 dataPtr = static_cast<const std::byte*>(packet.getData());
            const zb::Vector<std::byte> bytes(dataPtr, dataPtr + packet.getDataSize());
            const zb::Vector<std::byte> expectedBytes{std::byte{0x79}, std::byte{0xe9}, std::byte{0xf6}, std::byte{0x42}};
            CHECK((bytes == expectedBytes));
        }

        SECTION("double")
        {
            packet << 789.123;
            const auto*                 dataPtr = static_cast<const std::byte*>(packet.getData());
            const zb::Vector<std::byte> bytes(dataPtr, dataPtr + packet.getDataSize());
            const zb::Vector<std::byte> expectedBytes{std::byte{0x44},
                                                      std::byte{0x8b},
                                                      std::byte{0x6c},
                                                      std::byte{0xe7},
                                                      std::byte{0xfb},
                                                      std::byte{0xa8},
                                                      std::byte{0x88},
                                                      std::byte{0x40}};
            CHECK((bytes == expectedBytes));
        }
    }

    SECTION("Stream operators")
    {
        SECTION("bool")
        {
            CHECK_PACKET_STREAM_OPERATORS(true);
            CHECK_PACKET_STREAM_OPERATORS(false);
        }

        SECTION("zb::I8")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::I8(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::I8(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I8>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I8>::max());
        }

        SECTION("zb::U8")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::U8(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::U8(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U8>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U8>::max());
        }

        SECTION("zb::I16")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::I16(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::I16(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I16>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I16>::max());
        }

        SECTION("zb::U16")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::U16(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::U16(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U16>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U16>::max());
        }

        SECTION("zb::I32")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::I32(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::I32(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I32>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I32>::max());
        }

        SECTION("zb::U32")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::U32(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::U32(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U32>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U32>::max());
        }

        SECTION("zb::I64")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::I64(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::I64(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I64>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::I64>::max());
        }

        SECTION("zb::U64")
        {
            CHECK_PACKET_STREAM_OPERATORS(zb::U64(0));
            CHECK_PACKET_STREAM_OPERATORS(zb::U64(1));
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U64>::min());
            CHECK_PACKET_STREAM_OPERATORS(std::numeric_limits<zb::U64>::max());
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

        SECTION("std::string")
        {
            const std::string string = "testing";
            CHECK_PACKET_NARROW_STRING_STREAM_OPERATORS(string, string.size() + 4);
        }

        SECTION("zb::String")
        {
            const zb::String string = "testing";
            CHECK_PACKET_NARROW_STRING_STREAM_OPERATORS(string, string.size() + 4);
        }

        SECTION("std::wstring")
        {
            const std::wstring string = L"testing";
            CHECK_PACKET_WIDE_STRING_STREAM_OPERATORS(string, 4 * string.size() + 4);
        }

        SECTION("za::Utf8String")
        {
            const za::Utf8String string = "testing";
            // New wire format: 4-byte length + UTF-8 byte payload.
            CHECK_PACKET_NARROW_STRING_STREAM_OPERATORS(string, string.byteSize() + 4);
        }
    }

    SECTION("onSend")
    {
        Packet    packet;
        zb::SizeT size = 0;
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

    SECTION("Attempt overflow")
    {
        static constexpr struct
        {
            zb::U32 length{std::numeric_limits<decltype(length)>::max()};
            char    data[4]{'S', 'F', 'M', 'L'};
        } string;

        za::Packet packet;
        packet.append(&string, sizeof(string));

        std::string out;
        packet >> out; // Ensure this does not trigger a crash
        CHECK(out.empty());
    }
}
