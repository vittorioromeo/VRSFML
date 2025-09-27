#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundFileWriter.hpp"

#include "SFML/Base/InPlacePImpl.hpp"
#include "SFML/Base/IntTypes.hpp"


namespace sf
{
class Path;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Implementation of sound file writer that handles OGG/Vorbis files
///
////////////////////////////////////////////////////////////
class SoundFileWriterOgg : public SoundFileWriter
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Check if this writer can handle a file on disk
    ///
    /// \param filename Path of the sound file to check
    ///
    /// \return `true` if the file can be written by this writer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool check(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundFileWriterOgg();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundFileWriterOgg() override;

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for writing
    ///
    /// \param filename     Path of the file to open
    /// \param sampleRate   Sample rate of the sound
    /// \param channelCount Number of channels of the sound
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return `true` if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(const Path&       filename,
                            unsigned int      sampleRate,
                            unsigned int      channelCount,
                            const ChannelMap& channelMap) override;

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the open file
    ///
    /// \param samples Pointer to the sample array to write
    /// \param count   Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    void write(const base::I16* samples, base::U64 count) override;

private:
    ////////////////////////////////////////////////////////////
    /// \brief Flush blocks produced by the ogg stream, if any
    ///
    ////////////////////////////////////////////////////////////
    void flushBlocks();

    ////////////////////////////////////////////////////////////
    /// \brief Close the file
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 2048> m_impl; //!< Implementation details
};

} // namespace sf::priv
