#pragma once

// NOLINTBEGIN

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <SFML/Base/Assert.hpp>
#include <SFML/Base/PlacementNew.hpp>
#include <SFML/Base/SizeT.hpp>
#include <SFML/Base/Traits/IsSame.hpp>


// From:
// https://github.com/redorav/crstl/blob/master/include/crstl/utility/placement_new.h

namespace sfvr::impl
{

using SizeT = sf::base::SizeT;

} // namespace sfvr::impl


#if ((__GNUC__ >= 10) || defined(__clang__)) && !defined(_MSC_VER)
#define TINYVARIANT_SUPPORTS_HAS_BUILTIN
#endif

#ifdef TINYVARIANT_SUPPORTS_HAS_BUILTIN

// TODO: to header
#if __has_builtin(__type_pack_element)
#define TINYVARIANT_USE_TYPE_PACK_ELEMENT
#endif

// TODO: to header
#if __has_builtin(__make_integer_seq)
#define TINYVARIANT_USE_MAKE_INTEGER_SEQ
#elif __has_builtin(__integer_pack)
#define TINYVARIANT_USE_INTEGER_PACK
#else
#define TINYVARIANT_USE_STD_INDEX_SEQUENCE
#endif

#else
#define TINYVARIANT_USE_STD_INDEX_SEQUENCE
#endif

#ifdef TINYVARIANT_USE_STD_INDEX_SEQUENCE
#include <utility>
#endif

namespace sfvr::impl
{

template <typename T>
T&& declval();

#ifdef TINYVARIANT_USE_STD_INDEX_SEQUENCE

template <SizeT... Is>
using index_sequence = std::index_sequence<Is...>;

#else

template <SizeT...>
struct index_sequence
{
};

#endif

#ifdef TINYVARIANT_USE_MAKE_INTEGER_SEQ

template <typename, SizeT... X>
struct index_sequence_helper
{
    using type = index_sequence<X...>;
};

template <SizeT N>
using index_sequence_up_to = typename __make_integer_seq<index_sequence_helper, SizeT, N>::type;

#elif defined(TINYVARIANT_USE_INTEGER_PACK)

template <SizeT N>
using index_sequence_up_to = index_sequence<__integer_pack(N)...>;

#elif defined(TINYVARIANT_USE_STD_INDEX_SEQUENCE)

template <SizeT N>
using index_sequence_up_to = std::make_index_sequence<N>;

#else

#error "No integer sequence generation available."

#endif

template <auto X, auto... Xs>
[[nodiscard, gnu::always_inline]] consteval auto variadic_max() noexcept
{
    decltype(X) result = X;
    decltype(X) rest[]{Xs...};

    for (auto value : rest)
    {
        if (value > result)
        {
            result = value;
        }
    }

    return result;
}

enum : SizeT
{
    bad_index = static_cast<SizeT>(-1)
};

template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline]] consteval SizeT index_of() noexcept
{
    constexpr bool matches[]{SFML_BASE_IS_SAME(T, Ts)...};

    for (SizeT i = 0; i < sizeof...(Ts); ++i)
    {
        if (matches[i])
        {
            return i;
        }
    }

    return bad_index;
};

template <typename T>
struct type_wrapper
{
    using type = T;
};


// TODO: to header
#ifdef TINYVARIANT_USE_TYPE_PACK_ELEMENT

template <SizeT N, typename... Ts>
using type_at = __type_pack_element<N, Ts...>;

#else

template <SizeT N,
          typename T0 = void,
          typename T1 = void,
          typename T2 = void,
          typename T3 = void,
          typename T4 = void,
          typename T5 = void,
          typename T6 = void,
          typename T7 = void,
          typename T8 = void,
          typename T9 = void,
          typename... Ts>
[[nodiscard, gnu::always_inline]] consteval auto type_at_impl() noexcept
{
    // clang-format off
    if constexpr(N == 0)      { return type_wrapper<T0>{}; }
    else if constexpr(N == 1) { return type_wrapper<T1>{}; }
    else if constexpr(N == 2) { return type_wrapper<T2>{}; }
    else if constexpr(N == 3) { return type_wrapper<T3>{}; }
    else if constexpr(N == 4) { return type_wrapper<T4>{}; }
    else if constexpr(N == 5) { return type_wrapper<T5>{}; }
    else if constexpr(N == 6) { return type_wrapper<T6>{}; }
    else if constexpr(N == 7) { return type_wrapper<T7>{}; }
    else if constexpr(N == 8) { return type_wrapper<T8>{}; }
    else if constexpr(N == 9) { return type_wrapper<T9>{}; }
    else                      { return type_at_impl<N - 10, Ts...>(); }
    // clang-format on
}

template <SizeT N, typename... Ts>
using type_at = typename decltype(type_at_impl<N, Ts...>())::type;

#endif

template <typename>
struct inplace_type_t
{
};

template <SizeT>
struct inplace_index_t
{
};

// TODO: to header
template <typename... Fs>
struct [[nodiscard]] overload_set : Fs...
{
    [[nodiscard, gnu::always_inline]] explicit overload_set(Fs&&... fs) noexcept : Fs{static_cast<Fs&&>(fs)}...
    {
    }

    using Fs::operator()...;
};

template <typename... Fs>
overload_set(Fs...) -> overload_set<Fs...>;

template <typename T>
struct uncvref
{
    using type = T;
};
template <typename T>
struct uncvref<T&>
{
    using type = T;
};
template <typename T>
struct uncvref<T&&>
{
    using type = T;
};
template <typename T>
struct uncvref<const T&>
{
    using type = T;
};
template <typename T>
struct uncvref<const T&&>
{
    using type = T;
};

template <typename T>
using uncvref_t = typename uncvref<T>::type;

template <typename T>
inline constexpr bool is_reference = false;

template <typename T>
inline constexpr bool is_reference<T&> = true;

template <typename T>
inline constexpr bool is_reference<T&&> = true;


template <typename T>
inline constexpr bool is_void = false;

template <>
inline constexpr bool is_void<void> = true;

struct void_type
{
};

template <typename T>
struct regularize_void
{
    using type = T;
};

template <>
struct regularize_void<void>
{
    using type = void_type;
};

template <typename T>
using regularize_void_t = typename regularize_void<T>::type;

/*
template <typename... Ts>
using common_type_between_t = typename common_type_between<Ts...>::type;
*/

} // namespace sfvr::impl

namespace sfvr
{

template <typename T>
inline constexpr impl::inplace_type_t<T> inplace_type{};

template <impl::SizeT N>
inline constexpr impl::inplace_index_t<N> inplace_index{};

template <typename... Alternatives>
class [[nodiscard]] tinyvariant
{
private:
    using byte = unsigned char;

    enum : impl::SizeT
    {
        type_count    = sizeof...(Alternatives),
        max_alignment = impl::variadic_max<alignof(Alternatives)...>(),
        max_size      = impl::variadic_max<sizeof(Alternatives)...>()
    };

    using index_type = unsigned char; // Support up to 255 alternatives

    template <impl::SizeT I>
    using nth_type = impl::type_at<I, Alternatives...>;

public:
    template <typename T>
    static constexpr impl::SizeT index_of = impl::index_of<T, Alternatives...>();

private:
    static constexpr impl::index_sequence_up_to<type_count> alternative_index_sequence{};

    alignas(max_alignment) byte _buffer[max_size];
    index_type _index;

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 10)
#define TINYVARIANT_ALWAYS_INLINE_LAMBDA [[gnu::always_inline]]
#else
#define TINYVARIANT_ALWAYS_INLINE_LAMBDA
#endif

#define TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I)                                     \
    static_assert((I) != impl::bad_index, "Alternative type not supported by variant"); \
                                                                                        \
    static_assert((I) >= 0 && (I) < type_count, "Alternative index out of range")

#define TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(obj, Is, ...)                                          \
    do                                                                                               \
    {                                                                                                \
        if constexpr (sizeof...(Alternatives) == 1)                                                  \
        {                                                                                            \
            if (constexpr impl::SizeT Is = 0; (obj)._index == Is)                                    \
            {                                                                                        \
                __VA_ARGS__;                                                                         \
            }                                                                                        \
        }                                                                                            \
        else if constexpr (sizeof...(Alternatives) == 2)                                             \
        {                                                                                            \
            if (constexpr impl::SizeT Is = 0; (obj)._index == Is)                                    \
            {                                                                                        \
                __VA_ARGS__;                                                                         \
            }                                                                                        \
            else if (constexpr impl::SizeT Is = 1; (obj)._index == Is)                               \
            {                                                                                        \
                __VA_ARGS__;                                                                         \
            }                                                                                        \
        }                                                                                            \
        else                                                                                         \
        {                                                                                            \
            [&]<impl::SizeT... Is>(impl::index_sequence<Is...>) TINYVARIANT_ALWAYS_INLINE_LAMBDA     \
            { ((((obj)._index == Is) ? ((__VA_ARGS__), 0) : 0), ...); }(alternative_index_sequence); \
        }                                                                                            \
    } while (false)

#define TINYVARIANT_DO_WITH_CURRENT_INDEX(Is, ...) TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ((*this), Is, __VA_ARGS__)

    template <typename T, impl::SizeT I, typename... Args>
    [[nodiscard,
      gnu::always_inline]] explicit tinyvariant(impl::inplace_type_t<T>, impl::inplace_index_t<I>, Args&&... args) noexcept
    :
    _index{static_cast<index_type>(I)}
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        SFML_BASE_PLACEMENT_NEW(_buffer) T{static_cast<Args&&>(args)...};
    }

    template <impl::SizeT I>
    [[gnu::always_inline]] void destroy_at() noexcept
    {
        as<nth_type<I>>().~nth_type<I>();
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
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(impl::inplace_index_t<I> inplace_index, Args&&... args) noexcept
    :
    tinyvariant{inplace_type<nth_type<I>>, inplace_index, static_cast<Args&&>(args)...}
    {
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(T&& x) noexcept :
    tinyvariant{inplace_type<T>, static_cast<T&&>(x)}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit tinyvariant() noexcept : tinyvariant{inplace_index<0>}
    {
    }

    [[gnu::always_inline]] tinyvariant(const tinyvariant& rhs) : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          SFML_BASE_PLACEMENT_NEW(_buffer) nth_type<I>(static_cast<const nth_type<I>&>(
                                              *reinterpret_cast<const nth_type<I>*>(rhs._buffer))));
    }

    [[gnu::always_inline]] tinyvariant(tinyvariant&& rhs) noexcept : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          SFML_BASE_PLACEMENT_NEW(_buffer) nth_type<I>(
                                              static_cast<nth_type<I>&&>(*reinterpret_cast<nth_type<I>*>(rhs._buffer))));
    }

    // Avoid forwarding constructor hijack.
    [[gnu::always_inline]] tinyvariant(const tinyvariant&& rhs) noexcept :
    tinyvariant{static_cast<const tinyvariant&>(rhs)}
    {
    }

    // Avoid forwarding constructor hijack.
    [[gnu::always_inline]] tinyvariant(tinyvariant& rhs) : tinyvariant{static_cast<const tinyvariant&>(rhs)}
    {
    }

    [[gnu::always_inline]] ~tinyvariant()
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());
    }

    [[gnu::always_inline]] tinyvariant& operator=(const tinyvariant& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }

        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                              I,
                                              (SFML_BASE_PLACEMENT_NEW(_buffer) nth_type<I>(rhs.template as<nth_type<I>>())));
        _index = rhs._index;

        return *this;
    }

    [[gnu::always_inline]] tinyvariant& operator=(tinyvariant& rhs)
    {
        return ((*this) = static_cast<const tinyvariant&>(rhs));
    }

    [[gnu::always_inline]] tinyvariant& operator=(tinyvariant&& rhs) noexcept
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs,
                                              I,
                                              (SFML_BASE_PLACEMENT_NEW(_buffer) nth_type<I>(
                                                  static_cast<nth_type<I>&&>(rhs.template as<nth_type<I>>()))));
        _index = rhs._index;

        return *this;
    }

    [[gnu::always_inline]] tinyvariant& operator=(const tinyvariant&& rhs)
    {
        return ((*this) = static_cast<const tinyvariant&>(rhs));
    }

    template <typename T>
    [[gnu::always_inline]] tinyvariant& operator=(T&& x)
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I, destroy_at<I>());

        using type = impl::uncvref_t<T>;

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
    [[nodiscard, gnu::always_inline]] T& as() & noexcept
    {
        return *(reinterpret_cast<T*>(_buffer));
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] const T& as() const& noexcept
    {
        return *(reinterpret_cast<const T*>(_buffer));
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] T&& as() && noexcept
    {
        return static_cast<T&&>(*(reinterpret_cast<T*>(_buffer)));
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] auto& get_by_index() & noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<nth_type<I>>();
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] const auto& get_by_index() const& noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return as<nth_type<I>>();
    }

    template <impl::SizeT I>
    [[nodiscard, gnu::always_inline]] auto&& get_by_index() && noexcept
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        return static_cast<nth_type<I>&&>(as<nth_type<I>>());
    }

    template <typename Visitor, typename R = decltype(impl::declval<Visitor>()(impl::declval<nth_type<0>>()))>
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

    template <typename Visitor, typename R = decltype(impl::declval<Visitor>()(impl::declval<nth_type<0>>()))>
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
    [[nodiscard, gnu::always_inline]] auto recursive_match(
        Fs&&... fs) & -> decltype(recursive_visit(impl::overload_set{static_cast<Fs&&>(fs)...}))
    {
        return recursive_visit(impl::overload_set{static_cast<Fs&&>(fs)...});
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto recursive_match(
        Fs&&... fs) const& -> decltype(recursive_visit(impl::overload_set{static_cast<Fs&&>(fs)...}))
    {
        return recursive_visit(impl::overload_set{static_cast<Fs&&>(fs)...});
    }

    template <typename Visitor, typename R = decltype(impl::declval<Visitor>()(impl::declval<nth_type<0>>()))>
    [[nodiscard, gnu::always_inline]] R linear_visit(Visitor&& visitor) &
    {
        if constexpr (impl::is_reference<R>)
        {
            impl::uncvref_t<R>* ret;
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(get_by_index<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (impl::is_void<R>)
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

            return *(reinterpret_cast<R*>(ret_buffer));
        }
    }


    template <typename Visitor, typename R = decltype(impl::declval<Visitor>()(impl::declval<nth_type<0>>()))>
    [[nodiscard, gnu::always_inline]] R linear_visit(Visitor&& visitor) const&
    {
        if constexpr (impl::is_reference<R>)
        {
            impl::uncvref_t<R>* ret;
            TINYVARIANT_DO_WITH_CURRENT_INDEX(I, ret = &(visitor(get_by_index<I>())));
            return static_cast<R>(*ret);
        }
        else if constexpr (impl::is_void<R>)
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

            return *(reinterpret_cast<R*>(ret_buffer));
        }
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linear_match(
        Fs&&... fs) & -> decltype(linear_visit(impl::overload_set{static_cast<Fs&&>(fs)...}))
    {
        return linear_visit(impl::overload_set{static_cast<Fs&&>(fs)...});
    }

    template <typename... Fs>
    [[nodiscard, gnu::always_inline]] auto linear_match(
        Fs&&... fs) const& -> decltype(linear_visit(impl::overload_set{static_cast<Fs&&>(fs)...}))
    {
        return linear_visit(impl::overload_set{static_cast<Fs&&>(fs)...});
    }
};

#undef TINYVARIANT_DO_WITH_CURRENT_INDEX
#undef TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ
#undef TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY

#undef TINYVARIANT_USE_STD_INDEX_SEQUENCE
#undef TINYVARIANT_USE_INTEGER_PACK
#undef TINYVARIANT_USE_MAKE_INTEGER_SEQ
#undef TINYVARIANT_USE_TYPE_PACK_ELEMENT

} // namespace sfvr

#pragma GCC diagnostic pop

// NOLINTEND
