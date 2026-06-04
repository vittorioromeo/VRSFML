#include "Tst/Tst.hpp"

#include "Zancle/Network/Socket.hpp"

#include "Zancle/Network/UdpSocket.hpp"

#include "ZancleBase/Macros.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Trait/HasVirtualDestructor.hpp"
#include "ZancleBase/Trait/IsConstructible.hpp"
#include "ZancleBase/Trait/IsCopyAssignable.hpp"
#include "ZancleBase/Trait/IsCopyConstructible.hpp"
#include "ZancleBase/Trait/IsNothrowMoveAssignable.hpp"
#include "ZancleBase/Trait/IsNothrowMoveConstructible.hpp"


TEST_CASE("[Network] za::Socket")
{
    SECTION("Type traits")
    {
        STATIC_CHECK(!ZB_IS_CONSTRUCTIBLE(za::Socket));
        STATIC_CHECK(!ZB_IS_COPY_CONSTRUCTIBLE(za::Socket));
        STATIC_CHECK(!ZB_IS_COPY_ASSIGNABLE(za::Socket));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_CONSTRUCTIBLE(za::Socket));
        STATIC_CHECK(ZB_IS_NOTHROW_MOVE_ASSIGNABLE(za::Socket));
        STATIC_CHECK(!ZB_HAS_VIRTUAL_DESTRUCTOR(za::Socket));
    }

    SECTION("Constants")
    {
        STATIC_CHECK(za::Socket::AnyPort == 0);
    }

    // `za::Socket` is abstract (protected ctor); use `za::UdpSocket` to exercise
    // the move/blocking behaviour inherited from the base.
    SECTION("Factory produces a valid socket")
    {
        auto socketOpt = za::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(socketOpt.hasValue());

        CHECK(socketOpt->isBlocking());

        // Unbound: local port is 0 until `bind` is called.
        CHECK(socketOpt->getLocalPort() == 0);
    }

    SECTION("Set/get blocking")
    {
        auto socketOpt = za::UdpSocket::create(/* isBlocking */ true);
        REQUIRE(socketOpt.hasValue());

        socketOpt->setBlocking(false);
        CHECK(!socketOpt->isBlocking());
    }

    SECTION("Move semantics")
    {
        SECTION("Construction")
        {
            auto movedOpt = za::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(movedOpt.hasValue());

            movedOpt->setBlocking(false);


            const za::UdpSocket socket(ZB_MOVE(*movedOpt));
            CHECK(!socket.isBlocking());
        }

        SECTION("Assignment")
        {
            auto movedOpt = za::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(movedOpt.hasValue());

            movedOpt->setBlocking(false);

            auto targetOpt = za::UdpSocket::create(/* isBlocking */ true);
            REQUIRE(targetOpt.hasValue());

            *targetOpt = ZB_MOVE(*movedOpt);
            CHECK(!targetOpt->isBlocking());
        }
    }
}
