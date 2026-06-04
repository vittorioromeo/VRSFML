#include "Zancle/Network/TcpListener.hpp"

// Other 1st party headers
#include "Tst/Tst.hpp"
#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/TcpSocket.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Network] za::TcpListener")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::TcpListener));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::TcpListener));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::TcpListener));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::TcpListener));
    }

    SECTION("Factory: valid port")
    {
        auto listenerOpt = za::TcpListener::create(za::Socket::AnyPort, /* isBlocking */ true);
        REQUIRE(listenerOpt.hasValue());

        CHECK(listenerOpt->getLocalPort() != 0);
    }

    SECTION("Factory: rejects broadcast address")
    {
        CHECK(!za::TcpListener::create(za::Socket::AnyPort, /* isBlocking */ true, za::IpAddress::Broadcast).hasValue());
    }

    SECTION("accept() without a pending connection (non-blocking)")
    {
        auto listenerOpt = za::TcpListener::create(za::Socket::AnyPort, /* isBlocking */ false);
        REQUIRE(listenerOpt.hasValue());

        const auto result = listenerOpt->accept();
        CHECK(result.status == za::Socket::Status::NotReady);
        CHECK(!result.socket.hasValue());
    }
}
