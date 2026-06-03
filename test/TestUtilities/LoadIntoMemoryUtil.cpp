#include "LoadIntoMemoryUtil.hpp"

#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"


sf::base::Vector<char> loadIntoMemory(const char* path)
{
    sf::base::Vector<char> buffer;

    [[maybe_unused]] const bool ok = sf::readFromFile(sf::Path{path}, buffer);
    SFML_BASE_ASSERT(ok);

    return buffer;
}
