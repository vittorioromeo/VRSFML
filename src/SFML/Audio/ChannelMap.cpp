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


////////////////////////////////////////////////////////////
bool ChannelMap::isPermutationOf(const ChannelMap& rhs) const
{
    const base::SizeT lhsSize = m_channels.size();
    const base::SizeT rhsSize = rhs.m_channels.size();

    if (lhsSize != rhsSize)
        return false;

    if (lhsSize == 0u)
        return true;

    SFML_BASE_ASSERT(lhsSize <= 8u);
    bool rhsMatched[8] = {false};

    for (base::SizeT i = 0u; i < lhsSize; ++i)
    {
        bool foundMatchForIInLhs = false;

        for (base::SizeT j = 0u; j < rhsSize; ++j)
        {
            // Check if rhs[j] is not already used AND if it matches lhs[i]
            if (!rhsMatched[j] && m_channels[i] == rhs.m_channels[j])
            {
                rhsMatched[j]       = true;
                foundMatchForIInLhs = true;
                break; // Found a match for m_channels[i], move to the next element in lhs
            }
        }

        if (!foundMatchForIInLhs)
            return false;
    }

    return true;
}

} // namespace sf
