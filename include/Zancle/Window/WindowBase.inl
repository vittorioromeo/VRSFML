// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/WindowBase.hpp" // NOLINT(misc-header-include-cycle)
#include "ZancleBase/OverloadSet.hpp"


namespace za::priv
{
////////////////////////////////////////////////////////////
struct DelayOverloadResolution
{
    template <typename T>
    /* implicit */ DelayOverloadResolution(const T&)
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

} // namespace za::priv


namespace za
{
////////////////////////////////////////////////////////////
template <typename... Handlers, auto PollEventFn> // Using this weird `auto` parameter to avoid dependency on event header
void WindowBase::pollAndHandleEvents(Handlers&&... handlers)
{
    static_assert(sizeof...(Handlers) > 0, "Must provide at least one handler");

    auto visitor = zb::OverloadSet{priv::functionPointerToFunctionObject(static_cast<Handlers&&>(handlers))...,
                                   [](const priv::DelayOverloadResolution&) { /* ignore */ }};

    // Disable misc-const-correctness for this line since clang-tidy
    // complains about it even though the code would become incorrect

    // NOLINTNEXTLINE(misc-const-correctness)
    while (zb::Optional event = (this->*PollEventFn)())
        event->visit(visitor);
}

} // namespace za
