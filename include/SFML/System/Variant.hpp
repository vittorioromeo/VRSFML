#pragma once

#include <initializer_list> // TODO: remove dependency?
#include <new>              // TODO: remove dependency?

#include <cassert>

#if ((__GNUC__ >= 10) || defined(__clang__)) && !defined(_MSC_VER)
#define TINYVARIANT_SUPPORTS_HAS_BUILTIN
#endif

#ifdef TINYVARIANT_SUPPORTS_HAS_BUILTIN
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

#ifdef TINYVARIANT_SUPPORTS_HAS_BUILTIN
#if __has_builtin(__type_pack_element)
#define TINYVARIANT_USE_TYPE_PACK_ELEMENT
#endif
#endif

#ifdef TINYVARIANT_USE_STD_INDEX_SEQUENCE
#include <utility>
#endif

namespace sf::priv
{

template <typename T>
T&& declval();

using sz_t = decltype(sizeof(int));

#ifdef TINYVARIANT_USE_STD_INDEX_SEQUENCE

template <sz_t... Is>
using index_sequence = std::index_sequence<Is...>;

#else

template <sz_t...>
struct index_sequence
{
};

#endif

#ifdef TINYVARIANT_USE_MAKE_INTEGER_SEQ

template <typename, sz_t... X>
struct index_sequence_helper
{
    using type = index_sequence<X...>;
};

template <sz_t N>
using index_sequence_up_to = typename __make_integer_seq<index_sequence_helper, sz_t, N>::type;

#elif defined(TINYVARIANT_USE_INTEGER_PACK)

template <sz_t N>
using index_sequence_up_to = index_sequence<__integer_pack(N)...>;

#elif defined(TINYVARIANT_USE_STD_INDEX_SEQUENCE)

template <sz_t N>
using index_sequence_up_to = std::make_index_sequence<N>;

#else

#error "No integer sequence generation available."

#endif

template <typename, typename>
inline constexpr bool is_same_type = false;

template <typename T>
inline constexpr bool is_same_type<T, T> = true;

template <typename T>
[[nodiscard, gnu::always_inline]] constexpr T variadic_max(std::initializer_list<T> il) noexcept
{
    T result = 0;

    for (T value : il)
        if (value > result)
            result = value;

    return result;
}

enum : sz_t
{
    bad_index = static_cast<sz_t>(-1)
};

template <typename T, typename... Ts>
[[nodiscard, gnu::always_inline]] constexpr sz_t get_index_of() noexcept
{
    constexpr bool matches[]{is_same_type<T, Ts>...};

    for (sz_t i = 0; i < sizeof...(Ts); ++i)
        if (matches[i])
            return i;

    return bad_index;
};

template <typename T>
struct type_wrapper
{
    using type = T;
};

#ifdef TINYVARIANT_USE_TYPE_PACK_ELEMENT

template <sz_t N, typename... Ts>
using type_at = __type_pack_element<N, Ts...>;

#else

template <sz_t N,
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
[[nodiscard, gnu::always_inline]] constexpr auto type_at_impl() noexcept
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

template <sz_t N, typename... Ts>
using type_at = typename decltype(type_at_impl<N, Ts...>())::type;

#endif

template <typename>
struct tinyvariant_inplace_type_t
{
};

template <sz_t>
struct tinyvariant_inplace_index_t
{
};

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
inline constexpr tinyvariant_inplace_type_t<T> tinyvariant_inplace_type{};

template <sz_t N>
inline constexpr tinyvariant_inplace_index_t<N> tinyvariant_inplace_index{};

template <typename... Alternatives>
class [[nodiscard]] tinyvariant
{
private:
    using byte = unsigned char;

    enum : sz_t
    {
        type_count    = sizeof...(Alternatives),
        max_alignment = variadic_max({alignof(Alternatives)...}),
        max_size      = variadic_max({sizeof(Alternatives)...})
    };

    using index_type = unsigned char;

    template <sz_t I>
    using nth_type = type_at<I, Alternatives...>;

public:
    template <typename T>
    static constexpr sz_t index_of = get_index_of<T, Alternatives...>();

private:
    static constexpr index_sequence_up_to<type_count> alternative_index_sequence{};

    alignas(max_alignment) byte _buffer[max_size];
    index_type _index;

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ >= 10)
#define TINYVARIANT_ALWAYS_INLINE_LAMBDA [[gnu::always_inline]]
#else
#define TINYVARIANT_ALWAYS_INLINE_LAMBDA
#endif

#define TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I)                               \
    static_assert((I) != bad_index, "Alternative type not supported by variant"); \
                                                                                  \
    static_assert((I) >= 0 && (I) < type_count, "Alternative index out of range")

#define TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(obj, Is, ...)                                      \
    do                                                                                           \
    {                                                                                            \
        [&]<sz_t... Is>(index_sequence<Is...>) TINYVARIANT_ALWAYS_INLINE_LAMBDA                  \
        { ((((obj)._index == Is) ? ((__VA_ARGS__), 0) : 0), ...); }(alternative_index_sequence); \
    } while (false)

#define TINYVARIANT_DO_WITH_CURRENT_INDEX(Is, ...) TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ((*this), Is, __VA_ARGS__)

    template <typename T, sz_t I, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(tinyvariant_inplace_type_t<T>,
                                                           tinyvariant_inplace_index_t<I>,
                                                           Args&&... args) noexcept :
    _index{static_cast<index_type>(I)}
    {
        TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY(I);
        new (_buffer) T{static_cast<Args&&>(args)...};
    }

    template <sz_t I>
    [[gnu::always_inline]] void destroy_at() noexcept
    {
        as<nth_type<I>>().~nth_type<I>();
    }

public:
    template <typename T, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(tinyvariant_inplace_type_t<T> inplace_type, Args&&... args) noexcept
    :
    tinyvariant{inplace_type, tinyvariant_inplace_index<index_of<T>>, static_cast<Args&&>(args)...}
    {
    }

    template <sz_t I, typename... Args>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(tinyvariant_inplace_index_t<I> inplace_index, Args&&... args) noexcept
    :
    tinyvariant{tinyvariant_inplace_type<nth_type<I>>, inplace_index, static_cast<Args&&>(args)...}
    {
    }

    template <typename T>
    [[nodiscard, gnu::always_inline]] explicit tinyvariant(T&& x) noexcept :
    tinyvariant{tinyvariant_inplace_type<T>, static_cast<T&&>(x)}
    {
    }

    [[nodiscard, gnu::always_inline]] explicit tinyvariant() noexcept : tinyvariant{tinyvariant_inplace_index<0>}
    {
    }

    [[gnu::always_inline]] tinyvariant(const tinyvariant& rhs) : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          new (_buffer) nth_type<I>(static_cast<const nth_type<I>&>(
                                              *reinterpret_cast<const nth_type<I>*>(rhs._buffer))));
    }

    [[gnu::always_inline]] tinyvariant(tinyvariant&& rhs) noexcept : _index{rhs._index}
    {
        TINYVARIANT_DO_WITH_CURRENT_INDEX(I,
                                          new (_buffer) nth_type<I>(
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

        TINYVARIANT_DO_WITH_CURRENT_INDEX_OBJ(rhs, I, (new (_buffer) nth_type<I>(rhs.template as<nth_type<I>>())));
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
                                              (new (_buffer) nth_type<I>(
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

        using type = uncvref_t<T>;

        new (_buffer) type{static_cast<T&&>(x)};
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
};

#undef TINYVARIANT_DO_WITH_CURRENT_INDEX
#undef TINYVARIANT_STATIC_ASSERT_INDEX_VALIDITY

#undef TINYVARIANT_USE_STD_INDEX_SEQUENCE
#undef TINYVARIANT_USE_INTEGER_PACK
#undef TINYVARIANT_USE_MAKE_INTEGER_SEQ
#undef TINYVARIANT_USE_TYPE_PACK_ELEMENT

} // namespace sf::priv
