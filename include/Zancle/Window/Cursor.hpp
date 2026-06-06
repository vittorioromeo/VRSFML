#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Export.hpp"

#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Vocabulary/PassKey.hpp"

#include "Zancle/Base/IntTypes.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
struct SDL_Cursor;

namespace za
{
template <typename>
struct Vec2;
} // namespace za


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Cursor defines the appearance of a system cursor
///
////////////////////////////////////////////////////////////
class ZA_WINDOW_API Cursor
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Enumeration of the native system cursor types
    ///
    /// Refer to the following table to determine which cursor
    /// is available on which platform.
    ///
    ///  Type                                       | Linux | macOS | Windows  |
    /// --------------------------------------------|:-----:|:-----:|:--------:|
    ///  `za::Cursor::Type::Arrow`                  |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::ArrowWait`              |  no   | no    |   yes    |
    ///  `za::Cursor::Type::Wait`                   |  yes  | no    |   yes    |
    ///  `za::Cursor::Type::Text`                   |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::Hand`                   |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::SizeHorizontal`         |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::SizeVertical`           |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::SizeTopLeftBottomRight` |  no   | yes*  |   yes    |
    ///  `za::Cursor::Type::SizeBottomLeftTopRight` |  no   | yes*  |   yes    |
    ///  `za::Cursor::Type::SizeLeft`               |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeRight`              |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeTop`                |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeBottom`             |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeTopLeft`            |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeTopRight`           |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeBottomLeft`         |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeBottomRight`        |  yes  | yes** |   yes**  |
    ///  `za::Cursor::Type::SizeAll`                |  yes  | no    |   yes    |
    ///  `za::Cursor::Type::Cross`                  |  yes  | yes   |   yes    |
    ///  `za::Cursor::Type::Help`                   |  yes  | yes*  |   yes    |
    ///  `za::Cursor::Type::NotAllowed`             |  yes  | yes   |   yes    |
    ///
    ///  * These cursor types are undocumented so may not
    ///    be available on all versions, but have been tested on 10.13
    ///
    ///  ** On Windows and macOS, double-headed arrows are used
    ///
    ////////////////////////////////////////////////////////////
    enum class Type
    {
        Arrow,                  //!< Arrow cursor (default)
        ArrowWait,              //!< Busy arrow cursor
        Wait,                   //!< Busy cursor
        Text,                   //!< I-beam, cursor when hovering over a field allowing text entry
        Hand,                   //!< Pointing hand cursor
        SizeHorizontal,         //!< Horizontal double arrow cursor
        SizeVertical,           //!< Vertical double arrow cursor
        SizeTopLeftBottomRight, //!< Double arrow cursor going from top-left to bottom-right
        SizeBottomLeftTopRight, //!< Double arrow cursor going from bottom-left to top-right
        SizeLeft,               //!< Left arrow cursor on Linux, same as SizeHorizontal on other platforms
        SizeRight,              //!< Right arrow cursor on Linux, same as SizeHorizontal on other platforms
        SizeTop,                //!< Up arrow cursor on Linux, same as SizeVertical on other platforms
        SizeBottom,             //!< Down arrow cursor on Linux, same as SizeVertical on other platforms
        SizeTopLeft,            //!< Top-left arrow cursor on Linux, same as SizeTopLeftBottomRight on other platforms
        SizeBottomRight, //!< Bottom-right arrow cursor on Linux, same as SizeTopLeftBottomRight on other platforms
        SizeBottomLeft,  //!< Bottom-left arrow cursor on Linux, same as SizeBottomLeftTopRight on other platforms
        SizeTopRight,    //!< Top-right arrow cursor on Linux, same as SizeBottomLeftTopRight on other platforms
        SizeAll,         //!< Combination of SizeHorizontal and SizeVertical
        Cross,           //!< Crosshair cursor
        Help,            //!< Help cursor
        NotAllowed       //!< Action not allowed cursor
    };

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~Cursor();

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Cursor(const Cursor&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Cursor& operator=(const Cursor&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Cursor(Cursor&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Cursor& operator=(Cursor&&) noexcept;

    ////////////////////////////////////////////////////////////
    /// \brief Create a cursor with the provided image
    ///
    /// `pixels` must be an array of `size` pixels
    /// in 32-bit RGBA format. If not, this will cause undefined behavior.
    ///
    /// If `pixels` is `nullptr` or either of `size`'s
    /// properties are 0, the current cursor is left unchanged
    /// and the function will return `false`.
    ///
    /// In addition to specifying the pixel data, you can also
    /// specify the location of the hotspot of the cursor. The
    /// hotspot is the pixel coordinate within the cursor image
    /// which will be located exactly where the mouse pointer
    /// position is. Any mouse actions that are performed will
    /// return the window/screen location of the hotspot.
    ///
    /// \warning On Unix platforms which do not support colored
    ///          cursors, the pixels are mapped into a monochrome
    ///          bitmap: pixels with an alpha channel to 0 are
    ///          transparent, black if the RGB channel are close
    ///          to zero, and white otherwise.
    ///
    /// \param pixels   Array of pixels of the image
    /// \param size     Width and height of the image
    /// \param hotspot  (x,y) location of the hotspot
    /// \return Cursor if the cursor was successfully loaded;
    ///         `za::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::Optional<Cursor> loadFromPixels(const za::U8*      pixels,
                                                             Vec2<unsigned int> size,
                                                             Vec2<unsigned int> hotspot);

    ////////////////////////////////////////////////////////////
    /// \brief Create a native system cursor
    ///
    /// Refer to the list of cursor available on each system
    /// (see `za::Cursor::Type`) to know whether a given cursor is
    /// expected to load successfully or is not supported by
    /// the operating system.
    ///
    /// \param type Native system cursor type
    /// \return Cursor if and only if the corresponding cursor is
    ///         natively supported by the operating system;
    ///         `za::nullOpt` otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static za::Optional<Cursor> loadFromSystem(Type type);

private:
    friend class WindowBase;

public:
    ////////////////////////////////////////////////////////////
    /// \private
    ///
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Cursor(za::PassKey<Cursor>&&);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Get access to the underlying implementation
    ///
    /// This is primarily designed for `za::WindowBase::setMouseCursor`,
    /// hence the friendship.
    ///
    /// \return a reference to the OS-specific implementation
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] void* getImpl() const;

    ////////////////////////////////////////////////////////////
    /// \brief Re-create the underlying SDL cursor from a custom image
    ///
    /// Internal helper used by the `loadFromPixels` factory.
    /// Replaces the previously held SDL cursor (if any) with a
    /// new one built from the provided image.
    ///
    /// \param pixels  Pointer to the array of 32-bit RGBA pixels
    /// \param size    Width and height of the image
    /// \param hotspot Hotspot position within the image
    ///
    /// \return `true` on success, `false` on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool reloadFromPixels(const za::U8* pixels, Vec2<unsigned int> size, Vec2<unsigned int> hotspot);

    ////////////////////////////////////////////////////////////
    /// \brief Re-create the underlying SDL cursor from a system cursor
    ///
    /// Internal helper used by the `loadFromSystem` factory.
    /// Replaces the previously held SDL cursor (if any) with a
    /// new one built from the requested system cursor type.
    ///
    /// \param type Native system cursor type
    ///
    /// \return `true` on success, `false` if the cursor type is
    ///         not supported on the current platform
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool reloadFromSystem(Cursor::Type type);

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    SDL_Cursor* m_sdlCursor;
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::Cursor
/// \ingroup window
///
/// \warning Features related to Cursor are not supported on
///          iOS and Android.
///
/// This class abstracts the operating system resources
/// associated with either a native system cursor or a custom
/// cursor.
///
/// After loading the cursor graphical appearance
/// with either `loadFromPixels()` or `loadFromSystem()`, the
/// cursor can be changed with `za::WindowBase::setMouseCursor()`.
///
/// The behavior is undefined if the cursor is destroyed while
/// in use by the window.
///
/// Usage example:
/// \code
/// za::Window window;
///
/// // ... create window as usual ...
///
/// const auto cursor = za::Cursor::loadFromSystem(za::Cursor::Type::Hand).value();
/// window.setMouseCursor(cursor);
/// \endcode
///
/// \see `za::WindowBase::setMouseCursor`
///
////////////////////////////////////////////////////////////
