#pragma once

// NOLINTBEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/LambdaMacros.hpp"
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/OverloadSet.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/AddConst.hpp"
#include "SFML/Base/Traits/AddLvalueReference.hpp"
#include "SFML/Base/Traits/IsReference.hpp"
#include "SFML/Base/Traits/IsSame.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyConstructible.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveAssignable.hpp"
#include "SFML/Base/Traits/IsTriviallyMoveConstructible.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"
#include "SFML/Base/TypePackElement.hpp"


namespace sfvr::impl
{
using sf::base::SizeT;
} // namespace sfvr::impl

namespace sfvr::impl
{
[[nodiscard]] consteval auto variadic_max(auto X) noexcept
{
    return X;
}

[[nodiscard]] consteval auto variadic_max(auto X, auto... Xs) noexcept
{
    decltype(X) result = X;
    decltype(X) rest[]{Xs...};

    for (auto value : rest)
        if (result < value)
            result = value;

    return result;
}

enum : SizeT
{
    bad_index = static_cast<SizeT>(-1)
};

template <typename T, typename... Ts>
[[nodiscard]] consteval SizeT index_of() noexcept
{
    constexpr bool matches[]{SFML_BASE_IS_SAME(T, Ts)...};

    for (SizeT i = 0; i < sizeof...(Ts); ++i)
        if (matches[i])
            return i;

    return bad_index;
}

template <typename>
struct inplace_type_t
{
};

template <SizeT>
struct inplace_index_t
{
};

} // namespace sfvr::impl

namespace sfvr
{

template <typename T>
inline constexpr impl::inplace_type_t<T> inplace_type{};

template <impl::SizeT N>
inline constexpr impl::inplace_index_t<N> inplace_index{};

#define TINYVARIANT_NTH_TYPE(i) SFML_BASE_TYPE_PACK_ELEMENT(i, Alternatives...)

template <typename... Alternatives>
class [[nodiscard]] tinyvariant
{
private:
    using byte = unsigned char;

    enum : impl::SizeT
    {
        type_count    = sizeof...(Alternatives),
        max_alignment = impl::variadic_max(alignof(Alternatives)...),
        max_size      = impl::variadic_max(sizeof(Alternatives)...)
    };

    static inline constexpr bool triviallyDestructible = (... && sf::base::isTriviallyDestructible<Alternatives>);
    static inline constexpr bool triviallyCopyConstructible = (... && sf::base::isTriviallyCopyConstructible<Alternatives>);
    static inline constexpr bool triviallyMoveConstructible = (... && sf::base::isTriviallyMoveConstructible<Alternatives>);
    static inline constexpr bool triviallyCopyAssignable = (... && sf::base::isTriviallyCopyAssignable<Alternatives>);
    static inline constexpr bool triviallyMoveAssignable = (... && sf::base::isTriviallyMoveAssignable<Alternatives>);

    using index_type = unsigned char; // Support up to 255 alternatives

public:
    template <typename T>
    static constexpr impl::SizeT index_of = impl::index_of<T, Alternatives...>();

private:
    static constexpr sf::base::MakeIndexSequence<type_count> alternative_index_sequence{};

    alignas(max_alignment) byte _buffer[max_size];
    index_type _index;

#define TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I)                                     \
    static_assert((I) != impl::bad_index, "Alternative type not supported by variant"); \
                                                                                        \
    static_assert((I) >= 0 && (I) < type_count, "Alternative index out of range")

#define TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(obj, Is, ...)                                           \
    do                                                                                                \
    {                                                                                                 \
        [&]<impl::SizeT... Is>(sf::base::IndexSequence<Is...>) SFML_BASE_LAMBDA_ALWAYS_INLINE_FLATTEN \
        { (..., (((obj)._index == Is) ? ((__VA_ARGS__), 0) : 0)); }(alternative_index_sequence);      \
    } while (false)

#define TINYVARIANT_DO_WITH_CURRENT_INDEX(Is, ...) TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ((*this), Is, __VA_ARGS__)

    template <typename T, impl::SizeT I, typename... Args>
    [[nodiscard,
      gnu::always_inline]] explicit tinyvariant(impl::inplace_type_t<T>, impl::inplace_index_t<I>, Args&&... args) noexcept :
        _index{static_cast<index_type>(I)}
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        SFML_BASE_PLACEMENT_NEW(_buffer) T{static_cast<Args&&>(args)...};
    }

    template <impl::SizeT I>
    [[gnu::always_inline]] void destroy_at() noexcept
    {
        using type = TINYVARIANT_NTH_TYPE(I);

        as<type>().~type();
    }

    template <impl::SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursive_visit_impl(Visitor&& visitor)
    {
        if constexpr (I < sizeof...(Alternatives) - 1)
        {
            return (_index == I) ? visitor(get_by_index<I>())
                                 : recursive_visit_impl<I + 1, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return visitor(get_by_index<I>());
        }
    }

    template <impl::SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursive_visit_opt5_impl(Visitor&& visitor)
    {
        if constexpr (I == 0 && sizeof...(Alternatives) == 5)
        {
            // clang-format off
            return (_index == I + 0) ? visitor(get_by_index<I + 0>()) :
                   (_index == I + 1) ? visitor(get_by_index<I + 1>()) :
                   (_index == I + 2) ? visitor(get_by_index<I + 2>()) :
                   (_index == I + 3) ? visitor(get_by_index<I + 3>()) :
                                       visitor(get_by_index<I + 4>()) ;
            // clang-format on
        }
        else if constexpr (I + 4 < sizeof...(Alternatives))
        {
            // clang-format off
            return (_index == I + 0) ? visitor(get_by_index<I + 0>()) :
                   (_index == I + 1) ? visitor(get_by_index<I + 1>()) :
                   (_index == I + 2) ? visitor(get_by_index<I + 2>()) :
                   (_index == I + 3) ? visitor(get_by_index<I + 3>()) :
                   (_index == I + 4) ? visitor(get_by_index<I + 4>()) :
                   recursive_visit_opt5_impl<I + 5, R>(static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursive_visit_impl<I, R>(static_cast<Visitor&&>(visitor));
        }
    }

    template <impl::SizeT I, typename R, typename Visitor>
    [[nodiscard, gnu::always_inline]] R recursive_visit_opt10_impl(Visitor&& visitor)
    {
        if constexpr (I + 9 < sizeof...(Alternatives))
        {
            // clang-format off
            return (_index == I + 0) ? visitor(get_by_index<I + 0>()) :
                   (_index == I + 1) ? visitor(get_by_index<I + 1>()) :
                   (_index == I + 2) ? visitor(get_by_index<I + 2>()) :
                   (_index == I + 3) ? visitor(get_by_index<I + 3>()) :
                   (_index == I + 4) ? visitor(get_by_index<I + 4>()) :
                   (_index == I + 5) ? visitor(get_by_index<I + 5>()) :
                   (_index == I + 6) ? visitor(get_by_index<I + 6>()) :
                   (_index == I + 7) ? visitor(get_by_index<I + 7>()) :
                   (_index == I + 8) ? visitor(get_by_index<I + 8>()) :
                   (_index == I + 9) ? visitor(get_by_index<I + 9>()) :
                   recursive_visit_opt10_impl<I + 10, R>(static_cast<Visitor&&>(visitor));
            // clang-format on
        }
        else
        {
            return recursive_visit_opt5_impl<I, R>(static_cast<Visitor&&>(visitor));
        }
    }

public:
    template <typename T, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(impl::inplace_type_t<T> inplace_type, Args&&... args) noexcept :
        tinyvariant{inplace_type, inplace_index<index_of<T>>, static_cast<Args&&>(args)...}
    {
    }

    template <impl::SizeT I, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(impl::inplace_index_t<I> inplace_index, Args&&... args) noexcept :
        tinyvariant{inplace_type<TINYVARIANT_NTH_TYPE(I)>, inplace_index, static_cast<Args&&>(args)...}
    {
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(T&& x) noexcept
        requires(!sf::base::isSame<sf::base::RemoveCVRefIndirect<T>, tinyvariant>)
        : tinyvariant{inplace_type<sf::base::RemoveCVRefIndirect<T>>, static_cast<T&&>(x)}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit tinyvariant() noexcept : tinyvariant{inplace_index<0>}
    {
    }

    [[gnu::always_inline]] tinyvariant(const tinyvariant& rhs)
        requires(!triviallyCopyConstructible)
        : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          SFML_BASE_PLACEMENT_NEW(_buffer)
                                              TINYVARIANT_NTH_TYPE(I)(static_cast<const TINYVARIANT_NTH_TYPE(I) &>(
                                                  *SFML_BASE_LAUNDER_CAST(const TINYVARIANT_NTH_TYPE(I)*, rhs._buffer))));
    }

    [[gnu::always_inline]] tinyvariant(const tinyvariant& rhs)
        requires(triviallyCopyConstructible)
    = default;

    [[gnu::always_inline]] tinyvariant(tinyvariant&& rhs) noexcept
        requires(!triviallyMoveConstructible)
        : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          SFML_BASE_PLACEMENT_NEW(_buffer)
                                              TINYVARIANT_NTH_TYPE(I)(static_cast<TINYVARIANT_NTH_TYPE(I) &&>(
                                                  *SFML_BASE_LAUNDER_CAST(TINYVARIANT_NTH_TYPE(I)*, rhs._buffer))));
    }

    [[gnu::always_inline]] tinyvariant(tinyvariant&& rhs) noexcept
        requires(triviallyMoveConstructible)
    = default;

    [[gnu::always_inline]] ~tinyvariant()
        requires(!triviallyDestructible)
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());
    }

    [[gnu::always_inline]] ~tinyvariant()
        requires(triviallyDestructible)
    = default;

    [[gnu::always_inline]] tinyvariant& operator=(const tinyvariant& rhs)
        requires(!triviallyCopyAssignable)
    {
        if (this == &rhs)
            return *this;

        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                              I,
                                              (SFML_BASE_PLACEMENT_NEW(_buffer)
                                                   TINYVARIANT_NTH_TYPE(I)(rhs.template as<TINYVARIANT_NTH_TYPE(I)>())));
        _index = rhs._index;

        return *this;
    }

    [[gnu::always_inline]] tinyvariant& operator=(const tinyvariant& rhs)
        requires(triviallyCopyAssignable)
    = default;

    [[gnu::always_inline]] tinyvariant& operator=(tinyvariant&& rhs) noexcept
        requires(!triviallyMoveAssignable)
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                              I,
                                              (SFML_BASE_PLACEMENT_NEW(_buffer)
                                                   TINYVARIANT_NTH_TYPE(I)(static_cast<TINYVARIANT_NTH_TYPE(I) &&>(
                                                       rhs.template as<TINYVARIANT_NTH_TYPE(I)>()))));
        _index = rhs._index;

        return *this;
    }

    [[gnu::always_inline]] tinyvariant& operator=(tinyvariant&& rhs) noexcept
        requires(triviallyMoveAssignable)
    = default;

    template <typename T>
    [[gnu::always_inline]] tinyvariant& operator=(T&& x)
        requires(!sf::base::isSame<sf::base::RemoveCVRefIndirect<T>, tinyvariant>)
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        using type = SFML_BASE_REMOVE_CVREF(T);

        SFML_BASE_PLACEMENT_NEW(_buffer) type{static_cast<T&&>(x)};
        _index = index_of<type>;

        return *this;
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] bool is() const noexcept
    {
        return _index == index_of<T>;
    }

    [[nodiscard, gnu::always_inline]] bool has_index(index_type index) const noexcept
    {
        return _index == index;
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] T* get_if() & noexcept
    {
        return _index == index_of<T> ? SFML_BASE_LAUNDER_CAST(T*, _buffer) : nullptr;
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] const T* get_if() const& noexcept
    {
        return _index == index_of<T> ? SFML_BASE_LAUNDER_CAST(const T*, _buffer) : nullptr;
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] T* get_if() && noexcept
    {
        return _index == index_of<T> ? SFML_BASE_LAUNDER_CAST(T*, _buffer) : nullptr;
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] T& as() & noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(T*, _buffer);
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] const T& as() const& noexcept
    {
        return *SFML_BASE_LAUNDER_CAST(const T*, _buffer);
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] T&& as() && noexcept
    {
        return static_cast<T&&>(*SFML_BASE_LAUNDER_CAST(T*, _buffer));
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] auto& get_by_index() & noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<TINYVARIANT_NTH_TYPE(I)>();
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& get_by_index() const& noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<TINYVARIANT_NTH_TYPE(I)>();
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] auto&& get_by_index() && noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return static_cast<TINYVARIANT_NTH_TYPE(I) &&>(as<TINYVARIANT_NTH_TYPE(I)>());
    }

    template <typename Visitor,
              typename R = decltype(sf::base::declVal<Visitor&&>()(
                  sf::base::declVal<SFML_BASE_ADD_LVALUE_REFERENCE(TINYVARIANT_NTH_TYPE(0))>()))>
    [[nodiscard, gnu::always_inline]] R recursive_visit(Visitor&& visitor) &
    {
        if constexpr (sizeof...(Alternatives) >= 10)
        {
            return recursive_visit_opt10_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
        else if constexpr (sizeof...(Alternatives) >= 5)
        {
            return recursive_visit_opt5_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return recursive_visit_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
    }

    template <typename Visitor,
              typename R = decltype(sf::base::declVal<Visitor&&>()(
                  sf::base::declVal<SFML_BASE_ADD_LVALUE_REFERENCE(sf::base::AddConst<TINYVARIANT_NTH_TYPE(0)>)>()))>
    [[nodiscard, gnu::always_inline]] R recursive_visit(Visitor&& visitor) const&
    {
        if constexpr (sizeof...(Alternatives) >= 10)
        {
            return recursive_visit_opt10_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
        else if constexpr (sizeof...(Alternatives) >= 5)
        {
            return recursive_visit_opt5_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
        else
        {
            return recursive_visit_impl<0, R>(static_cast<Visitor&&>(visitor));
        }
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto recursive_match(Fs&&... fs) & -> decltype(recursive_visit(sf::base::OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return recursive_visit(sf::base::OverloadSet{static_cast<Fs&&>(fs)...});
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto recursive_match(
        Fs&&... fs) const& -> decltype(recursive_visit(sf::base::OverloadSet{static_cast<Fs&&>(fs)...}))
    {
        return recursive_visit(sf::base::OverloadSet{static_cast<Fs&&>(fs)...});
    }

    template <typename Visitor,
              typename R = decltype(sf::base::declVal<Visitor&&>()(
                  sf::base::declVal<SFML_BASE_ADD_LVALUE_REFERENCE(TINYVARIANT_NTH_TYPE(0))>()))>
    [[nodiscard, gnu::always_inline]] R linear_visit(Visitor&& visitor) &
    {
        if constexpr (SFML_BASE_IS_REFERENCE(R))
        {
            SFML_BASE_REMOVE_CVREF(R) * ret;
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(get_by_index<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (SFML_BASE_IS_SAME(R, void))
        {
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, (visitor(get_by_index<I>())));
        }
        else
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            alignas(R) byte ret_buffer[sizeof(R)];
#pragma GCC diagnostic pop

            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, SFML_BASE_PLACEMENT_NEW(ret_buffer) R(visitor(get_by_index<I>())));

            return *(SFML_BASE_LAUNDER_CAST(R*, ret_buffer));
        }
    }

    template <typename Visitor,
              typename R = decltype(sf::base::declVal<Visitor&&>()(
                  sf::base::declVal<SFML_BASE_ADD_LVALUE_REFERENCE(sf::base::AddConst<TINYVARIANT_NTH_TYPE(0)>)>()))>
    [[nodiscard, gnu::always_inline]] R linear_visit(Visitor&& visitor) const&
    {
        if constexpr (SFML_BASE_IS_REFERENCE(R))
        {
            SFML_BASE_REMOVE_CVREF(R) * ret;
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(get_by_index<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (SFML_BASE_IS_SAME(R, void))
        {
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, (visitor(get_by_index<I>())));
        }
        else
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
            alignas(R) byte ret_buffer[sizeof(R)];
#pragma GCC diagnostic pop

            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, SFML_BASE_PLACEMENT_NEW(ret_buffer) R(visitor(get_by_index<I>())));

            return *(SFML_BASE_LAUNDER_CAST(R*, ret_buffer));
        }
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linear_match(Fs&&... fs) & -> decltype(linear_visit(sf::base::OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return linear_visit(sf::base::OverloadSet{static_cast<Fs&&>(fs)...});
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linear_match(Fs&&... fs) const& -> decltype(linear_visit(sf::base::OverloadSet{
        static_cast<Fs&&>(fs)...}))
    {
        return linear_visit(sf::base::OverloadSet{static_cast<Fs&&>(fs)...});
    }
};

#undef TINYVARIANT_DO_WITH_CURRENT_INDEX
#undef TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ
#undef TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY
#undef TINYVARIANT_NTH_TYPE

} // namespace sfvr

#pragma GCC diagnostic pop

// NOLINTEND

// TODO P0: cleanup
