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
#include "SFML/Base/Traits/IsTriviallyDestructible.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
template <typename TItem, SizeT N>
class [[nodiscard]] InPlaceVector // NOLINT(cppcoreguidelines-pro-type-member-init)
{
    static_assert(N > 0);

private:
    ////////////////////////////////////////////////////////////
    alignas(TItem) unsigned char m_storage[sizeof(TItem) * N];
    SizeT m_size{0u};


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
    [[nodiscard]] InPlaceVector() = default;


    ////////////////////////////////////////////////////////////
    ~InPlaceVector()
    {
        priv::VectorUtils::destroyRange(data(), data() + m_size);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] explicit InPlaceVector(const SizeT initialSize) : m_size{initialSize}
    {
        SFML_BASE_ASSERT(initialSize <= N);
        priv::VectorUtils::defaultConstructRange(data(), data() + initialSize);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] explicit InPlaceVector(const SizeT initialSize, const TItem& value) : m_size{initialSize}
    {
        SFML_BASE_ASSERT(initialSize <= N);
        priv::VectorUtils::copyConstructRange(data(), data() + initialSize, value);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] explicit InPlaceVector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);
        SFML_BASE_ASSERT(srcCount <= N);

        priv::VectorUtils::copyRange(data(), srcBegin, srcEnd);
        m_size = srcCount;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit InPlaceVector(const std::initializer_list<TItem> iList) :
    InPlaceVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlaceVector(const InPlaceVector& rhs) : m_size{rhs.m_size}
    {
        priv::VectorUtils::copyRange(data(), rhs.data(), rhs.data() + m_size);
    }


    ////////////////////////////////////////////////////////////
    InPlaceVector& operator=(const InPlaceVector& rhs)
    {
        if (this == &rhs)
            return *this;

        clear();
        priv::VectorUtils::copyRange(data(), rhs.data(), rhs.data() + rhs.m_size);

        m_size = rhs.m_size;
        return *this;
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] InPlaceVector(InPlaceVector&& rhs) noexcept : m_size{rhs.m_size}
    {
        priv::VectorUtils::moveRange(data(), rhs.data(), rhs.data() + m_size);
        priv::VectorUtils::destroyRange(rhs.data(), rhs.data() + m_size);

        rhs.m_size = 0u;
    }


    ////////////////////////////////////////////////////////////
    InPlaceVector& operator=(InPlaceVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        clear();

        priv::VectorUtils::moveRange(data(), rhs.data(), rhs.data() + rhs.m_size);
        priv::VectorUtils::destroyRange(rhs.data(), rhs.data() + rhs.m_size);

        m_size     = rhs.m_size;
        rhs.m_size = 0u;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void resize(const SizeT newSize, auto&&... args)
    {
        SFML_BASE_ASSERT(newSize <= N);

        const auto   oldSize        = m_size;
        TItem* const currentDataPtr = data();

        if (newSize > oldSize)
        {
            for (auto* p = currentDataPtr + oldSize; p != currentDataPtr + newSize; ++p)
                SFML_BASE_PLACEMENT_NEW(p) TItem(args...); // intentionally not forwarding
        }
        else if (newSize < oldSize) // Shrinking
        {
            priv::VectorUtils::destroyRange(currentDataPtr + newSize, currentDataPtr + oldSize);
        }

        m_size = newSize;
    }


    ////////////////////////////////////////////////////////////
    void shrinkToFit() noexcept
    {
        // no-op, just for compatibility
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserve(const SizeT targetCapacity)
    {
        SFML_BASE_ASSERT(targetCapacity <= N);
        // no-op, just for compatibility
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void reserveMore(const SizeT n)
    {
        SFML_BASE_ASSERT(m_size + n <= N);
        // no-op, just for compatibility
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void assignRange(const TItem* const b, const TItem* const e)
    {
        SFML_BASE_ASSERT(b != nullptr);
        SFML_BASE_ASSERT(e != nullptr);
        SFML_BASE_ASSERT(b <= e);

        const auto count = static_cast<SizeT>(e - b);
        SFML_BASE_ASSERT(count <= N);

        clear();
        priv::VectorUtils::copyRange(data(), b, e);

        m_size = count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceRange(const TItem* const ptr, const SizeT count) noexcept
    {
        SFML_BASE_ASSERT(m_size + count <= N);          // Capacity check is vital here
        SFML_BASE_ASSERT(ptr != nullptr || count == 0); // ptr can be null only if count is 0

        priv::VectorUtils::copyRange(data() + m_size, ptr, ptr + count);
        m_size += count;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void emplaceRange(const TItem* const ptr, const SizeT count)
    {
        SFML_BASE_ASSERT(m_size + count <= N); // Ensure fits
        unsafeEmplaceRange(ptr, count);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeEmplaceOther(const InPlaceVector& rhs) noexcept
    {
        unsafeEmplaceRange(rhs.data(), rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear() noexcept
    {
        priv::VectorUtils::destroyRange(data(), data() + m_size);
        m_size = 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] constexpr SizeT capacity() const noexcept
    {
        return N;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] bool empty() const noexcept
    {
        return m_size == 0u;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] TItem& unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size < N);
        return *(SFML_BASE_PLACEMENT_NEW(data() + m_size++) TItem(static_cast<Ts&&>(xs)...));
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline, gnu::flatten]] TItem& emplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size < N);
        return unsafeEmplaceBack(static_cast<Ts&&>(xs)...);
    }


    ////////////////////////////////////////////////////////////
    template <typename T>
    [[gnu::always_inline, gnu::flatten]] TItem& pushBack(T&& x)
    {
        SFML_BASE_ASSERT(m_size < N);
        return unsafeEmplaceBack(static_cast<T&&>(x));
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const it)
    {
        const auto resultIt = priv::VectorUtils::eraseImpl(begin(), end(), it);
        --m_size;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            (data() + m_size)->~TItem();

        return resultIt;
    }


    ////////////////////////////////////////////////////////////
    TItem* erase(TItem* const first, TItem* const last)
    {
        SFML_BASE_ASSERT(first <= last);

        if (first == last)
            return first; // No elements to erase

        TItem* currWritePtr = priv::VectorUtils::eraseRangeImpl(begin(), end(), first, last);
        m_size              = static_cast<SizeT>(currWritePtr - data());

        // Return an iterator to the element that now occupies the position
        // where the first erased element (`first`) was. This is `first` itself,
        // as elements were shifted into this position, or it's the new `end()`.
        return first;
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(m_size + sizeof...(items) <= N);
        (SFML_BASE_PLACEMENT_NEW(data() + m_size++) TItem(static_cast<TItems&&>(items)), ...);
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] void pushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(m_size + sizeof...(items) <= N);
        unsafePushBackMultiple(static_cast<TItems&&>(items)...);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* data() noexcept
    {
        return reinterpret_cast<TItem*>(m_storage);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* data() const noexcept
    {
        return reinterpret_cast<const TItem*>(m_storage);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& operator[](const SizeT i) noexcept
    {
        SFML_BASE_ASSERT(i < m_size);
        return *(data() + i);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& operator[](const SizeT i) const noexcept
    {
        SFML_BASE_ASSERT(i < m_size);
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
        return data() + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* end() const noexcept
    {
        return data() + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cbegin() const noexcept
    {
        return data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* cend() const noexcept
    {
        return data() + m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept
    {
        SFML_BASE_ASSERT(m_size > 0u);
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept
    {
        SFML_BASE_ASSERT(m_size > 0u);
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept
    {
        SFML_BASE_ASSERT(m_size > 0u);
        return *(data() + m_size - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& back() const noexcept
    {
        SFML_BASE_ASSERT(m_size > 0u);
        return *(data() + m_size - 1u);
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
    [[gnu::always_inline, gnu::flatten]] void unsafeSetSize(SizeT newSize) noexcept
    {
        SFML_BASE_ASSERT(newSize <= N);
        m_size = newSize;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator==(const InPlaceVector& rhs) const
    {
        if (this == &rhs)
            return true;

        if (m_size != rhs.m_size)
            return false;

        for (SizeT i = 0u; i < m_size; ++i)
            if (operator[](i) != rhs.operator[](i))
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator!=(const InPlaceVector& rhs) const
    {
        return !(*this == rhs);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(InPlaceVector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        TItem* const lhsData = data();
        TItem* const rhsData = rhs.data();

        // Cache original sizes
        const SizeT s1 = m_size;
        const SizeT s2 = rhs.m_size;

        const SizeT commonSize = SFML_BASE_MIN(s1, s2);

        for (SizeT i = 0u; i < commonSize; ++i)
            base::swap(lhsData[i], rhsData[i]); // Swap elements in the common part

        if (s1 > s2) // `*this` is initially larger; its tail elements move to `rhs`
        {
            // Move elements from `(lhsData + commonSize)` up to `(lhsData + s1 - 1)`
            // into `(rhsData + commonSize)`
            for (SizeT i = commonSize; i < s1; ++i)
            {
                SFML_BASE_PLACEMENT_NEW(rhsData + i) TItem(static_cast<TItem&&>(lhsData[i]));

                if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
                    (lhsData + i)->~TItem(); // Destroy moved-from element in `*this`
            }
        }
        else if (s2 > s1) // `rhs` is initially larger; its tail elements move to `*this`
        {
            // Move elements from `(rhsData + commonSize)` up to `(rhsData + s2 - 1)`
            // into `(lhsData + commonSize)`
            for (SizeT i = commonSize; i < s2; ++i)
            {
                SFML_BASE_PLACEMENT_NEW(lhsData + i) TItem(static_cast<TItem&&>(rhsData[i]));

                if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
                    (rhsData + i)->~TItem(); // Destroy moved-from element in `rhs`
            }
        }

        // Swap the effective sizes
        base::swap(m_size, rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(InPlaceVector& lhs, InPlaceVector& rhs) noexcept
    {
        lhs.swap(rhs);
    }
};

} // namespace sf::base
