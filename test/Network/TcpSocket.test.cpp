#include "Zancle/Network/TcpSocket.hpp"

// Other 1st party headers
#include "Tst/Tst.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"

#include "Zancle/Chrono/Time.hpp"
#include "Zancle/String/Utf8String.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/Trait/HasVirtualDestructor.hpp"
#include "Zancle/Trait/IsCopyAssignable.hpp"
#include "Zancle/Trait/IsCopyConstructible.hpp"
#include "Zancle/Trait/IsNothrowMoveAssignable.hpp"
#include "Zancle/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Network] za::TcpSocket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZA_IS_COPY_CONSTRUCTIBLE(za::TcpSocket));
        STATIC_CHECK(!ZA_IS_COPY_ASSIGNABLE(za::TcpSocket));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::TcpSocket));
        STATIC_CHECK(ZA_IS_NOTHROW_MOVE_ASSIGNABLE(za::TcpSocket));
        STATIC_CHECK(!ZA_HAS_VIRTUAL_DESTRUCTOR(za::TcpSocket));
    }

    SECTION("Factory construction")
    {
        auto tcpSocketOpt = za::TcpSocket::create(/* isBlocking */ true);
        REQUIRE(tcpSocketOpt.hasValue());

        CHECK(tcpSocketOpt->getLocalPort() == 0);
        CHECK(!tcpSocketOpt->getRemoteAddress().hasValue());
        CHECK(tcpSocketOpt->getRemotePort() == 0);
        CHECK(!tcpSocketOpt->getCurrentCiphersuiteName().hasValue());
    }
}


#ifdef ZA_RUN_CONNECTION_TESTS

TEST_CASE("[Network] za::TcpSocket Connection")
{
    SECTION("Connection")
    {
        const auto githubAddress = za::IpAddressUtils::resolve("github.com");
        REQUIRE(githubAddress.hasValue());

        auto tcpSocketOpt = za::TcpSocket::create(/* isBlocking */ true);
        REQUIRE(tcpSocketOpt.hasValue());

        auto& tcpSocket = *tcpSocketOpt;

        CHECK(tcpSocket.setupTlsServer("", "") == za::TcpSocket::TlsStatus::NotConnected);
        CHECK(tcpSocket.setupTlsClient("") == za::TcpSocket::TlsStatus::NotConnected);

        SECTION("Non-TLS")
        {
            CHECK(tcpSocket.connect(*githubAddress, 80, za::milliseconds(1000)) == za::TcpSocket::Status::Done);
            CHECK_FALSE(tcpSocket.getCurrentCiphersuiteName().hasValue());
        }

        SECTION("TLS")
        {
            CHECK(tcpSocket.connect(*githubAddress, 443, za::milliseconds(1000)) == za::TcpSocket::Status::Done);
            CHECK(tcpSocket.setupTlsClient("github.com") == za::TcpSocket::TlsStatus::HandshakeComplete);

            SECTION("Ciphersuite")
            {
                REQUIRE(tcpSocket.getCurrentCiphersuiteName().hasValue());
                CHECK_FALSE(tcpSocket.getCurrentCiphersuiteName()->empty());
            }
        }
    }
}

#endif
