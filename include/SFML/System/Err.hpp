#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <iosfwd>


namespace sf::priv
{
////////////////////////////////////////////////////////////
struct PathDebugFormatter;

////////////////////////////////////////////////////////////
struct ErrFlushType
{
};

inline constexpr ErrFlushType errFlush;

////////////////////////////////////////////////////////////
class SFML_SYSTEM_API ErrStream
{
    friend ErrStream& err(bool multiLine);

private:
    class SFML_SYSTEM_API Guard
    {
    public:
        explicit Guard(std::ostream& stream, void* mutexPtr, bool multiLine);
        ~Guard();

        Guard(const Guard&)            = delete;
        Guard& operator=(const Guard&) = delete;

        Guard(Guard&&)            = delete;
        Guard& operator=(Guard&&) = delete;

        Guard& operator<<(std::ios_base& (*func)(std::ios_base&));
        Guard& operator<<(std::ostream& (*func)(std::ostream&));

        Guard& operator<<(const char* value);
        Guard& operator<<(ErrFlushType);
        Guard& operator<<(PathDebugFormatter);

        template <typename T>
        Guard& operator<<(const T& value);

    private:
        std::ostream& m_stream;
        void*         m_mutexPtr;
        bool          m_multiLine;
    };

    struct Impl;
    base::InPlacePImpl<Impl, 512> m_impl; //!< Implementation details

public:
    explicit ErrStream(std::streambuf* sbuf);

    Guard operator<<(std::ostream& (*func)(std::ostream&));

    std::streambuf* rdbuf();
    void            rdbuf(std::streambuf* sbuf);

    Guard operator<<(const char* value);
    Guard operator<<(ErrFlushType);
    Guard operator<<(PathDebugFormatter);

    template <typename T>
    Guard operator<<(const T& value);
};

////////////////////////////////////////////////////////////
/// \brief Standard stream used by SFML to output warnings and errors
///
////////////////////////////////////////////////////////////
[[nodiscard]] SFML_SYSTEM_API ErrStream& err(bool multiLine = false);

} // namespace sf::priv


////////////////////////////////////////////////////////////
/// \fn sf::err
/// \ingroup system
///
/// By default, `sf::priv::err()` outputs to the same location as `std::cerr`,
/// (-> the stderr descriptor) which is the console if there's
/// one available.
///
/// It is a standard `std::ostream` instance, so it supports all the
/// insertion operations defined by the STL
/// (operator <<, manipulators, etc.).
///
/// `sf::priv::err()` can be redirected to write to another output, independently
/// of `std::cerr`, by using the `rdbuf()` function provided by the
/// `std::ostream` class.
///
/// Example:
/// \code
/// // Redirect to a file
/// std::ofstream file("sfml-log.txt");
/// std::streambuf* previous = sf::priv::err().rdbuf(file.rdbuf());
///
/// // Redirect to nothing
/// sf::priv::err().rdbuf(nullptr);
///
/// // Restore the original output
/// sf::priv::err().rdbuf(previous);
/// \endcode
///
/// \return Reference to `std::ostream` representing the SFML error stream
///
////////////////////////////////////////////////////////////
