#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Err.hpp"
#include "SFML/System/RectPacker.hpp"

#include "SFML/Base/Optional.hpp"
#include "SFML/Base/TrivialVector.hpp"

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>


namespace sf
{
////////////////////////////////////////////////////////////
struct RectPacker::Impl
{
    base::TrivialVector<stbrp_node> nodes;
    stbrp_context                   context{};

    explicit Impl(Vector2u size) : nodes(size.x * 3 / 2)
    {
        stbrp_init_target(&context,
                          static_cast<int>(size.x),
                          static_cast<int>(size.y),
                          nodes.data(),
                          static_cast<int>(nodes.size()));
    }
};


////////////////////////////////////////////////////////////
RectPacker::RectPacker(Vector2u size) : m_impl(size)
{
}


////////////////////////////////////////////////////////////
RectPacker::~RectPacker() = default;


////////////////////////////////////////////////////////////
RectPacker::RectPacker(RectPacker&&) noexcept = default;


////////////////////////////////////////////////////////////
RectPacker& RectPacker::operator=(RectPacker&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Vector2u> RectPacker::pack(Vector2u rectSize)
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
