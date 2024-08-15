#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Unix/Display.hpp>

#include <SFML/System/Err.hpp>

#include <SFML/Base/Assert.hpp>

#include <X11/keysym.h>

#include <mutex>
#include <unordered_map>

#include <clocale>
#include <cstdlib>


namespace
{
// A nested named namespace is used here to allow unity builds of SFML.
namespace UnixDisplayImpl
{
std::weak_ptr<Display> weakSharedDisplay;
std::recursive_mutex   mutex;
} // namespace UnixDisplayImpl
} // namespace


namespace sf::priv
{
////////////////////////////////////////////////////////////
std::shared_ptr<Display> openDisplay()
{
    const std::lock_guard lock(UnixDisplayImpl::mutex);

    auto sharedDisplay = UnixDisplayImpl::weakSharedDisplay.lock();
    if (!sharedDisplay)
    {
        sharedDisplay.reset(XOpenDisplay(nullptr), XCloseDisplay);
        UnixDisplayImpl::weakSharedDisplay = sharedDisplay;

        // Opening display failed: The best we can do at the moment is to output a meaningful error message
        // and cause an abnormal program termination
        if (!sharedDisplay)
        {
            priv::err() << "Failed to open X11 display; make sure the DISPLAY environment variable is set correctly";
            std::abort();
        }
    }

    return sharedDisplay;
}


////////////////////////////////////////////////////////////
std::shared_ptr<_XIM> openXim()
{
    const std::lock_guard lock(UnixDisplayImpl::mutex);

    SFML_BASE_ASSERT(!UnixDisplayImpl::weakSharedDisplay.expired() &&
                     "Display is not initialized. Call priv::openDisplay() to initialize it.");

    static std::weak_ptr<_XIM> xim;

    auto sharedXIM = xim.lock();
    if (!sharedXIM)
    {
        // Create a new XIM instance

        // We need the default (environment) locale and X locale for opening
        // the IM and properly receiving text
        // First save the previous ones (this might be able to be written more elegantly?)
        const char*       p = nullptr;
        const std::string prevLoc((p = std::setlocale(LC_ALL, nullptr)) ? p : "");
        const std::string prevXLoc((p = XSetLocaleModifiers(nullptr)) ? p : "");

        // Set the locales from environment
        std::setlocale(LC_ALL, "");
        XSetLocaleModifiers("");

        // Create the input context
        sharedXIM.reset(XOpenIM(UnixDisplayImpl::weakSharedDisplay.lock().get(), nullptr, nullptr, nullptr), XCloseIM);
        xim = sharedXIM;

        // Restore the previous locale
        if (!prevLoc.empty())
            std::setlocale(LC_ALL, prevLoc.c_str());

        if (!prevXLoc.empty())
            XSetLocaleModifiers(prevXLoc.c_str());
    }

    return sharedXIM;
}


////////////////////////////////////////////////////////////
Atom getAtom(const std::string& name, bool onlyIfExists)
{
    static std::unordered_map<std::string, Atom> atoms;

    if (const auto it = atoms.find(name); it != atoms.end())
        return it->second;

    const auto display = openDisplay();
    const Atom atom    = XInternAtom(display.get(), name.c_str(), onlyIfExists ? True : False);
    atoms[name]        = atom;

    return atom;
}

} // namespace sf::priv
