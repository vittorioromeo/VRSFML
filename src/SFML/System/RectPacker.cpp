#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"
#include "SFML/System/RectPacker.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct RectPacker::Impl
{
    enum : int
    {
        MaxNodes = 2048
    };

    stbrp_node    nodes[MaxNodes]{};
    stbrp_context context{};

    explicit Impl(const Vec2u size)
    {
        stbrp_init_target(&context, static_cast<int>(size.x), static_cast<int>(size.y), nodes, MaxNodes);
    }
};


////////////////////////////////////////////////////////////
RectPacker::RectPacker(const Vec2u size) : m_impl(size)
{
}


////////////////////////////////////////////////////////////
RectPacker::~RectPacker() = default;


////////////////////////////////////////////////////////////
base::Optional<Vec2u> RectPacker::pack(const Vec2u rectSize)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failure packing rectangle with size {" << rectSize.x << ", " << rectSize.y << "}: " << what;
        return base::nullOpt;
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

    SFML_BASE_ASSERT(rc == /* success */ 1);
    SFML_BASE_ASSERT(toPack.was_packed != 0);

    return base::makeOptional<Vec2u>(static_cast<unsigned int>(toPack.x), static_cast<unsigned int>(toPack.y));
}


////////////////////////////////////////////////////////////
bool RectPacker::packMultiple(const base::Span<Vec2u> outPositions, const base::Span<const Vec2u> rectSizes)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failure packing multiple rectangles: " << what;
        return false;
    };

    if (outPositions.size() != rectSizes.size())
        return fail("mismatched output and input sizes");

    if (outPositions.size() > 512u)
        return fail("too many rectangles to pack (max 512)");

    stbrp_rect toPack[512];

    for (base::SizeT i = 0u; i < rectSizes.size(); ++i)
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

    SFML_BASE_ASSERT(rc == /* success */ 1);

    for (base::SizeT i = 0u; i < rectSizes.size(); ++i)
    {
        const auto& packed = toPack[i];
        SFML_BASE_ASSERT(packed.was_packed);

        outPositions[i] = {static_cast<unsigned int>(packed.x), static_cast<unsigned int>(packed.y)};
    }

    return true;
}


////////////////////////////////////////////////////////////
Vec2u RectPacker::getSize() const
{
    return {static_cast<unsigned int>(m_impl->context.width), static_cast<unsigned int>(m_impl->context.height)};
}

} // namespace sf
