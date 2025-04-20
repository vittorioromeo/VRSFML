#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename TItem>
class [[nodiscard]] NonTrivialVector
{
private:
    ////////////////////////////////////////////////////////////
    TItem* m_data{nullptr};        // Points to the first constructed element / start of allocation
    TItem* m_endSize{nullptr};     // Points one past the last constructed element
    TItem* m_endCapacity{nullptr}; // Points one past the end of the allocated buffer


    ////////////////////////////////////////////////////////////
    static_assert(!(SFML_BASE_IS_TRIVIALLY_COPYABLE(TItem) && SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem)));


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] static TItem* allocate(const SizeT capacity)
    {
        return capacity == 0u ? nullptr : static_cast<TItem*>(::operator new(capacity * sizeof(TItem)));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void deallocate(TItem* const beginPtr)
    {
        ::operator delete(beginPtr);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void destroyRange(TItem* first, TItem* const last) noexcept
    {
        for (; first != last; ++first)
            first->~TItem();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void uninitializedCopy(const TItem* first, const TItem* const last, TItem* destStart)
    {
        for (; first != last; ++first, ++destStart)
            SFML_BASE_PLACEMENT_NEW(destStart) TItem(*first);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void uninitializedMoveAndDestroy(TItem* first, TItem* const last, TItem* destStart)
    {
        for (; first != last; ++first, ++destStart)
        {
            SFML_BASE_PLACEMENT_NEW(destStart) TItem(SFML_BASE_MOVE(*first));
            first->~TItem();
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void reserveImpl(const SizeT targetCapacity)
    {
        SFML_BASE_ASSERT(targetCapacity > capacity()); // Should only be called to grow

        const SizeT  oldSize  = size();
        TItem* const oldBegin = m_data; // Keep track of old start for deallocation

        TItem*       newBegin       = allocate(targetCapacity);
        TItem* const newEndSize     = newBegin + oldSize;
        TItem* const newEndCapacity = newBegin + targetCapacity;

        if (oldBegin != nullptr)
        {
            // Move construct elements to the new buffer and destroy old ones
            uninitializedMoveAndDestroy(oldBegin, m_endSize, newBegin);

            // Deallocate the old memory
            deallocate(oldBegin);
        }

        m_data        = newBegin;
        m_endSize     = newEndSize;
        m_endCapacity = newEndCapacity;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void resizeImpl(const SizeT newSize, const bool constructWithDefault)
    {
        const SizeT oldSize = size();

        if (newSize > oldSize)
        {
            reserve(newSize);

            if (constructWithDefault)
            {
                for (TItem* p = m_endSize; p != m_data + newSize; ++p)
                    SFML_BASE_PLACEMENT_NEW(p) TItem();
            }

            m_endSize = m_data + newSize;
        }
        else if (newSize < oldSize)
        {
            destroyRange(m_data + newSize, m_endSize);
            m_endSize = m_data + newSize;
        }
    }

public:
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
    [[nodiscard]] explicit NonTrivialVector() = default;


    ////////////////////////////////////////////////////////////
    ~NonTrivialVector()
    {
        destroyRange(m_data, m_endSize);
        deallocate(m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit NonTrivialVector(const SizeT initialSize)
    {
        if (initialSize == 0u)
            return;

        m_data    = allocate(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        for (TItem* p = m_data; p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit NonTrivialVector(const SizeT initialSize, const TItem& value)
    {
        if (initialSize == 0u)
            return;

        m_data    = allocate(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        for (TItem* p = m_data; p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem(value);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit NonTrivialVector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        m_data    = allocate(srcCount);
        m_endSize = m_endCapacity = m_data + srcCount;

        uninitializedCopy(srcBegin, srcEnd, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit NonTrivialVector(const std::initializer_list<TItem> iList) :
    NonTrivialVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] NonTrivialVector(const NonTrivialVector& rhs)
    {
        const SizeT rhsSize = rhs.size();

        if (rhsSize == 0u)
            return;

        m_data    = allocate(rhsSize);
        m_endSize = m_endCapacity = m_data + rhsSize;

        uninitializedCopy(rhs.m_data, rhs.m_endSize, m_data);
    }


    ////////////////////////////////////////////////////////////
    NonTrivialVector& operator=(const NonTrivialVector& rhs)
    {
        if (this == &rhs)
            return *this;

        const SizeT rhsSize = rhs.size();

        // Simple approach without exception safety: Destroy old, allocate new, copy construct
        clear();            // Destroy existing elements
        deallocate(m_data); // Free old buffer

        // Reset pointers before allocation
        m_data = m_endSize = m_endCapacity = nullptr;

        if (rhsSize > 0u)
        {
            m_data    = allocate(rhsSize);
            m_endSize = m_endCapacity = m_data + rhsSize;

            uninitializedCopy(rhs.m_data, rhs.m_endSize, m_data);
        }

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] NonTrivialVector(NonTrivialVector&& rhs) noexcept :
    m_data{rhs.m_data},
    m_endSize{rhs.m_endSize},
    m_endCapacity{rhs.m_endCapacity}
    {
        rhs.m_data = rhs.m_endSize = rhs.m_endCapacity = nullptr;
    }


    ////////////////////////////////////////////////////////////
    NonTrivialVector& operator=(NonTrivialVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        // Destroy existing elements and free current resources
        clear();
        deallocate(m_data);

        // Steal resources from rhs
        m_data        = rhs.m_data;
        m_endSize     = rhs.m_endSize;
        m_endCapacity = rhs.m_endCapacity;

        // Leave moved-from object empty
        rhs.m_data = rhs.m_endSize = rhs.m_endCapacity = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    void resize(const SizeT newSize)
    {
        resizeImpl(newSize, /* constructWithDefault */ true);
    }


    ////////////////////////////////////////////////////////////
    void resize(const SizeT newSize, const TItem& item)
    {
        if (newSize > size())
        {
            reserve(newSize);

            for (TItem* p = m_endSize; p != m_data + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem(item);

            m_endSize = m_data + newSize;
        }
        else
        {
            resizeImpl(newSize, /* constructWithDefault */ false);
        }
    }


    ////////////////////////////////////////////////////////////
    void reserve(const SizeT targetCapacity)
    {
        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);
    }


    ////////////////////////////////////////////////////////////
    void reserveMore(const SizeT n)
    {
        const SizeT targetCapacity = size() + n;

        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl((targetCapacity * 3u / 2u) + n);
    }


    ////////////////////////////////////////////////////////////
    void shrinkToFit()
    {
        const SizeT currentSize = size();

        if (capacity() <= currentSize)
            return;

        if (currentSize == 0)
        {
            deallocate(m_data);
            m_data = m_endSize = m_endCapacity = nullptr;

            return;
        }

        TItem* newBegin = allocate(currentSize);

        uninitializedMoveAndDestroy(m_data, m_endSize, newBegin);
        deallocate(m_data);

        m_data    = newBegin;
        m_endSize = m_endCapacity = newBegin + currentSize;
    }


    ////////////////////////////////////////////////////////////
    void assignRange(const TItem* const b, const TItem* const e) noexcept
    {
        SFML_BASE_ASSERT(b != nullptr);
        SFML_BASE_ASSERT(e != nullptr);
        SFML_BASE_ASSERT(b <= e);

        const auto count = static_cast<SizeT>(e - b);

        clear();
        reserve(count);
        uninitializedCopy(b, e, m_data);

        m_endSize = m_data + count;
    }


    ////////////////////////////////////////////////////////////
    void emplaceRange(const TItem* ptr, SizeT count) noexcept
    {
        if (count == 0u)
            return;

        reserveMore(count);
        uninitializedCopy(ptr, ptr + count, m_endSize);

        m_endSize += count;
    }


    ////////////////////////////////////////////////////////////
    void emplaceOther(const NonTrivialVector& rhs) noexcept
    {
        emplaceRange(rhs.begin(), rhs.size());
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear() noexcept
    {
        destroyRange(m_data, m_endSize);
        m_endSize = m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_endSize - m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline SizeT capacity() const noexcept
    {
        return static_cast<SizeT>(m_endCapacity - m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline bool empty() const noexcept
    {
        return m_data == m_endSize;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] TItem& unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(size() <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        return *(SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<Ts&&>(xs)...));
    }


    ////////////////////////////////////////////////////////////
    // Emplace element at the back (perfect forward arguments)
    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] TItem& emplaceBack(Ts&&... xs)
    {
        reserveMore(1);
        return unsafeEmplaceBack(static_cast<Ts&&>(xs)...);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline, gnu::flatten]] TItem& pushBack(T&& x)
    {
        reserveMore(1);
        return unsafeEmplaceBack(static_cast<T&&>(x));
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const it)
    {
        SFML_BASE_ASSERT(it >= m_data);
        SFML_BASE_ASSERT(it < m_endSize);

        TItem* const nextElement = it + 1;

        if (nextElement != m_endSize)
        {
            TItem* currentWrite = it;
            TItem* currentRead  = nextElement;

            while (currentRead != m_endSize)
            {
                *currentWrite = SFML_BASE_MOVE(*currentRead);
                ++currentWrite;
                ++currentRead;
            }
        }

        --m_endSize;
        m_endSize->~TItem();

        return it;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem* data() noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem* data() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return m_data[i];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return m_data[i];
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem* begin() noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem* begin() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem* cbegin() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem* end() noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem* end() const noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem* cend() const noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem& front() noexcept
    {
        return this->operator[](0u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem& front() const noexcept
    {
        return this->operator[](0u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline TItem& back() noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] inline const TItem& back() const noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());

        --m_endSize;
        m_endSize->~TItem();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator==(const NonTrivialVector& rhs) const
    {
        if (this == &rhs)
            return true;

        const SizeT lhsSize = size();

        if (lhsSize != rhs.size())
            return false;

        for (SizeT i = 0u; i < lhsSize; ++i)
            if (m_data[i] != rhs.m_data[i])
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator!=(const NonTrivialVector& rhs) const
    {
        return !(*this == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(NonTrivialVector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        base::swap(m_data, rhs.m_data);
        base::swap(m_endSize, rhs.m_endSize);
        base::swap(m_endCapacity, rhs.m_endCapacity);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(NonTrivialVector& lhs, NonTrivialVector& rhs) noexcept
    {
        lhs.swap(rhs);
    }
};

} // namespace sf::base
