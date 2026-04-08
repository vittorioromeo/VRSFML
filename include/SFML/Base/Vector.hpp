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
/// \brief Heap-allocated dynamic array, lightweight `std::vector` replacement
///
/// Provides the usual `std::vector` interface plus a few opinionated
/// extras (`unsafeEmplaceBack`, `unsafePushBackMultiple`,
/// `reserveMore`, `unsafeSetSize`) tuned for hot paths where the caller
/// has already ensured enough capacity.
///
/// Implementation notes:
/// - Storage is tracked with three pointers (`begin`, `end`, `endCap`)
///   to make `size()`, `capacity()`, and `data()` extremely cheap and
///   to avoid recomputing offsets in tight loops.
/// - Growth is geometric (×1.5) and clamped to the requested target.
/// - `Vector` is itself trivially relocatable.
/// - Trivially relocatable element types are moved with `memcpy` rather
///   than per-element move constructors.
/// - Avoids the heavy `<vector>` standard header.
///
////////////////////////////////////////////////////////////
template <typename TItem>
class [[nodiscard]] Vector
{
private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    TItem* m_data{nullptr};        //!< Pointer to the beginning of the storage, or `nullptr`
    TItem* m_endSize{nullptr};     //!< Pointer one past the last constructed element
    TItem* m_endCapacity{nullptr}; //!< Pointer one past the end of the allocated storage


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
            priv::VectorUtils::relocateRange(newData, m_data, m_endSize);
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
    /// \brief Default constructor, creates an empty vector with no allocation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector() = default;


    ////////////////////////////////////////////////////////////
    /// \brief Destructor, destroys all elements and frees the storage
    ///
    ////////////////////////////////////////////////////////////
    ~Vector()
    {
        priv::VectorUtils::destroyRange(m_data, m_endSize);
        priv::VectorUtils::deallocate(m_data, capacity());
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a vector with `initialSize` default-constructed elements
    ///
    /// \param initialSize Number of elements to construct
    ///
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
    /// \brief Construct a vector with `initialSize` copies of `value`
    ///
    /// \param initialSize Number of elements to construct
    /// \param value       Value to copy into every element
    ///
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
    /// \brief Construct a vector by copying the range `[srcBegin, srcEnd)`
    ///
    /// \param srcBegin Pointer to the first source element
    /// \param srcEnd   Pointer one past the last source element
    ///
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
    /// \brief Construct a vector from a brace-enclosed initializer list
    ///
    /// \param iList Initializer list whose contents are copied into the vector
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] /* implicit */ Vector(const std::initializer_list<TItem> iList) : Vector(iList.begin(), iList.end())
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] Vector(const Vector& rhs)
    {
        const SizeT rhsSize = rhs.size();

        if (rhsSize == 0u)
            return;

        m_data        = priv::VectorUtils::allocate<TItem>(rhsSize);
        m_endSize     = m_data + rhsSize;
        m_endCapacity = m_data + rhsSize;

        priv::VectorUtils::copyRange(m_data, rhs.m_data, rhs.m_endSize);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
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
    /// \brief Move constructor (steals storage from `rhs`)
    ///
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
    /// \brief Move assignment (frees existing storage, then steals from `rhs`)
    ///
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
    /// \brief Resize the vector to `newSize` elements
    ///
    /// If growing, additional elements are constructed from `args...`
    /// (note: `args` are intentionally not perfect-forwarded so that they
    /// can be reused for every newly constructed element). If shrinking,
    /// trailing elements are destroyed in place; capacity is unchanged.
    ///
    /// \param newSize Target size after the call
    /// \param args    Arguments forwarded to new element constructors when growing
    ///
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
    /// \brief Construct a new element in-place at position `pos`
    ///
    /// May invalidate iterators if the underlying buffer needs to grow.
    /// `pos` is recomputed from its index after a potential reallocation.
    ///
    /// \param pos Iterator to the insertion position
    /// \param xs  Arguments forwarded to the element's constructor
    ///
    /// \return Iterator to the newly inserted element
    ///
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
    /// \brief Insert a copy of `value` at position `pos`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem* insert(TItem* const pos, const TItem& value)
    {
        return emplace(pos, value);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Insert a moved-from `value` at position `pos`
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem* insert(TItem* const pos, TItem&& value)
    {
        return emplace(pos, static_cast<TItem&&>(value));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Shrink the allocated storage to match `size()`
    ///
    /// If `size() == 0`, the buffer is freed entirely. Otherwise the
    /// elements are relocated into a freshly allocated buffer of exact
    /// size.
    ///
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

        priv::VectorUtils::relocateRange(newData, m_data, m_endSize);
        priv::VectorUtils::deallocate(m_data, capacity());

        m_data    = newData;
        m_endSize = m_endCapacity = m_data + currentSize;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Ensure capacity is at least `targetCapacity`
    ///
    /// If the current capacity is already large enough, this is a no-op.
    /// Returns a reference to the internal end pointer to enable hot-path
    /// fused reserve+write idioms.
    ///
    /// \param targetCapacity Desired minimum capacity
    ///
    /// \return Reference to the internal end-of-size pointer
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] TItem*& reserve(const SizeT targetCapacity)
    {
        if (capacity() < targetCapacity) [[unlikely]]
            reserveImpl(targetCapacity);

        return m_endSize;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Ensure capacity is at least `size() + n`
    ///
    /// Convenience wrapper around `reserve()`, useful when bulk-appending
    /// `n` known elements.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] TItem*& reserveMore(const SizeT n)
    {
        return reserve(size() + n);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Append `count` copies of the elements at `ptr` without growing
    ///
    /// The caller is responsible for ensuring that `size() + count <= capacity()`.
    /// Asserts in debug builds.
    ///
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
    /// \brief Destroy all elements; capacity is unchanged
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void clear() noexcept
    {
        priv::VectorUtils::destroyRange(m_data, m_endSize);
        m_endSize = m_data;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the number of constructed elements
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT size() const noexcept
    {
        return static_cast<SizeT>(m_endSize - m_data);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Get the number of elements the storage can hold without reallocating
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] SizeT capacity() const noexcept
    {
        return static_cast<SizeT>(m_endCapacity - m_data);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct an element at the back without growing the buffer
    ///
    /// The caller is responsible for ensuring `size() < capacity()`.
    /// Asserts in debug builds.
    ///
    /// \param xs Arguments forwarded to the element's constructor
    ///
    /// \return Reference to the newly constructed element
    ///
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
    /// \brief Erase the element at iterator `it`, shifting subsequent elements left
    ///
    /// \return Iterator to the element that now occupies `it`'s position
    ///
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
    /// \brief Erase the half-open range `[first, last)`, shifting subsequent elements left
    ///
    /// \return Iterator to the element that now occupies `first`'s position
    ///
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
    /// \brief Append several elements at the back without growing the buffer
    ///
    /// The caller is responsible for ensuring that
    /// `size() + sizeof...(items) <= capacity()`. Asserts in debug builds.
    ///
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
    /// \brief Pointer to the underlying contiguous storage (or `nullptr` if empty and unallocated)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem* data() noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Pointer to the underlying contiguous storage (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem* data() const noexcept
    {
        return m_data;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Force the logical size to `newSize` without constructing or destroying elements
    ///
    /// Useful when the storage was filled by external code (e.g. `memcpy`,
    /// I/O). The caller is responsible for ensuring `newSize <= capacity()`.
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline, gnu::flatten]] void unsafeSetSize(SizeT newSize) noexcept
    {
        SFML_BASE_ASSERT(newSize <= capacity());
        m_endSize = m_data + newSize;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Destroy the last element; asserts that the vector is non-empty
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void popBack() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        --m_endSize;

        if constexpr (!SFML_BASE_IS_TRIVIALLY_DESTRUCTIBLE(TItem))
            m_endSize->~TItem();
    }


    ////////////////////////////////////////////////////////////
    /// \brief Swap the contents of two vectors in O(1)
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] void swap(Vector& rhs) noexcept
    {
        if (this == &rhs)
            return;

        base::genericSwap(m_data, rhs.m_data);
        base::genericSwap(m_endSize, rhs.m_endSize);
        base::genericSwap(m_endCapacity, rhs.m_endCapacity);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Reference to the first element; asserts that the vector is non-empty
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& front() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *m_data;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Reference to the first element (const overload)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] const TItem& front() const noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *m_data;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Reference to the last element; asserts that the vector is non-empty
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten, gnu::pure]] TItem& back() noexcept
    {
        SFML_BASE_ASSERT(!empty());
        return *(m_endSize - 1u);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Reference to the last element (const overload)
    ///
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
