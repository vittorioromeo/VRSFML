// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Ftp.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/Socket.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/Time.hpp"

#include "SFML/Base/Algorithm/Copy.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Fmt/FmtToString.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/Scn/Scn.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
#include "SFML/Base/Scn/ScnStringSource.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/Span.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <cctype>


namespace sf
{
////////////////////////////////////////////////////////////
class Ftp::DataChannel
{
public:
    ////////////////////////////////////////////////////////////
    explicit DataChannel(Ftp& owner);

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    DataChannel(const DataChannel&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    DataChannel& operator=(const DataChannel&) = delete;

    ////////////////////////////////////////////////////////////
    Ftp::Response open(Ftp::TransferMode mode);

    ////////////////////////////////////////////////////////////
    void send(auto& stream);

    ////////////////////////////////////////////////////////////
    void receive(auto& stream);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Ftp&                      m_ftp;        //!< Reference to the owner Ftp instance
    base::Optional<TcpSocket> m_dataSocket; //!< Socket used for data transfers (created in `open`)
};


////////////////////////////////////////////////////////////
struct Ftp::Response::Impl
{
    Status       status;  //!< Status code returned from the server
    base::String message; //!< Last message received from the server
};


////////////////////////////////////////////////////////////
Ftp::Response::Response(Status code, base::StringView message) : m_impl{code, message.toString<base::String>()}
{
}


////////////////////////////////////////////////////////////
Ftp::Response::~Response()                                   = default;
Ftp::Response::Response(const Response&)                     = default;
Ftp::Response& Ftp::Response::operator=(const Response&)     = default;
Ftp::Response::Response(Response&&) noexcept                 = default;
Ftp::Response& Ftp::Response::operator=(Response&&) noexcept = default;


////////////////////////////////////////////////////////////
bool Ftp::Response::isOk() const
{
    return static_cast<int>(m_impl->status) < 400;
}


////////////////////////////////////////////////////////////
Ftp::Response::Status Ftp::Response::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
base::StringView Ftp::Response::getMessage() const
{
    return m_impl->message;
}


////////////////////////////////////////////////////////////
Ftp::DirectoryResponse::DirectoryResponse(const Ftp::Response& response) : Ftp::Response(response)
{
    if (isOk())
    {
        // Extract the directory from the server response
        const base::SizeT begin = getMessage().find('"', 0);
        const base::SizeT end   = getMessage().find('"', begin + 1);

        m_directory = getMessage().substrByPosLen(begin + 1, end - begin - 1).toString<base::String>();
    }
}


////////////////////////////////////////////////////////////
Ftp::DirectoryResponse::DirectoryResponse()  = default;
Ftp::DirectoryResponse::~DirectoryResponse() = default;


////////////////////////////////////////////////////////////
Ftp::DirectoryResponse::DirectoryResponse(const DirectoryResponse&)                 = default;
Ftp::DirectoryResponse& Ftp::DirectoryResponse::operator=(const DirectoryResponse&) = default;


////////////////////////////////////////////////////////////
Ftp::DirectoryResponse::DirectoryResponse(DirectoryResponse&&) noexcept                 = default;
Ftp::DirectoryResponse& Ftp::DirectoryResponse::operator=(DirectoryResponse&&) noexcept = default;


////////////////////////////////////////////////////////////
const Path& Ftp::DirectoryResponse::getDirectory() const
{
    return m_directory;
}


////////////////////////////////////////////////////////////
struct Ftp::ListingResponse::Impl
{
    base::Vector<base::String> listing; //!< Directory/file names extracted from the data
};


////////////////////////////////////////////////////////////
Ftp::ListingResponse::ListingResponse(const Ftp::Response& response, base::StringView data) : Ftp::Response(response)
{
    if (isOk())
    {
        // Fill the array of strings
        base::SizeT lastPos = 0;
        for (base::SizeT pos = data.find("\r\n"); pos != base::String::nPos; pos = data.find("\r\n", lastPos))
        {
            m_impl->listing.pushBack(data.substrByPosLen(lastPos, pos - lastPos).toString<base::String>());
            lastPos = pos + 2;
        }
    }
}


////////////////////////////////////////////////////////////
Ftp::ListingResponse::~ListingResponse() = default;


////////////////////////////////////////////////////////////
Ftp::ListingResponse::ListingResponse(const ListingResponse&)                 = default;
Ftp::ListingResponse& Ftp::ListingResponse::operator=(const ListingResponse&) = default;


////////////////////////////////////////////////////////////
Ftp::ListingResponse::ListingResponse(ListingResponse&&) noexcept                 = default;
Ftp::ListingResponse& Ftp::ListingResponse::operator=(ListingResponse&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Span<const base::String> Ftp::ListingResponse::getListing() const
{
    return {m_impl->listing.data(), m_impl->listing.size()};
}


////////////////////////////////////////////////////////////
struct Ftp::Impl
{
    base::Optional<TcpSocket> commandSocket; //!< Socket holding the control connection with the server
    base::String              receiveBuffer; //!< Received command data that is yet to be processed
};


////////////////////////////////////////////////////////////
Ftp::Ftp() = default;


////////////////////////////////////////////////////////////
Ftp::~Ftp()
{
    (void)disconnect();
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::connect(IpAddress server, unsigned short port, Time timeout)
{
    m_impl->commandSocket = TcpSocket::create(/* isBlocking */ true);
    if (!m_impl->commandSocket.hasValue())
        return Response(Response::Status::ConnectionFailed);

    if (m_impl->commandSocket->connect(server, port, timeout) != Socket::Status::Done)
    {
        m_impl->commandSocket.reset();
        return Response(Response::Status::ConnectionFailed);
    }

    // Get the response to the connection
    return getResponse();
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::login()
{
    return login("anonymous", "user@sfml-dev.org");
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::login(base::StringView name, base::StringView password)
{
    Response response = sendCommand("USER", name);
    if (response.isOk())
        response = sendCommand("PASS", password);

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::disconnect()
{
    Response response; // Use a single local variable for NRVO

    if (!m_impl->commandSocket.hasValue())
    {
        response = Response(Response::Status::ConnectionClosed);
        return response;
    }

    // Send the exit command
    response = sendCommand("QUIT");

    // Tear down the control connection unconditionally, even if QUIT failed
    // or the server never replied: otherwise the socket would linger until
    // the Ftp object is destroyed.
    m_impl->commandSocket.reset();
    m_impl->receiveBuffer.clear();

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::keepAlive()
{
    return sendCommand("NOOP");
}


////////////////////////////////////////////////////////////
Ftp::DirectoryResponse Ftp::getWorkingDirectory()
{
    return {sendCommand("PWD")};
}


////////////////////////////////////////////////////////////
Ftp::ListingResponse Ftp::getDirectoryListing(base::StringView directory)
{
    // Open a data channel on default port (20) using ASCII transfer mode
    base::String listing;
    DataChannel  data(*this);
    Response     response = data.open(TransferMode::Ascii);
    if (response.isOk())
    {
        // Tell the server to send us the listing
        response = sendCommand("NLST", directory);
        if (response.isOk())
        {
            // Receive the listing
            data.receive(listing);

            // Get the response from the server
            response = getResponse();
        }
    }

    return {response, SFML_BASE_MOVE(listing)};
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::changeDirectory(base::StringView directory)
{
    return sendCommand("CWD", directory);
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::parentDirectory()
{
    return sendCommand("CDUP");
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::createDirectory(base::StringView name)
{
    return sendCommand("MKD", name);
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::deleteDirectory(base::StringView name)
{
    return sendCommand("RMD", name);
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::renameFile(const Path& file, const Path& newName)
{
    Response response = sendCommand("RNFR", file.to<base::String>());
    if (response.isOk())
        response = sendCommand("RNTO", newName.to<base::String>());

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::deleteFile(const Path& name)
{
    return sendCommand("DELE", name.to<base::String>());
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::download(const Path& remoteFile, const Path& localPath, TransferMode mode)
{
    // Open a data channel using the given transfer mode
    DataChannel data(*this);
    Response    response = data.open(mode); // Use a single local variable for NRVO

    if (!response.isOk())
        return response;

    // Tell the server to start the transfer
    response = sendCommand("RETR", remoteFile.to<base::String>());

    if (!response.isOk())
        return response;

    // Create the file and truncate it if necessary
    const Path filepath = localPath / remoteFile.getFilename();

    auto optFile = OutFile::open(filepath.to<base::String>(), FileOpenMode::bin | FileOpenMode::trunc);
    if (!optFile.hasValue())
    {
        response = Response(Response::Status::InvalidFile);
        return response;
    }

    // Receive the file data. The file's destructor (end of scope below)
    // closes the OS handle.
    data.receive(*optFile);
    optFile.reset();

    // Get the response from the server
    response = getResponse();

    // If the download was unsuccessful, delete the partial file
    if (!response.isOk())
        if (!filepath.removeFromDisk())
            priv::errMsg("Failed to delete '{}{}", filepath, '\'');

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::upload(const Path& localFile, const Path& remotePath, TransferMode mode, bool append)
{
    Response response; //  Use a single local variable for NRVO

    // Get the contents of the file to send
    auto optFile = InFile::open(localFile.to<base::String>(), FileOpenMode::bin);
    if (!optFile.hasValue())
    {
        response = Response(Response::Status::InvalidFile);
        return response;
    }
    auto& file = *optFile;

    // Open a data channel using the given transfer mode
    DataChannel data(*this);
    response = data.open(mode);

    if (!response.isOk())
        return response;

    // Tell the server to start the transfer
    response = sendCommand(append ? "APPE" : "STOR", (remotePath / localFile.getFilename()).to<base::String>());

    if (!response.isOk())
        return response;

    // Send the file data
    data.send(file);

    // Get the response from the server
    response = getResponse();

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::sendCommand(base::StringView command, base::StringView parameter)
{
    // Build the command string
    auto commandStr = command.toString<base::String>();

    if (parameter.empty())
        commandStr += "\r\n";
    else
    {
        // TODO P2: concat utility from Open Hexagon?
        commandStr += ' ';
        commandStr += parameter.toString<base::String>();
        commandStr += "\r\n";
    }

    if (!m_impl->commandSocket.hasValue())
        return Response(Response::Status::ConnectionClosed);

    // Send it to the server
    if (m_impl->commandSocket->send(commandStr.cStr(), commandStr.size()) != Socket::Status::Done)
        return Response(Response::Status::ConnectionClosed);

    // Get the response
    return getResponse();
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::getResponse()
{
    // We'll use a variable to keep track of the last valid code.
    // It is useful in case of multi-lines responses, because the end of such a response
    // will start by the same code
    unsigned int lastCode          = 0;
    bool         isInsideMultiline = false;
    base::String message;

    for (;;)
    {
        // Receive the response from the server
        char        buffer[1024];
        base::SizeT length = 0;

        if (m_impl->receiveBuffer.empty())
        {
            if (!m_impl->commandSocket.hasValue() ||
                m_impl->commandSocket->receive(buffer, sizeof(buffer), length) != Socket::Status::Done)
                return Response(Response::Status::ConnectionClosed);
        }
        else
        {
            base::copy(m_impl->receiveBuffer.begin(), m_impl->receiveBuffer.end(), buffer);
            length = m_impl->receiveBuffer.size();
            m_impl->receiveBuffer.clear();
        }

        // There can be several lines inside the received buffer, extract them all
        base::ScnStringSource scanner{base::StringView{buffer, length}};
        while (!base::scnAtEnd(scanner))
        {
            // Try to extract the code
            unsigned int code = 0;
            if (base::scnInto(scanner, code))
            {
                // Extract the separator (next byte verbatim, no whitespace skip).
                // Failure here means the buffer ended right after the code -- the
                // response is malformed.
                const auto sep = scanner.peek();
                if (!sep)
                    return Response(Response::Status::InvalidResponse);
                const char separator = *sep;
                scanner.consume();

                // The '-' character means a multiline response
                if ((separator == '-') && !isInsideMultiline)
                {
                    // Set the multiline flag
                    isInsideMultiline = true;

                    // Keep track of the code
                    if (lastCode == 0)
                        lastCode = code;

                    // Extract the line. Failure leaves `message` indeterminate;
                    // the subsequent `erase(size - 1)` would underflow, so bail.
                    if (!base::scnReadLine(scanner, message))
                        return Response(Response::Status::InvalidResponse);

                    // Remove the ending '\r' (all lines are terminated by "\r\n")
                    message.erase(message.size() - 1);
                    message = separator + message + "\n";
                }
                else
                {
                    // We must make sure that the code is the same, otherwise it means
                    // we haven't reached the end of the multiline response
                    if ((separator != '-') && ((code == lastCode) || (lastCode == 0)))
                    {
                        // Extract the line; same rationale as above for failing on EOF.
                        base::String line;
                        if (!base::scnReadLine(scanner, line))
                            return Response(Response::Status::InvalidResponse);

                        // Remove the ending '\r' (all lines are terminated by "\r\n")
                        line.erase(line.size() - 1);

                        // Append it to the message
                        if (code == lastCode)
                        {
                            message += base::fmtToString("{}{}{}", code, base::StringView{&separator, 1u}, line);
                        }
                        else
                        {
                            message = separator + line;
                        }

                        // Save the remaining data for the next time getResponse() is called
                        m_impl->receiveBuffer.assign(buffer + scanner.bytesConsumed(), length - scanner.bytesConsumed());

                        // Return the response code and message
                        return Response(static_cast<Response::Status>(code), SFML_BASE_MOVE(message));
                    }

                    // The line we just read was actually not a response,
                    // only a new part of the current multiline response
                    base::String line;
                    if (!base::scnReadLine(scanner, line))
                        return Response(Response::Status::InvalidResponse);

                    if (!line.empty())
                    {
                        // Remove the ending '\r' (all lines are terminated by "\r\n")
                        line.erase(line.size() - 1);

                        // Append it to the current message
                        message += base::fmtToString("{}{}{}\n", code, base::StringView{&separator, 1u}, line);
                    }
                }
            }
            else if (lastCode != 0)
            {
                // We are in the middle of a multiline response: the current line
                // didn't start with a numeric code. `scnInto` for the int failed
                // without consuming any bytes (after the initial whitespace skip),
                // so the rest of the line is intact below.
                base::String line;
                if (!base::scnReadLine(scanner, line))
                    return Response(Response::Status::InvalidResponse);

                if (!line.empty())
                {
                    // Remove the ending '\r' (all lines are terminated by "\r\n")
                    line.erase(line.size() - 1);

                    // Append it to the current message
                    message += line + "\n";
                }
            }
            else
            {
                // Error: cannot extract the code, and we are not in a multiline response
                return Response(Response::Status::InvalidResponse);
            }
        }
    }

    // We never reach there
}


////////////////////////////////////////////////////////////
Ftp::DataChannel::DataChannel(Ftp& owner) : m_ftp(owner)
{
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::DataChannel::open(Ftp::TransferMode mode)
{
    // Open a data connection in passive mode (we connect to the server)
    Ftp::Response response = m_ftp.sendCommand("PASV"); // Use a single local variable for NRVO

    if (!response.isOk())
        return response;

    // Extract the connection address and port from the response
    const base::SizeT begin = response.getMessage().findFirstOf("0123456789");
    if (begin == base::String::nPos)
        return response;

    const auto        str     = response.getMessage().substrByPosLen(begin).toString<base::String>();
    const base::SizeT strSize = str.size();

    base::SizeT index   = 0;
    base::U8    data[6] = {0, 0, 0, 0, 0, 0};

    for (unsigned char& datum : data)
    {
        // Extract the current number
        while (index < strSize && std::isdigit(static_cast<unsigned char>(str[index])))
        {
            datum = static_cast<base::U8>(static_cast<base::U8>(datum * 10) + static_cast<base::U8>(str[index] - '0'));
            ++index;
        }

        // Skip separator
        ++index;
    }

    // Reconstruct connection port and address
    const auto      port = static_cast<base::U16>(data[4] * 256 + data[5]);
    const IpAddress address(data[0], data[1], data[2], data[3]);

    // Create and connect the data channel socket
    m_dataSocket = TcpSocket::create(/* isBlocking */ true);
    if (!m_dataSocket.hasValue())
    {
        response = Ftp::Response(Ftp::Response::Status::ConnectionFailed);
        return response;
    }

    if (m_dataSocket->connect(address, port) != Socket::Status::Done)
    {
        m_dataSocket.reset();

        response = Ftp::Response(Ftp::Response::Status::ConnectionFailed);
        return response;
    }

    // Translate the transfer mode to the corresponding FTP parameter
    base::String modeStr;
    switch (mode)
    {
        case Ftp::TransferMode::Binary:
            modeStr = "I";
            break;
        case Ftp::TransferMode::Ascii:
            modeStr = "A";
            break;
        case Ftp::TransferMode::Ebcdic:
            modeStr = "E";
            break;
    }

    // Set the transfer mode
    response = m_ftp.sendCommand("TYPE", modeStr);
    return response;
}


////////////////////////////////////////////////////////////
void Ftp::DataChannel::receive(auto& stream)
{
    SFML_BASE_ASSERT(m_dataSocket.hasValue() && "DataChannel::receive called without open()");

    // For sinks that report errors (`OutFile`), `write` returns `[[nodiscard]] bool`
    // and we propagate. For pure-memory sinks (`base::String`), `write` isn't
    // available -- fall back to `append`, which always succeeds.
    const auto sinkAccepts = [&stream](const char* data, base::SizeT n) -> bool
    {
        if constexpr (requires { stream.write(data, n); })
            return stream.write(data, n);
        else
        {
            stream.append(data, n);
            return true;
        }
    };

    // Receive data
    char        buffer[1024];
    base::SizeT received = 0;
    while (m_dataSocket->receive(buffer, sizeof(buffer), received) == Socket::Status::Done)
    {
        if (!sinkAccepts(buffer, received))
        {
            priv::errMsg("FTP Error: Writing to the file has failed");
            break;
        }
    }

    // Tear down the data socket
    m_dataSocket.reset();
}


////////////////////////////////////////////////////////////
void Ftp::DataChannel::send(auto& stream)
{
    SFML_BASE_ASSERT(m_dataSocket.hasValue() && "DataChannel::send called without open()");

    // Send data
    char buffer[1024];

    for (;;)
    {
        base::SizeT count = 0;
        if (!stream.read(buffer, sizeof(buffer), count))
        {
            priv::errMsg("FTP Error: Reading from the file has failed");
            break;
        }

        if (count == 0u) // EOF
            break;

        if (m_dataSocket->send(buffer, count) != Socket::Status::Done)
            break;
    }

    // Tear down the data socket
    m_dataSocket.reset();
}

} // namespace sf
