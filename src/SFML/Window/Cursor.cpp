#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Vec2.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>


namespace sf
{
////////////////////////////////////////////////////////////
[[nodiscard]] bool Cursor::reloadFromPixels(const base::U8* pixels, const Vec2u size, const Vec2u hotspot)
{
    auto surface = priv::getSDLLayerSingleton().createSurfaceFromPixels(size, pixels);
    if (surface == nullptr)
    {
        priv::err() << "Failed to reload cursor from pixels";
        return false;
    }

    SDL_Cursor* cursor = SDL_CreateColorCursor(surface.get(), static_cast<int>(hotspot.x), static_cast<int>(hotspot.y));
    if (cursor == nullptr)
    {
        priv::err() << "Failed to create cursor from surface: " << SDL_GetError();
        return false;
    }

    if (m_sdlCursor != nullptr)
        SDL_DestroyCursor(m_sdlCursor);

    m_sdlCursor = cursor;
    return true;
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool Cursor::reloadFromSystem(const Cursor::Type type)
{
    SDL_Cursor* cursor = SDL_CreateSystemCursor(priv::cursorTypeToSDLCursor(type));
    if (cursor == nullptr)
    {
        priv::err() << "Failed to create system cursor: " << SDL_GetError();
        return false;
    }

    if (m_sdlCursor != nullptr)
        SDL_DestroyCursor(m_sdlCursor);

    m_sdlCursor = cursor;
    return true;
}


////////////////////////////////////////////////////////////
Cursor::Cursor(base::PassKey<Cursor>&&) : m_sdlCursor{nullptr}
{
    (void)priv::getSDLLayerSingleton(); // TODO P0:
}


////////////////////////////////////////////////////////////
Cursor::~Cursor()
{
    if (m_sdlCursor != nullptr)
        SDL_DestroyCursor(m_sdlCursor);
}


////////////////////////////////////////////////////////////
Cursor::Cursor(Cursor&& rhs) noexcept : m_sdlCursor{rhs.m_sdlCursor}
{
    rhs.m_sdlCursor = nullptr;
}


////////////////////////////////////////////////////////////
Cursor& Cursor::operator=(Cursor&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    if (m_sdlCursor != nullptr)
        SDL_DestroyCursor(m_sdlCursor);

    m_sdlCursor     = rhs.m_sdlCursor;
    rhs.m_sdlCursor = nullptr;

    return *this;
}


////////////////////////////////////////////////////////////
base::Optional<Cursor> Cursor::loadFromPixels(const base::U8* pixels, Vec2u size, Vec2u hotspot)
{
    base::Optional<Cursor> cursor; // Use a single local variable for NRVO

    if ((pixels == nullptr) || (size.x == 0) || (size.y == 0))
        return cursor; // Empty optional

    cursor.emplace(base::PassKey<Cursor>{});
    if (!cursor->reloadFromPixels(pixels, size, hotspot))
    {
        priv::err() << "Failed to load cursor from pixels (invalid arguments)";
        return cursor; // Empty optional
    }

    // Error message generated in called function.
    return cursor;
}


////////////////////////////////////////////////////////////
base::Optional<Cursor> Cursor::loadFromSystem(Type type)
{
    auto cursor = base::makeOptional<Cursor>(base::PassKey<Cursor>{}); // Use a single local variable for NRVO

    if (!cursor->reloadFromSystem(type))
    {
        // Error message generated in called function.
        cursor.reset();
    }

    // Error message generated in called function.
    return cursor;
}


////////////////////////////////////////////////////////////
void* Cursor::getImpl() const
{
    return m_sdlCursor;
}

} // namespace sf
