#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/ScopeGuard.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/Conditional.hpp"
#include "SFML/Base/Trait/CopyCV.hpp"
#include "SFML/Base/Trait/IsReference.hpp"
#include "SFML/Base/Trait/IsRvalueReference.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Trait/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"
#include "SFML/Base/Trait/RemoveReference.hpp"
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
    decltype(x) result = x;
    ((result = (result < xs) ? xs : result), ...);
    return result;
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


////////////////////////////////////////////////////////////
// Models `std::forward_like<Self, T>`'s result type: propagates
// the cv-ref qualification of `Self` (as deduced by a C++23
// "deducing this" parameter `this Self&& self`) onto `T`.
template <typename Self, typename T>
using LikeT = Conditional<SFML_BASE_IS_RVALUE_REFERENCE(Self&&),
                          CopyCV<SFML_BASE_REMOVE_REFERENCE(Self), T>&&,
                          CopyCV<SFML_BASE_REMOVE_REFERENCE(Self), T>&>;

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Tag value used to construct a `Variant` in-place by alternative type
///
////////////////////////////////////////////////////////////
template <typename T>
inline constexpr priv::InPlaceType<T> inPlaceType{};


////////////////////////////////////////////////////////////
/// \brief Tag value used to construct a `Variant` in-place by alternative index
///
////////////////////////////////////////////////////////////
template <SizeT N>
inline constexpr priv::InPlaceIndex<N> inPlaceIndex{};


////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_NTH_TYPE(i) SFML_BASE_TYPE_PACK_ELEMENT(i, Alternatives...)


////////////////////////////////////////////////////////////
/// \brief Lightweight tagged-union replacement for `std::variant`
///
/// Holds exactly one value chosen from the type pack `Alternatives...`.
/// Storage is in-place and never allocates. Special members are
/// conditionally defaulted when all alternatives are trivially
/// copy/move/destructible to keep the type trivial whenever possible.
///
/// Compared to `std::variant`, `sf::base::Variant`:
/// - avoids the heavy `<variant>` standard header
/// - never enters the "valueless by exception" state
/// - exposes both a recursive and a linear visit strategy, so callers
///   can pick the one with the best codegen for their alternative count
/// - propagates trivial relocatability when all alternatives are
///   trivially relocatable
///
/// The number of alternatives is limited to 254 (the discriminator is
/// stored in a single byte).
///
////////////////////////////////////////////////////////////
template <typename... Alternatives>
class Variant
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
    using DiscriminatorType = unsigned char; // Support up to 255 alternatives


public:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        enableTrivialRelocation = (SFML_BASE_IS_TRIVIALLY_RELOCATABLE(Alternatives) && ...)
    };


    ////////////////////////////////////////////////////////////
    template <typename T>
    static constexpr SizeT indexOf = getTypePackIndex<T, Alternatives...>();


private:
    ////////////////////////////////////////////////////////////
    static constexpr MakeIndexSequence<alternativeCount> alternativeIndexSequence{};


    ////////////////////////////////////////////////////////////
    alignas(maxAlignment) Byte m_buffer[maxSize];
    DiscriminatorType m_index;


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I)                                            \
    static_assert((I) != ::sf::base::badTypePackIndex, "Alternative type not supported by variant"); \
    static_assert((I) >= 0 && (I) < alternativeCount, "Alternative index out of range")


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(obj, Is, ...)                               \
    do                                                                                          \
    {                                                                                           \
        [&]<SizeT... Is> [[gnu::always_inline]] (IndexSequence<Is...>)                          \
        { (..., (((obj).m_index == Is) ? ((__VA_ARGS__), 0) : 0)); }(alternativeIndexSequence); \
    } while (false)


    ////////////////////////////////////////////////////////////
#define SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(Is, ...) \
    SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ((*this), Is, __VA_ARGS__)


    ////////////////////////////////////////////////////////////
    // Unchecked buffer accessor used by internal paths whose callers
    // have already proven the active alternative (e.g. from inside
    // `DO_WITH_CURRENT_INDEX`). Public access goes through `as<T>()`,
    // which additionally asserts the discriminator.
    ////////////////////////////////////////////////////////////
    template <typename T, typename Self>
    [[nodiscard, gnu::always_inline]] auto&& bufferAs(this Self&& self) noexcept
    {
        using Ret = priv::LikeT<Self, T>;
        using Ptr = SFML_BASE_REMOVE_REFERENCE(Ret)*;
        return static_cast<Ret>(*SFML_BASE_LAUNDER_CAST(Ptr, self.m_buffer));
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I>
    [[gnu::always_inline]] void destroyAt() noexcept
    {
        using Type = SFML_BASE_VARIANT_NTH_TYPE(I);
        // Discriminator check omitted: callers gate on `m_index == I` via `DO_WITH_CURRENT_INDEX`.
        bufferAs<Type>().~Type();
    }


    ////////////////////////////////////////////////////////////
    // Each of the three recursive helpers below is a single static function
    // template that perfect-forwards `self`. `getByIndex` is itself a
    // deducing-this template, so `static_cast<Self&&>(self).getByIndex<I>()`
    // propagates the cvref onto the returned reference automatically.
    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Self, typename Visitor>
    [[nodiscard, gnu::always_inline]] static R recursiveVisitImpl(Self&& self, Visitor&& visitor)
    {
        if constexpr (I < sizeof...(Alternatives) - 1)
        {
            return (self.m_index == I)
                       ? visitor(static_cast<Self&&>(self).template getByIndex<I>())
                       : recursiveVisitImpl<I + 1, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
        }
        else
        {
            return visitor(static_cast<Self&&>(self).template getByIndex<I>());
        }
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Self, typename Visitor>
    [[nodiscard, gnu::always_inline]] static R recursiveVisitImplOpt5(Self&& self, Visitor&& visitor)
    {
        if constexpr (I == 0 && sizeof...(Alternatives) == 5)
        {
            // clang-format off
            return (self.m_index == I + 0) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 0>()) :
                   (self.m_index == I + 1) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 1>()) :
                   (self.m_index == I + 2) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 2>()) :
                   (self.m_index == I + 3) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 3>()) :
                                             visitor(static_cast<Self&&>(self).template getByIndex<I + 4>()) ;
            // clang-format on
        }
        else if constexpr (I + 4 < sizeof...(Alternatives))
        {
            // clang-format off
            return (self.m_index == I + 0) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 0>()) :
                   (self.m_index == I + 1) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 1>()) :
                   (self.m_index == I + 2) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 2>()) :
                   (self.m_index == I + 3) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 3>()) :
                   (self.m_index == I + 4) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 4>()) :
                   recursiveVisitImplOpt5<I + 5, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursiveVisitImpl<I, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
        }
    }


    ////////////////////////////////////////////////////////////
    template <SizeT I, typename R, typename Self, typename Visitor>
    [[nodiscard, gnu::always_inline]] static R recursiveVisitImplOpt10(Self&& self, Visitor&& visitor)
    {
        if constexpr (I + 9 < sizeof...(Alternatives))
        {
            // clang-format off
            return (self.m_index == I + 0) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 0>()) :
                   (self.m_index == I + 1) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 1>()) :
                   (self.m_index == I + 2) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 2>()) :
                   (self.m_index == I + 3) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 3>()) :
                   (self.m_index == I + 4) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 4>()) :
                   (self.m_index == I + 5) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 5>()) :
                   (self.m_index == I + 6) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 6>()) :
                   (self.m_index == I + 7) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 7>()) :
                   (self.m_index == I + 8) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 8>()) :
                   (self.m_index == I + 9) ? visitor(static_cast<Self&&>(self).template getByIndex<I + 9>()) :
                   recursiveVisitImplOpt10<I + 10, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursiveVisitImplOpt5<I, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
        }
    }


public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct an alternative in-place from its index
    ///
    /// \param args Arguments forwarded to the alternative's constructor
    ///
    ////////////////////////////////////////////////////////////
    template <SizeT I, typename... Args>
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] explicit Variant(const priv::InPlaceIndex<I>, Args&&... args) noexcept :
        m_index{static_cast<DiscriminatorType>(I)}
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(I){static_cast<Args&&>(args)...};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an alternative in-place from its type
    ///
    /// \param args Arguments forwarded to `T`'s constructor
    ///
    ////////////////////////////////////////////////////////////
    template <typename T, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit Variant(const priv::InPlaceType<T>, Args&&... args) noexcept :
        Variant{inPlaceIndex<indexOf<T>>, static_cast<Args&&>(args)...}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct from any value whose decayed type is one of the alternatives
    ///
    /// The alternative is chosen by exact type match (after removing
    /// cvref) -- there is no implicit conversion picking.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] explicit Variant(T&& x) noexcept
        requires(!isSame<RemoveCVRefIndirect<T>, Variant>)
        : Variant{inPlaceType<SFML_BASE_REMOVE_CVREF(T)>, static_cast<T&&>(x)}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Default constructor, holds the first alternative default-constructed
    ///
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
                                                SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(I)(
                                                    rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>()));
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
                                                    rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>())));
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

        if (m_index == rhs.m_index)
        {
            // Same alternative on both sides: delegate to the alternative's copy-assignment.
            // Safer than destroy+construct (no transient destroyed state) and preserves the
            // alternative's own self-assignment and exception-safety semantics.
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I,
                                                    bufferAs<SFML_BASE_VARIANT_NTH_TYPE(
                                                        I)>() = rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>());
            return *this;
        }

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                                    I,
                                                    (SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(I)(
                                                        rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>())));
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

        if (m_index == rhs.m_index)
        {
            // Same alternative on both sides: delegate to the alternative's move-assignment.
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I,
                                                    bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>() = static_cast<SFML_BASE_VARIANT_NTH_TYPE(
                                                        I) &&>(rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>()));
            return *this;
        }

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                                    I,
                                                    (SFML_BASE_PLACEMENT_NEW(m_buffer) SFML_BASE_VARIANT_NTH_TYPE(
                                                        I)(static_cast<SFML_BASE_VARIANT_NTH_TYPE(I) &&>(
                                                        rhs.template bufferAs<SFML_BASE_VARIANT_NTH_TYPE(I)>()))));
        m_index = rhs.m_index;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] Variant& operator=(Variant&& rhs) noexcept
        requires(triviallyMoveAssignable)
    = default;


    ////////////////////////////////////////////////////////////
    /// \brief Assign a value whose decayed type is one of the alternatives
    ///
    /// Destroys the current value and reconstructs the variant to hold a
    /// new alternative selected by exact type match (after removing cvref).
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline]] Variant& operator=(T&& x)
        requires(!isSame<RemoveCVRefIndirect<T>, Variant>)
    {
        using Type = SFML_BASE_REMOVE_CVREF(T);

        if (m_index == indexOf<Type>)
        {
            // Same alternative already active: delegate to the alternative's assignment.
            // Also the only aliasing-safe branch when `x` binds to the currently-held value
            // (e.g. `v = v.as<Type>()`); a destroy-then-construct sequence would read from
            // destroyed storage.
            bufferAs<Type>() = static_cast<T&&>(x);
            return *this;
        }

        SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX(I, destroyAt<I>());

        SFML_BASE_PLACEMENT_NEW(m_buffer) Type{static_cast<T&&>(x)};
        m_index = indexOf<Type>;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether the variant currently holds alternative `T`
    ///
    /// \return `true` if the active alternative is `T`, `false` otherwise
    ///
    ////////////////////////////////////////////////////////////
    template <typename T>
    [[nodiscard, gnu::always_inline]] bool is() const noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(indexOf<T>);
        return m_index == indexOf<T>;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether the variant currently holds the alternative at the given index
    ///
    /// \param index Discriminator value to compare against
    ///
    /// \return `true` if the active alternative index matches `index`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] bool hasIndex(const DiscriminatorType index) const noexcept
    {
        return m_index == index;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Pointer-typed access to alternative `T`, or `nullptr` if not active
    ///
    /// Constrained to lvalue variants: returning a pointer into an
    /// rvalue's storage would dangle at the end of the full expression.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T, typename Self>
    [[nodiscard, gnu::always_inline]] auto* getIf(this Self&& self) noexcept
        requires(!SFML_BASE_IS_RVALUE_REFERENCE(Self &&))
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(indexOf<T>);
        using Ptr = CopyCV<SFML_BASE_REMOVE_REFERENCE(Self), T>*;
        return self.m_index == indexOf<T> ? SFML_BASE_LAUNDER_CAST(Ptr, self.m_buffer) : nullptr;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Unchecked access to alternative `T` (asserts that it is active)
    ///
    /// Propagates the cv-ref qualification of `*this` onto the returned
    /// reference: `T&`, `const T&`, `T&&`, or `const T&&` as appropriate.
    ///
    ////////////////////////////////////////////////////////////
    template <typename T, typename Self>
    [[nodiscard, gnu::always_inline]] auto&& as(this Self&& self) noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(indexOf<T>);
        SFML_BASE_ASSERT(self.m_index == indexOf<T>);
        return static_cast<Self&&>(self).template bufferAs<T>();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Unchecked access to the alternative at index `I`
    ///
    /// Propagates the cv-ref qualification of `*this` onto the returned
    /// reference (same rules as `as<T>()`).
    ///
    ////////////////////////////////////////////////////////////
    template <SizeT I, typename Self>
    [[nodiscard, gnu::always_inline]] auto&& getByIndex(this Self&& self) noexcept
    {
        SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return static_cast<Self&&>(self).template as<SFML_BASE_VARIANT_NTH_TYPE(I)>();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Visit the active alternative using a tail-call recursive dispatch
    ///
    /// The visitor must be callable with a reference to every alternative
    /// type. The return type `R` is deduced from the visitor's call on the
    /// first alternative.
    ///
    /// Recursive dispatch is generally faster for small alternative
    /// counts; for larger counts the unrolled `Opt5` and `Opt10` helpers
    /// are selected automatically.
    ///
    /// \param visitor Callable invoked with the active alternative
    ///
    /// \return Whatever `visitor` returns
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self,
              typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(declVal<priv::LikeT<Self, SFML_BASE_VARIANT_NTH_TYPE(0)>>()))>
    [[nodiscard, gnu::always_inline, gnu::flatten]] R recursiveVisit(this Self&& self, Visitor&& visitor)
    {
        if constexpr (sizeof...(Alternatives) >= 10)
            return recursiveVisitImplOpt10<0, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
        else if constexpr (sizeof...(Alternatives) >= 5)
            return recursiveVisitImplOpt5<0, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
        else
            return recursiveVisitImpl<0, R>(static_cast<Self&&>(self), static_cast<Visitor&&>(visitor));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Like `recursiveVisit`, but takes one lambda per alternative
    ///
    /// The lambdas are combined into an `OverloadSet` and the resulting
    /// callable is then forwarded to `recursiveVisit`. This is the
    /// pattern-matching style entry point.
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self, typename... Fs>
    [[nodiscard, gnu::always_inline, gnu::flatten]] auto recursiveMatch(this Self&& self, Fs&&... fs)
        -> decltype(static_cast<Self&&>(self).recursiveVisit(OverloadSet{static_cast<Fs&&>(fs)...}))
    {
        return static_cast<Self&&>(self).recursiveVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }


    ////////////////////////////////////////////////////////////
    /// \brief Visit the active alternative using a single linear dispatch
    ///
    /// Unlike `recursiveVisit`, this implementation expands a single
    /// fold expression that compares the discriminator against every
    /// alternative index. It can produce better codegen for moderate
    /// alternative counts when the visitor has a non-void, non-reference
    /// return type.
    ///
    /// \param visitor Callable invoked with the active alternative
    ///
    /// \return Whatever `visitor` returns
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self,
              typename Visitor,
              typename R = decltype(declVal<Visitor&&>()(declVal<priv::LikeT<Self, SFML_BASE_VARIANT_NTH_TYPE(0)>>()))>
    [[nodiscard, gnu::always_inline]] R linearVisit(this Self&& self, Visitor&& visitor)
    {
        if constexpr (SFML_BASE_IS_REFERENCE(R))
        {
            SFML_BASE_REMOVE_CVREF(R) * ret; // NOLINT(cppcoreguidelines-init-variables)
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(self,
                                                        I,
                                                        ret = &(visitor(static_cast<Self&&>(self).template getByIndex<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (SFML_BASE_IS_SAME(R, void))
        {
            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(self, I, (visitor(static_cast<Self&&>(self).template getByIndex<I>())));
        }
        else
        {
            alignas(R) Byte retBuffer[sizeof(R)];

            SFML_BASE_SCOPE_GUARD({
                if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(R))
                    SFML_BASE_LAUNDER_CAST(R*, retBuffer)->~R();
            });

            SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ(self,
                                                        I,
                                                        SFML_BASE_PLACEMENT_NEW(retBuffer)
                                                            R(visitor(static_cast<Self&&>(self).template getByIndex<I>())));
            return *(SFML_BASE_LAUNDER_CAST(R*, retBuffer));
        }
    }


    ////////////////////////////////////////////////////////////
    /// \brief Like `linearVisit`, but takes one lambda per alternative
    ///
    /// The lambdas are combined into an `OverloadSet` and the resulting
    /// callable is then forwarded to `linearVisit`. This is the
    /// pattern-matching style entry point.
    ///
    ////////////////////////////////////////////////////////////
    template <typename Self, typename... Fs>
    [[nodiscard, gnu::always_inline, gnu::flatten]] auto linearMatch(this Self&& self, Fs&&... fs)
        -> decltype(static_cast<Self&&>(self).linearVisit(OverloadSet{static_cast<Fs&&>(fs)...}))
    {
        return static_cast<Self&&>(self).linearVisit(OverloadSet{static_cast<Fs&&>(fs)...});
    }
};

#undef SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX
#undef SFML_BASE_VARIANT_DO_WITH_CURRENT_INDEX_OBJ
#undef SFML_BASE_VARIANT_STATIC_ASSERT_INDEX_VALIDITY
#undef SFML_BASE_VARIANT_NTH_TYPE

} // namespace sf::base

#pragma GCC diagnostic pop
