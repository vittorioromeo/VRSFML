#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InitializerList.hpp" // IWYU pragma: keep
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/Priv/VectorUtils.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename TItem, SizeT N>
class [[nodiscard]] SmallVector
{
    static_assert(N > 0);

private:
    ////////////////////////////////////////////////////////////
    // `m_heapData == nullptr` means "using inline storage".
    // This makes SmallVector trivially relocatable: after memcpy,
    // nullptr still means "use my own inline storage" (recomputed from `this`).
    TItem* m_heapData{nullptr};
    SizeT  m_size{0u};
    SizeT  m_capacity{N};
    alignas(TItem) unsigned char m_inlineStorage[sizeof(TItem) * N];


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* getInlineStorage() noexcept
    {
        return reinterpret_cast<TItem*>(m_inlineStorage);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* getInlineStorage() const noexcept
    {
        return reinterpret_cast<const TItem*>(m_inlineStorage);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline, gnu::flatten]] void reserveImpl(const SizeT targetCapacity)
    {
        const auto currentCapacity = capacity();
        const auto geometricGrowthTarget = currentCapacity + (currentCapacity / 2u); // Equivalent to `currentCapacity * 1.5`
        const auto finalNewCapacity = SFML_BASE_MAX(targetCapacity, geometricGrowthTarget);

        SFML_BASE_ASSERT(finalNewCapacity > capacity()); // Should only be called to grow

        auto*      newData = priv::VectorUtils::allocate<TItem>(finalNewCapacity);
        const auto oldSize = m_size;

        TItem* const oldData = data();

        if (oldSize > 0u)
            priv::VectorUtils::relocateRange(newData, oldData, oldData + oldSize);

        if (isHeap())
            priv::VectorUtils::deallocate(m_heapData, currentCapacity);

        m_heapData = newData;
        m_capacity = finalNewCapacity;
    }


public:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        enableTrivialRelocation = true
    };


    ////////////////////////////////////////////////////////////
    using value_type      = TItem;
    using pointer         = TItem*;
    using const_pointer   = const TItem*;
    using reference       = TItem&;
    using const_reference = const TItem&;
    using size_type       = SizeT;
    using difference_type = PtrDiffT;
    using iterator        = TItem*;
    using const_iterator  = const TItem*;


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init, cppcoreguidelines-pro-type-reinterpret-cast)
    [[nodiscard]] SmallVector() = default;


    ////////////////////////////////////////////////////////////
    ~SmallVector()
    {
        priv::VectorUtils::destroyRange(data(), data() + m_size);

        if (isHeap())
            priv::VectorUtils::deallocate(m_heapData, m_capacity);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const SizeT initialSize, const TItem& value) : SmallVector{}
    {
        if (initialSize == 0u)
            return;

        reserve(initialSize);

        priv::VectorUtils::copyConstructRange(data(), data() + initialSize, value);
        m_size = initialSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const SizeT initialSize) : SmallVector{}
    {
        if (initialSize == 0u)
            return;

        reserve(initialSize);

        priv::VectorUtils::defaultConstructRange(data(), data() + initialSize);
        m_size = initialSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const TItem* const srcBegin, const TItem* const srcEnd) : SmallVector{}
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        reserve(srcCount);
        priv::VectorUtils::copyRange(data(), srcBegin, srcEnd);
        m_size = srcCount;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] /* implicit */ SmallVector(const std::initializer_list<TItem> iList) :
        SmallVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] SmallVector(const SmallVector& rhs) : SmallVector{}
    {
        reserve(rhs.m_size);
        priv::VectorUtils::copyRange(data(), rhs.data(), rhs.data() + rhs.m_size);
        m_size = rhs.m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isHeap() const noexcept
    {
        return m_heapData != nullptr;
    }


    ////////////////////////////////////////////////////////////
    SmallVector& operator=(const SmallVector& rhs)
    {
        if (this == &rhs)
            return *this;

        clear();
        reserve(rhs.m_size);
        priv::VectorUtils::copyRange(data(), rhs.data(), rhs.data() + rhs.m_size);

        m_size = rhs.m_size;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] SmallVector(SmallVector&& rhs) noexcept : SmallVector{}
    {
        if (rhs.isHeap())
        {
            // Steal the heap buffer
            m_heapData = rhs.m_heapData;
            m_size     = rhs.m_size;
            m_capacity = rhs.m_capacity;

            // Reset RHS to inline mode
            rhs.m_heapData = nullptr;
            rhs.m_size     = 0u;
            rhs.m_capacity = N;
        }
        else
        {
            // Move elements from RHS inline storage to LHS inline storage
            // No need to reserve, capacity is N
            priv::VectorUtils::relocateRange(getInlineStorage(), rhs.getInlineStorage(), rhs.getInlineStorage() + rhs.m_size);
            m_size     = rhs.m_size;
            rhs.m_size = 0u;
        }
    }


    ////////////////////////////////////////////////////////////
    SmallVector& operator=(SmallVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        // Destroy current elements
        clear();

        if (rhs.isHeap())
        {
            // If we have a heap buffer, deallocate it before stealing
            if (isHeap())
                priv::VectorUtils::deallocate(m_heapData, m_capacity);

            // Steal pointers
            m_heapData = rhs.m_heapData;
            m_size     = rhs.m_size;
            m_capacity = rhs.m_capacity;

            // Reset RHS
            rhs.m_heapData = nullptr;
            rhs.m_size     = 0u;
            rhs.m_capacity = N;

            return *this;
        }

        // Optimization: Reuse existing heap buffer
        if (isHeap() && m_capacity >= rhs.m_size)
        {
            priv::VectorUtils::relocateRange(m_heapData, rhs.data(), rhs.data() + rhs.m_size);
            m_size = rhs.m_size;

            rhs.m_size = 0u;

            return *this;
        }

        if (isHeap())
        {
            priv::VectorUtils::deallocate(m_heapData, m_capacity);
            m_heapData = nullptr;
            m_capacity = N;
        }

        m_size = 0u; // Reset size

        // Move elements
        priv::VectorUtils::relocateRange(data(), rhs.data(), rhs.data() + rhs.m_size);
        m_size = rhs.m_size;

        rhs.m_size = 0u;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT newSize, auto&&... args)
    {
        const auto oldSize = m_size;

        if (newSize > oldSize)
        {
            reserve(newSize);

            TItem* const d = data();
            for (auto* p = d + oldSize; p != d + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem(args...); // intentionally not forwarding
        }
        else
        {
            TItem* const d = data();
            priv::VectorUtils::destroyRange(d + newSize, d + oldSize);
        }

        m_size = newSize;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] TItem* emplace(TItem* const pos, Ts&&... xs)
    {
        SFML_BASE_ASSERT(pos >= begin() && pos <= end());

        // Save the index before `reserve` potentially invalidates `pos`.
        const auto index = static_cast<SizeT>(pos - data());

        reserve(m_size + 1);

        TItem* const d = data();

        // Restore the insertion position iterator, which may point to new memory.
        TItem* const currentPos = d + index;

        priv::VectorUtils::makeHole(currentPos, d + m_size);

        SFML_BASE_PLACEMENT_NEW(currentPos) TItem(static_cast<Ts&&>(xs)...);
        ++m_size;

        return currentPos;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem* insert(TItem* const pos, const TItem& value)
    {
        return emplace(pos, value);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem* insert(TItem* const pos, TItem&& value)
    {
        return emplace(pos, static_cast<TItem&&>(value));
    }


    ////////////////////////////////////////////////////////////
    void shrinkToFit()
    {
        if (!isHeap())
            return;

        const SizeT currentSize = m_size;

        if (currentSize <= N)
        {
            // Move back to inline storage
            TItem* const oldData     = m_heapData;
            const auto   oldCapacity = m_capacity;

            // Setup inline pointers
            TItem* const inlinePtr = getInlineStorage();

            priv::VectorUtils::relocateRange(inlinePtr, oldData, oldData + currentSize);
            priv::VectorUtils::deallocate(oldData, oldCapacity);

            m_heapData = nullptr;
            m_capacity = N;
        }
        else if (currentSize < m_capacity)
        {
            // Shrink heap allocation
            auto* const newData     = priv::VectorUtils::allocate<TItem>(currentSize);
            const auto  oldCapacity = m_capacity;

            priv::VectorUtils::relocateRange(newData, m_heapData, m_heapData + currentSize);
            priv::VectorUtils::deallocate(m_heapData, oldCapacity);

            m_heapData = newData;
            m_capacity = currentSize;
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserve(const SizeT targetCapacity)
    {
        if (m_capacity < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void reserveMore(const SizeT n)
    {
        reserve(m_size + n);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceBackRange(const TItem* const ptr, const SizeT count) noexcept
    {
        SFML_BASE_ASSERT(m_size + count <= m_capacity);

        TItem* const d = data();
        priv::VectorUtils::copyRange(d + m_size, ptr, ptr + count);
        m_size += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear() noexcept
    {
        TItem* const d = data();
        priv::VectorUtils::destroyRange(d, d + m_size);
        m_size = 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT capacity() const noexcept
    {
        return m_capacity;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] TItem& unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size < m_capacity);

        TItem* const slot = data() + m_size;
        ++m_size;

        return *(SFML_BASE_PLACEMENT_NEW(slot) TItem(static_cast<Ts&&>(xs)...));
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const it)
    {
        priv::VectorUtils::eraseImpl(begin(), end(), it);
        --m_size;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            (data() + m_size)->~TItem();

        return it;
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const first, TItem* const last)
    {
        SFML_BASE_ASSERT(first <= last);

        if (first == last)
            return first; // No elements to erase

        TItem* const newEnd = priv::VectorUtils::eraseRangeImpl(end(), first, last);
        m_size              = static_cast<SizeT>(newEnd - data());

        // Return an iterator to the element that now occupies the position
        // where the first erased element (`first`) was. This is `first` itself,
        // as elements were shifted into this position, or it's the new `end()`.
        return first;
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(m_size + sizeof...(items) <= m_capacity);

        TItem* d = data();
        (..., (SFML_BASE_PLACEMENT_NEW(d + m_size) TItem(static_cast<TItems&&>(items)), ++m_size));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* data() noexcept
    {
        return m_heapData != nullptr ? m_heapData : getInlineStorage();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* data() const noexcept
    {
        return m_heapData != nullptr ? m_heapData : getInlineStorage();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeSetSize(SizeT newSize) noexcept
    {
        SFML_BASE_ASSERT(newSize <= m_capacity);
        m_size = newSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        --m_size;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            (data() + m_size)->~TItem();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(SmallVector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        const bool lhsHeap = isHeap();
        const bool rhsHeap = rhs.isHeap();

        if (lhsHeap && rhsHeap)
        {
            // Both are heap allocated: standard pointer swap
            base::genericSwap(m_heapData, rhs.m_heapData);
            base::genericSwap(m_size, rhs.m_size);
            base::genericSwap(m_capacity, rhs.m_capacity);

            return;
        }

        if (!lhsHeap && !rhsHeap)
        {
            // Both are inline: use element-wise swap
            // (swapUnequalRanges also swaps the sizes via the SizeT& references)
            priv::VectorUtils::swapUnequalRanges(data(), m_size, rhs.data(), rhs.m_size);

            return;
        }

        // One is heap, one is inline
        SmallVector* const heapVec   = lhsHeap ? this : &rhs;
        SmallVector* const inlineVec = lhsHeap ? &rhs : this;

        // 1. Move elements from inlineVec's inline storage to heapVec's inline storage
        TItem* const heapVecInline = heapVec->getInlineStorage();
        priv::VectorUtils::relocateRange(heapVecInline,
                                         inlineVec->getInlineStorage(),
                                         inlineVec->getInlineStorage() + inlineVec->m_size);

        // 2. Capture heapVec's heap state
        TItem* const savedHeapData     = heapVec->m_heapData;
        const SizeT  savedHeapSize     = heapVec->m_size;
        const SizeT  savedHeapCapacity = heapVec->m_capacity;

        // 3. Point heapVec to its own inline storage (which now holds the data from inlineVec)
        heapVec->m_heapData = nullptr;
        heapVec->m_size     = inlineVec->m_size;
        heapVec->m_capacity = N;

        // 4. Point inlineVec to the captured heap buffer
        inlineVec->m_heapData = savedHeapData;
        inlineVec->m_size     = savedHeapSize;
        inlineVec->m_capacity = savedHeapCapacity;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *(data() + m_size - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& back() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *(data() + m_size - 1u);
    }


    ////////////////////////////////////////////////////////////
    SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(SmallVector);
};

} // namespace sf::base
