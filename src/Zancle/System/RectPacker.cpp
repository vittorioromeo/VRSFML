// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/System/RectPacker.hpp"

#include "Zancle/System/Err.hpp"
#include "Zancle/System/Priv/Vec2Base.hpp"

#include "ZancleBase/Assert.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Span.hpp"
#include "ZancleBase/Vector.hpp"

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>


namespace za
{
////////////////////////////////////////////////////////////
struct RectPacker::Impl
{
    zb::Vector<stbrp_node> nodes;
    stbrp_context            context{};

    explicit Impl(const Vec2u size) : nodes(size.x)
    {
        stbrp_init_target(&context,
                          static_cast<int>(size.x),
                          static_cast<int>(size.y),
                          nodes.data(),
                          static_cast<int>(nodes.size()));
    }
};


////////////////////////////////////////////////////////////
RectPacker::RectPacker(const Vec2u size) : m_impl(size)
{
}


////////////////////////////////////////////////////////////
RectPacker::~RectPacker() = default;


////////////////////////////////////////////////////////////
zb::Optional<Vec2u> RectPacker::pack(const Vec2u rectSize)
{
    const auto fail = [&](const char* what)
    {
        priv::errMsg("Failure packing rectangle with size {{{}, {}}}: {}", rectSize.x, rectSize.y, what);
        return zb::nullOpt;
    };

    if (rectSize.x == 0u || rectSize.y == 0u)
        return fail("zero-sized coordinate");

    stbrp_rect toPack{/* id */ 0,
                      /* input width */ static_cast<int>(rectSize.x),
                      /* input height */ static_cast<int>(rectSize.y),
                      /* output x */ {},
                      /* output y */ {},
                      /* was_packed */ {}};

    const int rc = stbrp_pack_rects(&m_impl->context, &toPack, /* num_rects */ 1);

    if (rc == /* failure */ 0)
        return fail("no room to pack");

    ZB_ASSERT(rc == /* success */ 1);
    ZB_ASSERT(toPack.was_packed != 0);

    return zb::makeOptional<Vec2u>(static_cast<unsigned int>(toPack.x), static_cast<unsigned int>(toPack.y));
}


////////////////////////////////////////////////////////////
bool RectPacker::packMultiple(const zb::Span<Vec2u> outPositions, const zb::Span<const Vec2u> rectSizes)
{
    const auto fail = [&](const char* what)
    {
        priv::errMsg("Failure packing multiple rectangles: {}", what);
        return false;
    };

    if (outPositions.size() != rectSizes.size())
        return fail("mismatched output and input sizes");

    if (outPositions.size() > 512u)
        return fail("too many rectangles to pack (max 512)");

    stbrp_rect toPack[512];

    for (zb::SizeT i = 0u; i < rectSizes.size(); ++i)
    {
        const auto& size = rectSizes[i];

        if (size.x == 0u || size.y == 0u)
            return fail("zero-sized input rect size");

        toPack[i] = {/* id */ static_cast<int>(i),
                     /* input width */ static_cast<int>(size.x),
                     /* input height */ static_cast<int>(size.y),
                     /* output x */ {},
                     /* output y */ {},
                     /* was_packed */ {}};
    }

    const int rc = stbrp_pack_rects(&m_impl->context, toPack, /* num_rects */ static_cast<int>(rectSizes.size()));

    if (rc == /* failure */ 0)
        return fail("no room to pack");

    ZB_ASSERT(rc == /* success */ 1);

    for (zb::SizeT i = 0u; i < rectSizes.size(); ++i)
    {
        const auto& packed = toPack[i];
        ZB_ASSERT(packed.was_packed);

        outPositions[i] = {static_cast<unsigned int>(packed.x), static_cast<unsigned int>(packed.y)};
    }

    return true;
}


////////////////////////////////////////////////////////////
Vec2u RectPacker::getSize() const
{
    return {static_cast<unsigned int>(m_impl->context.width), static_cast<unsigned int>(m_impl->context.height)};
}

} // namespace za
