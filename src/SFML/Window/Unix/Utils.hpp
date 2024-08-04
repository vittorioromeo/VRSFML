#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/UniquePtr.hpp>

#include <X11/Xlib.h>

namespace sf::priv
{
template <typename T>
struct RemoveArrayExtentsImpl
{
    using type = T;
};

template <typename T>
struct RemoveArrayExtentsImpl<T[]>
{
    using type = T;
};

template <typename T>
using RemoveArrayExtents = typename RemoveArrayExtentsImpl<T>::type;

////////////////////////////////////////////////////////////
/// \brief Class template for freeing X11 pointers
///
/// Specialized elsewhere for types that are freed through
/// other means than XFree(). XFree() is the most common use
/// case though so it is the default.
///
////////////////////////////////////////////////////////////
template <typename T>
struct XDeleter
{
    void operator()(T* data) const
    {
        XFree(data); // NOLINT(bugprone-multi-level-implicit-pointer-conversion)
    }
};

////////////////////////////////////////////////////////////
/// \brief Class template for wrapping owning raw pointers from X11
///
////////////////////////////////////////////////////////////
template <typename T>
using X11Ptr = UniquePtr<RemoveArrayExtents<T>, XDeleter<RemoveArrayExtents<T>>>;
} // namespace sf::priv
