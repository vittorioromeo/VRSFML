#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Builtin/Memcpy.hpp"
#include "SFML/Base/Builtin/Memmove.hpp"
#include "SFML/Base/FwdStdAlignedNewDelete.hpp"
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Trait/IsTriviallyCopyable.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"


namespace sf::base::priv::VectorUtils
{
////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void moveRange(T* target, T* const srcBegin, T* const srcEnd)
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
[[gnu::always_inline]] inline constexpr void copyRange(T* target, const T* const srcBegin, const T* const srcEnd)
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
[[gnu::always_inline, gnu::flatten]] inline constexpr void relocateRange(T* target, T* const srcBegin, T* const srcEnd)
{
    if constexpr (SFML_BASE_IS_TRIVIALLY_RELOCATABLE(T))
    {
        SFML_BASE_MEMCPY(static_cast<void*>(target), srcBegin, sizeof(T) * static_cast<SizeT>(srcEnd - srcBegin));
    }
    else
    {
        moveRange(target, srcBegin, srcEnd);
        destroyRange(srcBegin, srcEnd);
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void defaultConstructRange(T* const begin, T* const end)
{
    for (T* p = begin; p != end; ++p)
        SFML_BASE_PLACEMENT_NEW(p) T();
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void copyConstructRange(T* const begin, T* const end, const T& value)
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
[[gnu::always_inline]] inline constexpr void deallocate(T* const p, const SizeT /* capacity */) noexcept
{
    ::operator delete(p, std::align_val_t{alignof(T)});
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] inline constexpr void eraseImpl([[maybe_unused]] T* begin, T* end, T* const it)
{
    SFML_BASE_ASSERT(it >= begin && it < end);

    T* const nextElement = it + 1;

    if (nextElement < end)
    {
        if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T))
        {
            SFML_BASE_MEMMOVE(it,                                                 // Destination
                              nextElement,                                        // Source
                              static_cast<SizeT>(end - nextElement) * sizeof(T)); // Number of bytes
        }
        else
        {
            T* currentWrite = it;
            T* currentRead  = nextElement;

            while (currentRead != end)
                *currentWrite++ = static_cast<T&&>(*currentRead++);
        }
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline, gnu::flatten]] inline constexpr T* eraseRangeImpl(T* const end, T* const first, T* const last)
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
template <typename T>
[[gnu::always_inline, gnu::flatten]] inline constexpr void makeHole(T* const pos, T* const end)
{
    // Moves elements in `[pos, end)` to `[pos + 1, end + 1)`.
    // Assumes capacity is sufficient.
    SFML_BASE_ASSERT(pos <= end);

    if (pos == end)
        return; // Inserting at the end, no move needed.

    if constexpr (SFML_BASE_IS_TRIVIALLY_COPYABLE(T) || SFML_BASE_IS_TRIVIALLY_RELOCATABLE(T))
    {
        SFML_BASE_MEMMOVE(pos + 1,                                    // Destination
                          pos,                                        // Source
                          static_cast<SizeT>(end - pos) * sizeof(T)); // Number of bytes
    }
    else
    {
        // Move-construct a new element at the end from the old last element.
        SFML_BASE_PLACEMENT_NEW(end) T(static_cast<T&&>(*(end - 1)));

        // Move-assign elements backwards to shift them to the right.
        for (T* p = end - 1; p > pos; --p)
            *p = static_cast<T&&>(*(p - 1));

        // The element at `pos` has been moved from, but its memory is still valid.
        // It will be overwritten by the new element. If it's not trivially
        // destructible, we should destroy it first to release its resources.
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(T))
            pos->~T();
    }
}


////////////////////////////////////////////////////////////
template <typename T>
[[gnu::always_inline]] inline constexpr void swapUnequalRanges(T* lhsData, SizeT& lhsSize, T* rhsData, SizeT& rhsSize)
{
    const SizeT s1 = lhsSize;
    const SizeT s2 = rhsSize;

    const SizeT commonSize = SFML_BASE_MIN(s1, s2);

    for (SizeT i = 0u; i < commonSize; ++i)
    {
        using base::swap;
        swap(lhsData[i], rhsData[i]); // Swap elements in the common part
    }

    if (s1 > s2) // `lhs` is larger; its tail elements move to `rhs`
        relocateRange(rhsData + commonSize, lhsData + commonSize, lhsData + s1);
    else if (s2 > s1) // `rhs` is larger; its tail elements move to `lhs`
        relocateRange(lhsData + commonSize, rhsData + commonSize, rhsData + s2);

    base::swap(lhsSize, rhsSize);
}


////////////////////////////////////////////////////////////
#define SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(vectorType)                                                             \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem& operator[](const SizeT i) noexcept             \
    {                                                                                                                          \
        SFML_BASE_ASSERT(i < size());                                                                                          \
        return *(data() + i);                                                                                                  \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem& operator[](const SizeT i) const noexcept \
    {                                                                                                                          \
        SFML_BASE_ASSERT(i < size());                                                                                          \
        return *(data() + i);                                                                                                  \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem* begin() noexcept                               \
    {                                                                                                                          \
        return data();                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem* begin() const noexcept                   \
    {                                                                                                                          \
        return data();                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem* end() noexcept                                 \
    {                                                                                                                          \
        return data() + size();                                                                                                \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem* end() const noexcept                     \
    {                                                                                                                          \
        return data() + size();                                                                                                \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem* cbegin() const noexcept                  \
    {                                                                                                                          \
        return data();                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem* cend() const noexcept                    \
    {                                                                                                                          \
        return data() + size();                                                                                                \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const noexcept                                         \
    {                                                                                                                          \
        return size() == 0u;                                                                                                   \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard]] constexpr bool operator==(const vectorType& rhs) const                                                       \
    {                                                                                                                          \
        if (this == &rhs)                                                                                                      \
            return true;                                                                                                       \
                                                                                                                               \
        const SizeT lhsSize = size();                                                                                          \
                                                                                                                               \
        if (lhsSize != rhs.size())                                                                                             \
            return false;                                                                                                      \
                                                                                                                               \
        for (SizeT i = 0u; i < lhsSize; ++i)                                                                                   \
            if (operator[](i) != rhs.operator[](i))                                                                            \
                return false;                                                                                                  \
                                                                                                                               \
        return true;                                                                                                           \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard, gnu::always_inline]] constexpr bool operator!=(const vectorType& rhs) const                                   \
    {                                                                                                                          \
        return !(*this == rhs);                                                                                                \
    }                                                                                                                          \
                                                                                                                               \
    /* NOLINTNEXTLINE(bugprone-macro-parentheses) */                                                                           \
    [[gnu::always_inline]] constexpr friend void swap(vectorType& lhs, vectorType& rhs) noexcept                               \
    {                                                                                                                          \
        lhs.swap(rhs);                                                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    template <typename... Ts>                                                                                                  \
    [[gnu::always_inline, gnu::flatten]] constexpr TItem& emplaceBack(Ts&&... xs)                                              \
    {                                                                                                                          \
        reserve(size() + 1);                                                                                                   \
        return unsafeEmplaceBack(static_cast<Ts&&>(xs)...);                                                                    \
    }                                                                                                                          \
                                                                                                                               \
    template <typename T = TItem>                                                                                              \
    [[gnu::always_inline, gnu::flatten]] constexpr TItem& pushBack(T&& x)                                                      \
    {                                                                                                                          \
        reserve(size() + 1);                                                                                                   \
        return unsafeEmplaceBack(static_cast<T&&>(x));                                                                         \
    }                                                                                                                          \
                                                                                                                               \
    template <typename... Ts>                                                                                                  \
    [[gnu::always_inline]] constexpr TItem& reEmplaceByIterator(TItem* const it, Ts&&... xs)                                   \
    {                                                                                                                          \
        SFML_BASE_ASSERT(it >= begin() && it < end());                                                                         \
                                                                                                                               \
        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))                                                             \
            it->~TItem();                                                                                                      \
                                                                                                                               \
        return *(SFML_BASE_PLACEMENT_NEW(it) TItem(static_cast<Ts&&>(xs)...));                                                 \
    }                                                                                                                          \
                                                                                                                               \
    template <typename... Ts>                                                                                                  \
    [[gnu::always_inline]] constexpr TItem& reEmplaceByIndex(const SizeT index, Ts&&... xs)                                    \
    {                                                                                                                          \
        return reEmplaceByIterator(data() + index, static_cast<Ts&&>(xs)...);                                                  \
    }                                                                                                                          \
                                                                                                                               \
    template <typename... TItems>                                                                                              \
    [[gnu::always_inline]] constexpr void pushBackMultiple(TItems&&... items)                                                  \
    {                                                                                                                          \
        reserve(size() + sizeof...(items));                                                                                    \
        unsafePushBackMultiple(static_cast<TItems&&>(items)...);                                                               \
    }                                                                                                                          \
                                                                                                                               \
    [[gnu::always_inline]] constexpr void emplaceRange(const TItem* const ptr, const SizeT count)                              \
    {                                                                                                                          \
        reserve(size() + count);                                                                                               \
        unsafeEmplaceBackRange(ptr, count);                                                                                    \
    }                                                                                                                          \
                                                                                                                               \
    [[gnu::always_inline, gnu::flatten]] constexpr void unsafeEmplaceOther(const vectorType& rhs) noexcept                     \
    {                                                                                                                          \
        unsafeEmplaceBackRange(rhs.data(), rhs.size());                                                                        \
    }                                                                                                                          \
                                                                                                                               \
    [[gnu::always_inline]] constexpr void assignRange(const TItem* const b, const TItem* const e)                              \
    {                                                                                                                          \
        SFML_BASE_ASSERT(b != nullptr);                                                                                        \
        SFML_BASE_ASSERT(e != nullptr);                                                                                        \
        SFML_BASE_ASSERT(b <= e);                                                                                              \
                                                                                                                               \
        const auto count = static_cast<SizeT>(e - b);                                                                          \
                                                                                                                               \
        clear();                                                                                                               \
        reserve(count);                                                                                                        \
        priv::VectorUtils::copyRange(data(), b, e);                                                                            \
                                                                                                                               \
        unsafeSetSize(count);                                                                                                  \
    }                                                                                                                          \
                                                                                                                               \
    [[gnu::always_inline]] constexpr void eraseAt(const SizeT index)                                                           \
    {                                                                                                                          \
        SFML_BASE_ASSERT(index < size());                                                                                      \
        erase(data() + index);                                                                                                 \
    }                                                                                                                          \
                                                                                                                               \
    static_assert(true)


} // namespace sf::base::priv::VectorUtils
