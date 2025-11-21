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
#include "SFML/Base/Trait/IsTriviallyDestructible.hpp"
#include "SFML/Base/Trait/IsTriviallyRelocatable.hpp"


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
    enum : bool
    {
        enableTrivialRelocation = SFML_BASE_IS_TRIVIALLY_RELOCATABLE(TItem)
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
    [[nodiscard]] constexpr InPlaceVector() = default;


    ////////////////////////////////////////////////////////////
    constexpr ~InPlaceVector()
    {
        priv::VectorUtils::destroyRange(data(), data() + m_size);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] constexpr explicit InPlaceVector(const SizeT initialSize) : m_size{initialSize}
    {
        SFML_BASE_ASSERT(initialSize <= N);
        priv::VectorUtils::defaultConstructRange(data(), data() + initialSize);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] constexpr explicit InPlaceVector(const SizeT initialSize, const TItem& value) : m_size{initialSize}
    {
        SFML_BASE_ASSERT(initialSize <= N);
        priv::VectorUtils::copyConstructRange(data(), data() + initialSize, value);
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard]] constexpr explicit InPlaceVector(const TItem* const srcBegin, const TItem* const srcEnd)
    {
        SFML_BASE_ASSERT(srcBegin <= srcEnd);
        const auto srcCount = static_cast<SizeT>(srcEnd - srcBegin);
        SFML_BASE_ASSERT(srcCount <= N);

        priv::VectorUtils::copyRange(data(), srcBegin, srcEnd);
        m_size = srcCount;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard]] constexpr /* implicit */ InPlaceVector(const std::initializer_list<TItem> iList) :
        InPlaceVector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    [[nodiscard, gnu::always_inline]] constexpr InPlaceVector(const InPlaceVector& rhs) : m_size{rhs.m_size}
    {
        priv::VectorUtils::copyRange(data(), rhs.data(), rhs.data() + m_size);
    }


    ////////////////////////////////////////////////////////////
    constexpr InPlaceVector& operator=(const InPlaceVector& rhs)
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
    [[nodiscard, gnu::always_inline]] constexpr InPlaceVector(InPlaceVector&& rhs) noexcept : m_size{rhs.m_size}
    {
        priv::VectorUtils::relocateRange(data(), rhs.data(), rhs.data() + rhs.m_size);
        rhs.m_size = 0u;
    }


    ////////////////////////////////////////////////////////////
    constexpr InPlaceVector& operator=(InPlaceVector&& rhs) noexcept
    {
        if (this == &rhs)
            return *this;

        clear();

        priv::VectorUtils::relocateRange(data(), rhs.data(), rhs.data() + rhs.m_size);

        m_size     = rhs.m_size;
        rhs.m_size = 0u;

        return *this;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void resize(const SizeT newSize, auto&&... args)
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
    template <typename... Ts>
    [[gnu::always_inline]] constexpr TItem* emplace(TItem* const pos, Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size < N);
        SFML_BASE_ASSERT(pos >= begin() && pos <= end());

        const auto index = static_cast<SizeT>(pos - data());

        priv::VectorUtils::makeHole(pos, end());
        SFML_BASE_PLACEMENT_NEW(pos) TItem(static_cast<Ts&&>(xs)...);
        ++m_size;

        return data() + index;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr TItem* insert(TItem* const pos, const TItem& value)
    {
        return emplace(pos, value);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr TItem* insert(TItem* const pos, TItem&& value)
    {
        return emplace(pos, static_cast<TItem&&>(value));
    }


    ////////////////////////////////////////////////////////////
    constexpr void shrinkToFit() noexcept
    {
        // no-op, just for compatibility
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void reserve([[maybe_unused]] const SizeT targetCapacity)
    {
        SFML_BASE_ASSERT(targetCapacity <= N);
        // no-op, just for compatibility
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void reserveMore([[maybe_unused]] const SizeT n)
    {
        SFML_BASE_ASSERT(size() + n <= N);
        // no-op, just for compatibility
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void unsafeEmplaceBackRange(const TItem* const ptr, const SizeT count) noexcept
    {
        SFML_BASE_ASSERT(m_size + count <= N);
        SFML_BASE_ASSERT(ptr != nullptr);

        priv::VectorUtils::copyRange(data() + m_size, ptr, ptr + count);
        m_size += count;
    }

    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void clear() noexcept
    {
        priv::VectorUtils::destroyRange(data(), data() + m_size);
        m_size = 0u;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return m_size;
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::const]] constexpr SizeT capacity() const noexcept
    {
        return N;
    }


    ////////////////////////////////////////////////////////////
    template <typename... Ts>
    [[gnu::always_inline]] constexpr TItem& unsafeEmplaceBack(Ts&&... xs)
    {
        SFML_BASE_ASSERT(m_size < N);
        return *(SFML_BASE_PLACEMENT_NEW(data() + m_size++) TItem(static_cast<Ts&&>(xs)...));
    }


    ////////////////////////////////////////////////////////////
    constexpr TItem* erase(TItem* const it)
    {
        priv::VectorUtils::eraseImpl(begin(), end(), it);
        --m_size;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            (data() + m_size)->~TItem();

        return it;
    }


    ////////////////////////////////////////////////////////////
    constexpr TItem* erase(TItem* const first, TItem* const last)
    {
        SFML_BASE_ASSERT(first <= last);

        if (first == last)
            return first; // No elements to erase

        TItem* currWritePtr = priv::VectorUtils::eraseRangeImpl(end(), first, last);
        m_size              = static_cast<SizeT>(currWritePtr - data());

        // Return an iterator to the element that now occupies the position
        // where the first erased element (`first`) was. This is `first` itself,
        // as elements were shifted into this position, or it's the new `end()`.
        return first;
    }


    ////////////////////////////////////////////////////////////
    template <typename... TItems>
    [[gnu::always_inline]] constexpr void unsafePushBackMultiple(TItems&&... items)
    {
        SFML_BASE_ASSERT(m_size + sizeof...(items) <= N);
        (..., SFML_BASE_PLACEMENT_NEW(data() + m_size++) TItem(static_cast<TItems&&>(items)));
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem* data() noexcept
    {
        return reinterpret_cast<TItem*>(m_storage);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem* data() const noexcept
    {
        return reinterpret_cast<const TItem*>(m_storage);
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] constexpr void unsafeSetSize(SizeT newSize) noexcept
    {
        SFML_BASE_ASSERT(newSize <= N);
        m_size = newSize;
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        --m_size;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            (data() + m_size)->~TItem();
    }


    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void swap(InPlaceVector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        priv::VectorUtils::swapUnequalRanges(data(), m_size, rhs.data(), rhs.m_size);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem& front() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem& front() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *data();
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr TItem& back() noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] constexpr const TItem& back() const noexcept
    {
        return this->operator[](size() - 1u);
    }


    ////////////////////////////////////////////////////////////
    SFML_BASE_PRIV_DEFINE_COMMON_VECTOR_OPERATIONS(InPlaceVector);
};

} // namespace sf::base
