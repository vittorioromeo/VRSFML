// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/JoystickCapabilities.hpp"
#include "SFML/Window/JoystickIdentification.hpp"
#include "SFML/Window/JoystickManager.hpp"
#include "SFML/Window/JoystickState.hpp"
#include "SFML/Window/SDLLayer.hpp"

#include "SFML/System/Err.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"

// TODO P1: move to SDLLayer
#include <SDL3/SDL_dialog.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_guid.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_joystick.h>
#include <SDL3/SDL_stdinc.h>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::priv::JoystickCapabilities getCapabilitiesFromSDL(SDL_Joystick& handle)
{
    auto& sdlLayer = sf::priv::getSDLLayerSingleton();

    sf::priv::JoystickCapabilities caps{
        .buttonCount = sdlLayer.getJoystickButtonCount(handle),
    };

    const unsigned int numAxes = sdlLayer.getJoystickAxisCount(handle);

    for (unsigned int i = 0u; i < sf::Joystick::AxisCount; ++i)
        caps.axes[static_cast<sf::Joystick::Axis>(i)] = (i < static_cast<unsigned int>(numAxes));

    const unsigned int numHats = sdlLayer.getJoystickHatCount(handle);

    if (numHats > 0u)
    {
        caps.axes[sf::Joystick::Axis::PovX] = true;
        caps.axes[sf::Joystick::Axis::PovY] = true;
    }

    return caps;
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::priv::JoystickIdentification getIdentificationFromSDL(SDL_Joystick& handle)
{
    auto& sdlLayer = sf::priv::getSDLLayerSingleton();

    return {
        .name      = sdlLayer.getJoystickName(handle),
        .vendorId  = sdlLayer.getJoystickVendor(handle),
        .productId = sdlLayer.getJoystickProduct(handle),
    };
}


////////////////////////////////////////////////////////////
[[nodiscard]] sf::priv::JoystickState getStateFromSDL(SDL_Joystick& handle)
{
    auto& sdlLayer = sf::priv::getSDLLayerSingleton();

    sf::priv::JoystickState state;

    ////////////////////////////////////////////////////////////
    // Update axes
    const unsigned int numAxes = sdlLayer.getJoystickAxisCount(handle);
    const unsigned int numHats = sdlLayer.getJoystickHatCount(handle);

    for (unsigned int j = 0; j < sf::Joystick::AxisCount; ++j)
    {
        const auto i = static_cast<sf::Joystick::Axis>(j);

        if (i == sf::Joystick::Axis::PovX || i == sf::Joystick::Axis::PovY)
        {
            if (numHats <= 0)
            {
                state.axes[i] = 0.f;
                continue;
            }

            const Uint8 hatState = SDL_GetJoystickHat(&handle, 0);

            const float x = (hatState & SDL_HAT_LEFT) ? -1.f : (hatState & SDL_HAT_RIGHT) ? 1.f : 0.f;
            const float y = (hatState & SDL_HAT_UP) ? 1.f : (hatState & SDL_HAT_DOWN) ? -1.f : 0.f;

            const float normalizeDiagonalDiv = (x != 0.f && y != 0.f) ? 1.41421f : 1.f;

            state.axes[sf::Joystick::Axis::PovX] = x * 100.f / normalizeDiagonalDiv;
            state.axes[sf::Joystick::Axis::PovY] = y * 100.f / normalizeDiagonalDiv;

            continue;
        }

        if (static_cast<unsigned int>(i) >= static_cast<unsigned int>(numAxes))
        {
            state.axes[i] = 0.f;
            continue;
        }

        const Sint16 axisValue = SDL_GetJoystickAxis(&handle, static_cast<int>(i));
        state.axes[i]          = (static_cast<float>(axisValue) / 32768.f) * 100.f;
    }

    ////////////////////////////////////////////////////////////
    // Update buttons
    const int numButtons = SDL_GetNumJoystickButtons(&handle);
    for (unsigned int i = 0; i < sf::Joystick::ButtonCount; ++i)
        state.buttons[i] = (i < static_cast<unsigned int>(numButtons))
                               ? SDL_GetJoystickButton(&handle, static_cast<int>(i))
                               : false;

    return state;
}

} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct JoystickImpl
{
    SDL_GUID       guid{};          //!< GUID of the joystick
    SDL_JoystickID id{};            //!< ID of the joystick
    SDL_Joystick*  handle{nullptr}; //!< SDL handle to the joystick
};


////////////////////////////////////////////////////////////
struct JoystickManager::Impl
{
    base::Optional<JoystickImpl> impls[Joystick::MaxCount];           //!< Joystick implementations
    JoystickState                states[Joystick::MaxCount];          //!< Joystick states
    JoystickCapabilities         capabilities[Joystick::MaxCount];    //!< Joystick capabilities
    JoystickIdentification       identifications[Joystick::MaxCount]; //!< Joystick identifications
};


////////////////////////////////////////////////////////////
const JoystickCapabilities& JoystickManager::getCapabilities(const unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount);
    return m_impl->capabilities[joystickId];
}


////////////////////////////////////////////////////////////
const JoystickState& JoystickManager::getState(const unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount);
    return m_impl->states[joystickId];
}


////////////////////////////////////////////////////////////
const JoystickIdentification& JoystickManager::getIdentification(const unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount);
    return m_impl->identifications[joystickId];
}


////////////////////////////////////////////////////////////
bool JoystickManager::isConnected(const unsigned int joystickId) const
{
    SFML_BASE_ASSERT(joystickId < Joystick::MaxCount);
    return m_impl->impls[joystickId].hasValue();
}


////////////////////////////////////////////////////////////
void JoystickManager::update()
{
    auto& sdlLayer = priv::getSDLLayerSingleton();

    ////////////////////////////////////////////////////////////
    SDL_UpdateJoysticks();

    ////////////////////////////////////////////////////////////
    struct JoystickInfo
    {
        SDL_JoystickID id;
        SDL_GUID       guid;
    };

    base::Optional<JoystickInfo> connectedJoystickInfos[Joystick::MaxCount];
    base::SizeT                  nextInfoIdx = 0u;

    ////////////////////////////////////////////////////////////
    SDL_LockJoysticks();
    {
        int             count = 0;
        SDL_JoystickID* ids   = SDL_GetJoysticks(&count);

        for (int i = 0; i < count; ++i)
        {
            const auto guid = SDL_GetJoystickGUIDForID(ids[i]);
            SFML_BASE_ASSERT(!sdlLayer.areGUIDsEqual(guid, SDL_GUID{}));

            SFML_BASE_ASSERT(nextInfoIdx < Joystick::MaxCount);
            connectedJoystickInfos[nextInfoIdx++].emplace(ids[i], guid);
        }

        SDL_free(ids);
    }
    SDL_UnlockJoysticks();

    ////////////////////////////////////////////////////////////
    // Find newly disconnected joysticks
    for (base::SizeT i = 0u; i < Joystick::MaxCount; ++i)
    {
        auto& joyImpl = m_impl->impls[i];

        if (!joyImpl.hasValue())
            continue; // Empty slot

        if (base::anyOf(connectedJoystickInfos,
                        connectedJoystickInfos + nextInfoIdx,
                        [&](const base::Optional<JoystickInfo>& info)
        {
            SFML_BASE_ASSERT(info.hasValue());
            return info->id == joyImpl->id;
        }))
        {
            continue; // Not newly disconnected
        }

        SFML_BASE_ASSERT(joyImpl->handle != nullptr);
        SDL_CloseJoystick(joyImpl->handle);

        SFML_BASE_ASSERT(joyImpl.hasValue());
        joyImpl.reset();

        m_impl->capabilities[i]    = {};
        m_impl->states[i]          = {};
        m_impl->identifications[i] = {};
    }

    ////////////////////////////////////////////////////////////
    // Find newly connected joysticks
    for (base::SizeT iInfo = 0u; iInfo < nextInfoIdx; ++iInfo)
    {
        SFML_BASE_ASSERT(connectedJoystickInfos[iInfo].hasValue());
        const auto& [id, guid] = *connectedJoystickInfos[iInfo];

        if (base::anyOf(m_impl->impls,
                        m_impl->impls + Joystick::MaxCount,
                        [&](const base::Optional<JoystickImpl>& impl) { return impl.hasValue() && impl->id == id; }))
            continue; // Not newly connected

        // Find an empty slot
        for (base::SizeT iImpl = 0u; iImpl < Joystick::MaxCount; ++iImpl)
        {
            auto& joyImpl = m_impl->impls[iImpl];

            if (joyImpl.hasValue())
                continue; // Used slot

            joyImpl.emplace();
            joyImpl->id     = id;
            joyImpl->guid   = guid;
            joyImpl->handle = SDL_OpenJoystick(id);

            if (joyImpl->handle == nullptr)
            {
                priv::err() << "Failed to open joystick: " << SDL_GetError();
                joyImpl.reset();
            }
            else
            {
                m_impl->capabilities[iImpl]    = getCapabilitiesFromSDL(*joyImpl->handle);
                m_impl->identifications[iImpl] = getIdentificationFromSDL(*joyImpl->handle);
            }

            break;
        }
    }

    ////////////////////////////////////////////////////////////
    // Update all joysticks
    for (unsigned int i = 0; i < Joystick::MaxCount; ++i)
    {
        auto& [impls, states, capabilities, identifications] = *m_impl;

        if (!impls[i].hasValue())
            continue;

        SFML_BASE_ASSERT(impls[i]->handle != nullptr);
        states[i] = getStateFromSDL(*impls[i]->handle);
    }
}


////////////////////////////////////////////////////////////
JoystickManager::JoystickManager()
{
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_SetJoystickEventsEnabled(false);
}


////////////////////////////////////////////////////////////
JoystickManager::~JoystickManager()
{
    for (auto& joyImpl : m_impl->impls)
        if (joyImpl.hasValue())
            SDL_CloseJoystick(joyImpl->handle);

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
}

} // namespace sf::priv
