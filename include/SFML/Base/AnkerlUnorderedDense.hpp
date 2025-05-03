// Small fork of ankerl::unordered_dense to use SFML's base library.
// Removes allocator support.

///////////////////////// ankerl::unordered_dense::{map, set} /////////////////////////

// A fast & densely stored hashmap and hashset based on robin-hood backward shift deletion.
// Version 4.5.0
// https://github.com/martinus/unordered_dense
//
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2022-2024 Martin Leitner-Ankerl <martin.ankerl@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// NOLINTBEGIN(readability-identifier-naming)

#ifndef ANKERL_UNORDERED_DENSE_H
#define ANKERL_UNORDERED_DENSE_H


#pragma GCC system_header

// see https://semver.org/spec/v2.0.0.html
#define ANKERL_UNORDERED_DENSE_VERSION_MAJOR 4 // NOLINT(cppcoreguidelines-macro-usage) incompatible API changes
#define ANKERL_UNORDERED_DENSE_VERSION_MINOR 5 // NOLINT(cppcoreguidelines-macro-usage) backwards compatible functionality
#define ANKERL_UNORDERED_DENSE_VERSION_PATCH 0 // NOLINT(cppcoreguidelines-macro-usage) backwards compatible bug fixes

// API versioning with inline namespace, see https://www.foonathan.net/2018/11/inline-namespaces/

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ANKERL_UNORDERED_DENSE_VERSION_CONCAT1(major, minor, patch) v##major##_##minor##_##patch
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ANKERL_UNORDERED_DENSE_VERSION_CONCAT(major, minor, patch) ANKERL_UNORDERED_DENSE_VERSION_CONCAT1(major, minor, patch)
#define ANKERL_UNORDERED_DENSE_NAMESPACE   \
    ANKERL_UNORDERED_DENSE_VERSION_CONCAT( \
        ANKERL_UNORDERED_DENSE_VERSION_MAJOR, ANKERL_UNORDERED_DENSE_VERSION_MINOR, ANKERL_UNORDERED_DENSE_VERSION_PATCH)

#if defined(_MSVC_LANG)
#    define ANKERL_UNORDERED_DENSE_CPP_VERSION _MSVC_LANG
#else
#    define ANKERL_UNORDERED_DENSE_CPP_VERSION __cplusplus
#endif

#if defined(__GNUC__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#    define ANKERL_UNORDERED_DENSE_PACK(decl) decl __attribute__((__packed__))
#elif defined(_MSC_VER)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#    define ANKERL_UNORDERED_DENSE_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#endif

// defined in unordered_dense.cpp
#if !defined(ANKERL_UNORDERED_DENSE_EXPORT)
#    define ANKERL_UNORDERED_DENSE_EXPORT
#endif

#if ANKERL_UNORDERED_DENSE_CPP_VERSION < 201703L
#    error ankerl::unordered_dense requires C++17 or higher
#else

#include "SFML/Base/Abort.hpp"
#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Array.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/Memset.hpp"
#include "SFML/Base/DeclVal.hpp"
#include "SFML/Base/IndexSequence.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/MakeIndexSequence.hpp"
#include "SFML/Base/MinMax.hpp"
#include "SFML/Base/Vector.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/Conditional.hpp"
#include "SFML/Base/Traits/EnableIf.hpp"
#include "SFML/Base/Traits/IsConstructible.hpp"
#include "SFML/Base/Traits/IsConvertible.hpp"
#include "SFML/Base/Traits/IsEnum.hpp"
#include "SFML/Base/Traits/IsIntegral.hpp"
#include "SFML/Base/Traits/IsNothrowMoveAssignable.hpp"
#include "SFML/Base/Traits/IsNothrowSwappable.hpp"
#include "SFML/Base/Traits/IsSame.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"
#include "SFML/Base/Traits/IsVoid.hpp"
#include "SFML/Base/Traits/RemoveCVRef.hpp"
#include "SFML/Base/Vector.hpp"

#include <cstdint> // uintptr_t

#    include <tuple>            // for forward_as_tuple
#    include <utility>          // for pair, as_const, piece...

#    if defined(_MSC_VER) && defined(_M_X64)
#        include <intrin.h>
#        pragma intrinsic(_umul128)
#    endif

#    if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__clang__)
#        define ANKERL_UNORDERED_DENSE_LIKELY(x) __builtin_expect(x, 1)   // NOLINT(cppcoreguidelines-macro-usage)
#        define ANKERL_UNORDERED_DENSE_UNLIKELY(x) __builtin_expect(x, 0) // NOLINT(cppcoreguidelines-macro-usage)
#    else
#        define ANKERL_UNORDERED_DENSE_LIKELY(x) (x)   // NOLINT(cppcoreguidelines-macro-usage)
#        define ANKERL_UNORDERED_DENSE_UNLIKELY(x) (x) // NOLINT(cppcoreguidelines-macro-usage)
#    endif

#ifdef _LIBCPP_BEGIN_NAMESPACE_STD

_LIBCPP_BEGIN_NAMESPACE_STD

    template <typename> struct hash;

_LIBCPP_END_NAMESPACE_STD

#else

namespace std {
    template <typename> struct hash;
} // namespace std

#endif

namespace ankerl::unordered_dense {
inline namespace ANKERL_UNORDERED_DENSE_NAMESPACE {

namespace detail {

    template <typename T>
    using vector = sf::base::Vector<T>;

template <typename I>
concept subtractable = requires(const I& a, const I& b) {
    { b - a };
};

template <typename Iter>
constexpr sf::base::PtrDiffT my_distance(Iter first, Iter last)
{
    if constexpr (subtractable<Iter>)
    {
        return static_cast<sf::base::PtrDiffT>(last - first);
    }
    else
    {
        sf::base::PtrDiffT count = 0;
        while (first != last)
        {
            ++first;
            ++count;
        }
        return count;
    }
}

template <typename T>
class my_allocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = sf::base::SizeT;
    using difference_type = sf::base::PtrDiffT;

    template <typename U>
    struct rebind {
        using other = my_allocator<U>;
    };

    constexpr my_allocator() noexcept = default;

    template <typename U>
    constexpr my_allocator(const my_allocator<U>& /*other*/) noexcept {}

    ~my_allocator() = default;

    [[nodiscard]] pointer allocate(size_type n) {
        if (n == 0) {
            return nullptr; // Standard behavior for n=0 often implementation-defined, nullptr is safe.
        }

        if (n > max_size()) {
            sf::base::abort();
        }

        void* raw_mem = ::operator new(n * sizeof(T));
        return static_cast<pointer>(raw_mem);
    }

    void deallocate(pointer p, size_type /*n*/) noexcept {
        ::operator delete(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
        ::new (static_cast<void*>(p)) U(SFML_BASE_FORWARD(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept {
        p->~U();
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept {
        return static_cast<size_type>(-1) / sizeof(T);
    }

    pointer address(reference x) const noexcept {
        return &x;
    }

    const_pointer address(const_reference x) const noexcept {
        return &x;
    }

    template <typename U>
    constexpr bool operator==(const my_allocator<U>& /*other*/) const noexcept {
        return true;
    }

    template <typename U>
    constexpr bool operator!=(const my_allocator<U>& /*other*/) const noexcept {
        return false;
    }
};

template <>
class my_allocator<void> {
public:
    using value_type = void;
    using pointer = void*;
    using const_pointer = const void*;
    using size_type = sf::base::SizeT;
    using difference_type = sf::base::PtrDiffT;

    template <typename U>
    struct rebind {
        using other = my_allocator<U>;
    };

    template <typename U>
    constexpr my_allocator(const my_allocator<U>& /*other*/) noexcept {}
    constexpr my_allocator() noexcept = default;


    template <typename U>
    constexpr bool operator==(const my_allocator<U>& /*other*/) const noexcept {
        return true; // All stateless allocators are equal
    }

    template <typename U>
    constexpr bool operator!=(const my_allocator<U>& /*other*/) const noexcept {
        return false;
    }
};


template<class T = void>
struct EqualTo;

template<typename T>
struct EqualTo
{
  constexpr bool operator()(const T& x, const T& y) const { return x == y; }
};

} // namespace detail

// hash ///////////////////////////////////////////////////////////////////////

// This is a stripped-down implementation of wyhash: https://github.com/wangyi-fudan/wyhash
// No big-endian support (because different values on different machines don't matter),
// hardcodes seed and the secret, reformats the code, and clang-tidy fixes.
namespace detail::wyhash {

inline void mum(sf::base::U64* a, sf::base::U64* b) {
#    if defined(__SIZEOF_INT128__)
    __uint128_t r = *a;
    r *= *b;
    *a = static_cast<sf::base::U64>(r);
    *b = static_cast<sf::base::U64>(r >> 64U);
#    elif defined(_MSC_VER) && defined(_M_X64)
    *a = _umul128(*a, *b, b);
#    else
    sf::base::U64 ha = *a >> 32U;
    sf::base::U64 hb = *b >> 32U;
    sf::base::U64 la = static_cast<sf::base::U32>(*a);
    sf::base::U64 lb = static_cast<sf::base::U32>(*b);
    sf::base::U64 hi{};
    sf::base::U64 lo{};
    sf::base::U64 rh = ha * hb;
    sf::base::U64 rm0 = ha * lb;
    sf::base::U64 rm1 = hb * la;
    sf::base::U64 rl = la * lb;
    sf::base::U64 t = rl + (rm0 << 32U);
    auto c = static_cast<sf::base::U64>(t < rl);
    lo = t + (rm1 << 32U);
    c += static_cast<sf::base::U64>(lo < t);
    hi = rh + (rm0 >> 32U) + (rm1 >> 32U) + c;
    *a = lo;
    *b = hi;
#    endif
}

// multiply and xor mix function, aka MUM
[[nodiscard]] inline auto mix(sf::base::U64 a, sf::base::U64 b) -> sf::base::U64 {
    mum(&a, &b);
    return a ^ b;
}

// read functions. WARNING: we don't care about endianness, so results are different on big endian!
[[nodiscard]] inline auto r8(const sf::base::U8* p) -> sf::base::U64 {
    sf::base::U64 v{};
    SFML_BASE_MEMCPY(&v, p, 8U);
    return v;
}

[[nodiscard]] inline auto r4(const sf::base::U8* p) -> sf::base::U64 {
    sf::base::U32 v{};
    SFML_BASE_MEMCPY(&v, p, 4);
    return v;
}

// reads 1, 2, or 3 bytes
[[nodiscard]] inline auto r3(const sf::base::U8* p, sf::base::SizeT k) -> sf::base::U64 {
    return (static_cast<sf::base::U64>(p[0]) << 16U) | (static_cast<sf::base::U64>(p[k >> 1U]) << 8U) | p[k - 1];
}

[[maybe_unused]] [[nodiscard]] inline auto hash(void const* key, sf::base::SizeT len) -> sf::base::U64 {
    static constexpr auto secret = sf::base::Array{sf::base::U64(0xa0761d6478bd642f),
                                              sf::base::U64(0xe7037ed1a0b428db),
                                              sf::base::U64(0x8ebc6af09c88c6e3),
                                              sf::base::U64(0x589965cc75374cc3)};

    auto const* p = static_cast<sf::base::U8 const*>(key);
    sf::base::U64 seed = secret[0];
    sf::base::U64 a{};
    sf::base::U64 b{};
    if (ANKERL_UNORDERED_DENSE_LIKELY(len <= 16)) {
        if (ANKERL_UNORDERED_DENSE_LIKELY(len >= 4)) {
            a = (r4(p) << 32U) | r4(p + ((len >> 3U) << 2U));
            b = (r4(p + len - 4) << 32U) | r4(p + len - 4 - ((len >> 3U) << 2U));
        } else if (ANKERL_UNORDERED_DENSE_LIKELY(len > 0)) {
            a = r3(p, len);
            b = 0;
        } else {
            a = 0;
            b = 0;
        }
    } else {
        sf::base::SizeT i = len;
        if (ANKERL_UNORDERED_DENSE_UNLIKELY(i > 48)) {
            sf::base::U64 see1 = seed;
            sf::base::U64 see2 = seed;
            do {
                seed = mix(r8(p) ^ secret[1], r8(p + 8) ^ seed);
                see1 = mix(r8(p + 16) ^ secret[2], r8(p + 24) ^ see1);
                see2 = mix(r8(p + 32) ^ secret[3], r8(p + 40) ^ see2);
                p += 48;
                i -= 48;
            } while (ANKERL_UNORDERED_DENSE_LIKELY(i > 48));
            seed ^= see1 ^ see2;
        }
        while (ANKERL_UNORDERED_DENSE_UNLIKELY(i > 16)) {
            seed = mix(r8(p) ^ secret[1], r8(p + 8) ^ seed);
            i -= 16;
            p += 16;
        }
        a = r8(p + i - 16);
        b = r8(p + i - 8);
    }

    return mix(secret[1] ^ len, mix(a ^ secret[1], b ^ seed));
}

[[nodiscard]] inline auto hash(sf::base::U64 x) -> sf::base::U64 {
    return detail::wyhash::mix(x, sf::base::U64(0x9E3779B97F4A7C15));
}

} // namespace detail::wyhash

ANKERL_UNORDERED_DENSE_EXPORT template <typename T, typename Enable = void>
struct hash {
    auto operator()(T const& obj) const noexcept(noexcept(sf::base::declVal<std::hash<T>>().operator()(sf::base::declVal<T const&>())))
        -> sf::base::U64 {
        return std::hash<T>{}(obj);
    }
};

template <typename T>
struct hash<T, typename std::hash<T>::is_avalanching> {
    using is_avalanching = void;
    auto operator()(T const& obj) const noexcept(noexcept(sf::base::declVal<std::hash<T>>().operator()(sf::base::declVal<T const&>())))
        -> sf::base::U64 {
        return std::hash<T>{}(obj);
    }
};

template <class T>
struct hash<T*> {
    using is_avalanching = void;
    auto operator()(T* ptr) const noexcept -> sf::base::U64 {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return detail::wyhash::hash(reinterpret_cast<uintptr_t>(ptr));
    }
};

/*
template <class T>
struct hash<std::unique_ptr<T>> {
    using is_avalanching = void;
    auto operator()(std::unique_ptr<T> const& ptr) const noexcept -> sf::base::U64 {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return detail::wyhash::hash(reinterpret_cast<uintptr_t>(ptr.get()));
    }
};

template <class T>
struct hash<std::shared_ptr<T>> {
    using is_avalanching = void;
    auto operator()(std::shared_ptr<T> const& ptr) const noexcept -> sf::base::U64 {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        return detail::wyhash::hash(reinterpret_cast<uintptr_t>(ptr.get()));
    }
};
*/

template <typename Enum>
struct hash<Enum, sf::base::EnableIf<SFML_BASE_IS_ENUM(Enum)>> {
    using is_avalanching = void;
    auto operator()(Enum e) const noexcept -> sf::base::U64 {
        using underlying = typename std::underlying_type_t<Enum>;
        return detail::wyhash::hash(static_cast<underlying>(e));
    }
};

template <typename StringLike>
struct hash<StringLike, sf::base::EnableIf<requires (const StringLike& stringLike) {
    stringLike.data();
    stringLike.size();
}>> {
    using is_avalanching = void;
    auto operator()(const StringLike& stringLike) const noexcept -> sf::base::U64 {
        return detail::wyhash::hash(stringLike.data(), sizeof(SFML_BASE_REMOVE_CVREF(decltype(stringLike[0]))) * stringLike.size());
    }
};

template <typename... Args>
struct tuple_hash_helper {
    // Converts the value into 64bit. If it is an integral type, just cast it. Mixing is doing the rest.
    // If it isn't an integral we need to hash it.
    template <typename Arg>
    [[nodiscard]] constexpr static auto to64(Arg const& arg) -> sf::base::U64 {
        if constexpr (sf::base::isIntegral<Arg> || SFML_BASE_IS_ENUM(Arg)) {
            return static_cast<sf::base::U64>(arg);
        } else {
            return hash<Arg>{}(arg);
        }
    }

    [[nodiscard]] static auto mix64(sf::base::U64 state, sf::base::U64 v) -> sf::base::U64 {
        return detail::wyhash::mix(state + v, sf::base::U64{0x9ddfea08eb382d69});
    }

    // Creates a buffer that holds all the data from each element of the tuple. If possible we memcpy the data directly. If
    // not, we hash the object and use this for the array. Size of the array is known at compile time, and memcpy is optimized
    // away, so filling the buffer is highly efficient. Finally, call wyhash with this buffer.
    template <typename T, sf::base::SizeT... Idx>
    [[nodiscard]] static auto calc_hash(T const& t, sf::base::IndexSequence<Idx...>) noexcept -> sf::base::U64 {
        auto h = sf::base::U64{};
        ((h = mix64(h, to64(std::get<Idx>(t)))), ...);
        return h;
    }
};

template <typename... Args>
struct hash<std::tuple<Args...>> : tuple_hash_helper<Args...> {
    using is_avalanching = void;
    auto operator()(std::tuple<Args...> const& t) const noexcept -> sf::base::U64 {
        return tuple_hash_helper<Args...>::calc_hash(t, SFML_BASE_MAKE_INDEX_SEQUENCE(sizeof...(Args)){});
    }
};

template <typename A, typename B>
struct hash<std::pair<A, B>> : tuple_hash_helper<A, B> {
    using is_avalanching = void;
    auto operator()(std::pair<A, B> const& t) const noexcept -> sf::base::U64 {
        return tuple_hash_helper<A, B>::calc_hash(t, SFML_BASE_MAKE_INDEX_SEQUENCE(2){});
    }
};

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#    define ANKERL_UNORDERED_DENSE_HASH_STATICCAST(T)                    \
        template <>                                                      \
        struct hash<T> {                                                 \
            using is_avalanching = void;                                 \
            auto operator()(T const& obj) const noexcept -> sf::base::U64 {   \
                return detail::wyhash::hash(static_cast<sf::base::U64>(obj)); \
            }                                                            \
        }

#    if defined(__GNUC__) && !defined(__clang__)
#        pragma GCC diagnostic push
#        pragma GCC diagnostic ignored "-Wuseless-cast"
#    endif
// see https://en.cppreference.com/w/cpp/utility/hash
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(bool);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(char);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(signed char);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(unsigned char);
#    if ANKERL_UNORDERED_DENSE_CPP_VERSION >= 202002L && defined(__cpp_char8_t)
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(char8_t);
#    endif
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(char16_t);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(char32_t);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(wchar_t);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(short);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(unsigned short);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(int);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(unsigned int);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(long);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(long long);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(unsigned long);
ANKERL_UNORDERED_DENSE_HASH_STATICCAST(unsigned long long);

#    if defined(__GNUC__) && !defined(__clang__)
#        pragma GCC diagnostic pop
#    endif

// bucket_type //////////////////////////////////////////////////////////

namespace bucket_type {

struct standard {
    static constexpr sf::base::U32 dist_inc = 1U << 8U;             // skip 1 byte fingerprint
    static constexpr sf::base::U32 fingerprint_mask = dist_inc - 1; // mask for 1 byte of fingerprint

    sf::base::U32 m_dist_and_fingerprint; // upper 3 byte: distance to original bucket. lower byte: fingerprint from hash
    sf::base::U32 m_value_idx;            // index into the m_values vector.
};

ANKERL_UNORDERED_DENSE_PACK(struct big {
    static constexpr sf::base::U32 dist_inc = 1U << 8U;             // skip 1 byte fingerprint
    static constexpr sf::base::U32 fingerprint_mask = dist_inc - 1; // mask for 1 byte of fingerprint

    sf::base::U32 m_dist_and_fingerprint; // upper 3 byte: distance to original bucket. lower byte: fingerprint from hash
    sf::base::SizeT m_value_idx;              // index into the m_values vector.
});

} // namespace bucket_type

namespace detail {

struct nonesuch {};
struct default_container_t {};

template <class Default, class AlwaysVoid, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, std::void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detail::detector<detail::nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <typename T>
using detect_avalanching = typename T::is_avalanching;

template <typename T>
using detect_is_transparent = typename T::is_transparent;

template <typename T>
using detect_iterator = typename T::iterator;

template <typename T>
using detect_reserve = decltype(sf::base::declVal<T&>().reserve(sf::base::SizeT{}));

// enable_if helpers

template <typename Mapped>
constexpr bool is_map_v = !sf::base::isVoid<Mapped>;

// clang-format off
template <typename Hash, typename KeyEqual>
constexpr bool is_transparent_v = is_detected_v<detect_is_transparent, Hash> && is_detected_v<detect_is_transparent, KeyEqual>;
// clang-format on

template <typename From, typename To1, typename To2>
constexpr bool is_neither_convertible_v = !SFML_BASE_IS_CONVERTIBLE(From, To1) && !SFML_BASE_IS_CONVERTIBLE(From, To2);

template <typename T>
constexpr bool has_reserve = is_detected_v<detect_reserve, T>;

// base type for map has mapped_type
template <class T>
struct base_table_type_map {
    using mapped_type = T;
};

// base type for set doesn't have mapped_type
struct base_table_type_set {};

} // namespace detail

// Very much like std::deque, but faster for indexing (in most cases). As of now this doesn't implement the full detail::vector
// API, but merely what's necessary to work as an underlying container for ankerl::unordered_dense::{map, set}.
// It allocates blocks of equal size and puts them into the m_blocks vector. That means it can grow simply by adding a new
// block to the back of m_blocks, and doesn't double its size like an detail::vector. The disadvantage is that memory is not
// linear and thus there is one more indirection necessary for indexing.
template <typename T, sf::base::SizeT MaxSegmentSizeBytes = 4096>
class segmented_vector {
    template <bool IsConst>
    class iter_t;

public:
    using pointer = T*;
    using const_pointer = const T*;
    using difference_type = sf::base::PtrDiffT;
    using value_type = T;
    using size_type = sf::base::SizeT;
    using reference = T&;
    using const_reference = T const&;
    using iterator = iter_t<false>;
    using const_iterator = iter_t<true>;

private:
    detail::vector<pointer> m_blocks{};
    sf::base::SizeT m_size{};

    // Calculates the maximum number for x in  (s << x) <= max_val
    static constexpr auto num_bits_closest(sf::base::SizeT max_val, sf::base::SizeT s) -> sf::base::SizeT {
        auto f = sf::base::SizeT{0};
        while (s << (f + 1) <= max_val) {
            ++f;
        }
        return f;
    }

    using self_t = segmented_vector<T, MaxSegmentSizeBytes>;
    static constexpr auto num_bits = num_bits_closest(MaxSegmentSizeBytes, sizeof(T));
    static constexpr auto num_elements_in_block = 1U << num_bits;
    static constexpr auto mask = num_elements_in_block - 1U;

    /**
     * Iterator class doubles as const_iterator and iterator
     */
    template <bool IsConst>
    class iter_t {
        using ptr_t = typename sf::base::Conditional<IsConst, segmented_vector::const_pointer const*, segmented_vector::pointer*>;
        ptr_t m_data{};
        sf::base::SizeT m_idx{};

        template <bool B>
        friend class iter_t;

    public:
        using difference_type = segmented_vector::difference_type;
        using value_type = T;
        using reference = typename sf::base::Conditional<IsConst, value_type const&, value_type&>;
        using pointer = typename sf::base::Conditional<IsConst, segmented_vector::const_pointer, segmented_vector::pointer>;
        // using iterator_category = std::forward_iterator_tag;

        iter_t() noexcept = default;

        template <bool OtherIsConst>
        // NOLINTNEXTLINE(google-explicit-constructor,hicpp-explicit-conversions)
        constexpr iter_t(iter_t<OtherIsConst> const& other) noexcept
            requires (IsConst && !OtherIsConst) : m_data(other.m_data)
            , m_idx(other.m_idx) {}

        constexpr iter_t(ptr_t data, sf::base::SizeT idx) noexcept
            : m_data(data)
            , m_idx(idx) {}

        template <bool OtherIsConst>
        constexpr auto operator=(iter_t<OtherIsConst> const& other) noexcept -> iter_t& requires (IsConst && !OtherIsConst) {
            m_data = other.m_data;
            m_idx = other.m_idx;
            return *this;
        }

        constexpr auto operator++() noexcept -> iter_t& {
            ++m_idx;
            return *this;
        }

        constexpr auto operator++(int) noexcept -> iter_t {
            iter_t prev(*this);
            this->operator++();
            return prev;
        }

        constexpr auto operator+(difference_type diff) noexcept -> iter_t {
            return {m_data, static_cast<sf::base::SizeT>(static_cast<difference_type>(m_idx) + diff)};
        }

        template <bool OtherIsConst>
        constexpr auto operator-(iter_t<OtherIsConst> const& other) noexcept -> difference_type {
            return static_cast<difference_type>(m_idx) - static_cast<difference_type>(other.m_idx);
        }

        constexpr auto operator*() const noexcept -> reference {
            return m_data[m_idx >> num_bits][m_idx & mask];
        }

        constexpr auto operator->() const noexcept -> pointer {
            return &m_data[m_idx >> num_bits][m_idx & mask];
        }

        template <bool O>
        constexpr auto operator==(iter_t<O> const& o) const noexcept -> bool {
            return m_idx == o.m_idx;
        }

        template <bool O>
        constexpr auto operator!=(iter_t<O> const& o) const noexcept -> bool {
            return !(*this == o);
        }
    };

    // slow path: need to allocate a new segment every once in a while
    void increase_capacity() {
        auto ba = detail::my_allocator<pointer>{};
        pointer block = ba.allocate(num_elements_in_block);
        m_blocks.push_back(block);
    }

    // Moves everything from other
    void append_everything_from(segmented_vector&& other) {
        reserve(size() + other.size());
        for (auto&& o : other) {
            emplace_back(SFML_BASE_MOVE(o));
        }
    }

    // Copies everything from other
    void append_everything_from(segmented_vector const& other) {
        reserve(size() + other.size());
        for (auto const& o : other) {
            emplace_back(o);
        }
    }

    void dealloc() {
        auto ba = detail::my_allocator<pointer>{};
        for (auto ptr : m_blocks) {
            ba.deallocate(ptr, num_elements_in_block);
        }
    }

    [[nodiscard]] static constexpr auto calc_num_blocks_for_capacity(sf::base::SizeT capacity) {
        return (capacity + num_elements_in_block - 1U) / num_elements_in_block;
    }

public:
    segmented_vector() = default;

    segmented_vector(segmented_vector&& other) noexcept = default;

    segmented_vector(segmented_vector const& other) {
        append_everything_from(other);
    }

    auto operator=(segmented_vector const& other) -> segmented_vector& {
        if (this == &other) {
            return *this;
        }
        clear();
        append_everything_from(other);
        return *this;
    }

    auto operator=(segmented_vector&& other) noexcept -> segmented_vector& {
        clear();
        dealloc();

        m_blocks = SFML_BASE_MOVE(other.m_blocks);
        m_size = sf::base::exchange(other.m_size, {});

        return *this;
    }

    ~segmented_vector() {
        clear();
        dealloc();
    }

    [[nodiscard]] constexpr auto size() const -> sf::base::SizeT {
        return m_size;
    }

    [[nodiscard]] constexpr auto capacity() const -> sf::base::SizeT {
        return m_blocks.size() * num_elements_in_block;
    }

    // Indexing is highly performance critical
    [[nodiscard]] constexpr auto operator[](sf::base::SizeT i) const noexcept -> T const& {
        return m_blocks[i >> num_bits][i & mask];
    }

    [[nodiscard]] constexpr auto operator[](sf::base::SizeT i) noexcept -> T& {
        return m_blocks[i >> num_bits][i & mask];
    }

    [[nodiscard]] constexpr auto begin() -> iterator {
        return {m_blocks.data(), 0U};
    }
    [[nodiscard]] constexpr auto begin() const -> const_iterator {
        return {m_blocks.data(), 0U};
    }
    [[nodiscard]] constexpr auto cbegin() const -> const_iterator {
        return {m_blocks.data(), 0U};
    }

    [[nodiscard]] constexpr auto end() -> iterator {
        return {m_blocks.data(), m_size};
    }
    [[nodiscard]] constexpr auto end() const -> const_iterator {
        return {m_blocks.data(), m_size};
    }
    [[nodiscard]] constexpr auto cend() const -> const_iterator {
        return {m_blocks.data(), m_size};
    }

    [[nodiscard]] constexpr auto back() -> reference {
        return operator[](m_size - 1);
    }
    [[nodiscard]] constexpr auto back() const -> const_reference {
        return operator[](m_size - 1);
    }

    void pop_back() {
        back().~T();
        --m_size;
    }

    [[nodiscard]] auto empty() const {
        return 0 == m_size;
    }

    void reserve(sf::base::SizeT new_capacity) {
        m_blocks.reserve(calc_num_blocks_for_capacity(new_capacity));
        while (new_capacity > capacity()) {
            increase_capacity();
        }
    }

    template <class... Args>
    auto emplace_back(Args&&... args) -> reference {
        if (m_size == capacity()) {
            increase_capacity();
        }
        auto* ptr = static_cast<void*>(&operator[](m_size));
        auto& ref = *new (ptr) T(SFML_BASE_FORWARD(args)...);
        ++m_size;
        return ref;
    }

    void clear() {
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T)) {
            for (sf::base::SizeT i = 0, s = size(); i < s; ++i) {
                operator[](i).~T();
            }
        }
        m_size = 0;
    }

    void shrink_to_fit() {
        auto ba = detail::my_allocator<pointer>{};
        auto num_blocks_required = calc_num_blocks_for_capacity(m_size);
        while (m_blocks.size() > num_blocks_required) {
            ba.deallocate(m_blocks.back(), num_elements_in_block);
            m_blocks.popBack();
        }
        m_blocks.shrinkToFit();
    }
};

namespace detail {

// This is it, the table. Doubles as map and set, and uses `void` for T when its used as a set.
template <class Key,
          class T, // when void, treat it as a set.
          class Hash,
          class KeyEqual,
          class Bucket,
          class BucketContainer,
          bool IsSegmented>
class table : public sf::base::Conditional<is_map_v<T>, base_table_type_map<T>, base_table_type_set> {
    using underlying_value_type = typename sf::base::Conditional<is_map_v<T>, std::pair<Key, T>, Key>;
    using underlying_container_type = sf::base::Conditional<IsSegmented,
                                                         segmented_vector<underlying_value_type>,
                                                         detail::vector<underlying_value_type>>;

public:
    using value_container_type = underlying_container_type;

private:
    using default_bucket_container_type =
        sf::base::Conditional<IsSegmented, segmented_vector<Bucket>, detail::vector<Bucket>>;

    using bucket_container_type = sf::base::Conditional<SFML_BASE_IS_SAME(BucketContainer, detail::default_container_t),
                                                     default_bucket_container_type,
                                                     BucketContainer>;

    static constexpr sf::base::U8 initial_shifts = 64 - 2; // 2^(64-m_shift) number of buckets
    static constexpr float default_max_load_factor = 0.8F;

public:
    using key_type = Key;
    using value_type = typename value_container_type::value_type;
    using size_type = typename value_container_type::size_type;
    using difference_type = typename value_container_type::difference_type;
    using hasher = Hash;
    using key_equal = KeyEqual;
    using reference = typename value_container_type::reference;
    using const_reference = typename value_container_type::const_reference;
    using pointer = typename value_container_type::pointer;
    using const_pointer = typename value_container_type::const_pointer;
    using const_iterator = typename value_container_type::const_iterator;
    using iterator = sf::base::Conditional<is_map_v<T>, typename value_container_type::iterator, const_iterator>;
    using bucket_type = Bucket;

private:
    using value_idx_type = decltype(Bucket::m_value_idx);
    using dist_and_fingerprint_type = decltype(Bucket::m_dist_and_fingerprint);

    static_assert(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(Bucket), "assert there's no need to call destructor / std::destroy");
    static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(Bucket), "assert we can just memset / memcpy");

    value_container_type m_values{}; // Contains all the key-value pairs in one densely stored container. No holes.
    bucket_container_type m_buckets{};
    sf::base::SizeT m_max_bucket_capacity = 0;
    float m_max_load_factor = default_max_load_factor;
    Hash m_hash{};
    KeyEqual m_equal{};
    sf::base::U8 m_shifts = initial_shifts;

    [[nodiscard]] auto next(value_idx_type bucket_idx) const -> value_idx_type {
        return ANKERL_UNORDERED_DENSE_UNLIKELY(bucket_idx + 1U == bucket_count())
                   ? 0
                   : static_cast<value_idx_type>(bucket_idx + 1U);
    }

    // Helper to access bucket through pointer types
    [[nodiscard]] static constexpr auto at(bucket_container_type& bucket, sf::base::SizeT offset) -> Bucket& {
        return bucket[offset];
    }

    [[nodiscard]] static constexpr auto at(const bucket_container_type& bucket, sf::base::SizeT offset) -> const Bucket& {
        return bucket[offset];
    }

    // use the dist_inc and dist_dec functions so that uint16_t types work without warning
    [[nodiscard]] static constexpr auto dist_inc(dist_and_fingerprint_type x) -> dist_and_fingerprint_type {
        return static_cast<dist_and_fingerprint_type>(x + Bucket::dist_inc);
    }

    [[nodiscard]] static constexpr auto dist_dec(dist_and_fingerprint_type x) -> dist_and_fingerprint_type {
        return static_cast<dist_and_fingerprint_type>(x - Bucket::dist_inc);
    }

    // The goal of mixed_hash is to always produce a high quality 64bit hash.
    template <typename K>
    [[nodiscard]] constexpr auto mixed_hash(K const& key) const -> sf::base::U64 {
        if constexpr (is_detected_v<detect_avalanching, Hash>) {
            // we know that the hash is good because is_avalanching.
            if constexpr (sizeof(decltype(m_hash(key))) < sizeof(sf::base::U64)) {
                // 32bit hash and is_avalanching => multiply with a constant to avalanche bits upwards
                return m_hash(key) * sf::base::U64(0x9ddfea08eb382d69);
            } else {
                // 64bit and is_avalanching => only use the hash itself.
                return m_hash(key);
            }
        } else {
            // not is_avalanching => apply wyhash
            return wyhash::hash(m_hash(key));
        }
    }

    [[nodiscard]] constexpr auto dist_and_fingerprint_from_hash(sf::base::U64 hash) const -> dist_and_fingerprint_type {
        return Bucket::dist_inc | (static_cast<dist_and_fingerprint_type>(hash) & Bucket::fingerprint_mask);
    }

    [[nodiscard]] constexpr auto bucket_idx_from_hash(sf::base::U64 hash) const -> value_idx_type {
        return static_cast<value_idx_type>(hash >> m_shifts);
    }

    [[nodiscard]] static constexpr auto get_key(value_type const& vt) -> key_type const& {
        if constexpr (is_map_v<T>) {
            return vt.first;
        } else {
            return vt;
        }
    }

    template <typename K>
    [[nodiscard]] auto next_while_less(K const& key) const -> Bucket {
        auto hash = mixed_hash(key);
        auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
        auto bucket_idx = bucket_idx_from_hash(hash);

        while (dist_and_fingerprint < at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
        }
        return {dist_and_fingerprint, bucket_idx};
    }

    void place_and_shift_up(Bucket bucket, value_idx_type place) {
        while (0 != at(m_buckets, place).m_dist_and_fingerprint) {
            bucket = sf::base::exchange(at(m_buckets, place), bucket);
            bucket.m_dist_and_fingerprint = dist_inc(bucket.m_dist_and_fingerprint);
            place = next(place);
        }
        at(m_buckets, place) = bucket;
    }

    [[nodiscard]] static constexpr auto calc_num_buckets(sf::base::U8 shifts) -> sf::base::SizeT {
        return (sf::base::min)(max_bucket_count(), sf::base::SizeT{1} << (64U - shifts));
    }

    [[nodiscard]] constexpr auto calc_shifts_for_size(sf::base::SizeT s) const -> sf::base::U8 {
        auto shifts = initial_shifts;
        while (shifts > 0 && static_cast<sf::base::SizeT>(static_cast<float>(calc_num_buckets(shifts)) * max_load_factor()) < s) {
            --shifts;
        }
        return shifts;
    }

    // assumes m_values has data, m_buckets=m_buckets_end=nullptr, m_shifts is INITIAL_SHIFTS
    void copy_buckets(table const& other) {
        // assumes m_values has already the correct data copied over.
        if (empty()) {
            // when empty, at least allocate an initial buckets and clear them.
            allocate_buckets_from_shift();
            clear_buckets();
        } else {
            m_shifts = other.m_shifts;
            allocate_buckets_from_shift();
            if constexpr (IsSegmented || !SFML_BASE_IS_SAME(BucketContainer, default_container_t)) {
                for (auto i = 0UL; i < bucket_count(); ++i) {
                    at(m_buckets, i) = at(other.m_buckets, i);
                }
            } else {
                SFML_BASE_MEMCPY(m_buckets.data(), other.m_buckets.data(), sizeof(Bucket) * bucket_count());
            }
        }
    }

    /**
     * True when no element can be added any more without increasing the size
     */
    [[nodiscard]] auto is_full() const -> bool {
        return size() > m_max_bucket_capacity;
    }

    void deallocate_buckets() {
        m_buckets.clear();
        m_buckets.shrinkToFit();
        m_max_bucket_capacity = 0;
    }

    void allocate_buckets_from_shift() {
        auto num_buckets = calc_num_buckets(m_shifts);
        if constexpr (IsSegmented || !SFML_BASE_IS_SAME(BucketContainer, default_container_t)) {
            if constexpr (has_reserve<bucket_container_type>) {
                m_buckets.reserve(num_buckets);
            }
            for (sf::base::SizeT i = m_buckets.size(); i < num_buckets; ++i) {
                m_buckets.emplaceBack();
            }
        } else {
            m_buckets.resize(num_buckets);
        }
        if (num_buckets == max_bucket_count()) {
            // reached the maximum, make sure we can use each bucket
            m_max_bucket_capacity = max_bucket_count();
        } else {
            m_max_bucket_capacity = static_cast<value_idx_type>(static_cast<float>(num_buckets) * max_load_factor());
        }
    }

    void clear_buckets() {
        if constexpr (IsSegmented || !SFML_BASE_IS_SAME(BucketContainer, default_container_t)) {
            for (auto&& e : m_buckets) {
                SFML_BASE_MEMSET(&e, 0, sizeof(e));
            }
        } else {
            SFML_BASE_MEMSET(m_buckets.data(), 0, sizeof(Bucket) * bucket_count());
        }
    }

    void clear_and_fill_buckets_from_values() {
        clear_buckets();
        for (value_idx_type value_idx = 0, end_idx = static_cast<value_idx_type>(m_values.size()); value_idx < end_idx;
             ++value_idx) {
            auto const& key = get_key(m_values[value_idx]);
            auto [dist_and_fingerprint, bucket] = next_while_less(key);

            // we know for certain that key has not yet been inserted, so no need to check it.
            place_and_shift_up({dist_and_fingerprint, value_idx}, bucket);
        }
    }

    void increase_size() {
        if (m_max_bucket_capacity == max_bucket_count()) {
            // remove the value again, we can't add it!
            m_values.popBack();
            sf::base::abort(); // on_error_bucket_overflow();
        }
        --m_shifts;
        if constexpr (!IsSegmented || SFML_BASE_IS_SAME(BucketContainer, default_container_t)) {
            deallocate_buckets();
        }
        allocate_buckets_from_shift();
        clear_and_fill_buckets_from_values();
    }

    template <typename Op>
    void do_erase(value_idx_type bucket_idx, Op handle_erased_value) {
        auto const value_idx_to_remove = at(m_buckets, bucket_idx).m_value_idx;

        // shift down until either empty or an element with correct spot is found
        auto next_bucket_idx = next(bucket_idx);
        while (at(m_buckets, next_bucket_idx).m_dist_and_fingerprint >= Bucket::dist_inc * 2) {
            at(m_buckets, bucket_idx) = {dist_dec(at(m_buckets, next_bucket_idx).m_dist_and_fingerprint),
                                         at(m_buckets, next_bucket_idx).m_value_idx};
            bucket_idx = sf::base::exchange(next_bucket_idx, next(next_bucket_idx));
        }
        at(m_buckets, bucket_idx) = {};
        handle_erased_value(SFML_BASE_MOVE(m_values[value_idx_to_remove]));

        // update m_values
        if (value_idx_to_remove != m_values.size() - 1) {
            // no luck, we'll have to replace the value with the last one and update the index accordingly
            auto& val = m_values[value_idx_to_remove];
            val = SFML_BASE_MOVE(m_values.back());

            // update the values_idx of the moved entry. No need to play the info game, just look until we find the values_idx
            auto mh = mixed_hash(get_key(val));
            bucket_idx = bucket_idx_from_hash(mh);

            auto const values_idx_back = static_cast<value_idx_type>(m_values.size() - 1);
            while (values_idx_back != at(m_buckets, bucket_idx).m_value_idx) {
                bucket_idx = next(bucket_idx);
            }
            at(m_buckets, bucket_idx).m_value_idx = value_idx_to_remove;
        }
        m_values.popBack();
    }

    template <typename K, typename Op>
    auto do_erase_key(K&& key, Op handle_erased_value) -> sf::base::SizeT {
        if (empty()) {
            return 0;
        }

        auto [dist_and_fingerprint, bucket_idx] = next_while_less(key);

        while (dist_and_fingerprint == at(m_buckets, bucket_idx).m_dist_and_fingerprint &&
               !m_equal(key, get_key(m_values[at(m_buckets, bucket_idx).m_value_idx]))) {
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
        }

        if (dist_and_fingerprint != at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
            return 0;
        }
        do_erase(bucket_idx, handle_erased_value);
        return 1;
    }

    template <class K, class M>
    auto do_insert_or_assign(K&& key, M&& mapped) -> std::pair<iterator, bool> {
        auto it_isinserted = try_emplace(SFML_BASE_FORWARD(key), SFML_BASE_FORWARD(mapped));
        if (!it_isinserted.second) {
            it_isinserted.first->second = SFML_BASE_FORWARD(mapped);
        }
        return it_isinserted;
    }

    template <typename... Args>
    auto do_place_element(dist_and_fingerprint_type dist_and_fingerprint, value_idx_type bucket_idx, Args&&... args)
        -> std::pair<iterator, bool> {

        // emplace the new value. If that throws an exception, no harm done; index is still in a valid state
        m_values.emplaceBack(SFML_BASE_FORWARD(args)...);

        auto value_idx = static_cast<value_idx_type>(m_values.size() - 1);
        if (ANKERL_UNORDERED_DENSE_UNLIKELY(is_full())) {
            increase_size();
        } else {
            place_and_shift_up({dist_and_fingerprint, value_idx}, bucket_idx);
        }

        // place element and shift up until we find an empty spot
        return {begin() + static_cast<difference_type>(value_idx), true};
    }

    template <typename K, typename... Args>
    auto do_try_emplace(K&& key, Args&&... args) -> std::pair<iterator, bool> {
        auto hash = mixed_hash(key);
        auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
        auto bucket_idx = bucket_idx_from_hash(hash);

        while (true) {
            auto* bucket = &at(m_buckets, bucket_idx);
            if (dist_and_fingerprint == bucket->m_dist_and_fingerprint) {
                if (m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
                    return {begin() + static_cast<difference_type>(bucket->m_value_idx), false};
                }
            } else if (dist_and_fingerprint > bucket->m_dist_and_fingerprint) {
                return do_place_element(dist_and_fingerprint,
                                        bucket_idx,
                                        std::piecewise_construct,
                                        std::forward_as_tuple(SFML_BASE_FORWARD(key)),
                                        std::forward_as_tuple(SFML_BASE_FORWARD(args)...));
            }
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
        }
    }

    template <typename K>
    auto do_find(K const& key) -> iterator {
        if (ANKERL_UNORDERED_DENSE_UNLIKELY(empty())) {
            return end();
        }

        auto mh = mixed_hash(key);
        auto dist_and_fingerprint = dist_and_fingerprint_from_hash(mh);
        auto bucket_idx = bucket_idx_from_hash(mh);
        auto* bucket = &at(m_buckets, bucket_idx);

        // unrolled loop. *Always* check a few directly, then enter the loop. This is faster.
        if (dist_and_fingerprint == bucket->m_dist_and_fingerprint && m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
            return begin() + static_cast<difference_type>(bucket->m_value_idx);
        }
        dist_and_fingerprint = dist_inc(dist_and_fingerprint);
        bucket_idx = next(bucket_idx);
        bucket = &at(m_buckets, bucket_idx);

        if (dist_and_fingerprint == bucket->m_dist_and_fingerprint && m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
            return begin() + static_cast<difference_type>(bucket->m_value_idx);
        }
        dist_and_fingerprint = dist_inc(dist_and_fingerprint);
        bucket_idx = next(bucket_idx);
        bucket = &at(m_buckets, bucket_idx);

        while (true) {
            if (dist_and_fingerprint == bucket->m_dist_and_fingerprint) {
                if (m_equal(key, get_key(m_values[bucket->m_value_idx]))) {
                    return begin() + static_cast<difference_type>(bucket->m_value_idx);
                }
            } else if (dist_and_fingerprint > bucket->m_dist_and_fingerprint) {
                return end();
            }
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
            bucket = &at(m_buckets, bucket_idx);
        }
    }

    template <typename K>
    auto do_find(K const& key) const -> const_iterator {
        return const_cast<table*>(this)->do_find(key); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

    template <typename K, typename Q = T>
    auto do_at(K const& key) -> Q& requires (is_map_v<Q>) {
        if (auto it = find(key); ANKERL_UNORDERED_DENSE_LIKELY(end() != it)) {
            return it->second;
        }
        sf::base::abort(); // on_error_key_not_found();
    }

    template <typename K, typename Q = T>
    auto do_at(K const& key) const -> Q const& requires (is_map_v<Q>) {
        return const_cast<table*>(this)->at(key); // NOLINT(cppcoreguidelines-pro-type-const-cast)
    }

public:
    explicit table(sf::base::SizeT bucket_count = 0,
                   Hash const& hash = Hash(),
                   KeyEqual const& equal = KeyEqual())
        : m_values()
        , m_buckets()
        , m_hash(hash)
        , m_equal(equal) {
        if (0 != bucket_count) {
            reserve(bucket_count);
        } else {
            allocate_buckets_from_shift();
            clear_buckets();
        }
    }

    template <class InputIt>
    table(InputIt first,
          InputIt last,
          size_type bucket_count = 0,
          Hash const& hash = Hash(),
          KeyEqual const& equal = KeyEqual())
        : table(bucket_count, hash, equal) {
        insert(first, last);
    }

    table(table const& other)= default;
    table(table&& other) noexcept = default;

    table(std::initializer_list<value_type> ilist,
          sf::base::SizeT bucket_count = 0,
          Hash const& hash = Hash(),
          KeyEqual const& equal = KeyEqual())
        : table(bucket_count, hash, equal) {
        insert(ilist);
    }

    table(std::initializer_list<value_type> ilist, size_type bucket_count)
        : table(ilist, bucket_count, Hash(), KeyEqual()) {}

    table(std::initializer_list<value_type> init, size_type bucket_count, Hash const& hash)
        : table(init, bucket_count, hash, KeyEqual()) {}

    ~table() = default;

    auto operator=(table const& other) -> table& {
        if (&other != this) {
            m_values = other.m_values;
            m_max_load_factor = other.m_max_load_factor;
            m_hash = other.m_hash;
            m_equal = other.m_equal;
            m_shifts = initial_shifts;
            copy_buckets(other);
        }
        return *this;
    }

    auto operator=(table&& other) noexcept(noexcept(SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(value_container_type) &&
                                                    SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(Hash) &&
                                                    SFML_BASE_IS_NOTHROW_MOVE_ASSIGNABLE(KeyEqual))) -> table& {
        if (&other != this) {
            m_values = SFML_BASE_MOVE(other.m_values);
            m_buckets = SFML_BASE_MOVE(other.m_buckets);
            m_max_bucket_capacity = sf::base::exchange(other.m_max_bucket_capacity, sf::base::SizeT{0});
            m_shifts = sf::base::exchange(other.m_shifts, initial_shifts);
            m_max_load_factor = sf::base::exchange(other.m_max_load_factor, default_max_load_factor);
            m_hash = sf::base::exchange(other.m_hash, {});
            m_equal = sf::base::exchange(other.m_equal, {});

            // map "other" is now already usable, it's empty.
        }
        return *this;
    }

    auto operator=(std::initializer_list<value_type> ilist) -> table& {
        clear();
        insert(ilist);
        return *this;
    }

    // iterators //////////////////////////////////////////////////////////////

    auto begin() noexcept -> iterator {
        return m_values.begin();
    }

    auto begin() const noexcept -> const_iterator {
        return m_values.begin();
    }

    auto cbegin() const noexcept -> const_iterator {
        return m_values.cbegin();
    }

    auto end() noexcept -> iterator {
        return m_values.end();
    }

    auto cend() const noexcept -> const_iterator {
        return m_values.cend();
    }

    auto end() const noexcept -> const_iterator {
        return m_values.end();
    }

    // capacity ///////////////////////////////////////////////////////////////

    [[nodiscard]] auto empty() const noexcept -> bool {
        return m_values.empty();
    }

    [[nodiscard]] auto size() const noexcept -> sf::base::SizeT {
        return m_values.size();
    }

    [[nodiscard]] static constexpr auto max_size() noexcept -> sf::base::SizeT {
        if constexpr (value_idx_type(-1) == sf::base::SizeT(-1)) {
            return sf::base::SizeT{1} << (sizeof(value_idx_type) * 8 - 1);
        } else {
            return sf::base::SizeT{1} << (sizeof(value_idx_type) * 8);
        }
    }

    // modifiers //////////////////////////////////////////////////////////////

    void clear() {
        m_values.clear();
        clear_buckets();
    }

    auto insert(value_type const& value) -> std::pair<iterator, bool> {
        return emplace(value);
    }

    auto insert(value_type&& value) -> std::pair<iterator, bool> {
        return emplace(SFML_BASE_MOVE(value));
    }

    template <class P>
    auto insert(P&& value) -> std::pair<iterator, bool> requires (sf::base::isConstructible<value_type, P&&>) {
        return emplace(SFML_BASE_FORWARD(value));
    }

    auto insert(const_iterator /*hint*/, value_type const& value) -> iterator {
        return insert(value).first;
    }

    auto insert(const_iterator /*hint*/, value_type&& value) -> iterator {
        return insert(SFML_BASE_MOVE(value)).first;
    }

    template <class P>
    auto insert(const_iterator /*hint*/, P&& value) -> iterator requires (sf::base::isConstructible<value_type, P&&>) {
        return insert(SFML_BASE_FORWARD(value)).first;
    }

    template <class InputIt>
    void insert(InputIt first, InputIt last) {
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    void insert(std::initializer_list<value_type> ilist) {
        insert(ilist.begin(), ilist.end());
    }

    // nonstandard API: *this is emptied.
    // Also see "A Standard flat_map" https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p0429r9.pdf
    auto extract() && -> value_container_type {
        return SFML_BASE_MOVE(m_values);
    }

    // nonstandard API:
    // Discards the internally held container and replaces it with the one passed. Erases non-unique elements.
    auto replace(value_container_type&& container) {
        if (ANKERL_UNORDERED_DENSE_UNLIKELY(container.size() > max_size())) {
            sf::base::abort(); // on_error_too_many_elements();
        }
        auto shifts = calc_shifts_for_size(container.size());
        if (0 == bucket_count() || shifts < m_shifts || container.get_allocator() != m_values.get_allocator()) {
            m_shifts = shifts;
            deallocate_buckets();
            allocate_buckets_from_shift();
        }
        clear_buckets();

        m_values = SFML_BASE_MOVE(container);

        // can't use clear_and_fill_buckets_from_values() because container elements might not be unique
        auto value_idx = value_idx_type{};

        // loop until we reach the end of the container. duplicated entries will be replaced with back().
        while (value_idx != static_cast<value_idx_type>(m_values.size())) {
            auto const& key = get_key(m_values[value_idx]);

            auto hash = mixed_hash(key);
            auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
            auto bucket_idx = bucket_idx_from_hash(hash);

            bool key_found = false;
            while (true) {
                auto const& bucket = at(m_buckets, bucket_idx);
                if (dist_and_fingerprint > bucket.m_dist_and_fingerprint) {
                    break;
                }
                if (dist_and_fingerprint == bucket.m_dist_and_fingerprint &&
                    m_equal(key, get_key(m_values[bucket.m_value_idx]))) {
                    key_found = true;
                    break;
                }
                dist_and_fingerprint = dist_inc(dist_and_fingerprint);
                bucket_idx = next(bucket_idx);
            }

            if (key_found) {
                if (value_idx != static_cast<value_idx_type>(m_values.size() - 1)) {
                    m_values[value_idx] = SFML_BASE_MOVE(m_values.back());
                }
                m_values.popBack();
            } else {
                place_and_shift_up({dist_and_fingerprint, value_idx}, bucket_idx);
                ++value_idx;
            }
        }
    }

    template <class M, typename Q = T>
    auto insert_or_assign(Key const& key, M&& mapped) -> std::pair<iterator, bool> requires (is_map_v<Q>) {
        return do_insert_or_assign(key, SFML_BASE_FORWARD(mapped));
    }

    template <class M, typename Q = T>
    auto insert_or_assign(Key&& key, M&& mapped) -> std::pair<iterator, bool> requires (is_map_v<Q>) {
        return do_insert_or_assign(SFML_BASE_MOVE(key), SFML_BASE_FORWARD(mapped));
    }

    template <typename K,
              typename M,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto insert_or_assign(K&& key, M&& mapped) -> std::pair<iterator, bool> requires (is_map_v<Q> && is_transparent_v<H, KE>) {
        return do_insert_or_assign(SFML_BASE_FORWARD(key), SFML_BASE_FORWARD(mapped));
    }

    template <class M, typename Q = T>
    auto insert_or_assign(const_iterator /*hint*/, Key const& key, M&& mapped) -> iterator requires (is_map_v<Q>) {
        return do_insert_or_assign(key, SFML_BASE_FORWARD(mapped)).first;
    }

    template <class M, typename Q = T>
    auto insert_or_assign(const_iterator /*hint*/, Key&& key, M&& mapped) -> iterator requires (is_map_v<Q>) {
        return do_insert_or_assign(SFML_BASE_MOVE(key), SFML_BASE_FORWARD(mapped)).first;
    }

    template <typename K,
              typename M,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto insert_or_assign(const_iterator /*hint*/, K&& key, M&& mapped) -> iterator requires (is_map_v<Q> && is_transparent_v<H, KE>) {
        return do_insert_or_assign(SFML_BASE_FORWARD(key), SFML_BASE_FORWARD(mapped)).first;
    }

    // Single arguments for unordered set can be used without having to construct the value_type
    template <class K,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto emplace(K&& key) -> std::pair<iterator, bool> requires (!is_map_v<Q> && is_transparent_v<H, KE>) {
        auto hash = mixed_hash(key);
        auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
        auto bucket_idx = bucket_idx_from_hash(hash);

        while (dist_and_fingerprint <= at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
            if (dist_and_fingerprint == at(m_buckets, bucket_idx).m_dist_and_fingerprint &&
                m_equal(key, m_values[at(m_buckets, bucket_idx).m_value_idx])) {
                // found it, return without ever actually creating anything
                return {begin() + static_cast<difference_type>(at(m_buckets, bucket_idx).m_value_idx), false};
            }
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
        }

        // value is new, insert element first, so when exception happens we are in a valid state
        return do_place_element(dist_and_fingerprint, bucket_idx, SFML_BASE_FORWARD(key));
    }

    template <class... Args>
    auto emplace(Args&&... args) -> std::pair<iterator, bool> {
        // we have to instantiate the value_type to be able to access the key.
        // 1. emplace_back the object so it is constructed. 2. If the key is already there, pop it later in the loop.
        auto& key = get_key(m_values.emplaceBack(SFML_BASE_FORWARD(args)...));
        auto hash = mixed_hash(key);
        auto dist_and_fingerprint = dist_and_fingerprint_from_hash(hash);
        auto bucket_idx = bucket_idx_from_hash(hash);

        while (dist_and_fingerprint <= at(m_buckets, bucket_idx).m_dist_and_fingerprint) {
            if (dist_and_fingerprint == at(m_buckets, bucket_idx).m_dist_and_fingerprint &&
                m_equal(key, get_key(m_values[at(m_buckets, bucket_idx).m_value_idx]))) {
                m_values.popBack(); // value was already there, so get rid of it
                return {begin() + static_cast<difference_type>(at(m_buckets, bucket_idx).m_value_idx), false};
            }
            dist_and_fingerprint = dist_inc(dist_and_fingerprint);
            bucket_idx = next(bucket_idx);
        }

        // value is new, place the bucket and shift up until we find an empty spot
        auto value_idx = static_cast<value_idx_type>(m_values.size() - 1);
        if (ANKERL_UNORDERED_DENSE_UNLIKELY(is_full())) {
            // increase_size just rehashes all the data we have in m_values
            increase_size();
        } else {
            // place element and shift up until we find an empty spot
            place_and_shift_up({dist_and_fingerprint, value_idx}, bucket_idx);
        }
        return {begin() + static_cast<difference_type>(value_idx), true};
    }

    template <class... Args>
    auto emplace_hint(const_iterator /*hint*/, Args&&... args) -> iterator {
        return emplace(SFML_BASE_FORWARD(args)...).first;
    }

    template <class... Args, typename Q = T>
    auto try_emplace(Key const& key, Args&&... args) -> std::pair<iterator, bool> requires (is_map_v<Q>) {
        return do_try_emplace(key, SFML_BASE_FORWARD(args)...);
    }

    template <class... Args, typename Q = T>
    auto try_emplace(Key&& key, Args&&... args) -> std::pair<iterator, bool> requires (is_map_v<Q>) {
        return do_try_emplace(SFML_BASE_MOVE(key), SFML_BASE_FORWARD(args)...);
    }

    template <class... Args, typename Q = T>
    auto try_emplace(const_iterator /*hint*/, Key const& key, Args&&... args) -> iterator requires (is_map_v<Q>) {
        return do_try_emplace(key, SFML_BASE_FORWARD(args)...).first;
    }

    template <class... Args, typename Q = T>
    auto try_emplace(const_iterator /*hint*/, Key&& key, Args&&... args) -> iterator requires (is_map_v<Q>) {
        return do_try_emplace(SFML_BASE_MOVE(key), SFML_BASE_FORWARD(args)...).first;
    }

    template <
        typename K,
        typename... Args,
        typename Q = T,
        typename H = Hash,
        typename KE = KeyEqual>
    auto try_emplace(K&& key, Args&&... args) -> std::pair<iterator, bool> requires (is_map_v<Q> && is_transparent_v<H, KE> && is_neither_convertible_v<K&&, iterator, const_iterator>) {
        return do_try_emplace(SFML_BASE_FORWARD(key), SFML_BASE_FORWARD(args)...);
    }

    template <
        typename K,
        typename... Args,
        typename Q = T,
        typename H = Hash,
        typename KE = KeyEqual>
    auto try_emplace(const_iterator /*hint*/, K&& key, Args&&... args) -> iterator requires (is_map_v<Q> && is_transparent_v<H, KE> && is_neither_convertible_v<K&&, iterator, const_iterator>) {
        return do_try_emplace(SFML_BASE_FORWARD(key), SFML_BASE_FORWARD(args)...).first;
    }

    auto erase(iterator it) -> iterator {
        auto hash = mixed_hash(get_key(*it));
        auto bucket_idx = bucket_idx_from_hash(hash);

        auto const value_idx_to_remove = static_cast<value_idx_type>(it - cbegin());
        while (at(m_buckets, bucket_idx).m_value_idx != value_idx_to_remove) {
            bucket_idx = next(bucket_idx);
        }

        do_erase(bucket_idx, [](value_type&& /*unused*/) {
        });
        return begin() + static_cast<difference_type>(value_idx_to_remove);
    }

    auto extract(iterator it) -> value_type {
        auto hash = mixed_hash(get_key(*it));
        auto bucket_idx = bucket_idx_from_hash(hash);

        auto const value_idx_to_remove = static_cast<value_idx_type>(it - cbegin());
        while (at(m_buckets, bucket_idx).m_value_idx != value_idx_to_remove) {
            bucket_idx = next(bucket_idx);
        }

        auto tmp = sf::base::Optional<value_type>{};
        do_erase(bucket_idx, [&tmp](value_type&& val) {
            tmp = SFML_BASE_MOVE(val);
        });
        return SFML_BASE_MOVE(tmp).value();
    }

    template <typename Q = T>
    auto erase(const_iterator it) -> iterator requires (is_map_v<Q>) {
        return erase(begin() + (it - cbegin()));
    }

    template <typename Q = T>
    auto extract(const_iterator it) -> value_type requires (is_map_v<Q>) {
        return extract(begin() + (it - cbegin()));
    }

    auto erase(const_iterator first, const_iterator last) -> iterator {
        auto const idx_first = first - cbegin();
        auto const idx_last = last - cbegin();
        auto const first_to_last = detail::my_distance(first, last);
        auto const last_to_end = detail::my_distance(last, cend());

        // remove elements from left to right which moves elements from the end back
        auto const mid = idx_first + (sf::base::min)(first_to_last, last_to_end);
        auto idx = idx_first;
        while (idx != mid) {
            erase(begin() + idx);
            ++idx;
        }

        // all elements from the right are moved, now remove the last element until all done
        idx = idx_last;
        while (idx != mid) {
            --idx;
            erase(begin() + idx);
        }

        return begin() + idx_first;
    }

    auto erase(Key const& key) -> sf::base::SizeT {
        return do_erase_key(key, [](value_type&& /*unused*/) {
        });
    }

    auto extract(Key const& key) -> sf::base::Optional<value_type> {
        auto tmp = sf::base::Optional<value_type>{};
        do_erase_key(key, [&tmp](value_type&& val) {
            tmp = SFML_BASE_MOVE(val);
        });
        return tmp;
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto erase(K&& key) -> sf::base::SizeT requires (is_transparent_v<H, KE>) {
        return do_erase_key(SFML_BASE_FORWARD(key), [](value_type&& /*unused*/) {
        });
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto extract(K&& key) -> sf::base::Optional<value_type> requires (is_transparent_v<H, KE>) {
        auto tmp = sf::base::Optional<value_type>{};
        do_erase_key(SFML_BASE_FORWARD(key), [&tmp](value_type&& val) {
            tmp = SFML_BASE_MOVE(val);
        });
        return tmp;
    }

    void swap(table& other) noexcept(noexcept(SFML_BASE_IS_NOTHROW_SWAPPABLE(value_container_type) &&
                                              SFML_BASE_IS_NOTHROW_SWAPPABLE(Hash) && SFML_BASE_IS_NOTHROW_SWAPPABLE(KeyEqual))) {
        using std::swap;

        swap(m_values, other.m_values);
        swap(m_buckets, other.m_buckets);
        swap(m_max_bucket_capacity, other.m_max_bucket_capacity);
        swap(m_max_load_factor, other.m_max_load_factor);
        swap(m_hash, other.m_hash);
        swap(m_equal, other.m_equal);
        swap(m_shifts, other.m_shifts);
    }

    friend void swap(table& lhs, table& rhs) noexcept(noexcept(lhs.swap(rhs))) {
        lhs.swap(rhs);
    }

    // lookup /////////////////////////////////////////////////////////////////

    template <typename Q = T>
    auto at(key_type const& key) -> Q& requires (is_map_v<Q>) {
        return do_at(key);
    }

    template <typename K,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto at(K const& key) -> Q& requires (is_map_v<Q> && is_transparent_v<H, KE>) {
        return do_at(key);
    }

    template <typename Q = T>
    auto at(key_type const& key) const -> Q const& requires (is_map_v<Q>) {
        return do_at(key);
    }

    template <typename K,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto at(K const& key) const -> Q const& requires (is_map_v<Q> && is_transparent_v<H, KE>) {
        return do_at(key);
    }

    template <typename Q = T>
    auto operator[](Key const& key) -> Q& requires (is_map_v<Q>) {
        return try_emplace(key).first->second;
    }

    template <typename Q = T>
    auto operator[](Key&& key) -> Q& requires (is_map_v<Q>) {
        return try_emplace(SFML_BASE_MOVE(key)).first->second;
    }

    template <typename K,
              typename Q = T,
              typename H = Hash,
              typename KE = KeyEqual>
    auto operator[](K&& key) -> Q& requires (is_map_v<Q> && is_transparent_v<H, KE>) {
        return try_emplace(SFML_BASE_FORWARD(key)).first->second;
    }

    auto count(Key const& key) const -> sf::base::SizeT {
        return find(key) == end() ? 0 : 1;
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto count(K const& key) const -> sf::base::SizeT requires (is_transparent_v<H, KE>) {
        return find(key) == end() ? 0 : 1;
    }

    auto find(Key const& key) -> iterator {
        return do_find(key);
    }

    auto find(Key const& key) const -> const_iterator {
        return do_find(key);
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto find(K const& key) -> iterator requires (is_transparent_v<H, KE>) {
        return do_find(key);
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto find(K const& key) const -> const_iterator requires (is_transparent_v<H, KE>) {
        return do_find(key);
    }

    auto contains(Key const& key) const -> bool {
        return find(key) != end();
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto contains(K const& key) const -> bool requires (is_transparent_v<H, KE>) {
        return find(key) != end();
    }

    auto equal_range(Key const& key) -> std::pair<iterator, iterator> {
        auto it = do_find(key);
        return {it, it == end() ? end() : it + 1};
    }

    auto equal_range(const Key& key) const -> std::pair<const_iterator, const_iterator> {
        auto it = do_find(key);
        return {it, it == end() ? end() : it + 1};
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto equal_range(K const& key) -> std::pair<iterator, iterator> requires (is_transparent_v<H, KE>) {
        auto it = do_find(key);
        return {it, it == end() ? end() : it + 1};
    }

    template <class K, class H = Hash, class KE = KeyEqual>
    auto equal_range(K const& key) const -> std::pair<const_iterator, const_iterator> requires (is_transparent_v<H, KE>) {
        auto it = do_find(key);
        return {it, it == end() ? end() : it + 1};
    }

    // bucket interface ///////////////////////////////////////////////////////

    auto bucket_count() const noexcept -> sf::base::SizeT { // NOLINT(modernize-use-nodiscard)
        return m_buckets.size();
    }

    static constexpr auto max_bucket_count() noexcept -> sf::base::SizeT { // NOLINT(modernize-use-nodiscard)
        return max_size();
    }

    // hash policy ////////////////////////////////////////////////////////////

    [[nodiscard]] auto load_factor() const -> float {
        return bucket_count() ? static_cast<float>(size()) / static_cast<float>(bucket_count()) : 0.0F;
    }

    [[nodiscard]] auto max_load_factor() const -> float {
        return m_max_load_factor;
    }

    void max_load_factor(float ml) {
        m_max_load_factor = ml;
        if (bucket_count() != max_bucket_count()) {
            m_max_bucket_capacity = static_cast<value_idx_type>(static_cast<float>(bucket_count()) * max_load_factor());
        }
    }

    void rehash(sf::base::SizeT count) {
        count = (sf::base::min)(count, max_size());
        auto shifts = calc_shifts_for_size((sf::base::max)(count, size()));
        if (shifts != m_shifts) {
            m_shifts = shifts;
            deallocate_buckets();
            m_values.shrinkToFit();
            allocate_buckets_from_shift();
            clear_and_fill_buckets_from_values();
        }
    }

    void reserve(sf::base::SizeT capa) {
        capa = (sf::base::min)(capa, max_size());
        if constexpr (has_reserve<value_container_type>) {
            // std::deque doesn't have reserve(). Make sure we only call when available
            m_values.reserve(capa);
        }
        auto shifts = calc_shifts_for_size((sf::base::max)(capa, size()));
        if (0 == bucket_count() || shifts < m_shifts) {
            m_shifts = shifts;
            deallocate_buckets();
            allocate_buckets_from_shift();
            clear_and_fill_buckets_from_values();
        }
    }

    // observers //////////////////////////////////////////////////////////////

    auto hash_function() const -> hasher {
        return m_hash;
    }

    auto key_eq() const -> key_equal {
        return m_equal;
    }

    // nonstandard API: expose the underlying values container
    [[nodiscard]] auto values() const noexcept -> value_container_type const& {
        return m_values;
    }

    // non-member functions ///////////////////////////////////////////////////

    friend auto operator==(table const& a, table const& b) -> bool {
        if (&a == &b) {
            return true;
        }
        if (a.size() != b.size()) {
            return false;
        }
        for (auto const& b_entry : b) {
            auto it = a.find(get_key(b_entry));
            if constexpr (is_map_v<T>) {
                // map: check that key is here, then also check that value is the same
                if (a.end() == it || !(b_entry.second == it->second)) {
                    return false;
                }
            } else {
                // set: only check that the key is here
                if (a.end() == it) {
                    return false;
                }
            }
        }
        return true;
    }

    friend auto operator!=(table const& a, table const& b) -> bool {
        return !(a == b);
    }
};

} // namespace detail

ANKERL_UNORDERED_DENSE_EXPORT template <class Key,
                                        class T,
                                        class Hash = hash<Key>,
                                        class KeyEqual = detail::EqualTo<Key>,
                                        class Bucket = bucket_type::standard,
                                        class BucketContainer = detail::default_container_t>
using map = detail::table<Key, T, Hash, KeyEqual, Bucket, BucketContainer, false>;

ANKERL_UNORDERED_DENSE_EXPORT template <class Key,
                                        class T,
                                        class Hash = hash<Key>,
                                        class KeyEqual = detail::EqualTo<Key>,
                                        class Bucket = bucket_type::standard,
                                        class BucketContainer = detail::default_container_t>
using segmented_map = detail::table<Key, T, Hash, KeyEqual, Bucket, BucketContainer, true>;

ANKERL_UNORDERED_DENSE_EXPORT template <class Key,
                                        class Hash = hash<Key>,
                                        class KeyEqual = detail::EqualTo<Key>,
                                        class Bucket = bucket_type::standard,
                                        class BucketContainer = detail::default_container_t>
using set = detail::table<Key, void, Hash, KeyEqual, Bucket, BucketContainer, false>;

ANKERL_UNORDERED_DENSE_EXPORT template <class Key,
                                        class Hash = hash<Key>,
                                        class KeyEqual = detail::EqualTo<Key>,
                                        class Bucket = bucket_type::standard,
                                        class BucketContainer = detail::default_container_t>
using segmented_set = detail::table<Key, void, Hash, KeyEqual, Bucket, BucketContainer, true>;

// deduction guides ///////////////////////////////////////////////////////////

// deduction guides for alias templates are only possible since C++20
// see https://en.cppreference.com/w/cpp/language/class_template_argument_deduction

} // namespace ANKERL_UNORDERED_DENSE_NAMESPACE
} // namespace ankerl::unordered_dense

// std extensions /////////////////////////////////////////////////////////////

namespace std { // NOLINT(cert-dcl58-cpp)

ANKERL_UNORDERED_DENSE_EXPORT template <class Key,
                                        class T,
                                        class Hash,
                                        class KeyEqual,
                                        class Bucket,
                                        class Pred,
                                        class BucketContainer,
                                        bool IsSegmented>
// NOLINTNEXTLINE(cert-dcl58-cpp)
auto erase_if(
    ankerl::unordered_dense::detail::table<Key, T, Hash, KeyEqual, Bucket, BucketContainer, IsSegmented>&
        map,
    Pred pred) -> sf::base::SizeT {
    using map_t = ankerl::unordered_dense::detail::
        table<Key, T, Hash, KeyEqual, Bucket, BucketContainer, IsSegmented>;

    // going back to front because erase() invalidates the end iterator
    auto const old_size = map.size();
    auto idx = old_size;
    while (idx) {
        --idx;
        auto it = map.begin() + static_cast<typename map_t::difference_type>(idx);
        if (pred(*it)) {
            map.erase(it);
        }
    }

    return old_size - map.size();
}

} // namespace std

#endif
#endif

// NOLINTEND(readability-identifier-naming)
