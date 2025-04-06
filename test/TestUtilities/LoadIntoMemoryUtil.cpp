#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Assert.hpp"

#include <LoadIntoMemoryUtil.hpp>


sf::base::TrivialVector<unsigned char> loadIntoMemory(const char* path)
{
    sf::InFileStream file(sf::Path{path}, sf::FileOpenMode::bin | sf::FileOpenMode::ate);
    SFML_BASE_ASSERT(file);

    const auto size = file.tellg();
    file.seekg(0, sf::SeekDir::beg);

    sf::base::TrivialVector<unsigned char> buffer(static_cast<sf::base::SizeT>(size));

    [[maybe_unused]] const auto& result = file.read(reinterpret_cast<char*>(buffer.data()),
                                                    static_cast<sf::base::PtrDiffT>(size));

    SFML_BASE_ASSERT(result);
    return buffer;
}
