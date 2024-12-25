#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/WindowBase.hpp" // NOLINT(misc-header-include-cycle)


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct DelayOverloadResolution
{
    template <typename T>
    explicit(false) DelayOverloadResolution(const T&)
    {
    }

#if defined(_MSC_VER) && !defined(__clang__)
    unsigned char dummy; // Dummy variable to ensure that this struct is not empty thus avoiding a crash due to an MSVC bug
#endif
};

} // namespace sf::priv

namespace sf
{
////////////////////////////////////////////////////////////
template <typename... Handlers, auto PollEventFn> // Using this weird `auto` parameter to avoid dependency on event header
void WindowBase::pollAndHandleEvents(Handlers&&... handlers)
{
    static_assert(sizeof...(Handlers) > 0, "Must provide at least one handler");

    while (const base::Optional event = (this->*PollEventFn)())
        event->match(static_cast<Handlers&&>(handlers)..., [](const priv::DelayOverloadResolution&) { /* ignore */ });
}

} // namespace sf
