#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


namespace sf
{
////////////////////////////////////////////////////////////
/// \ingroup window
/// \brief Enumeration of display orientations
///
////////////////////////////////////////////////////////////
enum class [[nodiscard]] DisplayOrientation : unsigned char
{
    Unknown          = 0u, //!< Orientation can't be determined
    Landscape        = 1u, //!< Landscape mode, with the right side up, relative to portrait mode
    LandscapeFlipped = 2u, //!< Landscape mode, with the left side up, relative to portrait mode
    Portrait         = 3u, //!< Portrait mode
    PortraitFlipped  = 4u, //!< Portrait mode, upside down
};

////////////////////////////////////////////////////////////
enum : unsigned int
{
    DisplayOrientationCount = static_cast<unsigned int>(DisplayOrientation::PortraitFlipped) +
                              1u //!< Total number of possible display orientations
};

} // namespace sf
