#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Builtins/Memcpy.hpp"
#include "SFML/Base/Builtins/Memmove.hpp"
#include "SFML/Base/InitializerList.hpp" // used
#include "SFML/Base/Launder.hpp"
#include "SFML/Base/PlacementNew.hpp"
#include "SFML/Base/PtrDiffT.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Swap.hpp"
#include "SFML/Base/Traits/IsTriviallyCopyable.hpp"
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


namespace sf::base::priv
{
////////////////////////////////////////////////////////////
template <bool Trivial, typename TItem>
union [[nodiscard]] ItemUnion
{
    TItem item;

    [[gnu::always_inline]] ItemUnion()
    {
    }
};


////////////////////////////////////////////////////////////
template <typename TItem>
union [[nodiscard]] ItemUnion<false, TItem>
{
    TItem item;

    [[gnu::always_inline]] ItemUnion()
    {
    }

    [[gnu::always_inline]] ~ItemUnion()
    {
    }
};

} // namespace sf::base::priv


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename TItem>
class [[nodiscard]] TrivialVector
{
private:
    ////////////////////////////////////////////////////////////
    enum : bool
    {
        triviallyDestructible = SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem),
        triviallyCopyable     = SFML_BASE_IS_TRIVIALLY_COPYABLE(TItem),
    };


    ////////////////////////////////////////////////////////////
    using ItemUnion = priv::ItemUnion<triviallyDestructible, TItem>;


    ////////////////////////////////////////////////////////////
    static_assert(sizeof(ItemUnion) == sizeof(TItem));
    static_assert(alignof(ItemUnion) == alignof(TItem));
    static_assert(SFML_BASE_IS_TRIVIALLY_COPYABLE(ItemUnion) == triviallyCopyable);
    static_assert(SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(ItemUnion) == triviallyDestructible);


    ////////////////////////////////////////////////////////////
    ItemUnion* m_data{nullptr};
    TItem*     m_endSize{nullptr};
    TItem*     m_endCapacity{nullptr};


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static TItem* asItemPtr(ItemUnion* p) noexcept
    {
        return SFML_BASE_LAUNDER_CAST(TItem*, p);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static TItem* asItemPtr(TItem* p) noexcept
    {
        return p;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static const TItem* asItemPtr(const ItemUnion* p) noexcept
    {
        return SFML_BASE_LAUNDER_CAST(const TItem*, p);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] static const TItem* asItemPtr(const TItem* p) noexcept
    {
        return p;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void moveRangeImpl(TItem* target, TItem* const srcBegin, TItem* const srcEnd)
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
    [[gnu::always_inline, gnu::flatten]] static void moveRange(auto* target, auto* const srcBegin, auto* const srcEnd)
    {
        moveRangeImpl(asItemPtr(target), asItemPtr(srcBegin), asItemPtr(srcEnd));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void copyRangeImpl(TItem* target, const TItem* const srcBegin, const TItem* const srcEnd)
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
    [[gnu::always_inline, gnu::flatten]] static void copyRange(auto* target, const auto* const srcBegin, const auto* const srcEnd)
    {
        copyRangeImpl(asItemPtr(target), asItemPtr(srcBegin), asItemPtr(srcEnd));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] static void destroyRangeImpl(TItem* const srcBegin, TItem* const srcEnd)
    {
        if constexpr (!triviallyDestructible)
        {
            for (auto* p = srcBegin; p != srcEnd; ++p)
                p->~TItem();
        }
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] static void destroyRange(auto* const srcBegin, auto* const srcEnd)
    {
        destroyRangeImpl(asItemPtr(srcBegin), asItemPtr(srcEnd));
    }


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline]] void reserveImpl(const SizeT targetCapacity)
    {
        SFML_BASE_ASSERT(targetCapacity > capacity()); // Should only be called to grow

        auto*      newData = new ItemUnion[targetCapacity];
        const auto oldSize = size();

        if (m_data != nullptr)
        {
            moveRange(newData, m_data, m_endSize);
            destroyRange(m_data, m_endSize);
            delete[] m_data;
        }
        else
        {
            SFML_BASE_ASSERT(size() == 0);
            SFML_BASE_ASSERT(capacity() == 0);
        }

        m_data        = newData;
        m_endSize     = data() + oldSize;
        m_endCapacity = data() + targetCapacity;
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
    [[nodiscard]] TrivialVector() = default;


    ////////////////////////////////////////////////////////////
    ~TrivialVector()
    {
        destroyRange(m_data, m_endSize);
        delete[] m_data;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TrivialVector(const SizeT initialSize)
    {
        if (initialSize == 0u)
            return;

        m_data    = new ItemUnion[initialSize];
        m_endSize = m_endCapacity = data() + initialSize;

        for (TItem* p = data(); p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TrivialVector(const SizeT initialSize, const TItem& value)
    {
        if (initialSize == 0u)
            return;

        m_data    = new ItemUnion[initialSize];
        m_endSize = m_endCapacity = data() + initialSize;

        for (TItem* p = data(); p < m_endSize; ++p)
            SFML_BASE_PLACEMENT_NEW(p) TItem(value);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TrivialVector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        m_data    = new ItemUnion[srcCount];
        m_endSize = m_endCapacity = data() + srcCount;

        copyRange(m_data, srcBegin, srcEnd);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit TrivialVector(const std::initializer_list<TItem> iList) :
    TrivialVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TrivialVector(const TrivialVector& rhs) :
    m_data{rhs.m_data == nullptr ? nullptr : new ItemUnion[rhs.size()]},
    m_endSize{data() + rhs.size()},
    m_endCapacity{data() + rhs.size()}
    {
        copyRange(m_data, rhs.m_data, rhs.m_endSize);
    }


    ////////////////////////////////////////////////////////////
    TrivialVector& operator=(const TrivialVector& rhs)
    {
        if (this == &rhs)
            return *this;

        clear();
        reserve(rhs.size());
        copyRange(m_data, rhs.data(), rhs.m_endSize);

        m_endSize = data() + rhs.size();

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] TrivialVector(TrivialVector&& rhs) noexcept :
    m_data{rhs.m_data},
    m_endSize{rhs.m_endSize},
    m_endCapacity{rhs.m_endCapacity}
    {
        rhs.m_data    = nullptr;
        rhs.m_endSize = rhs.m_endCapacity = nullptr;
    }


    ////////////////////////////////////////////////////////////
    TrivialVector& operator=(TrivialVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        destroyRange(m_data, m_endSize);
        delete[] m_data;

        m_data        = rhs.m_data;
        m_endSize     = rhs.m_endSize;
        m_endCapacity = rhs.m_endCapacity;

        rhs.m_data    = nullptr;
        rhs.m_endSize = rhs.m_endCapacity = nullptr;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT newSize)
    {
        const auto oldSize = size();

        if (newSize > oldSize)
        {
            reserve(newSize);

            for (auto* p = data() + oldSize; p != data() + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem();
        }
        else
        {
            destroyRange(data() + newSize, m_endSize);
        }

        m_endSize = data() + newSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT newSize, const TItem& item)
    {
        // TODO P0: repetition with above

        const auto oldSize = size();

        if (newSize > oldSize)
        {
            reserve(newSize);

            for (auto* p = data() + oldSize; p != data() + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem(item);
        }
        else
        {
            destroyRange(data() + newSize, m_endSize);
            m_endSize = data() + newSize;
        }
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
            delete[] m_data;

            m_data    = nullptr;
            m_endSize = m_endCapacity = nullptr;

            return;
        }

        auto* newData = new ItemUnion[currentSize];

        // TODO P0: optimize into single one
        moveRange(newData, m_data, m_endSize);
        destroyRange(m_data, m_endSize);

        delete[] m_data;

        m_data    = newData;
        m_endSize = m_endCapacity = data() + currentSize;
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

        m_endSize = data() + count;
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
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceOther(const TrivialVector& rhs) noexcept
    {
        unsafeEmplaceRange(rhs.data(), rhs.size());
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void clear() noexcept
    {
        destroyRange(m_data, m_endSize);
        m_endSize = data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_endSize - data());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT capacity() const noexcept
    {
        return static_cast<SizeT>(m_endCapacity - data());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool empty() const noexcept
    {
        return data() == m_endSize;
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
        return SFML_BASE_LAUNDER_CAST(TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* data() const noexcept
    {
        return SFML_BASE_LAUNDER_CAST(const TItem*, m_data);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return *(data() + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(m_data != nullptr);
        SFML_BASE_ASSERT(i < size());

        return *(data() + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* begin() noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* begin() const noexcept
    {
        return data();
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
        return data();
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
        m_endSize = SFML_BASE_LAUNDER_CAST(TItem*, m_data) + newSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator==(const TrivialVector& rhs) const
    {
        if (this == &rhs)
            return true;

        const SizeT lhsSize = size();

        if (lhsSize != rhs.size())
            return false;

        for (SizeT i = 0u; i < lhsSize; ++i)
            if (m_data[i].item != rhs.m_data[i].item)
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator!=(const TrivialVector& rhs) const
    {
        return !(*this == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(TrivialVector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        base::swap(m_data, rhs.m_data);
        base::swap(m_endSize, rhs.m_endSize);
        base::swap(m_endCapacity, rhs.m_endCapacity);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(TrivialVector& lhs, TrivialVector& rhs) noexcept
    {
        lhs.swap(rhs);
    }
};

} // namespace sf::base
