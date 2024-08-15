#include <SFML/Network/Socket.hpp>

#include <SFML/Base/Macros.hpp>

#include <Doctest.hpp>

#include <CommonTraits.hpp>

class TestSocket : public sf::Socket
{
public:
    TestSocket() : sf::Socket(sf::Socket::Type::Udp, /* isBlocking */ true)
    {
    }

    using sf::Socket::close;
    using sf::Socket::create;
    using sf::Socket::getNativeHandle;
};

TEST_CASE("[Network] sf::Socket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!SFML_BASE_IS_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_IS_COPY_ASSIGNABLE(sf::Socket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_CONSTRUCTIBLE(sf::Socket));
        STATIC_CHECK(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(sf::Socket));
        STATIC_CHECK(!SFML_BASE_HAS_VIRTUAL_DESTRUCTOR(sf::Socket));
    }

    SECTION("Constants")
    {
        STATIC_CHECK(sf::Socket::AnyPort == 0);
    }

    const auto invalidHandle = static_cast<sf::SocketHandle>(-1);

    SECTION("Construction")
    {
        const TestSocket testSocket;
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() == invalidHandle);
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            TestSocket movedTestSocket;
            movedTestSocket.setBlocking(false);
            CHECK(movedTestSocket.create());
            const TestSocket testSocket(SFML_BASE_MOVE(movedTestSocket));
            CHECK(!testSocket.isBlocking());
            CHECK(testSocket.getNativeHandle() != invalidHandle);
        }

        SECTION("Assignment")
        {
            TestSocket movedTestSocket;
            movedTestSocket.setBlocking(false);
            CHECK(movedTestSocket.create());
            TestSocket testSocket;
            testSocket = SFML_BASE_MOVE(movedTestSocket);
            CHECK(!testSocket.isBlocking());
            CHECK(testSocket.getNativeHandle() != invalidHandle);
        }
    }

    SECTION("Set/get blocking")
    {
        TestSocket testSocket;
        testSocket.setBlocking(false);
        CHECK(!testSocket.isBlocking());
    }

    SECTION("create()")
    {
        TestSocket testSocket;
        CHECK(testSocket.create());
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() != invalidHandle);

        // Recreate socket to ensure nothing changed
        CHECK(!testSocket.create()); // Fails because socket was already created
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() != invalidHandle);
    }

    SECTION("close()")
    {
        TestSocket testSocket;
        CHECK(testSocket.create());
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() != invalidHandle);
        CHECK(testSocket.close());
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() == invalidHandle);

        // Reclose socket to ensure nothing changed
        CHECK(!testSocket.close()); // Fails because socket was already closed
        CHECK(testSocket.isBlocking());
        CHECK(testSocket.getNativeHandle() == invalidHandle);
    }
}
