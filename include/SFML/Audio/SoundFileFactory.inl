// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileFactory.hpp" // NOLINT(misc-header-include-cycle)

namespace sf::priv
{
////////////////////////////////////////////////////////////
template <typename T>
base::UniquePtr<SoundFileReader> createReader()
{
    return base::makeUnique<T>();
}


////////////////////////////////////////////////////////////
template <typename T>
base::UniquePtr<SoundFileWriter> createWriter()
{
    return base::makeUnique<T>();
}

} // namespace sf::priv


namespace sf
{
////////////////////////////////////////////////////////////
template <typename T>
void SoundFileFactory::registerReader()
{
    registerReaderImpl(&priv::createReader<T>, &T::check);
}


////////////////////////////////////////////////////////////
template <typename T>
void SoundFileFactory::unregisterReader()
{
    unregisterReaderImpl(&priv::createReader<T>);
}


////////////////////////////////////////////////////////////
template <typename T>
bool SoundFileFactory::isReaderRegistered()
{
    return isReaderRegisteredImpl(&priv::createReader<T>);
}


////////////////////////////////////////////////////////////
template <typename T>
void SoundFileFactory::registerWriter()
{
    registerWriterImpl(&priv::createWriter<T>, &T::check);
}


////////////////////////////////////////////////////////////
template <typename T>
void SoundFileFactory::unregisterWriter()
{
    unregisterWriterImpl(&priv::createWriter<T>);
}


////////////////////////////////////////////////////////////
template <typename T>
bool SoundFileFactory::isWriterRegistered()
{
    return isWriterRegisteredImpl(&priv::createWriter<T>);
}

} // namespace sf
