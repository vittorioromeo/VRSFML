#pragma once
// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Export.hpp"

#include "ZancleBase/FwdStdString.hpp"
#include "ZancleBase/IntTypes.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/Vector.hpp"


////////////////////////////////////////////////////////////
// Forward declarations
////////////////////////////////////////////////////////////
namespace za
{
class Utf8String;
} // namespace za

namespace zb
{
class String;
} // namespace zb


namespace za
{
////////////////////////////////////////////////////////////
/// \brief Utility class to build blocks of data to transfer
///        over the network
///
/// \note The wire format stores multi-byte values in host byte
/// order and assumes both peers are little-endian. Every platform
/// Zancle targets (x86_64, ARM/AArch64 in its standard LE mode,
/// WebAssembly, Apple Silicon, modern consoles) is little-endian.
/// Big-endian hosts are unsupported.
///
////////////////////////////////////////////////////////////
class ZA_NETWORK_API Packet
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    /// Creates an empty packet.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Packet() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Virtual destructor
    ///
    ////////////////////////////////////////////////////////////
    virtual ~Packet() = default;

    ////////////////////////////////////////////////////////////
    /// \brief Copy constructor
    ///
    ////////////////////////////////////////////////////////////
    Packet(const Packet&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Copy assignment
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator=(const Packet&) = default;

    ////////////////////////////////////////////////////////////
    /// \brief Move constructor
    ///
    ////////////////////////////////////////////////////////////
    Packet(Packet&&) noexcept = default;

    ////////////////////////////////////////////////////////////
    /// \brief Move assignment
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator=(Packet&&) noexcept = default;

    ////////////////////////////////////////////////////////////
    /// \brief Append data to the end of the packet
    ///
    /// \param data        Pointer to the sequence of bytes to append
    ///                    (must not be `nullptr`)
    /// \param sizeInBytes Number of bytes to append (must be `> 0`)
    ///
    /// \pre `data != nullptr`
    /// \pre `sizeInBytes > 0`
    ///
    /// Both preconditions are checked via `ZB_ASSERT_AND_ASSUME`:
    /// in debug builds a violation aborts; in release the compiler
    /// assumes the conditions and may optimize accordingly. Guard
    /// at the call-site if the source can legitimately be empty.
    ///
    /// \see `clear`
    /// \see `getReadPosition`
    ///
    ////////////////////////////////////////////////////////////
    Packet& append(const void* data, zb::SizeT sizeInBytes);

    ////////////////////////////////////////////////////////////
    /// \brief Get the current reading position in the packet
    ///
    /// The next read operation will read data from this position
    ///
    /// \return The byte offset of the current read position
    ///
    /// \see `append`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::SizeT getReadPosition() const;

    ////////////////////////////////////////////////////////////
    /// \brief Clear the packet
    ///
    /// After calling Clear, the packet is empty.
    ///
    /// \see `append`
    ///
    ////////////////////////////////////////////////////////////
    void clear();

    ////////////////////////////////////////////////////////////
    /// \brief Get a pointer to the data contained in the packet
    ///
    /// Warning: the returned pointer may become invalid after
    /// you append data to the packet, therefore it should never
    /// be stored.
    /// The return pointer is a `nullptr` if the packet is empty.
    ///
    /// \return Pointer to the data
    ///
    /// \see `getDataSize`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] const void* getData() const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the size of the data contained in the packet
    ///
    /// This function returns the number of bytes pointed to by
    /// what `getData` returns.
    ///
    /// \return Data size, in bytes
    ///
    /// \see `getData`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::SizeT getDataSize() const;

    ////////////////////////////////////////////////////////////
    /// \brief Tell if the reading position has reached the
    ///        end of the packet
    ///
    /// This function is useful to know if there is some data
    /// left to be read, without actually reading it.
    ///
    /// \return `true` if all data was read, `false` otherwise
    ///
    /// \see `operator` bool
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool endOfPacket() const;

    ////////////////////////////////////////////////////////////
    /// \brief Test the validity of the packet, for reading
    ///
    /// This operator allows to test the packet as a boolean
    /// variable, to check if a reading operation was successful.
    ///
    /// A packet will be in an invalid state if it has no more
    /// data to read.
    ///
    /// This behavior is the same as standard C++ streams.
    ///
    /// Usage example:
    /// \code
    /// float x;
    /// packet >> x;
    /// if (packet)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    ///
    /// // -- or --
    ///
    /// float x;
    /// if (packet >> x)
    /// {
    ///    // ok, x was extracted successfully
    /// }
    /// \endcode
    ///
    /// \return `true` if last data extraction from packet was successful
    ///
    /// \see `endOfPacket`
    ///
    ////////////////////////////////////////////////////////////
    explicit operator bool() const;

    ////////////////////////////////////////////////////////////
    /// Overload of `operator>>` to read data from the packet
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator>>(bool& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::I8& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::U8& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::I16& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::U16& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::I32& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::U32& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::I64& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::U64& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(float& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(double& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(std::string& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(zb::String& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(std::wstring& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator>>(Utf8String& data);

    ////////////////////////////////////////////////////////////
    /// Overload of `operator<<` to write data into the packet
    ///
    ////////////////////////////////////////////////////////////
    Packet& operator<<(bool data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::I8 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::U8 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::I16 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::U16 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::I32 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::U32 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::I64 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(zb::U64 data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(float data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(double data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(const std::string& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(const zb::String& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(const std::wstring& data);

    ////////////////////////////////////////////////////////////
    /// \overload
    ////////////////////////////////////////////////////////////
    Packet& operator<<(const Utf8String& data);

protected:
    friend class TcpSocket;
    friend class UdpSocket;

    ////////////////////////////////////////////////////////////
    /// \brief Called before the packet is sent over the network
    ///
    /// This function can be defined by derived classes to
    /// transform the data before it is sent; this can be
    /// used for compression, encryption, etc.
    /// The function must return a pointer to the modified data,
    /// as well as the number of bytes pointed.
    /// The default implementation provides the packet's data
    /// without transforming it.
    ///
    /// \param size Variable to fill with the size of data to send
    ///
    /// \return Pointer to the array of bytes to send
    ///
    /// \see `onReceive`
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] virtual const void* onSend(zb::SizeT& size);

    ////////////////////////////////////////////////////////////
    /// \brief Called after the packet is received over the network
    ///
    /// This function can be defined by derived classes to
    /// transform the data after it is received; this can be
    /// used for decompression, decryption, etc.
    /// The function receives a pointer to the received data,
    /// and must fill the packet with the transformed bytes.
    /// The default implementation fills the packet directly
    /// without transforming the data, by forwarding to `append`
    /// (and therefore inherits its `data != nullptr && size > 0`
    /// precondition). Zancle's own `TcpSocket`/`UdpSocket`
    /// guard the call against empty/null inputs.
    ///
    /// \param data Pointer to the received bytes
    /// \param size Number of bytes
    ///
    /// \see `onSend`
    ///
    ////////////////////////////////////////////////////////////
    virtual void onReceive(const void* data, zb::SizeT size);

private:
    ////////////////////////////////////////////////////////////
    /// \brief Check if the packet can extract a given number of bytes
    ///
    /// This function updates accordingly the state of the packet.
    ///
    /// \param size Size to check
    ///
    /// \return `true` if \a size bytes can be read from the packet
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool checkSize(zb::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Bytes available to read from the current position
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::SizeT remaining() const;

    ////////////////////////////////////////////////////////////
    /// \brief Bounds-checked memcpy from the read cursor; advances on success
    ///
    ////////////////////////////////////////////////////////////
    Packet& readBytes(void* dst, zb::SizeT size);

    ////////////////////////////////////////////////////////////
    /// \brief Return a reference to the send position used by
    ///        `TcpSocket` to track partial sends across calls.
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] zb::SizeT& getSendPos();

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    zb::Vector<unsigned char> m_data;          //!< Data stored in the packet
    zb::SizeT                 m_readPos{};     //!< Current reading position in the packet
    zb::SizeT                 m_sendPos{};     //!< Current send position in the packet (for handling partial sends)
    bool                        m_isValid{true}; //!< Reading state of the packet
};

} // namespace za


////////////////////////////////////////////////////////////
/// \class za::Packet
/// \ingroup network
///
/// Packets provide a safe and easy way to serialize data,
/// in order to send it over the network using sockets
/// (`za::TcpSocket`, `za::UdpSocket`).
///
/// Packets solve 2 fundamental problems that arise when
/// transferring data over the network:
/// \li trivial types are serialized with a fixed wire size (so a
///     `zb::I32` is always 4 bytes on the wire regardless of host)
/// \li the bounds of the packet are preserved (one send == one receive)
///
/// The wire format is host byte order and assumes both peers are
/// little-endian (see the `\note` on the class above).
///
/// The `za::Packet` class provides both input and output modes.
/// It is designed to follow the behavior of standard C++ streams,
/// using operators >> and << to extract and insert data.
///
/// It is recommended to use only fixed-size types (like `zb::I32`, etc.),
/// to avoid possible differences between the sender and the receiver.
/// Indeed, the native C++ types may have different sizes on two platforms
/// and your data may be corrupted if that happens.
///
/// Usage example:
/// \code
/// zb::U32 x = 24;
/// std::string s = "hello";
/// double d = 5.89;
///
/// // Group the variables to send into a packet
/// za::Packet packet;
/// packet << x << s << d;
///
/// // Send it over the network (socket is a valid za::TcpSocket)
/// socket.send(packet);
///
/// -----------------------------------------------------------------
///
/// // Receive the packet at the other end
/// za::Packet packet;
/// socket.receive(packet);
///
/// // Extract the variables contained in the packet
/// zb::U32 x;
/// std::string s;
/// double d;
/// if (packet >> x >> s >> d)
/// {
///     // Data extracted successfully...
/// }
/// \endcode
///
/// Packets have built-in `operator>>` and << overloads for
/// standard types:
/// \li `bool`
/// \li fixed-size integer types (`zb::I8`/`zb::U8` ... `zb::I64`/`zb::U64`)
/// \li floating point numbers (`float`, `double`)
/// \li string types (`std::string`, `std::wstring`, `zb::String`, `za::Utf8String`)
///
/// Raw `char*` / `wchar_t*` overloads are intentionally not provided
/// because they would either rely on caller-supplied null termination
/// on the write side or write past an unbounded output buffer on the
/// read side. Use `.append(ptr, size)` and a string type instead.
///
/// Like standard streams, it is also possible to define your own
/// overloads of operators >> and << in order to handle your
/// custom types.
///
/// \code
/// struct MyStruct
/// {
///     float       number{};
///     zb::I8 integer{};
///     std::string str;
/// };
///
/// za::Packet& operator <<(za::Packet& packet, const MyStruct& m)
/// {
///     return packet << m.number << m.integer << m.str;
/// }
///
/// za::Packet& operator >>(za::Packet& packet, MyStruct& m)
/// {
///     return packet >> m.number >> m.integer >> m.str;
/// }
/// \endcode
///
/// Packets also provide an extra feature that allows to apply
/// custom transformations to the data before it is sent,
/// and after it is received. This is typically used to
/// handle automatic compression or encryption of the data.
/// This is achieved by inheriting from `za::Packet`, and overriding
/// the onSend and onReceive functions.
///
/// Here is an example:
/// \code
/// class ZipPacket : public za::Packet
/// {
///     const void* onSend(zb::SizeT& size) override
///     {
///         const void* srcData = getData();
///         zb::SizeT srcSize = getDataSize();
///
///         return MySuperZipFunction(srcData, srcSize, &size);
///     }
///
///     void onReceive(const void* data, zb::SizeT size) override
///     {
///         zb::SizeT dstSize;
///         const void* dstData = MySuperUnzipFunction(data, size, &dstSize);
///
///         append(dstData, dstSize);
///     }
/// };
///
/// // Use like regular packets:
/// ZipPacket packet;
/// packet << x << s << d;
/// ...
/// \endcode
///
/// \see `za::TcpSocket`, `za::UdpSocket`
///
////////////////////////////////////////////////////////////
