// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/Zancle/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Http.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "Zancle/Network/Socket.hpp"
#include "Zancle/Network/TcpSocket.hpp"

#include "Zancle/System/IO.hpp"
#include "Zancle/System/Time.hpp"
#include "Zancle/System/Utf8String.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtToString.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Radix.hpp"
#include "ZancleBase/Scn/Scn.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/Scn/ScnStringSource.hpp"
#include "ZancleBase/SizeT.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"

#include <map>

#include <cctype>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] zb::String toLower(zb::String str)
{
    for (char& c : str)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return str;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] bool stringViewLowercaseEq(const zb::StringView a, const zb::StringView b)
{
    if (a.size() != b.size())
        return false;

    for (zb::SizeT i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<int>(a[i])) != std::tolower(static_cast<int>(b[i])))
            return false;

    return true;
}


////////////////////////////////////////////////////////////
using FieldTable = std::map<zb::String, zb::String>; // Use an ordered map for predictable payloads


////////////////////////////////////////////////////////////
void parseFields(zb::ScnStringSource& scanner, FieldTable& fields)
{
    zb::String line;
    while (zb::scnReadLine(scanner, line) && line.size() > 2)
    {
        const auto lineView = line.toStringView();

        const auto pos = lineView.find(": ");
        if (pos == zb::String::nPos)
            continue;

        // Extract the field name and its value
        const auto field = zb::String{lineView.substrByPosLen(0, pos)};
        auto       value = zb::String{lineView.substrByPosLen(pos + 2)};

        // Remove any trailing '\r' (CRLF line endings)
        if (!value.empty() && (value.back() == '\r'))
            value.erase(value.size() - 1);

        // Add the field
        fields[toLower(field)] = value;
    }
}


////////////////////////////////////////////////////////////
/// \brief Prepare the final request to send to the server
///
/// This is used internally by Http before sending the
/// request to the web server.
///
/// \return String containing the request, ready to be sent
///
////////////////////////////////////////////////////////////
[[nodiscard]] zb::String prepareRequest(
    const FieldTable&               fields,
    const za::Http::Request::Method method,
    const zb::String&               uri,
    const unsigned int              majorVersion,
    const unsigned int              minorVersion,
    const zb::String&               body)
{
    // Convert the method to its string representation
    const char* methodStr = "";
    switch (method)
    {
        case za::Http::Request::Method::Get:
            methodStr = "GET";
            break;
        case za::Http::Request::Method::Post:
            methodStr = "POST";
            break;
        case za::Http::Request::Method::Head:
            methodStr = "HEAD";
            break;
        case za::Http::Request::Method::Put:
            methodStr = "PUT";
            break;
        case za::Http::Request::Method::Delete:
            methodStr = "DELETE";
            break;
    }

    // Request line
    zb::String request;
    (void)zb::fmtTo(request, "{} {} HTTP/{}.{}\r\n", methodStr, uri, majorVersion, minorVersion);

    // Fields, blank line, body
    for (const auto& [fieldKey, fieldValue] : fields)
        (void)zb::fmtTo(request, "{}: {}\r\n", fieldKey, fieldValue);

    request += "\r\n";
    request += body;

    return request;
}

} // namespace


namespace za
{
////////////////////////////////////////////////////////////
struct Http::Request::Impl
{
    FieldTable   fields;          //!< Fields of the header associated to their value
    Method       method;          //!< Method to use for the request
    zb::String   uri;             //!< Target URI of the request
    unsigned int majorVersion{1}; //!< Major HTTP version
    unsigned int minorVersion{};  //!< Minor HTTP version
    zb::String   body;            //!< Body of the request

    explicit Impl(Method theMethod) : method(theMethod)
    {
    }
};


////////////////////////////////////////////////////////////
Http::Request::Request(const zb::String& uri, Method method, const zb::String& body) : m_impl(method)
{
    setUri(uri);
    setBody(body);
}


////////////////////////////////////////////////////////////
Http::Request::Request(const zb::String& uri, Method method) : Http::Request::Request(uri, method, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::Request(const zb::String& uri) : Http::Request::Request(uri, Method::Get, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::Request() : Http::Request::Request("/", Method::Get, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::~Request() = default;


////////////////////////////////////////////////////////////
void Http::Request::setField(const zb::String& field, const zb::String& value)
{
    m_impl->fields[toLower(field)] = value;
}


////////////////////////////////////////////////////////////
void Http::Request::setMethod(Http::Request::Method method)
{
    m_impl->method = method;
}


////////////////////////////////////////////////////////////
void Http::Request::setUri(const zb::String& uri)
{
    m_impl->uri = uri;

    // Make sure it starts with a '/'
    if (m_impl->uri.empty() || (m_impl->uri[0] != '/'))
        m_impl->uri.insert(0u, '/');
}


////////////////////////////////////////////////////////////
void Http::Request::setHttpVersion(unsigned int major, unsigned int minor)
{
    m_impl->majorVersion = major;
    m_impl->minorVersion = minor;
}


////////////////////////////////////////////////////////////
void Http::Request::setBody(const zb::String& body)
{
    m_impl->body = body;
}


////////////////////////////////////////////////////////////
bool Http::Request::hasField(const zb::String& field) const
{
    return m_impl->fields.contains(toLower(field));
}


////////////////////////////////////////////////////////////
struct Http::Response::Impl
{
    FieldTable   fields;                           //!< Fields of the header
    Status       status{Status::ConnectionFailed}; //!< Status code
    unsigned int majorVersion{};                   //!< Major HTTP version
    unsigned int minorVersion{};                   //!< Minor HTTP version
    zb::String   body;                             //!< Body of the response
};


////////////////////////////////////////////////////////////
Http::Response::Response() = default;


////////////////////////////////////////////////////////////
Http::Response::~Response() = default;


////////////////////////////////////////////////////////////
const zb::String& Http::Response::getField(const zb::String& field) const
{
    if (const auto it = m_impl->fields.find(toLower(field)); it != m_impl->fields.end())
        return it->second;

    static const zb::String empty;
    return empty;
}


////////////////////////////////////////////////////////////
Http::Response::Status Http::Response::getStatus() const
{
    return m_impl->status;
}


////////////////////////////////////////////////////////////
unsigned int Http::Response::getMajorHttpVersion() const
{
    return m_impl->majorVersion;
}


////////////////////////////////////////////////////////////
unsigned int Http::Response::getMinorHttpVersion() const
{
    return m_impl->minorVersion;
}


////////////////////////////////////////////////////////////
const zb::String& Http::Response::getBody() const
{
    return m_impl->body;
}


////////////////////////////////////////////////////////////
void Http::Response::parse(const zb::String& data)
{
    zb::ScnStringSource scanner{data.toStringView()};

    // Extract the HTTP version from the first line
    zb::String version;
    if (zb::scnInto(scanner, version))
    {
        const auto prefix = version.substrByPosLen(0u, 5u);

        if ((version.size() >= 8) && (version[6] == '.') && (stringViewLowercaseEq(prefix, "http/")) &&
            std::isdigit(version[5]) && std::isdigit(version[7]))
        {
            m_impl->majorVersion = static_cast<unsigned int>(version[5] - '0');
            m_impl->minorVersion = static_cast<unsigned int>(version[7] - '0');
        }
        else
        {
            // Invalid HTTP version
            m_impl->status = Status::InvalidResponse;
            return;
        }
    }

    // Extract the status code from the first line
    int status = 0;
    if (!zb::scnInto(scanner, status))
    {
        // Invalid status code
        m_impl->status = Status::InvalidResponse;
        return;
    }

    m_impl->status = static_cast<Status>(status);

    // Ignore the end of the first line
    zb::scnSkipPast(scanner, '\n');

    // Parse the other lines, which contain fields, one by one
    parseFields(scanner, m_impl->fields);

    m_impl->body.clear();

    // Determine whether the transfer is chunked
    if (toLower(getField("transfer-encoding")) != "chunked")
    {
        // Drain whatever remains of the source into the body, byte by byte.
        while (auto c = scanner.peek())
        {
            m_impl->body.pushBack(*c);
            scanner.consume();
        }
    }
    else
    {
        // Chunked - have to read chunk by chunk
        zb::SizeT length = 0;

        // Read all chunks, identified by a chunk-size not being 0
        while (zb::scnRadix(scanner, length, zb::Radix::Hex) && length != 0u)
        {
            // Drop the rest of the line (chunk-extension)
            zb::scnSkipPast(scanner, '\n');

            // Copy the actual content data
            for (zb::SizeT i = 0u; i < length; ++i)
            {
                auto c = scanner.peek();
                if (!c)
                    break;
                m_impl->body.pushBack(*c);
                scanner.consume();
            }
        }

        // Drop the rest of the line (chunk-extension)
        zb::scnSkipPast(scanner, '\n');

        // Read all trailers (if present)
        parseFields(scanner, m_impl->fields);
    }
}


////////////////////////////////////////////////////////////
struct Http::Impl
{
    zb::Optional<IpAddress> host;         //!< Web host address
    zb::String              hostName;     //!< Web host name
    unsigned short          port{0u};     //!< Port used for connection with host
    bool                    https{false}; //!< Use HTTPS
};


////////////////////////////////////////////////////////////
Http::Http() = default;


////////////////////////////////////////////////////////////
Http::~Http() = default;


////////////////////////////////////////////////////////////
Http::Http(const zb::String& host, unsigned short port)
{
    setHost(host, port);
}


////////////////////////////////////////////////////////////
bool Http::setHost(const zb::String& host, unsigned short port)
{
    // Check the protocol
    if (stringViewLowercaseEq(host.substrByPosLen(0u, 7u), "http://"))
    {
        // HTTP protocol
        m_impl->hostName = host.substrByPosLen(7u);
        m_impl->port     = (port != 0 ? port : 80);
    }
    else if (stringViewLowercaseEq(host.substrByPosLen(0u, 8u), "https://"))
    {
        // HTTPS protocol
        m_impl->hostName = host.substrByPosLen(8u);
        m_impl->port     = (port != 0 ? port : 443);
        m_impl->https    = true;
    }
    else
    {
        // Undefined protocol - use HTTP
        m_impl->hostName = host;
        m_impl->port     = (port != 0 ? port : 80);
    }

    // Remove any trailing '/' from the host name
    if (!m_impl->hostName.empty() && (m_impl->hostName.back() == '/'))
        m_impl->hostName.erase(m_impl->hostName.size() - 1);

    m_impl->host = IpAddressUtils::resolve(m_impl->hostName);
    return m_impl->host.hasValue();
}


////////////////////////////////////////////////////////////
Http::Response Http::sendRequest(const Http::Request& request, Time timeout, const bool verifyServer)
{
    // First make sure that the request is valid -- add missing mandatory fields
    Request toSend(request);

    if (!toSend.hasField("From"))
        toSend.setField("From", "user@zancle-dev.org");

    if (!toSend.hasField("User-Agent"))
        toSend.setField("User-Agent", "libzancle-network/3.x");

    if (!toSend.hasField("Host"))
        toSend.setField("Host", m_impl->hostName);

    if (!toSend.hasField("Content-Length"))
        toSend.setField("Content-Length", zb::fmtToString("{}", toSend.m_impl->body.size()));

    if ((toSend.m_impl->method == Request::Method::Post) && !toSend.hasField("Content-Type"))
        toSend.setField("Content-Type", "application/x-www-form-urlencoded");

    if ((toSend.m_impl->majorVersion * 10 + toSend.m_impl->minorVersion >= 11) && !toSend.hasField("Connection"))
        toSend.setField("Connection", "close");

    // Prepare the response
    Response received;

    if (!m_impl->host.hasValue())
        return received;

    // Create a fresh socket for this request
    auto connectionOpt = TcpSocket::create(/* isBlocking */ true);
    if (!connectionOpt.hasValue())
        return received;

    auto& connection = *connectionOpt;

    // Connect the socket to the host
    if (connection.connect(*m_impl->host, m_impl->port, timeout) != Socket::Status::Done)
        return received;

    if (m_impl->https &&
        (connection.setupTlsClient(m_impl->hostName, verifyServer) != TcpSocket::TlsStatus::HandshakeComplete))
    {
        connection.disconnect();
        return received;
    }

    // Convert the request to string and send it through the connected socket
    const zb::String requestStr = prepareRequest(toSend.m_impl->fields,
                                                 toSend.m_impl->method,
                                                 toSend.m_impl->uri,
                                                 toSend.m_impl->majorVersion,
                                                 toSend.m_impl->minorVersion,
                                                 toSend.m_impl->body);

    if (!requestStr.empty())
    {
        // Send it through the socket
        if (connection.send(requestStr.cStr(), requestStr.size()) == Socket::Status::Done)
        {
            // Wait for the server's response
            zb::String receivedStr;
            zb::SizeT  size = 0;
            char       buffer[1024];

            // When the HTTPS connection makes use of TLS 1.3 new session ticket
            // messages can be received by the client from the server at any time
            // When these messages are received the receive function will return Socket::Status::Partial
            // In this case We just continue to call receive until actual payload
            // data is available, the connection is closed or an error occurs
            auto result = connection.receive(buffer, sizeof(buffer), size);

            while ((result == Socket::Status::Done) || (result == Socket::Status::Partial))
            {
                // Only append payload data when it has been completely received
                if (result == Socket::Status::Done)
                    receivedStr.append(buffer, size);

                result = connection.receive(buffer, sizeof(buffer), size);
            }

            // Build the Response object from the received data
            received.parse(receivedStr);
        }
    }

    connection.disconnect();
    return received;
}

} // namespace za
