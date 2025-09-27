#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/Export.hpp"

#include "SFML/System/Angle.hpp"
#include "SFML/System/Vec3.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief The audio listener is the point in the scene
///        from where all the sounds are heard for a given
///        playback device
///
////////////////////////////////////////////////////////////
struct SFML_AUDIO_API Listener
{
    ////////////////////////////////////////////////////////////
    /// \brief Structure defining the properties of a directional cone
    ///
    /// Sounds will play at gain 1 when they are positioned
    /// within the inner angle of the cone. Sounds will play
    /// at outerGain when they are positioned outside the
    /// outer angle of the cone. The gain declines linearly
    /// from 1 to outerGain as the sound moves from the inner
    /// angle to the outer angle.
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Cone
    {
        Angle innerAngle;  //!< Inner angle
        Angle outerAngle;  //!< Outer angle
        float outerGain{}; //!< Outer gain
    };

    float volume{1.f};         //!< Volume (between `0` and `1`)
    Vec3f position{0, 0, 0};   //!< Position where audio is heard from
    Vec3f direction{0, 0, -1}; //!< Forward-pointing vec3 from listener's perspective (not necessarily normalized)
    Vec3f velocity{0, 0, 0};   //!< Velocity of the listener
    Listener::Cone cone{Angle::Full, Angle::Full, 1}; //!< Cone properties (affects directional attenuation)
    Vec3f upVec{0, 1, 0}; //!< Upward-pointing vec3 from the listener's perspective (not necessarily normalized)
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \namespace sf::Listener
/// \ingroup audio
///
/// The audio listener defines the properties of the audio environment,
/// for a particular playback device. It defines where/how sounds/musics
/// are heard. If `sf::View` is the eyes of the user, then `sf::Listener`
/// is their ears (by the way, they are often linked together --
/// same position, orientation, etc.).
///
/// `sf::Listener` is a simple interface, which allows to setup the
/// listener in the 3D audio environment (position, direction and
/// up vec3), and to adjust the global volume.
///
/// Usage example:
/// \code
/// sf::Listener listener;
///
/// // Move the listener to the position (1, 0, -5)
/// listener.position = {1, 0, -5};
///
/// // Make it face the right axis (1, 0, 0)
/// listener.direction = {1, 0, 0};
///
/// // Reduce the global volume
/// listener.volume = 0.5f; // 50%
///
/// // Apply the listener properties
/// somePlaybackDevice.updateListener(listener);
/// \endcode
///
////////////////////////////////////////////////////////////
