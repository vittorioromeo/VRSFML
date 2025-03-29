#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Cursor.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/Vector2.hpp"

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_surface.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] constexpr SDL_SystemCursor cursorTypeToSDLCursor(const sf::Cursor::Type type) noexcept
{
    // clang-format off
    switch (type)
    {
        case sf::Cursor::Type::Arrow:                  return SDL_SYSTEM_CURSOR_DEFAULT;
        case sf::Cursor::Type::ArrowWait:              return SDL_SYSTEM_CURSOR_PROGRESS;
        case sf::Cursor::Type::Wait:                   return SDL_SYSTEM_CURSOR_WAIT;
        case sf::Cursor::Type::Text:                   return SDL_SYSTEM_CURSOR_TEXT;
        case sf::Cursor::Type::Hand:                   return SDL_SYSTEM_CURSOR_POINTER;
        case sf::Cursor::Type::SizeHorizontal:         return SDL_SYSTEM_CURSOR_EW_RESIZE;
        case sf::Cursor::Type::SizeVertical:           return SDL_SYSTEM_CURSOR_NS_RESIZE;
        case sf::Cursor::Type::SizeTopLeftBottomRight: return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
        case sf::Cursor::Type::SizeBottomLeftTopRight: return SDL_SYSTEM_CURSOR_NESW_RESIZE;
        case sf::Cursor::Type::SizeLeft:               return SDL_SYSTEM_CURSOR_W_RESIZE;
        case sf::Cursor::Type::SizeRight:              return SDL_SYSTEM_CURSOR_E_RESIZE;
        case sf::Cursor::Type::SizeTop:                return SDL_SYSTEM_CURSOR_N_RESIZE;
        case sf::Cursor::Type::SizeBottom:             return SDL_SYSTEM_CURSOR_S_RESIZE;
        case sf::Cursor::Type::SizeTopLeft:            return SDL_SYSTEM_CURSOR_NW_RESIZE;
        case sf::Cursor::Type::SizeBottomRight:        return SDL_SYSTEM_CURSOR_SE_RESIZE;
        case sf::Cursor::Type::SizeBottomLeft:         return SDL_SYSTEM_CURSOR_SW_RESIZE;
        case sf::Cursor::Type::SizeTopRight:           return SDL_SYSTEM_CURSOR_NE_RESIZE;
        case sf::Cursor::Type::SizeAll:                return SDL_SYSTEM_CURSOR_MOVE;
        case sf::Cursor::Type::Cross:                  return SDL_SYSTEM_CURSOR_CROSSHAIR;
        case sf::Cursor::Type::NotAllowed:             return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
        case sf::Cursor::Type::Help:                   break; // not supported
    }
    // clang-format on

    return SDL_SYSTEM_CURSOR_DEFAULT;
}

} // namespace


namespace sf
{

////////////////////////////////////////////////////////////
[[nodiscard]] bool Cursor::reloadFromPixels(const base::U8* pixels, const Vector2u size, const Vector2u hotspot)
{
    SDL_Surface* surface = SDL_CreateSurfaceFrom(static_cast<int>(size.x),
                                                 static_cast<int>(size.y),
                                                 SDL_PIXELFORMAT_RGBA32,
                                                 const_cast<Uint8*>(pixels), // SDL requires non-const, but data is read-only
                                                 static_cast<int>(size.x * 4));

    if (surface == nullptr)
    {
        priv::err() << "Failed to create surface from pixels: " << SDL_GetError();
        return false;
    }

    SDL_Cursor* cursor = SDL_CreateColorCursor(surface, static_cast<int>(hotspot.x), static_cast<int>(hotspot.y));
    SDL_DestroySurface(surface);

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
    SDL_Cursor* cursor = SDL_CreateSystemCursor(cursorTypeToSDLCursor(type));

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
base::Optional<Cursor> Cursor::loadFromPixels(const base::U8* pixels, Vector2u size, Vector2u hotspot)
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
