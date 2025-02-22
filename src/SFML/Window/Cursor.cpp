#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/CursorImpl.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Vector2.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Cursor::Cursor(base::PassKey<Cursor>&&)
{
}


////////////////////////////////////////////////////////////
Cursor::~Cursor() = default;


////////////////////////////////////////////////////////////
Cursor::Cursor(Cursor&&) noexcept = default;


////////////////////////////////////////////////////////////
Cursor& Cursor::operator=(Cursor&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Cursor> Cursor::loadFromPixels(const base::U8* pixels, Vector2u size, Vector2u hotspot)
{
    base::Optional<Cursor> cursor; // Use a single local variable for NRVO

    if ((pixels == nullptr) || (size.x == 0) || (size.y == 0))
        return cursor; // Empty optional

    cursor.emplace(base::PassKey<Cursor>{});
    if (!cursor->m_impl->loadFromPixels(pixels, size, hotspot))
    {
        priv::err() << "Failed to load cursor from pixels (invalid arguments)";
        return cursor; // Empty optional
    }

    cursor.emplace(base::PassKey<Cursor>{});
    if (!cursor->m_impl->loadFromPixels(pixels, size, hotspot))
    {
        // Error message generated in called function.
        cursor.reset();
    }

    // Error message generated in called function.
    return cursor;
}


////////////////////////////////////////////////////////////
base::Optional<Cursor> Cursor::loadFromSystem(Type type)
{
    auto cursor = base::makeOptional<Cursor>(base::PassKey<Cursor>{}); // Use a single local variable for NRVO

    if (!cursor->m_impl->loadFromSystem(type))
    {
        // Error message generated in called function.
        cursor.reset();
    }

    // Error message generated in called function.
    return cursor;
}


////////////////////////////////////////////////////////////
const priv::CursorImpl& Cursor::getImpl() const
{
    return *m_impl;
}

} // namespace sf
