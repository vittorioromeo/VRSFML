#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsConvertible.hpp"
#include "SFML/Base/Trait/IsSame.hpp"
#include "SFML/Base/Trait/RemoveCVRef.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Non-owning view over a contiguous range of `T`
///
/// Lightweight `std::span` replacement. Stores a pointer plus a size,
/// implicitly converts from C arrays and any range-like type with
/// `data()` and `size()`. A `Span<T>` implicitly converts to a
/// `Span<const T>` for read-only views.
///
/// `Span` does not own its memory; the caller must ensure that the
/// referenced range outlives the span.
///
////////////////////////////////////////////////////////////
template <typename T>
struct Span
{
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor, creates an empty span (`data = nullptr`, `size = 0`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span() = default;


    ////////////////////////////////////////////////////////////
    /// \brief Construct a span from a pointer and a size
    ///
    /// \param data Pointer to the first element (may be `nullptr` only if `size == 0`)
    /// \param size Number of elements in the range
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline]] constexpr Span(T* data, SizeT size) : theData{data}, theSize{size}
    {
        SFML_BASE_ASSERT(theData != nullptr || (theData == nullptr && theSize == 0u));
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a span from a C-style array (deduced size)
    ///
    ////////////////////////////////////////////////////////////
    template <SizeT N>
    [[nodiscard, gnu::always_inline]] constexpr Span(T (&array)[N]) : theData{array}, theSize{N}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Construct a span from any contiguous range exposing `data()` and `size()`
    ///
    /// Disabled when `Range` is itself a `Span` so it does not shadow the
    /// implicitly generated copy constructor.
    ///
    ////////////////////////////////////////////////////////////
    template <typename Range>
        requires(!isSame<RemoveCVRefIndirect<Range>, Span> &&
                 requires(Range&& r) {
                     requires isConvertible<decltype(r.data()), T*>;
                     r.size();
                 })
    [[nodiscard, gnu::always_inline]] constexpr Span(Range&& range) : theData{range.data()}, theSize{range.size()}
    {
    }


    ////////////////////////////////////////////////////////////
    /// \brief Implicit conversion to a read-only span
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::flatten]] constexpr operator Span<const T>() const
    {
        return Span<const T>{theData, theSize};
    }


    ////////////////////////////////////////////////////////////
    /// \brief Iterator to the first element
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* begin() const
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Iterator one past the last element
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T* end() const
    {
        return theData + theSize;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Indexed element access (asserts `i < size()`)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr T& operator[](SizeT i) const
    {
        SFML_BASE_ASSERT(i < theSize);
        return *(theData + i);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Pointer to the first element (may be `nullptr` if empty)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const T* data() const noexcept
    {
        return theData;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Number of elements in the span
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr SizeT size() const noexcept
    {
        return theSize;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether the span has zero elements
    ///
    /// Note: returns `true` for both `(nullptr, 0)` and `(non-null, 0)`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool empty() const
    {
        return theSize == 0u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Check whether the span has zero elements *or* a null data pointer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool isNullOrEmpty() const
    {
        return theData == nullptr || theSize == 0u;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Element-wise comparison of two ranges
    ///
    /// Returns `true` only if both ranges have the same length and every
    /// pair of elements compares equal with `operator==`.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const T* rhsData, SizeT rhsSize) const
    {
        if (theSize != rhsSize)
            return false;

        for (SizeT i = 0u; i < theSize; ++i)
            if (theData[i] != rhsData[i])
                return false;

        return true;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Element-wise comparison with another span
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr bool valueEquals(const Span& rhs) const
    {
        return valueEquals(rhs.theData, rhs.theSize);
    }


    ////////////////////////////////////////////////////////////
    /// \brief Swap the data pointer and size of two spans
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] friend void swap(Span& lhs, Span& rhs) noexcept
    {
        const auto swapImpl = []<typename U>(U& a, U& b)
        {
            U tempA = a;
            a       = b;
            b       = tempA;
        };

        swapImpl(lhs.theData, rhs.theData);
        swapImpl(lhs.theSize, rhs.theSize);
    }


    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    T*    theData{nullptr};
    SizeT theSize{0u};
};

} // namespace sf::base
