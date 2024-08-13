#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Base/FixedFunction.hpp>


namespace sf::priv
{
class EglContext;
} // namespace sf::priv


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Emscripten implementation
///
////////////////////////////////////////////////////////////
class EmscriptenImpl
{
public:
    using VSyncEnabler = base::FixedFunction<void(), 64>;

    static void setVSyncEnabler(const VSyncEnabler& vSyncEnabler);
    static void invokeAndClearVSyncEnabler();

    static void killWindow();

private:
    static inline thread_local VSyncEnabler tlVSyncEnabler{}; ///< TODO P1: docs
};

} // namespace sf::priv
