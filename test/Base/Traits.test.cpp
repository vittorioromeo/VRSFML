#include "ZancleBase/Trait/CommonType.hpp"
#include "ZancleBase/Trait/Decay.hpp"
#include "ZancleBase/Trait/IsBaseOf.hpp"
#include "ZancleBase/Trait/IsEnum.hpp"
#include "ZancleBase/Trait/IsFloatingPoint.hpp"
#include "ZancleBase/Trait/IsRvalueReference.hpp"
#include "ZancleBase/Trait/IsSame.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyCopyable.hpp"
#include "ZancleBase/Trait/IsTriviallyDestructible.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveAssignable.hpp"
#include "ZancleBase/Trait/IsTriviallyMoveConstructible.hpp"
#include "ZancleBase/Trait/IsTriviallyRelocatable.hpp"
#include "ZancleBase/Trait/IsUnsigned.hpp"
#include "ZancleBase/Trait/RemoveCVRef.hpp"
#include "ZancleBase/Trait/RemoveReference.hpp"
#include "ZancleBase/Trait/UnderlyingType.hpp"


namespace
{
namespace TraitsTest // for unity builds
{
////////////////////////////////////////////////////////////
static_assert(!zb::isUnsigned<float>);
static_assert(!zb::isUnsigned<char>);
static_assert(!zb::isUnsigned<int>);
static_assert(zb::isUnsigned<bool>);
static_assert(zb::isUnsigned<unsigned char>);
static_assert(zb::isUnsigned<unsigned int>);


////////////////////////////////////////////////////////////
struct B
{
};

struct D : B
{
};

static_assert(ZB_IS_BASE_OF(B, D));
static_assert(!ZB_IS_BASE_OF(D, B));
static_assert(!ZB_IS_BASE_OF(int, D));


////////////////////////////////////////////////////////////
enum E : int
{
};

enum class EC : int
{
};

static_assert(ZB_IS_ENUM(E));
static_assert(ZB_IS_ENUM(EC));
static_assert(!ZB_IS_ENUM(int));


////////////////////////////////////////////////////////////
static_assert(zb::isFloatingPoint<float>);
static_assert(zb::isFloatingPoint<double>);
static_assert(zb::isFloatingPoint<long double>);
static_assert(!zb::isFloatingPoint<int>);
static_assert(!zb::isFloatingPoint<float&>);


////////////////////////////////////////////////////////////
static_assert(zb::isRvalueReference<int&&>);
static_assert(zb::isRvalueReference<const int&&>);
static_assert(!zb::isRvalueReference<const int&>);
static_assert(!zb::isRvalueReference<int&>);
static_assert(!zb::isRvalueReference<int>);


////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(int, int));
static_assert(!ZB_IS_SAME(int, float));


////////////////////////////////////////////////////////////
struct NonTrivial
{
    static inline int si{};

    NonTrivial(const NonTrivial&) : i(si)
    {
    }

    // NOLINTNEXTLINE(modernize-use-equals-default)
    ~NonTrivial()
    {
    }

    int& i; // NOLINT(cppcoreguidelines-use-default-member-init, modernize-use-default-member-init)
};

static_assert(ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(int));
static_assert(!ZB_IS_TRIVIALLY_COPY_ASSIGNABLE(NonTrivial));

static_assert(ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(int));
static_assert(!ZB_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(NonTrivial));

static_assert(ZB_IS_TRIVIALLY_COPYABLE(int));
static_assert(!ZB_IS_TRIVIALLY_COPYABLE(NonTrivial));

static_assert(ZB_IS_TRIVIALLY_DESTRUCTIBLE(int));
static_assert(!ZB_IS_TRIVIALLY_DESTRUCTIBLE(NonTrivial));

static_assert(ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(int));
static_assert(!ZB_IS_TRIVIALLY_MOVE_ASSIGNABLE(NonTrivial));

static_assert(ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(int));
static_assert(!ZB_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(NonTrivial));


////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(ZB_REMOVE_CVREF(int), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_CVREF(int&), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_CVREF(const int&), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_CVREF(volatile int&), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_CVREF(const volatile int&&), int));


////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(ZB_REMOVE_REFERENCE(int), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_REFERENCE(int&), int));
static_assert(ZB_IS_SAME(ZB_REMOVE_REFERENCE(const int&), const int));
static_assert(ZB_IS_SAME(ZB_REMOVE_REFERENCE(volatile int&), volatile int));
static_assert(ZB_IS_SAME(ZB_REMOVE_REFERENCE(const volatile int&&), const volatile int));


////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(ZB_UNDERLYING_TYPE(E), int));
static_assert(ZB_IS_SAME(ZB_UNDERLYING_TYPE(EC), int));


////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(ZB_COMMON_TYPE(int, int), int));
static_assert(ZB_IS_SAME(ZB_COMMON_TYPE(int, float), float));
static_assert(ZB_IS_SAME(ZB_COMMON_TYPE(double, float), double));

////////////////////////////////////////////////////////////
static_assert(ZB_IS_SAME(ZB_DECAY(int), int));
static_assert(ZB_IS_SAME(ZB_DECAY(int*), int*));
static_assert(ZB_IS_SAME(ZB_DECAY(int (&)[1]), int*));
static_assert(ZB_IS_SAME(ZB_DECAY(int (&)[2]), int*));
static_assert(ZB_IS_SAME(ZB_DECAY(const int), int));
static_assert(ZB_IS_SAME(ZB_DECAY(const int&), int));
static_assert(ZB_IS_SAME(ZB_DECAY(int&), int));

////////////////////////////////////////////////////////////
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(int));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(char));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(float));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(int*));

////////////////////////////////////////////////////////////
struct Custom0
{
};

struct Custom1
{
    ~Custom1() // NOLINT(modernize-use-equals-default)
    {
    }
};


static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(Custom0));
static_assert(!ZB_IS_TRIVIALLY_RELOCATABLE(Custom1));


static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(B));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(D));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(E));
static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(EC));
static_assert(!ZB_IS_TRIVIALLY_RELOCATABLE(NonTrivial));

struct Custom2
{
    ~Custom2() // NOLINT(modernize-use-equals-default)
    {
    }
};

} // namespace TraitsTest
} // namespace


namespace zb
{

template <>
inline constexpr bool enableTrivialRelocation<TraitsTest::Custom2> = true;

} // namespace zb


namespace
{

static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(TraitsTest::Custom2));


struct Custom3
{
    enum : bool
    {
        enableTrivialRelocation = true
    };

    ~Custom3() // NOLINT(modernize-use-equals-default)
    {
    }
};

static_assert(ZB_IS_TRIVIALLY_RELOCATABLE(Custom3));

} // namespace
