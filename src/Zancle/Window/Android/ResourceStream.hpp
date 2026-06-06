#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "Zancle/Config.hpp"

#include "Zancle/IO/InputStream.hpp"

#include "Zancle/Vocabulary/UniquePtr.hpp"

#include <android/asset_manager.h>


namespace za
{
class Path;
} // namespace za


namespace za::priv
{
////////////////////////////////////////////////////////////
/// \brief Read from Android asset files
///
////////////////////////////////////////////////////////////
class ZA_SYSTEM_API ResourceStream final : public InputStream
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// \param filename Filename of the asset
    ///
    ////////////////////////////////////////////////////////////
    ResourceStream() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Open the stream from a file path
    ///
    /// \param filename Name of the file to open
    ///
    /// \return `true` on success, `false` on error
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Read data from the asset
    ///
    /// \param data Buffer where the asset data is copied
    /// \param size Number of bytes read
    ///
    /// \return The number of bytes actually read, or `za::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    za::Optional<za::SizeT> read(void* data, za::SizeT size) override;

    ////////////////////////////////////////////////////////////
    /// \brief Change the current reading position in the asset file
    ///
    /// \param position The position to seek to, from the beginning
    ///
    /// \return The position actually sought to, or `za::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    za::Optional<za::SizeT> seek(za::SizeT position) override;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current reading position in the asset file
    ///
    /// \return The current position, or `za::nullOpt` on error.
    ///
    ////////////////////////////////////////////////////////////
    za::Optional<za::SizeT> tell() override;

    ////////////////////////////////////////////////////////////
    /// \brief Return the size of the asset file
    ///
    /// \return The total number of bytes available in the asset, or `za::nullOpt` on error
    ///
    ////////////////////////////////////////////////////////////
    za::Optional<za::SizeT> getSize() override;

private:
    ////////////////////////////////////////////////////////////
    // Types
    ////////////////////////////////////////////////////////////
    struct AAssetDeleter
    {
        void operator()(AAsset*);
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    za::UniquePtr<AAsset, AAssetDeleter> m_file; ///< The asset file to read
};

} // namespace za::priv
