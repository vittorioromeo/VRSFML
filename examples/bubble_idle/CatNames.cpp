#include "CatNames.hpp"

#include "CatType.hpp"

#include "SFML/Base/Algorithm/Shuffle.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <random>


////////////////////////////////////////////////////////////
template <typename TRng>
[[nodiscard]] sf::base::Vector<sf::base::StringView> getShuffledCatNames(const CatType catType, TRng& randomEngine)
{
    const auto span = catNamesPerType[asIdx(catType)];

    sf::base::Vector<sf::base::StringView> names;
    names.reserve(span.size());

    for (const char* str : span)
        names.emplaceBack(str);

    sf::base::shuffle(names.begin(),
                      names.end(),
                      [&](const sf::base::SizeT min, const sf::base::SizeT max)
    {
        std::uniform_int_distribution<sf::base::SizeT> dist{min, max};
        return dist(randomEngine);
    });

    return names;
}


////////////////////////////////////////////////////////////
template sf::base::Vector<sf::base::StringView> getShuffledCatNames<std::minstd_rand0>(const CatType      catType,
                                                                                       std::minstd_rand0& randomEngine);
