#include "ExampleUtils/LoadedSound.hpp"

#include "Zancle/Audio/SoundBuffer.hpp"

#include "Zancle/IO/Path.hpp"


////////////////////////////////////////////////////////////
LoadedSound::LoadedSound(const za::Path& filename) :
    buffer(za::SoundBuffer::loadFromFile(za::Path{"resources"} / filename).value())
{
}


////////////////////////////////////////////////////////////
LoadedSound::LoadedSound(const char* const filename) : LoadedSound(za::Path{filename})
{
}
