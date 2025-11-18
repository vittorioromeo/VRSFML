// LICENSE AND COPYRIGHT (C) INFORMATION
// https://github.com/vittorioromeo/VRSFML/blob/master/license.md


////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Http.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"
#include "SFML/Network/TcpSocket.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/IO.hpp"

#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/SizeT.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"

#include <map>

#include <cctype>


namespace
{
////////////////////////////////////////////////////////////
[[nodiscard]] sf::base::String toLower(sf::base::String str)
{
    for (char& c : str)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    return str;
}


////////////////////////////////////////////////////////////
[[nodiscard, gnu::const]] bool stringViewLowercaseEq(const sf::base::StringView a, const sf::base::StringView b)
{
    if (a.size() != b.size())
        return false;

    for (sf::base::SizeT i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<int>(a[i])) != std::tolower(static_cast<int>(b[i])))
            return false;

    return true;
}


////////////////////////////////////////////////////////////
using FieldTable = std::map<sf::base::String, sf::base::String>; // Use an ordered map for predictable payloads


////////////////////////////////////////////////////////////
void parseFields(auto& in, FieldTable& fields)
{
    sf::base::String line;
    while (sf::getLine(in, line) && (line.size() > 2))
    {
        const auto lineView = line.toStringView();

        const auto pos = lineView.find(": ");
        if (pos == sf::base::String::nPos)
            continue;

        // Extract the field name and its value
        const auto field = sf::base::String{lineView.substrByPosLen(0, pos)};
        auto       value = sf::base::String{lineView.substrByPosLen(pos + 2)};

        // Remove any trailing \r
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
[[nodiscard]] sf::base::String prepareRequest(
    const FieldTable&               fields,
    const sf::Http::Request::Method method,
    const sf::base::String&         uri,
    const unsigned int              majorVersion,
    const unsigned int              minorVersion,
    const sf::base::String&         body)
{
    sf::OutStringStream oss;

    // Convert the method to its string representation
    switch (method)
    {
        case sf::Http::Request::Method::Get:
            oss << "GET";
            break;
        case sf::Http::Request::Method::Post:
            oss << "POST";
            break;
        case sf::Http::Request::Method::Head:
            oss << "HEAD";
            break;
        case sf::Http::Request::Method::Put:
            oss << "PUT";
            break;
        case sf::Http::Request::Method::Delete:
            oss << "DELETE";
            break;
    }

    // Write the first line containing the request type
    oss << " " << uri << " ";
    oss << "HTTP/" << majorVersion << "." << minorVersion << "\r\n";

    // Write fields
    for (const auto& [fieldKey, fieldValue] : fields)
        oss << fieldKey << ": " << fieldValue << "\r\n";

    // Use an extra \r\n to separate the header from the body
    oss << "\r\n";

    // Add the body
    oss << body;

    return oss.to<sf::base::String>();
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Http::Request::Impl
{
    FieldTable   fields;          //!< Fields of the header associated to their value
    Method       method;          //!< Method to use for the request
    base::String uri;             //!< Target URI of the request
    unsigned int majorVersion{1}; //!< Major HTTP version
    unsigned int minorVersion{};  //!< Minor HTTP version
    base::String body;            //!< Body of the request

    explicit Impl(Method theMethod) : method(theMethod)
    {
    }
};


////////////////////////////////////////////////////////////
Http::Request::Request(const base::String& uri, Method method, const base::String& body) : m_impl(method)
{
    setUri(uri);
    setBody(body);
}


////////////////////////////////////////////////////////////
Http::Request::Request(const base::String& uri, Method method) : Http::Request::Request(uri, method, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::Request(const base::String& uri) : Http::Request::Request(uri, Method::Get, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::Request() : Http::Request::Request("/", Method::Get, "")
{
}


////////////////////////////////////////////////////////////
Http::Request::~Request() = default;


////////////////////////////////////////////////////////////
void Http::Request::setField(const base::String& field, const base::String& value)
{
    m_impl->fields[toLower(field)] = value;
}


////////////////////////////////////////////////////////////
void Http::Request::setMethod(Http::Request::Method method)
{
    m_impl->method = method;
}


////////////////////////////////////////////////////////////
void Http::Request::setUri(const base::String& uri)
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
void Http::Request::setBody(const base::String& body)
{
    m_impl->body = body;
}


////////////////////////////////////////////////////////////
bool Http::Request::hasField(const base::String& field) const
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
    base::String body;                             //!< Body of the response
};


////////////////////////////////////////////////////////////
Http::Response::Response() = default;


////////////////////////////////////////////////////////////
Http::Response::~Response() = default;


////////////////////////////////////////////////////////////
const base::String& Http::Response::getField(const base::String& field) const
{
    if (const auto it = m_impl->fields.find(toLower(field)); it != m_impl->fields.end())
        return it->second;

    static const base::String empty;
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
const base::String& Http::Response::getBody() const
{
    return m_impl->body;
}


////////////////////////////////////////////////////////////
void Http::Response::parse(const base::String& data)
{
    sf::InStringStream in(data);

    // Extract the HTTP version from the first line
    base::String version;
    if (in >> version)
    {
        const auto prefix = version.toStringView().substrByPosLen(0, 5);

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
    if (in >> status)
    {
        m_impl->status = static_cast<Status>(status);
    }
    else
    {
        // Invalid status code
        m_impl->status = Status::InvalidResponse;
        return;
    }

    // Ignore the end of the first line
    in.ignore(2'147'483'647, '\n');

    // Parse the other lines, which contain fields, one by one
    parseFields(in, m_impl->fields);

    m_impl->body.clear();

    // Determine whether the transfer is chunked
    if (toLower(getField("transfer-encoding")) != "chunked")
    {
        while (!in.isEOF())
        {
            char c; // NOLINT(cppcoreguidelines-init-variables)
            in.get(c);

            m_impl->body.pushBack(c);
        }
    }
    else
    {
        // Chunked - have to read chunk by chunk
        base::SizeT length = 0;

        // Read all chunks, identified by a chunk-size not being 0
        while (in >> Hex{} >> length)
        {
            // Drop the rest of the line (chunk-extension)
            in.ignore(2'147'483'647, '\n');

            // Copy the actual content data
            for (base::SizeT i = 0; ((i < length) && (!in.isEOF())); ++i)
            {
                char c; // NOLINT(cppcoreguidelines-init-variables)
                in.get(c);

                m_impl->body.pushBack(c);
            }
        }

        // Drop the rest of the line (chunk-extension)
        in.ignore(2'147'483'647, '\n');

        // Read all trailers (if present)
        parseFields(in, m_impl->fields);
    }
}


////////////////////////////////////////////////////////////
struct Http::Impl
{
    TcpSocket                 connection; //!< Connection to the host
    base::Optional<IpAddress> host;       //!< Web host address
    base::String              hostName;   //!< Web host name
    unsigned short            port{};     //!< Port used for connection with host

    explicit Impl() : connection(/* isBlocking */ true)
    {
    }
};


////////////////////////////////////////////////////////////
Http::Http() = default;


////////////////////////////////////////////////////////////
Http::~Http() = default;


////////////////////////////////////////////////////////////
Http::Http(const base::String& host, unsigned short port)
{
    setHost(host, port);
}


////////////////////////////////////////////////////////////
void Http::setHost(const base::String& host, unsigned short port)
{
    // Check the protocol
    if (stringViewLowercaseEq(host.toStringView().substrByPosLen(0, 7), "http://"))
    {
        // HTTP protocol
        m_impl->hostName = host.toStringView().substrByPosLen(7);
        m_impl->port     = (port != 0 ? port : 80);
    }
    else if (stringViewLowercaseEq(host.toStringView().substrByPosLen(0, 8), "https://"))
    {
        // HTTPS protocol -- unsupported (requires encryption and certificates and stuff...)
        priv::err() << "HTTPS protocol is not supported by sf::Http";
        m_impl->hostName.clear();
        m_impl->port = 0;
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
}


////////////////////////////////////////////////////////////
Http::Response Http::sendRequest(const Http::Request& request, Time timeout)
{
    // First make sure that the request is valid -- add missing mandatory fields
    Request toSend(request);

    if (!toSend.hasField("From"))
        toSend.setField("From", "user@sfml-dev.org");

    if (!toSend.hasField("User-Agent"))
        toSend.setField("User-Agent", "libsfml-network/3.x");

    if (!toSend.hasField("Host"))
        toSend.setField("Host", m_impl->hostName);

    if (!toSend.hasField("Content-Length"))
    {
        OutStringStream oss;
        oss << toSend.m_impl->body.size();
        toSend.setField("Content-Length", oss.to<base::String>());
    }

    if ((toSend.m_impl->method == Request::Method::Post) && !toSend.hasField("Content-Type"))
        toSend.setField("Content-Type", "application/x-www-form-urlencoded");

    if ((toSend.m_impl->majorVersion * 10 + toSend.m_impl->minorVersion >= 11) && !toSend.hasField("Connection"))
        toSend.setField("Connection", "close");

    // Prepare the response
    Response received;

    // Connect the socket to the host
    if (m_impl->connection.connect(m_impl->host.value(), m_impl->port, timeout) == Socket::Status::Done)
    {
        // Convert the request to string and send it through the connected socket
        const base::String requestStr = prepareRequest(toSend.m_impl->fields,
                                                       toSend.m_impl->method,
                                                       toSend.m_impl->uri,
                                                       toSend.m_impl->majorVersion,
                                                       toSend.m_impl->minorVersion,
                                                       toSend.m_impl->body);

        if (!requestStr.empty())
        {
            // Send it through the socket
            if (m_impl->connection.send(requestStr.cStr(), requestStr.size()) == Socket::Status::Done)
            {
                // Wait for the server's response
                base::String receivedStr;
                base::SizeT  size = 0;
                char         buffer[1024];
                while (m_impl->connection.receive(buffer, sizeof(buffer), size) == Socket::Status::Done)
                {
                    receivedStr.append(buffer, size);
                }

                // Build the Response object from the received data
                received.parse(receivedStr);
            }
        }

        // Close the connection
        [[maybe_unused]] const bool rc = m_impl->connection.disconnect();
        SFML_BASE_ASSERT(rc);
    }

    return received;
}

} // namespace sf
