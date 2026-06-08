// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Window/Android/ResourceStream.hpp"

#include "Zancle/Window/Android/Activity.hpp"

#include "Zancle/IO/Path.hpp"

#include "Zancle/Vocabulary/Optional.hpp"

#include "Zancle/Base/Assert.hpp"

#include <mutex>


namespace za::priv
{
////////////////////////////////////////////////////////////
bool ResourceStream::open(const Path& filename)
{
    ActivityStates&       states = getActivity();
    const std::lock_guard lock(states.mutex);
    m_file.reset(AAssetManager_open(states.activity->assetManager, filename.c_str(), AASSET_MODE_UNKNOWN));
    return m_file != nullptr;
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> ResourceStream::read(void* data, za::SizeT size)
{
    ZA_ASSERT(m_file && "ResourceStream::read() cannot be called when file is not initialized");
    const auto numBytesRead = AAsset_read(m_file.get(), data, size);
    return numBytesRead < 0 ? za::nullOpt : za::makeOptional<za::SizeT>(numBytesRead);
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> ResourceStream::seek(za::SizeT position)
{
    ZA_ASSERT(m_file && "ResourceStream::seek() cannot be called when file is not initialized");
    const auto newPosition = AAsset_seek(m_file.get(), static_cast<off_t>(position), SEEK_SET);
    return newPosition < 0 ? za::nullOpt : za::makeOptional<za::SizeT>(newPosition);
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> ResourceStream::tell()
{
    ZA_ASSERT(m_file && "ResourceStream::tell() cannot be called when file is not initialized");
    return getSize().value() - static_cast<za::SizeT>(AAsset_getRemainingLength(m_file.get()));
}


////////////////////////////////////////////////////////////
za::Optional<za::SizeT> ResourceStream::getSize()
{
    ZA_ASSERT(m_file && "ResourceStream::getSize() cannot be called when file is not initialized");
    return AAsset_getLength(m_file.get());
}


////////////////////////////////////////////////////////////
void ResourceStream::AAssetDeleter::operator()(AAsset* file)
{
    if (file != nullptr)
        AAsset_close(file);
}

} // namespace za::priv
