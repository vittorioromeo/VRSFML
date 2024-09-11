#include "SFML/Base/Assert.hpp"

#include <LoadIntoMemoryUtil.hpp>

#include <fstream>


sf::base::TrivialVector<unsigned char> loadIntoMemory(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    SFML_BASE_ASSERT(file);

    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    sf::base::TrivialVector<unsigned char> buffer;
    buffer.resize(static_cast<sf::base::SizeT>(size));

    [[maybe_unused]] const auto& result = file.read(reinterpret_cast<char*>(buffer.data()),
                                                    static_cast<std::streamsize>(size));

    SFML_BASE_ASSERT(result);
    return buffer;
}
