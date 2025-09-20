#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/Memmove.hpp"
#include "SFML/Base/FwdStdAlignedNewDelete.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


namespace sf::base::priv::VectorUtils
{
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void moveRange(T* target, T* const srcBegin, T* const srcEnd)
{
    if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T))
    {
        SFML_BASE_MEMCPY(target, srcBegin, sizeof(T) * static_cast<SizeT>(srcEnd - srcBegin));
    }
    else
    {
        for (T* p = srcBegin; p != srcEnd; ++p, ++target)
            SFML_BASE_PLACEMENT_NEW(target) T(static_cast<T&&>(*p)); // not exception-safe
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void copyRange(T* target, const T* const srcBegin, const T* const srcEnd)
{
    if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T))
    {
        SFML_BASE_MEMCPY(target, srcBegin, sizeof(T) * static_cast<SizeT>(srcEnd - srcBegin));
    }
    else
    {
        for (const T* p = srcBegin; p != srcEnd; ++p, ++target)
            SFML_BASE_PLACEMENT_NEW(target) T(*p);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void destroyRange(T* const srcBegin, T* const srcEnd)
{
    if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T))
    {
        for (T* p = srcBegin; p != srcEnd; ++p)
            p->~T();
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void defaultConstructRange(T* const begin, T* const end)
{
    for (T* p = begin; p != end; ++p)
        SFML_BASE_PLACEMENT_NEW(p) T();
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void copyConstructRange(T* const begin, T* const end, const T& value)
{
    for (T* p = begin; p != end; ++p)
        SFML_BASE_PLACEMENT_NEW(p) T(value);
}


////////////////////////////////////////////////////////////
template <typename T>
[[nodiscard, gnu::always_inline]] inline T* allocate(const SizeT capacity)
{
    return capacity == 0u ? nullptr : static_cast<T*>(::operator new(capacity * sizeof(T), std::align_val_t{alignof(T)}));
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline void deallocate(T* const p, const SizeT /* capacity */) noexcept
{
    ::operator delete(p, std::align_val_t{alignof(T)});
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] inline T* eraseImpl(T* begin, T* end, T* const it)
{
    SFML_BASE_ASSERT(it >= begin && it < end);

    const auto size = static_cast<decltype(end - begin)>(end - begin);

    T* const currentEnd  = begin + size;
    T* const nextElement = it + 1;

    if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T))
    {
        const auto numElementsToMove = static_cast<SizeT>(currentEnd - nextElement);

        if (numElementsToMove > 0u)
        {
            // Move elements from `[it + 1, end())` to `[it, end() - 1)`
            SFML_BASE_MEMMOVE(it,                             // Destination
                              nextElement,                    // Source
                              numElementsToMove * sizeof(T)); // Number of bytes
        }
    }
    else if (nextElement != currentEnd) // If 'it' is not the last element
    {
        T* currentWrite = it;
        T* currentRead  = nextElement;

        while (currentRead != currentEnd)
            *currentWrite++ = static_cast<T&&>(*currentRead++);
    }

    return it;
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] inline T* eraseRangeImpl(T* const end, T* const first, T* const last)
{
    SFML_BASE_ASSERT(first <= end);
    SFML_BASE_ASSERT(last <= end);
    SFML_BASE_ASSERT(first != last);

    // Tracks the position where the next non-erased element should be moved to
    T* currWritePtr = first;

    // If `last` is not the end, elements from `last` onwards need to be shifted to the left to fill the gap
    if (last != end)
    {
        if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T))
        {
            SFML_BASE_MEMMOVE(first,                                       // Destination
                              last,                                        // Source
                              static_cast<SizeT>(end - last) * sizeof(T)); // Number of bytes

            // Update `currWritePtr` to the new logical end of the moved block
            currWritePtr = first + (end - last);
        }
        else
        {
            T* currReadPtr = last;

            // Loop until all elements from `last` to `m_endSize` have been processed
            while (currReadPtr != end)
                *currWritePtr++ = static_cast<T&&>(*currReadPtr++);

            // After the loop, `currWritePtr` points to the position after the last moved element
        }
    }

    // If `last == m_endSize`, all elements from `first` to `m_endSize` are being erased
    // No elements need to be moved. `currWritePtr` remains `first`.

    destroyRange(currWritePtr, end);
    return currWritePtr;
}


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(vectorType)                                                   \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& operator[](const SizeT i) noexcept             \
    {                                                                                                                \
        SFML_BASE_ASSERT(i < size());                                                                                \
        return *(data() + i);                                                                                        \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& operator[](const SizeT i) const noexcept \
    {                                                                                                                \
        SFML_BASE_ASSERT(i < size());                                                                                \
        return *(data() + i);                                                                                        \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* begin() noexcept                               \
    {                                                                                                                \
        return data();                                                                                               \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* begin() const noexcept                   \
    {                                                                                                                \
        return data();                                                                                               \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* end() noexcept                                 \
    {                                                                                                                \
        return data() + size();                                                                                      \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* end() const noexcept                     \
    {                                                                                                                \
        return data() + size();                                                                                      \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cbegin() const noexcept                  \
    {                                                                                                                \
        return data();                                                                                               \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cend() const noexcept                    \
    {                                                                                                                \
        return data() + size();                                                                                      \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept                               \
    {                                                                                                                \
        SFML_BASE_ASSERT(size() > 0u);                                                                               \
        return *data();                                                                                              \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept                   \
    {                                                                                                                \
        SFML_BASE_ASSERT(size() > 0u);                                                                               \
        return *data();                                                                                              \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept                                \
    {                                                                                                                \
        return this->operator[](size() - 1u);                                                                        \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& back() const noexcept                    \
    {                                                                                                                \
        return this->operator[](size() - 1u);                                                                        \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline, gnu::pure]] bool empty() const noexcept                                         \
    {                                                                                                                \
        return size() == 0u;                                                                                         \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard]] bool operator==(const vectorType& rhs) const                                                       \
    {                                                                                                                \
        if (this == &rhs)                                                                                            \
            return true;                                                                                             \
                                                                                                                     \
        const SizeT lhsSize = size();                                                                                \
                                                                                                                     \
        if (lhsSize != rhs.size())                                                                                   \
            return false;                                                                                            \
                                                                                                                     \
        for (SizeT i = 0u; i < lhsSize; ++i)                                                                         \
            if (operator[](i) != rhs.operator[](i))                                                                  \
                return false;                                                                                        \
                                                                                                                     \
        return true;                                                                                                 \
    }                                                                                                                \
                                                                                                                     \
    [[nodiscard, gnu::always_inline]] bool operator!=(const vectorType& rhs) const                                   \
    {                                                                                                                \
        return !(*this == rhs);                                                                                      \
    }                                                                                                                \
                                                                                                                     \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                                                                 \
    [[gnu::always_inline]] friend void swap(vectorType& lhs, vectorType& rhs) noexcept                               \
    {                                                                                                                \
        lhs.swap(rhs);                                                                                               \
    }                                                                                                                \
                                                                                                                     \
    template <typename... Ts>                                                                                        \
    [[gnu::always_inline, gnu::flatten]] TItem& emplaceBack(Ts&&... xs)                                              \
    {                                                                                                                \
        reserveMore(1);                                                                                              \
        return unsafeEmplaceBack(static_cast<Ts&&>(xs)...);                                                          \
    }                                                                                                                \
                                                                                                                     \
    template <typename T = TItem>                                                                                    \
    [[gnu::always_inline, gnu::flatten]] TItem& pushBack(T&& x)                                                      \
    {                                                                                                                \
        reserveMore(1);                                                                                              \
        return unsafeEmplaceBack(static_cast<T&&>(x));                                                               \
    }                                                                                                                \
                                                                                                                     \
    template <typename... Ts>                                                                                        \
    [[gnu::always_inline]] TItem& reEmplaceByIterator(TItem* const it, Ts&&... xs)                                   \
    {                                                                                                                \
        SFML_BASE_ASSERT(it >= begin() && it < end());                                                               \
                                                                                                                     \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))                                                   \
            it->~TItem();                                                                                            \
                                                                                                                     \
        return *(SFML_BASE_PLACEMENT_NEW(it) TItem(static_cast<Ts&&>(xs)...));                                       \
    }                                                                                                                \
                                                                                                                     \
    template <typename... Ts>                                                                                        \
    [[gnu::always_inline]] TItem& reEmplaceByIndex(const base::SizeT index, Ts&&... xs)                              \
    {                                                                                                                \
        return reEmplaceByIterator(data() + index, static_cast<Ts&&>(xs)...);                                        \
    }                                                                                                                \
                                                                                                                     \
    template <typename... TItems>                                                                                    \
    [[gnu::always_inline]] void pushBackMultiple(TItems&&... items)                                                  \
    {                                                                                                                \
        reserveMore(sizeof...(items));                                                                               \
        unsafePushBackMultiple(static_cast<TItems&&>(items)...);                                                     \
    }                                                                                                                \
                                                                                                                     \
    [[gnu::always_inline]] void emplaceRange(const TItem* const ptr, const SizeT count)                              \
    {                                                                                                                \
        reserveMore(count);                                                                                          \
        unsafeEmplaceRange(ptr, count);                                                                              \
    }                                                                                                                \
                                                                                                                     \
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceOther(const vectorType& rhs) noexcept                     \
    {                                                                                                                \
        unsafeEmplaceRange(rhs.data(), rhs.size());                                                                  \
    }                                                                                                                \
                                                                                                                     \
    [[gnu::always_inline]] void assignRange(const TItem* const b, const TItem* const e)                              \
    {                                                                                                                \
        SFML_BASE_ASSERT(b != nullptr);                                                                              \
        SFML_BASE_ASSERT(e != nullptr);                                                                              \
        SFML_BASE_ASSERT(b <= e);                                                                                    \
                                                                                                                     \
        const auto count = static_cast<SizeT>(e - b);                                                                \
                                                                                                                     \
        clear();                                                                                                     \
        reserve(count);                                                                                              \
        priv::VectorUtils::copyRange(data(), b, e);                                                                  \
                                                                                                                     \
        unsafeSetSize(count);                                                                                        \
    }                                                                                                                \
                                                                                                                     \
    static_assert(true)


} // namespace sf::base::priv::VectorUtils
