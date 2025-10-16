// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Ftp.hpp"

#include "SFML/Network/IpAddress.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"
#include "SFML/System/Path.hpp"

#include "SFML/Base/Algorithm/Copy.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/IntTypes.hpp"
#include "SFML/Base/Macros.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/StringView.hpp"
#include "SFML/Base/Vector.hpp"

#include <string>

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
    Ftp&      m_ftp;        //!< Reference to the owner Ftp instance
    TcpSocket m_dataSocket; //!< Socket used for data transfers
};


////////////////////////////////////////////////////////////
struct Ftp::Response::Impl
{
    Status      status;  //!< Status code returned from the server
    std::string message; //!< Last message received from the server
};


////////////////////////////////////////////////////////////
Ftp::Response::Response(Status code, base::StringView message) : m_impl{code, message.toString<std::string>()}
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
        const std::string::size_type begin = getMessage().find('"', 0);
        const std::string::size_type end   = getMessage().find('"', begin + 1);
        m_directory = getMessage().substrByPosLen(begin + 1, end - begin - 1).toString<std::string>();
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
    base::Vector<std::string> listing; //!< Directory/file names extracted from the data
};


////////////////////////////////////////////////////////////
Ftp::ListingResponse::ListingResponse(const Ftp::Response& response, base::StringView data) : Ftp::Response(response)
{
    if (isOk())
    {
        // Fill the array of strings
        std::string::size_type lastPos = 0;
        for (std::string::size_type pos = data.find("\r\n"); pos != std::string::npos; pos = data.find("\r\n", lastPos))
        {
            m_impl->listing.pushBack(data.substrByPosLen(lastPos, pos - lastPos).toString<std::string>());
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
base::Span<const std::string> Ftp::ListingResponse::getListing() const
{
    return {m_impl->listing.data(), m_impl->listing.size()};
}


////////////////////////////////////////////////////////////
struct Ftp::Impl
{
    TcpSocket   commandSocket; //!< Socket holding the control connection with the server
    std::string receiveBuffer; //!< Received command data that is yet to be processed
};


////////////////////////////////////////////////////////////
Ftp::Ftp() : m_impl{TcpSocket{/* isBlocking */ true}, ""}
{
}


////////////////////////////////////////////////////////////
Ftp::~Ftp()
{
    (void)disconnect();
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::connect(IpAddress server, unsigned short port, Time timeout)
{
    // Connect to the server
    if (m_impl->commandSocket.connect(server, port, timeout) != Socket::Status::Done)
        return Response(Response::Status::ConnectionFailed);

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
    // Send the exit command
    Response response = sendCommand("QUIT");
    if (response.isOk())
    {
        [[maybe_unused]] const bool rc = m_impl->commandSocket.disconnect();
        SFML_BASE_ASSERT(rc);
    }

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
    OutStringStream oss;
    DataChannel     data(*this);
    Response        response = data.open(TransferMode::Ascii);
    if (response.isOk())
    {
        // Tell the server to send us the listing
        response = sendCommand("NLST", directory);
        if (response.isOk())
        {
            // Receive the listing
            data.receive(oss);

            // Get the response from the server
            response = getResponse();
        }
    }

    return {response, oss.getString()};
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
    Response response = sendCommand("RNFR", file.to<std::string>());
    if (response.isOk())
        response = sendCommand("RNTO", newName.to<std::string>());

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::deleteFile(const Path& name)
{
    return sendCommand("DELE", name.to<std::string>());
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
    response = sendCommand("RETR", remoteFile.to<std::string>());

    if (!response.isOk())
        return response;

    // Create the file and truncate it if necessary
    const Path    filepath = localPath / remoteFile.filename();
    OutFileStream file(filepath.to<std::string>(), FileOpenMode::bin | FileOpenMode::trunc);

    if (!file)
    {
        response = Response(Response::Status::InvalidFile);
        return response;
    }

    // Receive the file data
    data.receive(file);

    // Close the file
    file.close();

    // Get the response from the server
    response = getResponse();

    // If the download was unsuccessful, delete the partial file
    if (!response.isOk())
        if (!filepath.remove())
            priv::err() << "Failed to delete '" << filepath << '\'';

    return response;
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::upload(const Path& localFile, const Path& remotePath, TransferMode mode, bool append)
{
    Response response; //  Use a single local variable for NRVO

    // Get the contents of the file to send
    InFileStream file(localFile.to<std::string>(), FileOpenMode::bin);
    if (!file)
    {
        response = Response(Response::Status::InvalidFile);
        return response;
    }

    // Open a data channel using the given transfer mode
    DataChannel data(*this);
    response = data.open(mode);

    if (!response.isOk())
        return response;

    // Tell the server to start the transfer
    response = sendCommand(append ? "APPE" : "STOR", (remotePath / localFile.filename()).to<std::string>());

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
    auto commandStr = command.toString<std::string>();

    if (parameter.empty())
        commandStr += "\r\n";
    else
    {
        // TODO P2: concat utility from Open Hexagon?
        commandStr += ' ';
        commandStr += parameter.toString<std::string>();
        commandStr += "\r\n";
    }

    // Send it to the server
    if (m_impl->commandSocket.send(commandStr.c_str(), commandStr.length()) != Socket::Status::Done)
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
    std::string  message;

    for (;;)
    {
        // Receive the response from the server
        char        buffer[1024];
        base::SizeT length = 0;

        if (m_impl->receiveBuffer.empty())
        {
            if (m_impl->commandSocket.receive(buffer, sizeof(buffer), length) != Socket::Status::Done)
                return Response(Response::Status::ConnectionClosed);
        }
        else
        {
            base::copy(m_impl->receiveBuffer.begin(), m_impl->receiveBuffer.end(), buffer);
            length = m_impl->receiveBuffer.size();
            m_impl->receiveBuffer.clear();
        }

        // There can be several lines inside the received buffer, extract them all
        InStringStream in(std::string(buffer, length), FileOpenMode::bin);
        while (in)
        {
            // Try to extract the code
            unsigned int code = 0;
            if (in >> code)
            {
                // Extract the separator
                char separator = 0;
                in.get(separator);

                // The '-' character means a multiline response
                if ((separator == '-') && !isInsideMultiline)
                {
                    // Set the multiline flag
                    isInsideMultiline = true;

                    // Keep track of the code
                    if (lastCode == 0)
                        lastCode = code;

                    // Extract the line
                    sf::getLine(in, message);

                    // Remove the ending '\r' (all lines are terminated by "\r\n")
                    message.erase(message.length() - 1);
                    message = separator + message + "\n";
                }
                else
                {
                    // We must make sure that the code is the same, otherwise it means
                    // we haven't reached the end of the multiline response
                    if ((separator != '-') && ((code == lastCode) || (lastCode == 0)))
                    {
                        // Extract the line
                        std::string line;
                        sf::getLine(in, line);

                        // Remove the ending '\r' (all lines are terminated by "\r\n")
                        line.erase(line.length() - 1);

                        // Append it to the message
                        if (code == lastCode)
                        {
                            OutStringStream oss;
                            oss << code << separator << line;
                            message += oss.getString();
                        }
                        else
                        {
                            message = separator + line;
                        }

                        // Save the remaining data for the next time getResponse() is called
                        m_impl->receiveBuffer.assign(buffer + static_cast<base::SizeT>(in.tellg()),
                                                     length - static_cast<base::SizeT>(in.tellg()));

                        // Return the response code and message
                        return Response(static_cast<Response::Status>(code), SFML_BASE_MOVE(message));
                    }

                    // The line we just read was actually not a response,
                    // only a new part of the current multiline response

                    // Extract the line
                    std::string line;
                    sf::getLine(in, line);

                    if (!line.empty())
                    {
                        // Remove the ending '\r' (all lines are terminated by "\r\n")
                        line.erase(line.length() - 1);

                        // Append it to the current message
                        OutStringStream oss;
                        oss << code << separator << line << '\n';
                        message += oss.getString();
                    }
                }
            }
            else if (lastCode != 0)
            {
                // It seems we are in the middle of a multiline response

                // Clear the error bits of the stream
                in.clear();

                // Extract the line
                std::string line;
                sf::getLine(in, line);

                if (!line.empty())
                {
                    // Remove the ending '\r' (all lines are terminated by "\r\n")
                    line.erase(line.length() - 1);

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
Ftp::DataChannel::DataChannel(Ftp& owner) : m_ftp(owner), m_dataSocket(/* isBlocking */ true)
{
}


////////////////////////////////////////////////////////////
Ftp::Response Ftp::DataChannel::open(Ftp::TransferMode mode)
{
    // Open a data connection in active mode (we connect to the server)
    Ftp::Response response = m_ftp.sendCommand("PASV");
    if (response.isOk())
    {
        // Extract the connection address and port from the response
        const std::string::size_type begin = response.getMessage().findFirstOf("0123456789");
        if (begin != std::string::npos)
        {
            base::U8    data[6] = {0, 0, 0, 0, 0, 0};
            auto        str     = response.getMessage().substrByPosLen(begin).toString<std::string>();
            base::SizeT index   = 0;
            for (unsigned char& datum : data)
            {
                // Extract the current number
                while (std::isdigit(str[index]))
                {
                    datum = static_cast<base::U8>(
                        static_cast<base::U8>(datum * 10) + static_cast<base::U8>(str[index] - '0'));
                    ++index;
                }

                // Skip separator
                ++index;
            }

            // Reconstruct connection port and address
            const auto      port = static_cast<base::U16>(data[4] * 256 + data[5]);
            const IpAddress address(data[0], data[1], data[2], data[3]);

            // Connect the data channel to the server
            if (m_dataSocket.connect(address, port) == Socket::Status::Done)
            {
                // Translate the transfer mode to the corresponding FTP parameter
                std::string modeStr;
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
            }
            else
            {
                // Failed to connect to the server
                response = Ftp::Response(Ftp::Response::Status::ConnectionFailed);
            }
        }
    }

    return response;
}


////////////////////////////////////////////////////////////
void Ftp::DataChannel::receive(auto& stream)
{
    // Receive data
    char        buffer[1024];
    base::SizeT received = 0;
    while (m_dataSocket.receive(buffer, sizeof(buffer), received) == Socket::Status::Done)
    {
        stream.write(buffer, static_cast<sf::base::PtrDiffT>(received));

        if (!stream.isGood())
        {
            priv::err() << "FTP Error: Writing to the file has failed";
            break;
        }
    }

    // Close the data socket
    [[maybe_unused]] const bool rc = m_dataSocket.disconnect();
    SFML_BASE_ASSERT(rc);
}


////////////////////////////////////////////////////////////
void Ftp::DataChannel::send(auto& stream)
{
    // Send data
    char        buffer[1024];
    base::SizeT count = 0;

    for (;;)
    {
        // read some data from the stream
        stream.read(buffer, sizeof(buffer));

        if (!stream.isGood() && !stream.isEOF())
        {
            priv::err() << "FTP Error: Reading from the file has failed";
            break;
        }

        count = static_cast<base::SizeT>(stream.gcount());

        if (count > 0)
        {
            // we could read more data from the stream: send them
            if (m_dataSocket.send(buffer, count) != Socket::Status::Done)
                break;
        }
        else
        {
            // no more data: exit the loop
            break;
        }
    }

    // Close the data socket
    [[maybe_unused]] const bool rc = m_dataSocket.disconnect();
    SFML_BASE_ASSERT(rc);
}

} // namespace sf
