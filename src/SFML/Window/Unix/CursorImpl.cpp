#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Window/Unix/CursorImpl.hpp"
#include "SFML/Window/Unix/Display.hpp"
#include "SFML/Window/Unix/Utils.hpp"

#include "SFML/Base/Assert.hpp"

#include <X11/Xcursor/Xcursor.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <vector>

#include <cstdlib>

namespace sf::priv
{
template <>
struct XDeleter<XcursorImage>
{
    void operator()(XcursorImage* cursorImage) const
    {
        if (cursorImage != nullptr)
            XcursorImageDestroy(cursorImage);
    }
};


////////////////////////////////////////////////////////////
CursorImpl::CursorImpl() : m_display(openDisplay())
{
    SFML_BASE_ASSERT(m_display != nullptr);
}


////////////////////////////////////////////////////////////
CursorImpl::~CursorImpl()
{
    release();
}


////////////////////////////////////////////////////////////
bool CursorImpl::loadFromPixels(const base::U8* pixels, Vector2u size, Vector2u hotspot)
{
    release();

    if (isColorCursorSupported())
        return loadFromPixelsARGB(pixels, size, hotspot);

    return loadFromPixelsMonochrome(pixels, size, hotspot);
}


////////////////////////////////////////////////////////////
bool CursorImpl::loadFromPixelsARGB(const base::U8* pixels, Vector2u size, Vector2u hotspot)
{
    // Create cursor image, convert from RGBA to ARGB.
    const auto cursorImage = X11Ptr<XcursorImage>(XcursorImageCreate(static_cast<int>(size.x), static_cast<int>(size.y)));
    cursorImage->xhot = hotspot.x;
    cursorImage->yhot = hotspot.y;

    const base::SizeT numPixels = static_cast<base::SizeT>(size.x) * static_cast<base::SizeT>(size.y);
    for (base::SizeT pixelIndex = 0; pixelIndex < numPixels; ++pixelIndex)
    {
        cursorImage->pixels[pixelIndex] = static_cast<base::U32>(
            pixels[pixelIndex * 4 + 2] + (pixels[pixelIndex * 4 + 1] << 8) + (pixels[pixelIndex * 4 + 0] << 16) +
            (pixels[pixelIndex * 4 + 3] << 24));
    }

    // Create the cursor.
    m_cursor = XcursorImageLoadCursor(m_display.get(), cursorImage.get());

    // We assume everything went fine...
    return true;
}


////////////////////////////////////////////////////////////
bool CursorImpl::loadFromPixelsMonochrome(const base::U8* pixels, Vector2u size, Vector2u hotspot)
{
    // Convert the image into a bitmap (monochrome!).
    // The bit data is stored packed into bytes. If the number of pixels on each row of the image
    // does not fit exactly into (width/8) bytes, one extra byte is allocated at the end of each
    // row to store the extra pixels.
    const base::SizeT     packedWidth = (size.x + 7) / 8;
    const base::SizeT     bytes       = packedWidth * size.y;
    std::vector<base::U8> mask(bytes, 0); // Defines which pixel is opaque (1) or transparent (0).
    std::vector<base::U8> data(bytes, 0); // Defines which pixel is white (1) or black (0).

    for (base::SizeT j = 0; j < size.y; ++j)
    {
        for (base::SizeT i = 0; i < size.x; ++i)
        {
            const base::SizeT pixelIndex = i + j * size.x;
            const base::SizeT byteIndex  = i / 8 + j * packedWidth;
            const base::SizeT bitIndex   = i % 8;

            // Turn on pixel that are not transparent
            const base::U8 opacity = pixels[pixelIndex * 4 + 3] > 0;
            mask[byteIndex] |= static_cast<base::U8>(opacity << bitIndex);

            // Choose between black/background & white/foreground color for each pixel,
            // based on the pixel color intensity: on average, if a channel is "active"
            // at 50%, the bit is white.
            const int intensity = (pixels[pixelIndex * 4 + 0] + pixels[pixelIndex * 4 + 1] + pixels[pixelIndex * 4 + 2]) / 3;
            const base::U8 bit = intensity > 128;
            data[byteIndex] |= static_cast<base::U8>(bit << bitIndex);
        }
    }

    const Pixmap maskPixmap = XCreateBitmapFromData(m_display.get(),
                                                    XDefaultRootWindow(m_display.get()),
                                                    reinterpret_cast<char*>(mask.data()),
                                                    size.x,
                                                    size.y);
    const Pixmap dataPixmap = XCreateBitmapFromData(m_display.get(),
                                                    XDefaultRootWindow(m_display.get()),
                                                    reinterpret_cast<char*>(data.data()),
                                                    size.x,
                                                    size.y);

    // Define the foreground color as white and the background as black.
    XColor fg;
    XColor bg;
    fg.red   = 0xFFFF;
    fg.blue  = 0xFFFF;
    fg.green = 0xFFFF;
    bg.red   = 0x0000;
    bg.blue  = 0x0000;
    bg.green = 0x0000;

    // Create the monochrome cursor.
    m_cursor = XCreatePixmapCursor(m_display.get(), dataPixmap, maskPixmap, &fg, &bg, hotspot.x, hotspot.y);

    // Free the resources
    XFreePixmap(m_display.get(), dataPixmap);
    XFreePixmap(m_display.get(), maskPixmap);

    // We assume everything went fine...
    return true;
}


////////////////////////////////////////////////////////////
bool CursorImpl::loadFromSystem(Cursor::Type type)
{
    release();

    unsigned int shape = 0;

    // clang-format off
    switch (type)
    {
        default: return false;

        case Cursor::Type::Arrow:           shape = XC_arrow;               break;
        case Cursor::Type::Wait:            shape = XC_watch;               break;
        case Cursor::Type::Text:            shape = XC_xterm;               break;
        case Cursor::Type::Hand:            shape = XC_hand2;               break;
        case Cursor::Type::SizeHorizontal:  shape = XC_sb_h_double_arrow;   break;
        case Cursor::Type::SizeVertical:    shape = XC_sb_v_double_arrow;   break;
        case Cursor::Type::SizeLeft:        shape = XC_left_side;           break;
        case Cursor::Type::SizeRight:       shape = XC_right_side;          break;
        case Cursor::Type::SizeTop:         shape = XC_top_side;            break;
        case Cursor::Type::SizeBottom:      shape = XC_bottom_side;         break;
        case Cursor::Type::SizeTopLeft:     shape = XC_top_left_corner;     break;
        case Cursor::Type::SizeBottomRight: shape = XC_bottom_right_corner; break;
        case Cursor::Type::SizeBottomLeft:  shape = XC_bottom_left_corner;  break;
        case Cursor::Type::SizeTopRight:    shape = XC_top_right_corner;    break;
        case Cursor::Type::SizeAll:         shape = XC_fleur;               break;
        case Cursor::Type::Cross:           shape = XC_crosshair;           break;
        case Cursor::Type::Help:            shape = XC_question_arrow;      break;
        case Cursor::Type::NotAllowed:      shape = XC_X_cursor;            break;
    }
    // clang-format on

    m_cursor = XCreateFontCursor(m_display.get(), shape);
    return true;
}


////////////////////////////////////////////////////////////
bool CursorImpl::isColorCursorSupported()
{
    return XcursorSupportsARGB(m_display.get());
}


////////////////////////////////////////////////////////////
void CursorImpl::release()
{
    if (m_cursor == None || m_display == nullptr)
        return;

    XFreeCursor(m_display.get(), m_cursor);
    m_cursor = None;
}


} // namespace sf::priv
