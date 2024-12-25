#include "SFML/Base/Traits/IsBaseOf.hpp"
#include "SFML/Base/Traits/IsEnum.hpp"
#include "SFML/Base/Traits/IsFloatingPoint.hpp"
#include "SFML/Base/Traits/IsRvalueReference.hpp"
#include "SFML/Base/Traits/IsSame.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"
#include "SFML/Base/Traits/RemoveReference.hpp"
#include "SFML/Base/Traits/UnderlyingType.hpp"


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

    int& i;
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

} // namespace
