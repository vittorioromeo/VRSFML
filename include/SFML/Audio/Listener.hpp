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
/// \brief Position, orientation and global volume of the audio "ear"
///
/// `Listener` is a plain data structure that describes where
/// and how sounds are heard for a given `sf::PlaybackDevice`.
/// It is the audio counterpart of a camera/view: position,
/// forward direction, up vector, velocity (for Doppler), an
/// optional directional cone, and a global volume.
///
/// `Listener` itself does not push anything to the audio
/// engine. To make changes effective, the listener must be
/// applied to a playback device through
/// `sf::PlaybackDevice::applyListener`.
///
/// \see `sf::PlaybackDevice`, `sf::Sound`, `sf::Music`
///
////////////////////////////////////////////////////////////
struct SFML_AUDIO_API Listener
{
    ////////////////////////////////////////////////////////////
    /// \brief Structure defining the properties of a directional cone
    ///
    /// Sounds will play at gain 1 when the listener is
    /// positioned within the inner angle of the cone. Sounds
    /// will play at `outerGain` when the listener is positioned
    /// outside the outer angle of the cone. The gain declines
    /// linearly from 1 to `outerGain` as the listener moves
    /// from the inner angle to the outer angle.
    ///
    ////////////////////////////////////////////////////////////
    struct [[nodiscard]] Cone
    {
        Angle innerAngle;  //!< Inner cone half-angle (full gain inside)
        Angle outerAngle;  //!< Outer cone half-angle (gain reaches `outerGain` here)
        float outerGain{}; //!< Gain applied outside the outer angle (in `[0, 1]`)

        ////////////////////////////////////////////////////////////
        /// \brief Default member-wise equality
        ///
        ////////////////////////////////////////////////////////////
        [[nodiscard]] bool operator==(const Cone&) const noexcept = default;
    };

    float volume{1.f};         //!< Global volume multiplier in `[0, 1]` applied to every sound source
    Vec3f position{0, 0, 0};   //!< Position from which audio is heard
    Vec3f direction{0, 0, -1}; //!< Forward-pointing direction (not necessarily normalized)
    Vec3f velocity{0, 0, 0};   //!< Velocity of the listener (used for Doppler shift)
    Listener::Cone cone{Angle::Full, Angle::Full, 1}; //!< Listener directional cone (full sphere = no directional attenuation)
    Vec3f upVec{0, 1, 0};                             //!< Up vector (not necessarily normalized)

    ////////////////////////////////////////////////////////////
    /// \brief Default member-wise equality
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool operator==(const Listener&) const noexcept = default;
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \struct sf::Listener
/// \ingroup audio
///
/// The audio listener defines the properties of the audio
/// environment for a particular playback device. It defines
/// where and how sounds/musics are heard. If a `sf::View` is
/// the eyes of the user, then `sf::Listener` is their ears
/// (the two are often linked together with the same position
/// and orientation).
///
/// `sf::Listener` is a simple aggregate that you mutate
/// directly and then apply to a playback device via
/// `sf::PlaybackDevice::applyListener`. Each playback device
/// keeps its own listener state.
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
/// (void)somePlaybackDevice.applyListener(listener);
/// \endcode
///
////////////////////////////////////////////////////////////
