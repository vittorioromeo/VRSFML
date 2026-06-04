// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/String.hpp"

#include <cstdio>


namespace
{
////////////////////////////////////////////////////////////
// Sentinel for "cache is empty". Real `getchar` returns either a non-
// negative byte cast through `unsigned char` or `EOF` (commonly `-1`).
// `-2` is therefore safely outside that range.
constexpr int cacheEmpty = -2;

} // namespace


namespace zb
{
////////////////////////////////////////////////////////////
ScnStdinSource::ScnStdinSource() noexcept : m_peeked{cacheEmpty}
{
}


////////////////////////////////////////////////////////////
ScnStdinSource::~ScnStdinSource()
{
    // If a byte was peeked but not consumed, push it back so the next
    // reader (this source's destruction may be followed by another
    // construction, or by direct stdio calls) sees it again.
    if (m_peeked != cacheEmpty && m_peeked != EOF)
        (void)std::ungetc(m_peeked, stdin);
}


////////////////////////////////////////////////////////////
ScnStdinSource::ScnStdinSource(ScnStdinSource&& rhs) noexcept : m_peeked{rhs.m_peeked}
{
    rhs.m_peeked = cacheEmpty;
}


////////////////////////////////////////////////////////////
ScnStdinSource& ScnStdinSource::operator=(ScnStdinSource&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    if (m_peeked != cacheEmpty && m_peeked != EOF)
        (void)std::ungetc(m_peeked, stdin);

    m_peeked     = rhs.m_peeked;
    rhs.m_peeked = cacheEmpty;

    return *this;
}


////////////////////////////////////////////////////////////
Optional<char> ScnStdinSource::peek()
{
    if (m_peeked == cacheEmpty)
        m_peeked = std::getchar();

    if (m_peeked == EOF)
        return nullOpt;

    return makeOptional(static_cast<char>(static_cast<unsigned char>(m_peeked)));
}


////////////////////////////////////////////////////////////
void ScnStdinSource::consume() noexcept
{
    // Pre: a successful `peek()` preceded this call. Drop the cache so
    // the next `peek()` pulls a fresh byte.
    m_peeked = cacheEmpty;
}


////////////////////////////////////////////////////////////
void scnStdinIgnoreLine()
{
    // Avoid the per-call `ScnStdinSource` machinery: drain bytes
    // directly via `fgetc` until we hit `'\n'` or EOF.
    for (;;)
    {
        const int c = std::getchar();

        if (c == EOF || c == '\n')
            return;
    }
}


////////////////////////////////////////////////////////////
bool scnStdinReadLine(String& out)
{
    ScnStdinSource src;
    return scnReadLine(src, out);
}

} // namespace zb
