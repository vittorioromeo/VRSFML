#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundChannel.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
ChannelMap::ChannelMap(std::initializer_list<SoundChannel> channels)
{
    m_channels.reserve(channels.size());

    for (SoundChannel sc : channels)
        m_channels.unsafeEmplaceBack(sc);
}


////////////////////////////////////////////////////////////
[[nodiscard]] base::SizeT ChannelMap::getSize() const
{
    return m_channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool ChannelMap::isEmpty() const
{
    return m_channels.empty();
}


////////////////////////////////////////////////////////////
[[nodiscard]] SoundChannel* ChannelMap::begin()
{
    return m_channels.data();
}


////////////////////////////////////////////////////////////
[[nodiscard]] SoundChannel* ChannelMap::end()
{
    return m_channels.data() + m_channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel* ChannelMap::begin() const
{
    return m_channels.data();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel* ChannelMap::end() const
{
    return m_channels.data() + m_channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel& ChannelMap::operator[](base::SizeT index) const
{
    return m_channels[index];
}


////////////////////////////////////////////////////////////
void ChannelMap::reserve(base::SizeT count)
{
    m_channels.reserve(count);
}


////////////////////////////////////////////////////////////
void ChannelMap::append(SoundChannel channel)
{
    m_channels.pushBack(channel);
}

} // namespace sf
