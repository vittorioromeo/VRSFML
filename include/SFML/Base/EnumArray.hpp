#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Traits/IsEnum.hpp"


namespace sf::base
{
////////////////////////////////////////////////////////////
/// \brief Fixed-size array container indexed by an enumeration
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
    [[gnu::always_inline]] constexpr void fill(const Value fillValue)
    {
        for (Value& value : data)
            value = fillValue;
    }


    ////////////////////////////////////////////////////////////
    Value data[Count];
};

} // namespace sf::base
