////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundFileFactory.hpp> // NOLINT(misc-header-include-cycle)


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
