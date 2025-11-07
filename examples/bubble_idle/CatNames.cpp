#include "CatNames.hpp"

#include "CatType.hpp"

#include "SFML/Base/Algorithm/Shuffle.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"


////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::Vector<sf::base::StringView> getShuffledCatNames(const CatType catType, RNGFast& rng)
{
    const auto span = catNamesPerType[asIdx(catType)];

    sf::base::Vector<sf::base::StringView> names;
    names.reserve(span.size());

    for (const char* str : span)
        names.emplaceBack(str);

    sf::base::shuffle(names.begin(), names.end(), [&](const sf::base::SizeT min, const sf::base::SizeT max) {
        return rng.getI<sf::base::SizeT>(min, max);
    });

    return names;
}
