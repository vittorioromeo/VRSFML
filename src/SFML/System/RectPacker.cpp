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
enum : base::SizeT
{
    MaxNodes = 1024ul
};


////////////////////////////////////////////////////////////
struct RectPacker::Impl
{
    stbrp_node    nodes[MaxNodes]{};
    stbrp_context context{};
    base::SizeT   numPacked{0ul};

    explicit Impl(Vector2u size)
    {
        stbrp_init_target(&context, static_cast<int>(size.x), static_cast<int>(size.y), nodes, MaxNodes);
    }
};


////////////////////////////////////////////////////////////
RectPacker::RectPacker(Vector2u size) : m_impl(size)
{
}

////////////////////////////////////////////////////////////
RectPacker::~RectPacker() = default;


////////////////////////////////////////////////////////////
base::Optional<Vector2u> RectPacker::pack(Vector2u rectSize)
{
    const auto fail = [&](const char* what)
    {
        priv::err() << "Failure packing rectangle with size {" << rectSize.x << ", " << rectSize.y << "}: " << what;
        return base::nullOpt;
    };

    if (m_impl->numPacked >= MaxNodes)
        return fail("no nodes left");

    if (rectSize.x == 0u || rectSize.y == 0u)
        return fail("zero-sized coordinate");

    stbrp_rect toPack{/* id */ 0,
                      /* input width */ static_cast<int>(rectSize.x),
                      /* input height */ static_cast<int>(rectSize.y),
                      /* output x*/ {},
                      /* output y */ {},
                      /* was_packed */ {}};

    const int rc = stbrp_pack_rects(&m_impl->context, &toPack, /* num_rects */ 1);

    if (rc == /* failure */ 0)
        return fail("no room to pack");

    SFML_BASE_ASSERT(rc == /* success */ 1);
    SFML_BASE_ASSERT(toPack.was_packed != 0);

    return base::makeOptional<Vector2u>(static_cast<unsigned int>(toPack.x), static_cast<unsigned int>(toPack.y));
}


////////////////////////////////////////////////////////////
Vector2u RectPacker::getSize() const
{
    return {static_cast<unsigned int>(m_impl->context.width), static_cast<unsigned int>(m_impl->context.height)};
}

} // namespace sf
