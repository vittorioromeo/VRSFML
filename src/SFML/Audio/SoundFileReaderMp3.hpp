#pragma once
#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/SoundFileReader.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/Optional.hpp"

#include <cstdint>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Implementation of sound file reader that handles MP3 files
///
////////////////////////////////////////////////////////////
class SoundFileReaderMp3 : public SoundFileReader
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Check if this reader can handle a file given by an input stream
    ///
    /// \param stream Source stream to check
    ///
    /// \return True if the file is supported by this reader
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool check(InputStream& stream);

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundFileReaderMp3();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundFileReaderMp3() override;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for reading
    ///
    /// \param stream Source stream to read from
    ///
    /// \return Properties of the loaded sound if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] base::Optional<Info> open(InputStream& stream) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current read position to the given sample offset
    ///
    /// The sample offset takes the channels into account.
    /// If you have a time offset instead, you can easily find
    /// the corresponding sample offset with the following formula:
    /// `timeInSeconds * sampleRate * channelCount`
    /// If the given offset exceeds to total number of samples,
    /// this function must jump to the end of the file.
    ///
    /// \param sampleOffset Index of the sample to jump to, relative to the beginning
    ///
    ////////////////////////////////////////////////////////////
    void seek(std::uint64_t sampleOffset) override;

    ////////////////////////////////////////////////////////////
    /// \brief Read audio samples from the open file
    ///
    /// \param samples  Pointer to the sample array to fill
    /// \param maxCount Maximum number of samples to read
    ///
    /// \return Number of samples actually read (may be less than \a maxCount)
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] std::uint64_t read(std::int16_t* samples, std::uint64_t maxCount) override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 12288> m_impl; //!< Implementation details
};

} // namespace sf::priv
