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

////////////////////////////////////////////////////////////
template <typename T>
auto&& functionPointerToFunctionObject(T&& f)
{
    return static_cast<T&&>(f);
}

template <typename R, typename... Args>
auto functionPointerToFunctionObject(R (*f)(Args...))
{
    return [f](Args... args) -> R { return f(args...); };
}

} // namespace sf::priv

namespace sf
{
////////////////////////////////////////////////////////////
template <typename... Handlers, auto PollEventFn> // Using this weird `auto` parameter to avoid dependency on event header
void WindowBase::pollAndHandleEvents(Handlers&&... handlers)
{
    static_assert(sizeof...(Handlers) > 0, "Must provide at least one handler");

    auto visitor = sf::base::OverloadSet{priv::functionPointerToFunctionObject(static_cast<Handlers&&>(handlers))...,
                                         [](const priv::DelayOverloadResolution&) { /* ignore */ }};

    // Disable misc-const-correctness for this line since clang-tidy
    // complains about it even though the code would become incorrect

    // NOLINTNEXTLINE(misc-const-correctness)
    while (base::Optional event = (this->*PollEventFn)())
        event->visit(visitor);
}

} // namespace sf
