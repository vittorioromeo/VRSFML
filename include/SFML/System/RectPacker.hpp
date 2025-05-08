#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Vec2.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/UniquePtr.hpp"


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
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    RectPacker(RectPacker&& rhs) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment operator
    ///
    ////////////////////////////////////////////////////////////
    RectPacker& operator=(RectPacker&& rhs) noexcept;

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
    // TODO P1: add overload to pack multiple rects at once
    [[nodiscard]] base::Optional<Vec2u> pack(Vec2u rectSize);

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
    base::UniquePtr<Impl> m_impl; //!< Implementation details (needs address stability)
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
