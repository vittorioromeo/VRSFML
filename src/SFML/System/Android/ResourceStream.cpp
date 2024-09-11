#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/System/Android/Activity.hpp"
#include "SFML/System/Android/ResourceStream.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Optional.hpp"

#include <mutex>


namespace sf::priv
{

////////////////////////////////////////////////////////////
ResourceStream::ResourceStream(const Path& filename)
{
    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);
    m_file.reset(AAssetManager_open(states.activity->assetManager, filename.c_str(), AASSET_MODE_UNKNOWN));
    SFML_BASE_ASSERT(m_file && "Failed to initialize ResourceStream file");
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> ResourceStream::read(void* data, base::SizeT size)
{
    const auto numBytesRead = AAsset_read(m_file.get(), data, size);
    return numBytesRead < 0 ? base::nullOpt : base::makeOptional<base::SizeT>(numBytesRead);
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> ResourceStream::seek(base::SizeT position)
{
    const auto newPosition = AAsset_seek(m_file.get(), static_cast<off_t>(position), SEEK_SET);
    return newPosition < 0 ? base::nullOpt : base::makeOptional<base::SizeT>(newPosition);
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> ResourceStream::tell()
{
    return getSize().value() - static_cast<base::SizeT>(AAsset_getRemainingLength(m_file.get()));
}


////////////////////////////////////////////////////////////
base::Optional<base::SizeT> ResourceStream::getSize()
{
    return AAsset_getLength(m_file.get());
}


////////////////////////////////////////////////////////////
void ResourceStream::AAssetDeleter::operator()(AAsset* file)
{
    AAsset_close(file);
}

} // namespace sf::priv
