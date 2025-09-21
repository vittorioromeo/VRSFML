#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


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
template <typename TItem>
class [[nodiscard]] Vector
{
private:
    ////////////////////////////////////////////////////////////
    TItem* m_data{nullptr};
    TItem* m_endSize{nullptr};
    TItem* m_endCapacity{nullptr};


    ////////////////////////////////////////////////////////////
    [[gnu::cold, gnu::noinline, gnu::flatten]] void reserveImpl(const SizeT targetCapacity)
    {
        const auto currentCapacity = capacity();
        const auto geometricGrowthTarget = currentCapacity + (currentCapacity / 2u); // Equivalent to `currentCapacity * 1.5`
        const auto finalNewCapacity = SFML_BASE_MAX(targetCapacity, geometricGrowthTarget);

        SFML_BASE_ASSERT(finalNewCapacity > capacity()); // Should only be called to grow

        auto*      newData = priv::VectorUtils::allocate<TItem>(finalNewCapacity);
        const auto oldSize = size();

        if (m_data != nullptr)
        {
            priv::VectorUtils::moveRange(newData, m_data, m_endSize);
            priv::VectorUtils::destroyRange(m_data, m_endSize);
            priv::VectorUtils::deallocate(m_data, currentCapacity);
        }
        else
        {
            SFML_BASE_ASSERT(size() == 0u);
            SFML_BASE_ASSERT(currentCapacity == 0u);
        }

        m_data        = newData;
        m_endSize     = m_data + oldSize;
        m_endCapacity = m_data + finalNewCapacity;
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
        priv::VectorUtils::destroyRange(m_data, m_endSize);
        priv::VectorUtils::deallocate(m_data, capacity());
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const SizeT initialSize)
    {
        if (initialSize == 0u)
            return;

        m_data    = priv::VectorUtils::allocate<TItem>(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        priv::VectorUtils::defaultConstructRange(m_data, m_endSize);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const SizeT initialSize, const TItem& value)
    {
        if (initialSize == 0u)
            return;

        m_data    = priv::VectorUtils::allocate<TItem>(initialSize);
        m_endSize = m_endCapacity = m_data + initialSize;

        priv::VectorUtils::copyConstructRange(m_data, m_endSize, value);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit Vector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);

        if (srcCount == 0u)
            return;

        m_data    = priv::VectorUtils::allocate<TItem>(srcCount);
        m_endSize = m_endCapacity = m_data + srcCount;

        priv::VectorUtils::copyRange(m_data, srcBegin, srcEnd);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit(false) Vector(const std::initializer_list<TItem> iList) : Vector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] Vector(const Vector& rhs) :
        m_data{rhs.m_data == nullptr ? nullptr : priv::VectorUtils::allocate<TItem>(rhs.size())},
        m_endSize{m_data + rhs.size()},
        m_endCapacity{m_data + rhs.size()}
    {
        priv::VectorUtils::copyRange(m_data, rhs.m_data, rhs.m_endSize);
    }


    ////////////////////////////////////////////////////////////
    Vector& operator=(const Vector& rhs)
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

        priv::VectorUtils::destroyRange(m_data, m_endSize);
        priv::VectorUtils::deallocate(m_data, capacity());

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
            priv::VectorUtils::destroyRange(m_data + newSize, m_endSize);
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
            priv::VectorUtils::destroyRange(m_data, m_endSize);
            priv::VectorUtils::deallocate(m_data, capacity());

            m_data    = nullptr;
            m_endSize = m_endCapacity = nullptr;

            return;
        }

        auto* newData = priv::VectorUtils::allocate<TItem>(currentSize);

        priv::VectorUtils::moveRange(newData, m_data, m_endSize);
        priv::VectorUtils::destroyRange(m_data, m_endSize);

        priv::VectorUtils::deallocate(m_data, capacity());

        m_data    = newData;
        m_endSize = m_endCapacity = m_data + currentSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem*& reserve(const SizeT targetCapacity)
    {
        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);

        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] TItem*& reserveMore([[maybe_unused]] const SizeT n)
    {
        return reserve(size() + n);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceRange(const TItem* const ptr, const SizeT count) noexcept
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
        const auto resultIt = priv::VectorUtils::eraseImpl(begin(), end(), it);
        --m_endSize;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            m_endSize->~TItem();

        return resultIt;
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
    [[gnu::always_inline]] void swap(Vector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        base::swap(m_data, rhs.m_data);
        base::swap(m_endSize, rhs.m_endSize);
        base::swap(m_endCapacity, rhs.m_endCapacity);
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
    SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(Vector);
};

} // namespace sf::base
