#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/AddConst.hpp"
#include "SFML/Base/Trait/AddLvalueReference.hpp"
#include "SFML/Base/Trait/IsReference.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/TypePackElement.hpp"
#include "SFML/Base/TypePackIndex.hpp"


////////////////////////////////////////////////////////////
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
[[nodiscard]] consteval auto variadicMax(auto x, auto... xs) noexcept
{
    if constexpr (sizeof...(xs) == 0)
    {
        return x;
    }
    else
    {
        decltype(x) result = x;
        decltype(x) rest[]{xs...};

        for (auto value : rest)
            if (result < value)
                result = value;

        return result;
    }
}


////////////////////////////////////////////////////////////
template <typename>
struct InPlaceType
{
};


////////////////////////////////////////////////////////////
template <SizeT>
struct InPlaceIndex
{
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr priv::InPlaceType<T> inPlaceType{};


////////////////////////////////////////////////////////////
template <SizeT N>
inline constexpr priv::InPlaceIndex<N> inPlaceIndex{};


////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_NTH_TYPE(i) SFML_BASE_TYPE_PACK_ELEMENT(i, Alternatives...)


////////////////////////////////////////////////////////////
template <typename... Alternatives>
class [[nodiscard]] Variant
{
    static_assert(sizeof...(Alternatives) < 255);

private:
    ////////////////////////////////////////////////////////////
    using Byte = unsigned char;


    ////////////////////////////////////////////////////////////
    enum : SizeT
    {
        alternativeCount = sizeof...(Alternatives),
        maxAlignment     = priv::variadicMax(alignof(Alternatives)...),
        maxSize          = priv::variadicMax(sizeof(Alternatives)...)
    };


    ////////////////////////////////////////////////////////////
    static inline constexpr bool triviallyDestructible      = (... && isTriviallyDestructible<Alternatives>);
    static inline constexpr bool triviallyCopyConstructible = (... && isTriviallyCopyConstructible<Alternatives>);
    static inline constexpr bool triviallyMoveConstructible = (... && isTriviallyMoveConstructible<Alternatives>);
    static inline constexpr bool triviallyCopyAssignable    = (... && isTriviallyCopyAssignable<Alternatives>);
    static inline constexpr bool triviallyMoveAssignable    = (... && isTriviallyMoveAssignable<Alternatives>);


    ////////////////////////////////////////////////////////////
    using IndexType = unsigned char; // Support up to 255 alternatives


public:
    ////////////////////////////////////////////////////////////
    template <typename T>
    static constexpr SizeT indexOf = getTypePackIndex<T, Alternatives...>();


private:
    ////////////////////////////////////////////////////////////
    static constexpr MakeIndexSequence<alternativeCount> alternativeIndexSequence{};


    ////////////////////////////////////////////////////////////
    alignas(maxAlignment) Byte m_buffer[maxSize];
    IndexType m_index;


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I)                                            \
    static_assert((I) != ::sf::base::badTypePackIndex, "Alternative type not supported by variant"); \
    static_assert((I) >= 0 && (I) < alternativeCount, "Alternative index out of range")


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(obj, Is, ...)                               \
    do                                                                                          \
    {                                                                                           \
        [&]<SizeT... Is>(IndexSequence<Is...>) SFML_BASE_LAMBDA_ALWAYS_INLINE                   \
        { (..., (((obj).m_index == Is) ? ((__VA_ARGS__), 0) : 0)); }(alternativeIndexSequence); \
    } while (false)


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(Is, ...) \
    SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ((*this), Is, __VA_ARGS__)


    ////////////////////////////////////////////////////////////
    template <typename T, SizeT I, typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit Variant(const priv::InPlaceType<T>, priv::InPlaceIndex<I>, Args&&... args) noexcept
        :
        m_index{static_cast<IndexType>(I)}
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        SFML_BASE_PLACEMENT_NEW(m_buffer) T{static_cast<Args&&>(args)...};
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[gnu::always_inline]] void destroyAt() noexcept
    {
        using Type = SFML_BASE_VARIANT_NTH_TYPE(I);
        as<Type>().~Type();
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursiveVisitImpl(Visitor&& visitor)
    {
        if constexpr (I < sizeof...(Alternatives) - 1)
        {
            return (m_index == I) ? visitor(getByIndex<I>())
                                  : recursiveVisitImpl<I + 1, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return visitor(getByIndex<I>());
        }
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursiveVisitImplOpt5(Visitor&& visitor)
    {
        if constexpr (I == 0 && sizeof...(Alternatives) == 5)
        {
            // clang-format off
            return (m_index == I + 0) ? visitor(getByIndex<I + 0>()) :
                   (m_index == I + 1) ? visitor(getByIndex<I + 1>()) :
                   (m_index == I + 2) ? visitor(getByIndex<I + 2>()) :
                   (m_index == I + 3) ? visitor(getByIndex<I + 3>()) :
                                       visitor(getByIndex<I + 4>()) ;
            // clang-format on
        }
        else if constexpr (I + 4 < sizeof...(Alternatives))
        {
            // clang-format off
            return (m_index == I + 0) ? visitor(getByIndex<I + 0>()) :
                   (m_index == I + 1) ? visitor(getByIndex<I + 1>()) :
                   (m_index == I + 2) ? visitor(getByIndex<I + 2>()) :
                   (m_index == I + 3) ? visitor(getByIndex<I + 3>()) :
                   (m_index == I + 4) ? visitor(getByIndex<I + 4>()) :
                   recursiveVisitImplOpt5<I + 5, R>(static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursiveVisitImpl<I, R>(static_cast<Visitor&&>(visitor));
        }
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursiveVisitImplOpt10(Visitor&& visitor)
    {
        if constexpr (I + 9 < sizeof...(Alternatives))
        {
            // clang-format off
            return (m_index == I + 0) ? visitor(getByIndex<I + 0>()) :
                   (m_index == I + 1) ? visitor(getByIndex<I + 1>()) :
                   (m_index == I + 2) ? visitor(getByIndex<I + 2>()) :
                   (m_index == I + 3) ? visitor(getByIndex<I + 3>()) :
                   (m_index == I + 4) ? visitor(getByIndex<I + 4>()) :
                   (m_index == I + 5) ? visitor(getByIndex<I + 5>()) :
                   (m_index == I + 6) ? visitor(getByIndex<I + 6>()) :
                   (m_index == I + 7) ? visitor(getByIndex<I + 7>()) :
                   (m_index == I + 8) ? visitor(getByIndex<I + 8>()) :
                   (m_index == I + 9) ? visitor(getByIndex<I + 9>()) :
                   recursiveVisitImplOpt10<I + 10, R>(static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursiveVisitImplOpt5<I, R>(static_cast<Visitor&&>(visitor));
        }
    }


public:
    ////////////////////////////////////////////////////////////
    template <typename T, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit Variant(const priv::InPlaceType<T> inPlaceType, Args&&... args) noexcept :
        Variant{inPlaceType, inPlaceIndex<indexOf<T>>, static_cast<Args&&>(args)...}
    {
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit Variant(const priv::InPlaceIndex<I> inPlaceIndex, Args&&... args) noexcept :
        Variant{inPlaceType<SFML_BASE_VARIANT_NTH_TYPE(I)>, inPlaceIndex, static_cast<Args&&>(args)...}
    {
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] explicit Variant(T&& x) noexcept
        requires(!isSame<RemoveCVRefIndirect<T>, Variant>)
        : Variant{inPlaceType<RemoveCVRefIndirect<T>>, static_cast<T&&>(x)}
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] explicit Variant() noexcept : Variant{inPlaceIndex<0>}
    {
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[gnu::always_inline]] Variant(const Variant& rhs)
        requires(!triviallyCopyConstructible)
        : m_index{rhs.m_index}
    {
        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I,
                                                SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(
                                                    I)(static_cast<const SFML_BASE_VARIANT_NTH_TYPE(I) &>(
                                                    *SFML_BASE_LAUNDER_CAST(const SFML_BASE_VARIANT_NTH_TYPE(I)*,
                                                                            rhs.m_buffer))));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant(const Variant& rhs)
        requires(triviallyCopyConstructible)
    = default;


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[gnu::always_inline]] Variant(Variant&& rhs) noexcept
        requires(!triviallyMoveConstructible)
        : m_index{rhs.m_index}
    {
        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I,
                                                SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(
                                                    I)(static_cast<SFML_BASE_VARIANT_NTH_TYPE(I) &&>(
                                                    *SFML_BASE_LAUNDER_CAST(SFML_BASE_VARIANT_NTH_TYPE(I)*, rhs.m_buffer))));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant(Variant&& rhs) noexcept
        requires(triviallyMoveConstructible)
    = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~Variant()
        requires(!triviallyDestructible)
    {
        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] ~Variant()
        requires(triviallyDestructible)
    = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant& operator=(const Variant& rhs)
        requires(!triviallyCopyAssignable)
    {
        if (this == &rhs)
            return *this;

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                                    I,
                                                    (SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(I)(
                                                        rhs.template as<SFML_BASE_VARIANT_NTH_TYPE(I)>())));
        m_index = rhs.m_index;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant& operator=(const Variant& rhs)
        requires(triviallyCopyAssignable)
    = default;


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant& operator=(Variant&& rhs) noexcept
        requires(!triviallyMoveAssignable)
    {
        if (this == &rhs)
            return *this;

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                                    I,
                                                    (SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(
                                                        I)(static_cast<SFML_BASE_VARIANT_NTH_TYPE(I) &&>(
                                                        rhs.template as<SFML_BASE_VARIANT_NTH_TYPE(I)>()))));
        m_index = rhs.m_index;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant& operator=(Variant&& rhs) noexcept
        requires(triviallyMoveAssignable)
    = default;


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline]] Variant& operator=(T&& x)
        requires(!isSame<RemoveCVRefIndirect<T>, Variant>)
    {
        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        using Type = SFML_BASE_REMOVE_CVREF(T);

        SFML_BASE_PLACEMENT_NEW(m_buffer) Type{static_cast<T&&>(x)};
        m_index = indexOf<Type>;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] bool is() const noexcept
    {
        return m_index == indexOf<T>;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool hasIndex(const IndexType index) const noexcept
    {
        return m_index == index;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] T* getIf() noexcept
    {
        return m_index == indexOf<T> ? SFML_BASE_LAUNDER_CAST(T*, m_buffer) : nullptr;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] const T* getIf() const noexcept
    {
        return m_index == indexOf<T> ? SFML_BASE_LAUNDER_CAST(const T*, m_buffer) : nullptr;
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] T& as() & noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] const T& as() const& noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(const T*, m_buffer);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] T&& as() && noexcept
    {
        return static_cast<T&&>(*SFML_BASE_LAUNDER_CAST(T*, m_buffer));
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[nodiscard, gnu::always_inline]] auto& getByIndex() & noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<SFML_BASE_VARIANT_NTH_TYPE(I)>();
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& getByIndex() const& noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<SFML_BASE_VARIANT_NTH_TYPE(I)>();
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[nodiscard, gnu::always_inline]] auto&& getByIndex() && noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return static_cast<SFML_BASE_VARIANT_NTH_TYPE(I) &&>(as<SFML_BASE_VARIANT_NTH_TYPE(I)>());
    }


    ////////////////////////////////////////////////////////////
    template <typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(declVal<SFML_BASE_ADD_LVALUE_REFERENCE(SFML_BASE_VARIANT_NTH_TYPE(0))>()))>
    [[nodiscard, gnu::always_inline]] R recursiveVisit(Visitor&& visitor) &
    {
        if constexpr (sizeof...(Alternatives) >= 10)
        {
            return recursiveVisitImplOpt10<0, R>(static_cast<Visitor&&>(visitor));
        }
        else if constexpr (sizeof...(Alternatives) >= 5)
        {
            return recursiveVisitImplOpt5<0, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return recursiveVisitImpl<0, R>(static_cast<Visitor&&>(visitor));
        }
    }


    ////////////////////////////////////////////////////////////
    template <typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(
                  declVal<SFML_BASE_ADD_LVALUE_REFERENCE(AddConst<SFML_BASE_VARIANT_NTH_TYPE(0)>)>()))>
    [[nodiscard, gnu::always_inline]] R recursiveVisit(Visitor&& visitor) const&
    {
        if constexpr (sizeof...(Alternatives) >= 10)
        {
            return recursiveVisitImplOpt10<0, R>(static_cast<Visitor&&>(visitor));
        }
        else if constexpr (sizeof...(Alternatives) >= 5)
        {
            return recursiveVisitImplOpt5<0, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return recursiveVisitImpl<0, R>(static_cast<Visitor&&>(visitor));
        }
    }


    ////////////////////////////////////////////////////////////
    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto recursiveMatch(Fs&&... fs) & -> decltype(recursiveVisit(OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return recursiveVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }


    ////////////////////////////////////////////////////////////
    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto recursiveMatch(Fs&&... fs) const& -> decltype(recursiveVisit(OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return recursiveVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }


    ////////////////////////////////////////////////////////////
    template <typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(declVal<SFML_BASE_ADD_LVALUE_REFERENCE(SFML_BASE_VARIANT_NTH_TYPE(0))>()))>
    [[nodiscard, gnu::always_inline]] R linearVisit(Visitor&& visitor) &
    {
        if constexpr (SFML_BASE_IS_REFERENCE(R))
        {
            SFML_BASE_REMOVE_CVREF(R) * ret; // NOLINT(cppcoreguidelines-init-variables)
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(getByIndex<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (SFML_BASE_IS_SAME(R, void))
        {
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, (visitor(getByIndex<I>())));
        }
        else
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            alignas(R) Byte retBuffer[sizeof(R)];
#pragma GCC diagnostic pop

            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, SFML_BASE_PLACEMENT_NEW(retBuffer) R(visitor(getByIndex<I>())));
            return *(SFML_BASE_LAUNDER_CAST(R*, retBuffer));
        }
    }


    ////////////////////////////////////////////////////////////
    template <typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(
                  declVal<SFML_BASE_ADD_LVALUE_REFERENCE(AddConst<SFML_BASE_VARIANT_NTH_TYPE(0)>)>()))>
    [[nodiscard, gnu::always_inline]] R linearVisit(Visitor&& visitor) const&
    {
        if constexpr (SFML_BASE_IS_REFERENCE(R))
        {
            SFML_BASE_REMOVE_CVREF(R) * ret; // NOLINT(cppcoreguidelines-init-variables)
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(getByIndex<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (SFML_BASE_IS_SAME(R, void))
        {
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, (visitor(getByIndex<I>())));
        }
        else
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            alignas(R) Byte retBuffer[sizeof(R)];
#pragma GCC diagnostic pop

            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, SFML_BASE_PLACEMENT_NEW(retBuffer) R(visitor(getByIndex<I>())));
            return *(SFML_BASE_LAUNDER_CAST(R*, retBuffer));
        }
    }


    ////////////////////////////////////////////////////////////
    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linearMatch(Fs&&... fs) & -> decltype(linearVisit(OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return linearVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }


    ////////////////////////////////////////////////////////////
    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linearMatch(Fs&&... fs) const& -> decltype(linearVisit(OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return linearVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }
};

#undef SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX
#undef SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ
#undef SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY
#undef SFML_BASE_VARIANT_NTH_TYPE

} // namespace sf::base

#pragma GCC diagnostic pop
