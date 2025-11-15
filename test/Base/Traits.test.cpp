#include "SFML/Base/Trait/CommonType.hpp"
#include "SFML/Base/Trait/Decay.hpp"
#include "SFML/Base/Trait/IsBaseOf.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"
#include "SFML/Base/Trait/IsFloatingPoint.hpp"
#include "SFML/Base/Trait/IsRvalueReference.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/Trait/RemoveReference.hpp"
#include "SFML/Base/Trait/UnderlyingType.hpp"


namespace
{

////////////////////////////////////////////////////////////
struct B
{
};

struct D : B
{
};

static_assert(SFML_BASE_IS_BASE_OF(B, D));
static_assert(!SFML_BASE_IS_BASE_OF(D, B));
static_assert(!SFML_BASE_IS_BASE_OF(int, D));


////////////////////////////////////////////////////////////
enum E : int
{
};

enum class EC : int
{
};

static_assert(SFML_BASE_IS_ENUM(E));
static_assert(SFML_BASE_IS_ENUM(EC));
static_assert(!SFML_BASE_IS_ENUM(int));


////////////////////////////////////////////////////////////
static_assert(sf::base::isFloatingPoint<float>);
static_assert(sf::base::isFloatingPoint<double>);
static_assert(sf::base::isFloatingPoint<long double>);
static_assert(!sf::base::isFloatingPoint<int>);
static_assert(!sf::base::isFloatingPoint<float&>);


////////////////////////////////////////////////////////////
static_assert(sf::base::isRvalueReference<int&&>);
static_assert(sf::base::isRvalueReference<const int&&>);
static_assert(!sf::base::isRvalueReference<const int&>);
static_assert(!sf::base::isRvalueReference<int&>);
static_assert(!sf::base::isRvalueReference<int>);


////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(int, int));
static_assert(!SFML_BASE_IS_SAME(int, float));


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

static_assert(SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_COPY_ASSIGNABLE(NonTrivial));

static_assert(SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_COPY_CONSTRUCTIBLE(NonTrivial));

static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_COPYABLE(NonTrivial));

static_assert(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(NonTrivial));

static_assert(SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_MOVE_ASSIGNABLE(NonTrivial));

static_assert(SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(int));
static_assert(!SFML_BASE_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE(NonTrivial));


////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_CVREF(int), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_CVREF(int&), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_CVREF(const int&), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_CVREF(volatile int&), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_CVREF(const volatile int&&), int));


////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_REFERENCE(int), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_REFERENCE(int&), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_REFERENCE(const int&), const int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_REFERENCE(volatile int&), volatile int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_REMOVE_REFERENCE(const volatile int&&), const volatile int));


////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(SFML_BASE_UNDERLYING_TYPE(E), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_UNDERLYING_TYPE(EC), int));


////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(SFML_BASE_COMMON_TYPE(int, int), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_COMMON_TYPE(int, float), float));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_COMMON_TYPE(double, float), double));

////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(int), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(int*), int*));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(int (&)[1]), int*));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(int (&)[2]), int*));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(const int), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(const int&), int));
static_assert(SFML_BASE_IS_SAME(SFML_BASE_DECAY(int&), int));

////////////////////////////////////////////////////////////
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(int));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(char));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(float));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(int*));

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


static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(Custom0));
static_assert(!SFML_BASE_IS_TRIVIALLY_RELOCATABLE(Custom1));


static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(B));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(D));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(E));
static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(EC));
static_assert(!SFML_BASE_IS_TRIVIALLY_RELOCATABLE(NonTrivial));

struct Custom2
{
    ~Custom2() // NOLINT(modernize-use-equals-default)
    {
    }
};

} // namespace


namespace sf::base
{

template <>
inline constexpr bool enableTrivialRelocation<Custom2> = true;

} // namespace sf::base


namespace
{

static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(Custom2));


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

static_assert(SFML_BASE_IS_TRIVIALLY_RELOCATABLE(Custom3));

} // namespace
