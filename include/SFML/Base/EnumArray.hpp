#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Trait/IsEnum.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Fixed-size array container indexed by an enumeration
///
/// `EnumArray<Enum, Value, Count>` stores `Count` instances of `Value`
/// and exposes them through `operator[]` taking an `Enum` key. The
/// underlying enum value is converted to its integer representation,
/// which must be in `[0, Count)` (asserted in debug builds).
///
/// Useful for tables keyed by an enum class, e.g. per-event-type or
/// per-shader-channel state.
///
////////////////////////////////////////////////////////////
template <typename Enum, typename Value, SizeT Count>
struct EnumArray
{
    ////////////////////////////////////////////////////////////
    static_assert(SFML_BASE_IS_ENUM(Enum));


    ////////////////////////////////////////////////////////////
    /// \brief Returns a reference to the element associated to specified \a key
    ///
    /// No bounds checking is performed in release builds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr Value& operator[](const Enum key)
    {
        const auto index = static_cast<SizeT>(key);

        SFML_BASE_ASSERT(index < Count);
        return data[index];
    }


    ////////////////////////////////////////////////////////////
    /// \brief Returns a reference to the element associated to specified \a key
    ///
    /// No bounds checking is performed in release builds.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard, gnu::always_inline, gnu::pure]] constexpr const Value& operator[](const Enum key) const
    {
        const auto index = static_cast<SizeT>(key);

        SFML_BASE_ASSERT(index < Count);
        return data[index];
    }


    ////////////////////////////////////////////////////////////
    /// \brief Assign `fillValue` to every element of the array
    ///
    ////////////////////////////////////////////////////////////
    [[gnu::always_inline]] constexpr void fill(const Value& fillValue)
    {
        for (Value& value : data)
            value = fillValue;
    }


    ////////////////////////////////////////////////////////////
    /// \brief Underlying contiguous storage; exposed publicly to remain an aggregate
    ///
    ////////////////////////////////////////////////////////////
    Value data[Count];
};

} // namespace sf::base
