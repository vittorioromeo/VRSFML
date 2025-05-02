#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Automatic wrapper for saving and restoring the current framebuffer binding
///
////////////////////////////////////////////////////////////
class FramebufferSaver
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// The current framebuffer binding is saved.
    ///
    ////////////////////////////////////////////////////////////
    FramebufferSaver();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    /// The previous framebuffer binding is restored.
    ///
    ////////////////////////////////////////////////////////////
    ~FramebufferSaver();

    ////////////////////////////////////////////////////////////
    FramebufferSaver(const FramebufferSaver&)            = delete;
    FramebufferSaver& operator=(const FramebufferSaver&) = delete;

    ////////////////////////////////////////////////////////////
    FramebufferSaver(FramebufferSaver&&)            = delete;
    FramebufferSaver& operator=(FramebufferSaver&&) = delete;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    int m_drawFramebufferBinding; //!< Draw framebuffer binding to restore
    int m_readFramebufferBinding; //!< Read framebuffer binding to restore
};

} // namespace sf::priv
