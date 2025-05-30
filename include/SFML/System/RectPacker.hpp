#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Span.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Packs 2D rectangles into a larger texture area efficiently.
///
/// This class implements an algorithm to find positions for smaller
/// rectangles within a larger, fixed-size area (bin). It's commonly used
/// for creating texture atlases, where multiple smaller images are packed
/// into a single larger texture to optimize rendering.
///
/// The internals of `RectPacker` require address stability, so
/// it is a non-copyable and non-movable class.
///
////////////////////////////////////////////////////////////
class RectPacker
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Construct a rectangle packer for a given bin size.
    ///
    /// \param size The dimensions (width, height) of the area to pack rectangles into.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] explicit RectPacker(Vec2u size);

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~RectPacker();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    RectPacker(const RectPacker& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment operator
    ///
    ////////////////////////////////////////////////////////////
    RectPacker& operator=(const RectPacker& rhs) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move constructor
    ///
    ////////////////////////////////////////////////////////////
    RectPacker(RectPacker&& rhs) noexcept = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    RectPacker& operator=(RectPacker&& rhs) noexcept = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Attempt to pack a rectangle of the given size.
    ///
    /// Tries to find an available space within the bin for a rectangle
    /// with the specified dimensions.
    ///
    /// \param rectSize The size (width, height) of the rectangle to pack.
    ///                 Both dimensions must be greater than zero.
    ///
    /// \return An `Optional<Vec2u>` containing the top-left position
    ///         where the rectangle was packed if successful, or `nullOpt`
    ///         if there was no room left in the bin.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Vec2u> pack(Vec2u rectSize);

    ////////////////////////////////////////////////////////////
    /// \brief ATtempt to pack multiple rectangles at once.
    ///
    /// This function tries to pack multiple rectangles into the bin
    /// in a single operation. The positions of the packed rectangles
    /// are returned in the `outPositions` span, which must have the
    /// same size as the `rectSizes` span.
    ///
    /// \param outPositions A span to fill with the top-left positions
    ///                     of the packed rectangles. Must be large enough
    ///                     to hold all rectangles specified in `rectSizes`.
    ///
    /// \param rectSizes A span containing the sizes of the rectangles to pack.
    ///                  Each rectangle size must have both dimensions greater than zero.
    ///
    /// \return `true` if all rectangles were packed successfully, `false` otherwise.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool packMultiple(base::Span<Vec2u> outPositions, base::Span<const Vec2u> rectSizes);

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the packing area (the bin).
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vec2u getSize() const;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 36'864> m_impl; //!< Implementation details
};

} // namespace sf


////////////////////////////////////////////////////////////
/// \class sf::RectPacker
/// \ingroup system
///
/// `sf::RectPacker` helps arrange smaller rectangular areas within a larger
/// one without overlapping. This is useful for tasks like creating texture atlases,
/// where you want to combine multiple small images into a single larger texture
/// to improve performance.
///
/// You create a `RectPacker` with the dimensions of the target area (the "bin").
/// Then, you repeatedly call `pack()` with the dimensions of the rectangles you
/// want to place. `pack()` returns the top-left position within the bin where
/// the rectangle was placed, or `nullOpt` if it couldn't fit.
///
/// \see sf::Rect
///
////////////////////////////////////////////////////////////
