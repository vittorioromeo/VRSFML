#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/MinMaxMacros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/Priv/VectorUtils.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename TItem, SizeT N>
class [[nodiscard]] SmallVector
{
    static_assert(N > 0);

private:
    ////////////////////////////////////////////////////////////
    TItem* m_data{nullptr};
    TItem* m_endSize{nullptr};
    TItem* m_endCapacity{nullptr};
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
        const auto oldSize = size();

        if (m_data == nullptr)
        {
            // Should not happen given constructors init to inline
            SFML_BASE_ASSERT(size() == 0u);
        }
        else
        {
            priv::VectorUtils::relocateRange(newData, m_data, m_endSize);

            if (isHeap())
                priv::VectorUtils::deallocate(m_data, currentCapacity);
        }

        m_data        = newData;
        m_endSize     = m_data + oldSize;
        m_endCapacity = m_data + finalNewCapacity;
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
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] SmallVector() : m_data{getInlineStorage()}, m_endSize{m_data}, m_endCapacity{m_data + N}
    {
    }


    ////////////////////////////////////////////////////////////
    ~SmallVector()
    {
        priv::VectorUtils::destroyRange(m_data, m_endSize);

        if (isHeap())
            priv::VectorUtils::deallocate(m_data, capacity());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const SizeT initialSize, const TItem& value) : SmallVector{}
    {
        if (initialSize == 0u)
            return;

        reserve(initialSize);

        m_endSize = m_data + initialSize;
        priv::VectorUtils::copyConstructRange(m_data, m_endSize, value);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const SizeT initialSize) : SmallVector{}
    {
        if (initialSize == 0u)
            return;

        reserve(initialSize);

        m_endSize = m_data + initialSize;
        priv::VectorUtils::defaultConstructRange(m_data, m_endSize);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit SmallVector(const TItem* const srcBegin, const TItem* const srcEnd) : SmallVector{}
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        reserve(srcCount);
        priv::VectorUtils::copyRange(m_data, srcBegin, srcEnd);
        m_endSize = m_data + srcCount;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] /* implicit */ SmallVector(const std::initializer_list<TItem> iList) :
        SmallVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] SmallVector(const SmallVector& rhs) : SmallVector{}
    {
        reserve(rhs.size());
        priv::VectorUtils::copyRange(m_data, rhs.m_data, rhs.m_endSize);
        m_endSize = m_data + rhs.size();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] bool isHeap() const noexcept
    {
        return m_data != reinterpret_cast<const TItem*>(m_inlineStorage);
    }


    ////////////////////////////////////////////////////////////
    SmallVector& operator=(const SmallVector& rhs)
    {
        if (this == &rhs)
            return *this;

        clear();
        reserve(rhs.size());
        priv::VectorUtils::copyRange(m_data, rhs.m_data, rhs.m_endSize);

        m_endSize = m_data + rhs.size();

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] SmallVector(SmallVector&& rhs) noexcept : SmallVector{}
    {
        if (rhs.isHeap())
        {
            // Steal the heap buffer
            m_data        = rhs.m_data;
            m_endSize     = rhs.m_endSize;
            m_endCapacity = rhs.m_endCapacity;

            // Reset RHS to its own inline storage
            rhs.m_data        = rhs.getInlineStorage();
            rhs.m_endSize     = rhs.m_data;
            rhs.m_endCapacity = rhs.m_data + N;
        }
        else
        {
            // Move elements from RHS inline storage to LHS inline storage
            // No need to reserve, capacity is N
            priv::VectorUtils::relocateRange(m_data, rhs.m_data, rhs.m_endSize);
            m_endSize     = m_data + rhs.size();
            rhs.m_endSize = rhs.m_data;
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
                priv::VectorUtils::deallocate(m_data, capacity());

            // Steal pointers
            m_data        = rhs.m_data;
            m_endSize     = rhs.m_endSize;
            m_endCapacity = rhs.m_endCapacity;

            // Reset RHS
            rhs.m_data        = rhs.getInlineStorage();
            rhs.m_endSize     = rhs.m_data;
            rhs.m_endCapacity = rhs.m_data + N;

            return *this;
        }

        // Optimization: Reuse existing heap buffer
        if (isHeap() && capacity() >= rhs.size())
        {
            priv::VectorUtils::relocateRange(m_data, rhs.m_data, rhs.m_endSize);
            m_endSize = m_data + rhs.size();

            rhs.clear(); // Reset rhs

            return *this;
        }

        if (isHeap())
        {
            priv::VectorUtils::deallocate(m_data, capacity());
            m_data        = getInlineStorage();
            m_endCapacity = m_data + N;
        }

        m_endSize = m_data; // Reset size

        // Move elements
        priv::VectorUtils::relocateRange(m_data, rhs.m_data, rhs.m_endSize);
        m_endSize = m_data + rhs.size();

        rhs.clear();

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT newSize, auto&&... args)
    {
        const auto oldSize = size();

        if (newSize > oldSize)
        {
            reserve(newSize);

            for (auto* p = m_data + oldSize; p != m_data + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem(args...); // intentionally not forwarding
        }
        else
        {
            priv::VectorUtils::destroyRange(m_data + newSize, m_endSize);
        }

        m_endSize = m_data + newSize;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] TItem* emplace(TItem* const pos, Ts&&... xs)
    {
        SFML_BASE_ASSERT(pos >= begin() && pos <= end());

        // Save the index before `reserve` potentially invalidates `pos`.
        const auto index = static_cast<SizeT>(pos - m_data);

        reserve(size() + 1);

        // Restore the insertion position iterator, which may point to new memory.
        TItem* const currentPos = m_data + index;

        priv::VectorUtils::makeHole(currentPos, m_endSize);

        SFML_BASE_PLACEMENT_NEW(currentPos) TItem(static_cast<Ts&&>(xs)...);
        ++m_endSize;

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

        const SizeT currentSize = size();

        if (currentSize <= N)
        {
            // Move back to inline storage
            TItem* const oldData     = m_data;
            const auto   oldCapacity = capacity();

            // Setup inline pointers
            TItem* const inlinePtr = getInlineStorage();

            priv::VectorUtils::relocateRange(inlinePtr, oldData, oldData + currentSize);
            priv::VectorUtils::deallocate(oldData, oldCapacity);

            m_data        = inlinePtr;
            m_endSize     = m_data + currentSize;
            m_endCapacity = m_data + N;
        }
        else if (currentSize < capacity())
        {
            // Shrink heap allocation
            auto* const newData     = priv::VectorUtils::allocate<TItem>(currentSize);
            const auto  oldCapacity = capacity();

            priv::VectorUtils::relocateRange(newData, m_data, m_endSize);
            priv::VectorUtils::deallocate(m_data, oldCapacity);

            m_data    = newData;
            m_endSize = m_endCapacity = m_data + currentSize;
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem*& reserve(const SizeT targetCapacity)
    {
        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);

        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] TItem*& reserveMore(const SizeT n)
    {
        return reserve(size() + n);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceBackRange(const TItem* const ptr, const SizeT count) noexcept
    {
        SFML_BASE_ASSERT(size() + count <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        priv::VectorUtils::copyRange(m_endSize, ptr, ptr + count);
        m_endSize += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear() noexcept
    {
        priv::VectorUtils::destroyRange(m_data, m_endSize);
        m_endSize = m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_endSize - m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT capacity() const noexcept
    {
        return static_cast<SizeT>(m_endCapacity - m_data);
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] TItem& unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_endSize < m_endCapacity);
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        return *(SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<Ts&&>(xs)...));
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const it)
    {
        priv::VectorUtils::eraseImpl(begin(), end(), it);
        --m_endSize;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            m_endSize->~TItem();

        return it;
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const first, TItem* const last)
    {
        SFML_BASE_ASSERT(first <= last);

        if (first == last)
            return first; // No elements to erase

        m_endSize = priv::VectorUtils::eraseRangeImpl(end(), first, last);

        // Return an iterator to the element that now occupies the position
        // where the first erased element (`first`) was. This is `first` itself,
        // as elements were shifted into this position, or it's the new `end()`.
        return first;
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(size() + sizeof...(items) <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        (..., SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<TItems&&>(items)));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* data() noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* data() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeSetSize(SizeT newSize) noexcept
    {
        SFML_BASE_ASSERT(newSize <= capacity());
        m_endSize = m_data + newSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        --m_endSize;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            m_endSize->~TItem();
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
            base::swap(m_data, rhs.m_data);
            base::swap(m_endSize, rhs.m_endSize);
            base::swap(m_endCapacity, rhs.m_endCapacity);

            return;
        }

        if (!lhsHeap && !rhsHeap)
        {
            // Both are inline: use element-wise swap
            SizeT lhsSz = size();
            SizeT rhsSz = rhs.size();

            priv::VectorUtils::swapUnequalRanges(m_data, lhsSz, rhs.m_data, rhsSz);

            // Pointers data and capacity stay the same (pointing to respective inline storage)
            // Update end pointers based on new sizes
            m_endSize     = m_data + lhsSz;
            rhs.m_endSize = rhs.m_data + rhsSz;

            return;
        }

        // One is heap, one is inline
        SmallVector* const heapVec   = lhsHeap ? this : &rhs;
        SmallVector* const inlineVec = lhsHeap ? &rhs : this;

        // 1. Move elements from inlineVec's inline storage to heapVec's inline storage
        TItem* const heapVecInline = heapVec->getInlineStorage();
        priv::VectorUtils::relocateRange(heapVecInline, inlineVec->m_data, inlineVec->m_endSize);

        // 2. Capture heapVec's heap pointers
        TItem* const heapData = heapVec->m_data;
        TItem* const heapEnd  = heapVec->m_endSize;
        TItem* const heapCap  = heapVec->m_endCapacity;

        // 3. Point heapVec to its own inline storage (which now holds the data from inlineVec)
        heapVec->m_data        = heapVecInline;
        heapVec->m_endSize     = heapVecInline + inlineVec->size();
        heapVec->m_endCapacity = heapVecInline + N;

        // 4. Point inlineVec to the captured heap buffer
        inlineVec->m_data        = heapData;
        inlineVec->m_endSize     = heapEnd;
        inlineVec->m_endCapacity = heapCap;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *(m_endSize - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& back() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *(m_endSize - 1u);
    }


    ////////////////////////////////////////////////////////////
    SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(SmallVector);
};

} // namespace sf::base
