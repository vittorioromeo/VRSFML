#pragma once

#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


struct Doll;


////////////////////////////////////////////////////////////
// Maximum number of cats that a single witchcat (or a copycat mimicking
// a witchcat) can keep hexed simultaneously. Bumping this constant is
// sufficient for the underlying systems to support more concurrent hexes;
// an upgrade that raises the effective limit can be layered on top later.
inline constexpr zb::SizeT maxConcurrentHexes = 2u;


////////////////////////////////////////////////////////////
// A single in-flight hex cast. Owns the dolls the player must collect
// and remembers which cat the hex is targeting via an index into
// `Playthrough::cats`.
struct [[nodiscard]] HexSession
{
    zb::SizeT        catIdx{0u};
    zb::Vector<Doll> dolls;

    HexSession();
    ~HexSession();
};
