// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Audio/ChannelMap.hpp"

#include "Zancle/Audio/SoundChannel.hpp"

#include "Zancle/Base/Assert.hpp"
#include "Zancle/Base/InitializerList.hpp"
#include "Zancle/Base/SizeT.hpp"

namespace za
{
////////////////////////////////////////////////////////////
ChannelMap::ChannelMap(std::initializer_list<SoundChannel> channels)
{
    m_channels.reserve(channels.size());

    for (SoundChannel sc : channels)
        m_channels.unsafeEmplaceBack(sc);
}


////////////////////////////////////////////////////////////
[[nodiscard]] za::SizeT ChannelMap::getSize() const
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
[[nodiscard]] const SoundChannel& ChannelMap::operator[](za::SizeT index) const
{
    return m_channels[index];
}


////////////////////////////////////////////////////////////
void ChannelMap::append(SoundChannel channel)
{
    m_channels.pushBack(channel);
}


////////////////////////////////////////////////////////////
bool ChannelMap::isPermutationOf(const ChannelMap& rhs) const
{
    const za::SizeT lhsSize = m_channels.size();
    const za::SizeT rhsSize = rhs.m_channels.size();

    if (lhsSize != rhsSize)
        return false;

    if (lhsSize == 0u)
        return true;

    ZA_ASSERT(lhsSize <= SoundChannelCount);
    bool rhsMatched[SoundChannelCount] = {};

    for (za::SizeT i = 0u; i < lhsSize; ++i)
    {
        bool foundMatchForIInLhs = false;

        for (za::SizeT j = 0u; j < rhsSize; ++j)
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

} // namespace za
