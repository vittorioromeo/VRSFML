#include <SFML/System/Err.hpp>

#include <Doctest.hpp>

#include <sstream>

TEST_CASE("[System] sf::err")
{
    SECTION("Overflow default buffer")
    {
        // No assertion macros in this section since nothing about this can be directly observed.
        // Intention is to ensure DefaultErrStreamBuf::overflow gets called.
        sf::priv::err() << "SFML is a simple, fast, cross-platform and object-oriented multimedia API."
                           "It provides access to windowing, graphics, audio and network."
                           "It is written in C++, and has bindings for various languages such as C, .Net, Ruby, "
                           "Python.";
    }

    SECTION("Redirect buffer to observe contents")
    {
        sf::priv::err() << "We'll never be able to observe this"; // Ensure buffer is flushed
        auto* const defaultStreamBuffer = sf::priv::err().rdbuf();
        CHECK(defaultStreamBuffer != nullptr);

        const std::stringstream stream;
        sf::priv::err().rdbuf(stream.rdbuf());
        sf::priv::err() << "Something went wrong!\n";
        CHECK(stream.str().find("Something went wrong!\n") != std::string::npos);

        sf::priv::err().rdbuf(nullptr);
        sf::priv::err() << "Sent to the abyss";
        CHECK(stream.str().find("Something went wrong!\n") != std::string::npos);

        sf::priv::err().rdbuf(stream.rdbuf());
        sf::priv::err() << "Back to the stringstream :)\n";
        CHECK(stream.str().find("Something went wrong!\n") != std::string::npos);
        CHECK(stream.str().find("Back to the stringstream :)\n") != std::string::npos);

        // Restore sf::err to default stream defaultStreamBuffer
        sf::priv::err().rdbuf(defaultStreamBuffer);
        CHECK(sf::priv::err().rdbuf() == defaultStreamBuffer);
    }
}
