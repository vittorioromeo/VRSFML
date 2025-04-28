#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/Memmove.hpp"
#include "SFML/Base/FwdStdAlignedNewDelete.hpp"
#include "SFML/Base/InitializerList.hpp" // used
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
class [[nodiscard]] Vector
{
private:
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static TItem* allocate(const SizeT capacity)
    {
        if (capacity == 0u)
            return nullptr;

        return static_cast<TItem*>(::operator new(capacity * sizeof(TItem), std::align_val_t{alignof(TItem)}));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void deallocate(TItem* const p, const SizeT /* capacity */) noexcept
    {
        ::operator delete(p, std::align_val_t{alignof(TItem)});
    }


    ////////////////////////////////////////////////////////////
    enum : bool
    {
        triviallyDestructible = SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem),
        triviallyCopyable     = SFML_BASE_IS_TRIVIALLY_COPYABLE(TItem),
    };


    ////////////////////////////////////////////////////////////
    TItem* m_data{nullptr};
    TItem* m_endSize{nullptr};
    TItem* m_endCapacity{nullptr};


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void moveRange(TItem* target, TItem* const srcBegin, TItem* const srcEnd)
    {
        if constexpr (triviallyCopyable)
        {
            SFML_BASE_MEMCPY(target, srcBegin, sizeof(TItem) * static_cast<SizeT>(srcEnd - srcBegin));
        }
        else
        {
            for (auto* p = srcBegin; p != srcEnd; ++p, ++target)
                SFML_BASE_PLACEMENT_NEW(target) TItem(SFML_BASE_MOVE(*p));
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void copyRange(TItem* target, const TItem* const srcBegin, const TItem* const srcEnd)
    {
        if constexpr (triviallyCopyable)
        {
            SFML_BASE_MEMCPY(target, srcBegin, sizeof(TItem) * static_cast<SizeT>(srcEnd - srcBegin));
        }
        else
        {
            for (auto* p = srcBegin; p != srcEnd; ++p, ++target)
                SFML_BASE_PLACEMENT_NEW(target) TItem(*p);
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void destroyRange(TItem* const srcBegin, TItem* const srcEnd)
    {
        if constexpr (!triviallyDestructible)
        {
            for (auto* p = srcBegin; p != srcEnd; ++p)
                p->~TItem();
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void reserveImpl(const SizeT targetCapacity)
    {
        SFML_BASE_ASSERT(targetCapacity > capacity()); // Should only be called to grow

        auto*      newData = allocate(targetCapacity);
        const auto oldSize = size();

        if (m_data != nullptr)
        {
            moveRange(newData, m_data, m_endSize);
            destroyRange(m_data, m_endSize);
            deallocate(m_data, capacity());
        }
        else
        {
            SFML_BASE_ASSERT(size() == 0u);
            SFML_BASE_ASSERT(capacity() == 0u);
        }

        m_data        = newData;
        m_endSize     = m_data + oldSize;
        m_endCapacity = m_data + targetCapacity;
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
    [[nodiscard]] Vector() = default;


    ////////////////////////////////////////////////////////////
    ~Vector()
    {
        destroyRange(m_data, m_endSize);
        deallocate(m_data, capacity());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const SizeT initialSize)
    {
        if (initialSize == 0u)
            return;

        m_data    = allocate(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        for (TItem* p = m_data; p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const SizeT initialSize, const TItem& value)
    {
        if (initialSize == 0u)
            return;

        m_data    = allocate(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        for (TItem* p = m_data; p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem(value);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        m_data    = allocate(srcCount);
        m_endSize = m_endCapacity = m_data + srcCount;

        copyRange(m_data, srcBegin, srcEnd);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const std::initializer_list<TItem> iList) : Vector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] Vector(const Vector& rhs) :
    m_data{rhs.m_data == nullptr ? nullptr : allocate(rhs.size())},
    m_endSize{m_data + rhs.size()},
    m_endCapacity{m_data + rhs.size()}
    {
        copyRange(m_data, rhs.m_data, rhs.m_endSize);
    }


    ////////////////////////////////////////////////////////////
    Vector& operator=(const Vector& rhs)
    {
        if (this == &rhs)
            return *this;

        clear();
        reserve(rhs.size());
        copyRange(m_data, rhs.m_data, rhs.m_endSize);

        m_endSize = m_data + rhs.size();

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] Vector(Vector&& rhs) noexcept :
    m_data{rhs.m_data},
    m_endSize{rhs.m_endSize},
    m_endCapacity{rhs.m_endCapacity}
    {
        rhs.m_data    = nullptr;
        rhs.m_endSize = rhs.m_endCapacity = nullptr;
    }


    ////////////////////////////////////////////////////////////
    Vector& operator=(Vector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        destroyRange(m_data, m_endSize);
        deallocate(m_data, capacity());

        m_data        = rhs.m_data;
        m_endSize     = rhs.m_endSize;
        m_endCapacity = rhs.m_endCapacity;

        rhs.m_data    = nullptr;
        rhs.m_endSize = rhs.m_endCapacity = nullptr;

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
            destroyRange(m_data + newSize, m_endSize);
        }

        m_endSize = m_data + newSize;
    }


    ////////////////////////////////////////////////////////////
    void shrinkToFit()
    {
        const SizeT currentSize = size();

        if (capacity() <= currentSize)
            return;

        if (currentSize == 0u)
        {
            destroyRange(m_data, m_endSize);
            deallocate(m_data, capacity());

            m_data    = nullptr;
            m_endSize = m_endCapacity = nullptr;

            return;
        }

        auto* newData = allocate(currentSize);

        moveRange(newData, m_data, m_endSize);
        destroyRange(m_data, m_endSize);

        deallocate(m_data, capacity());

        m_data    = newData;
        m_endSize = m_endCapacity = m_data + currentSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem*& reserveMore(const SizeT n)
    {
        const SizeT targetCapacity = size() + n;

        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl((targetCapacity * 3u / 2u) + n);

        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem*& reserve(const SizeT targetCapacity)
    {
        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);

        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void assignRange(const TItem* const b, const TItem* const e)
    {
        SFML_BASE_ASSERT(b != nullptr);
        SFML_BASE_ASSERT(e != nullptr);
        SFML_BASE_ASSERT(b <= e);

        const auto count = static_cast<SizeT>(e - b);

        clear();
        reserve(count);
        copyRange(m_data, b, e);

        m_endSize = m_data + count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceRange(const TItem* const ptr, const SizeT count) noexcept
    {
        SFML_BASE_ASSERT(size() + count <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        copyRange(m_endSize, ptr, ptr + count);

        m_endSize += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void emplaceRange(const TItem* const ptr, const SizeT count)
    {
        reserveMore(count);
        unsafeEmplaceRange(ptr, count);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceOther(const Vector& rhs) noexcept
    {
        unsafeEmplaceRange(rhs.m_data, rhs.size());
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear() noexcept
    {
        destroyRange(m_data, m_endSize);
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
    [[nodiscard, gnu::always_inline, gnu::pure]] bool empty() const noexcept
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
    template <typename... Ts>
    [[gnu::always_inline]] TItem& emplaceBack(Ts&&... xs)
    {
        reserveMore(1);
        return unsafeEmplaceBack(static_cast<Ts&&>(xs)...);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline]] TItem& pushBack(T&& x)
    {
        reserveMore(1);
        return unsafeEmplaceBack(static_cast<T&&>(x));
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const it)
    {
        SFML_BASE_ASSERT(it >= begin());
        SFML_BASE_ASSERT(it < end());

        TItem* const nextElement = it + 1;

        if constexpr (triviallyCopyable)
        {
            const auto numElementsToMove = static_cast<SizeT>(end() - nextElement);

            if (numElementsToMove > 0u)
            {
                // Move elements from `[it + 1, end())` to `[it, end() - 1)`
                SFML_BASE_MEMMOVE(it,                                 // Destination
                                  nextElement,                        // Source
                                  numElementsToMove * sizeof(TItem)); // Number of bytes
            }
        }
        else
        {
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
        }

        --m_endSize;

        if constexpr (!triviallyDestructible)
        {
            m_endSize->~TItem();
        }

        return it;
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline, gnu::flatten]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(size() + sizeof...(items) <= capacity());
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(m_endSize != nullptr);

        (SFML_BASE_PLACEMENT_NEW(m_endSize++) TItem(static_cast<TItems&&>(items)), ...);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void pushBackMultiple(TItems&&... items)
    {
        reserveMore(sizeof...(items));
        unsafePushBackMultiple(static_cast<TItems&&>(items)...);
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
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return *(m_data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return *(m_data + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* begin() noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* begin() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* end() noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* end() const noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cbegin() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cend() const noexcept
    {
        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept
    {
        return this->operator[](0u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept
    {
        return this->operator[](0u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& back() const noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        --m_endSize;

        if constexpr (!triviallyDestructible)
        {
            m_endSize->~TItem();
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeSetSize(SizeT newSize) noexcept
    {
        m_endSize = m_data + newSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator==(const Vector& rhs) const
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
    [[nodiscard]] bool operator!=(const Vector& rhs) const
    {
        return !(*this == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(Vector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        base::swap(m_data, rhs.m_data);
        base::swap(m_endSize, rhs.m_endSize);
        base::swap(m_endCapacity, rhs.m_endCapacity);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(Vector& lhs, Vector& rhs) noexcept
    {
        lhs.swap(rhs);
    }
};

} // namespace sf::base
