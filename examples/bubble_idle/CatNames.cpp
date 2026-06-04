#include "CatNames.hpp"
#include "CatType.hpp"

#include "ZancleBase/Algorithm/Shuffle.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/StringView.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
[[nodiscard]] zb::Vector<zb::StringView> getShuffledCatNames(const CatType catType, RNGFast& rng)
{
    const auto span = catNamesPerType[asIdx(catType)];

    zb::Vector<zb::StringView> names;
    names.reserve(span.size());

    for (const char* str : span)
        names.emplaceBack(str);

    zb::shuffle(names.begin(), names.end(), [&](const zb::SizeT min, const zb::SizeT max) {
        return rng.getI<zb::SizeT>(min, max);
    });

    return names;
}
