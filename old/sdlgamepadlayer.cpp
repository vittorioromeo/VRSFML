

////////////////////////////////////////////////////////////
struct SFML_BASE_TRIVIAL_ABI UniquePtrSDLGamepadClose
{
    [[gnu::always_inline]] void operator()(SDL_Gamepad* const ptr) const noexcept
    {
        SDL_CloseGamepad(ptr);
    }
};




////////////////////////////////////////////////////////////
template <typename T>
using SDLGamepadUPtr = base::UniquePtr<T, UniquePtrSDLGamepadClose>;

    ////////////////////////////////////////////////////////////
    [[nodiscard]] SDLAllocatedArray<SDL_JoystickID> getGamepadIDs()
    {
        int             gamepadCount = 0;
        SDL_JoystickID* gamepads     = SDL_GetGamepads(&gamepadCount);

        if (gamepads == nullptr)
        {
            err() << "`SDL_GetGamepads` failed: " << SDL_GetError();
            return nullptr;
        }

        return SDLAllocatedArray<SDL_JoystickID>{SDLUPtr<SDL_JoystickID>{gamepads}, static_cast<base::SizeT>(gamepadCount)};
    }
