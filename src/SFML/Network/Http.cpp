#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Network/Http.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/IpAddressUtils.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/StringUtils.hpp>

#include <SFML/Base/Optional.hpp>

#include <iterator>
#include <limits>
#include <map>
#include <sstream>

#include <cctype>
#include <cstddef>

namespace
{
////////////////////////////////////////////////////////////
using FieldTable = std::map<std::string, std::string>; // Use an ordered map for predictable payloads


////////////////////////////////////////////////////////////
void parseFields(std::istream& in, FieldTable& fields)
{
    std::string line;
    while (std::getline(in, line) && (line.size() > 2))
    {
        const std::string::size_type pos = line.find(": ");
        if (pos != std::string::npos)
        {
            // Extract the field name and its value
            const std::string field = line.substr(0, pos);
            std::string       value = line.substr(pos + 2);

            // Remove any trailing \r
            if (!value.empty() && (*value.rbegin() == '\r'))
                value.erase(value.size() - 1);

            // Add the field
            fields[sf::priv::toLower(field)] = value;
        }
    }
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct Http::Request::Impl
{
    FieldTable   fields;          //!< Fields of the header associated to their value
    Method       method;          //!< Method to use for the request
    std::string  uri;             //!< Target URI of the request
    unsigned int majorVersion{1}; //!< Major HTTP version
    unsigned int minorVersion{};  //!< Minor HTTP version
    std::string  body;            //!< Body of the request

    explicit Impl(Method theMethod) : method(theMethod)
    {
    }
};


////////////////////////////////////////////////////////////
Http::Request::Request(const std::string& uri, Method method, const std::string& body) : m_impl(method)
{
    setUri(uri);
    setBody(body);
}


////////////////////////////////////////////////////////////
Http::Request::~Request() = default;


////////////////////////////////////////////////////////////
void Http::Request::setField(const std::string& field, const std::string& value)
{
    m_impl->fields[priv::toLower(field)] = value;
}


////////////////////////////////////////////////////////////
void Http::Request::setMethod(Http::Request::Method method)
{
    m_impl->method = method;
}


////////////////////////////////////////////////////////////
void Http::Request::setUri(const std::string& uri)
{
    m_impl->uri = uri;

    // Make sure it starts with a '/'
    if (m_impl->uri.empty() || (m_impl->uri[0] != '/'))
        m_impl->uri.insert(m_impl->uri.begin(), '/');
}


////////////////////////////////////////////////////////////
void Http::Request::setHttpVersion(unsigned int major, unsigned int minor)
{
    m_impl->majorVersion = major;
    m_impl->minorVersion = minor;
}


////////////////////////////////////////////////////////////
void Http::Request::setBody(const std::string& body)
{
    m_impl->body = body;
}


////////////////////////////////////////////////////////////
std::string Http::Request::prepare() const
{
    std::ostringstream out;

    // Convert the method to its string representation
    switch (m_impl->method)
    {
        case Method::Get:
            out << "GET";
            break;
        case Method::Post:
            out << "POST";
            break;
        case Method::Head:
            out << "HEAD";
            break;
        case Method::Put:
            out << "PUT";
            break;
        case Method::Delete:
            out << "DELETE";
            break;
    }

    // Write the first line containing the request type
    out << " " << m_impl->uri << " ";
    out << "HTTP/" << m_impl->majorVersion << "." << m_impl->minorVersion << "\r\n";

    // Write fields
    for (const auto& [fieldKey, fieldValue] : m_impl->fields)
    {
        out << fieldKey << ": " << fieldValue << "\r\n";
    }

    // Use an extra \r\n to separate the header from the body
    out << "\r\n";

    // Add the body
    out << m_impl->body;

    return out.str();
}


////////////////////////////////////////////////////////////
bool Http::Request::hasField(const std::string& field) const
{
    return m_impl->fields.find(priv::toLower(field)) != m_impl->fields.end();
}


////////////////////////////////////////////////////////////
struct Http::Response::Impl
{
    FieldTable   fields;                           //!< Fields of the header
    Status       status{Status::ConnectionFailed}; //!< Status code
    unsigned int majorVersion{};                   //!< Major HTTP version
    unsigned int minorVersion{};                   //!< Minor HTTP version
    std::string  body;                             //!< Body of the response
};


////////////////////////////////////////////////////////////
Http::Response::Response() = default;


////////////////////////////////////////////////////////////
Http::Response::~Response() = default;


////////////////////////////////////////////////////////////
const std::string& Http::Response::getField(const std::string& field) const
{
    if (const auto it = m_impl->fields.find(priv::toLower(field)); it != m_impl->fields.end())
    {
        return it->second;
    }

    static const std::string empty;
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
const std::string& Http::Response::getBody() const
{
    return m_impl->body;
}


////////////////////////////////////////////////////////////
void Http::Response::parse(const std::string& data)
{
    std::istringstream in(data);

    // Extract the HTTP version from the first line
    std::string version;
    if (in >> version)
    {
        if ((version.size() >= 8) && (version[6] == '.') && (priv::toLower(version.substr(0, 5)) == "http/") &&
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
    in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // Parse the other lines, which contain fields, one by one
    parseFields(in, m_impl->fields);

    m_impl->body.clear();

    // Determine whether the transfer is chunked
    if (priv::toLower(getField("transfer-encoding")) != "chunked")
    {
        // Not chunked - just read everything at once
        for (auto it = std::istreambuf_iterator<char>(in); it != std::istreambuf_iterator<char>(); ++it)
            m_impl->body.push_back(*it);
    }
    else
    {
        // Chunked - have to read chunk by chunk
        std::size_t length = 0;

        // Read all chunks, identified by a chunk-size not being 0
        while (in >> std::hex >> length)
        {
            // Drop the rest of the line (chunk-extension)
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            // Copy the actual content data
            std::istreambuf_iterator<char>       it(in);
            const std::istreambuf_iterator<char> itEnd;
            for (std::size_t i = 0; ((i < length) && (it != itEnd)); ++i)
            {
                m_impl->body.push_back(*it);
                ++it; // Iterate in separate expression to work around false positive -Wnull-dereference warning in GCC 12.1.0
            }
        }

        // Drop the rest of the line (chunk-extension)
        in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        // Read all trailers (if present)
        parseFields(in, m_impl->fields);
    }
}


////////////////////////////////////////////////////////////
struct Http::Impl
{
    TcpSocket                 connection; //!< Connection to the host
    base::Optional<IpAddress> host;       //!< Web host address
    std::string               hostName;   //!< Web host name
    unsigned short            port{};     //!< Port used for connection with host
};


////////////////////////////////////////////////////////////
Http::Http() = default;


////////////////////////////////////////////////////////////
Http::~Http() = default;


////////////////////////////////////////////////////////////
Http::Http(const std::string& host, unsigned short port)
{
    setHost(host, port);
}


////////////////////////////////////////////////////////////
void Http::setHost(const std::string& host, unsigned short port)
{
    // Check the protocol
    if (priv::toLower(host.substr(0, 7)) == "http://")
    {
        // HTTP protocol
        m_impl->hostName = host.substr(7);
        m_impl->port     = (port != 0 ? port : 80);
    }
    else if (priv::toLower(host.substr(0, 8)) == "https://")
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
    if (!m_impl->hostName.empty() && (*m_impl->hostName.rbegin() == '/'))
        m_impl->hostName.erase(m_impl->hostName.size() - 1);

    m_impl->host = IpAddressUtils::resolve(m_impl->hostName);
}


////////////////////////////////////////////////////////////
Http::Response Http::sendRequest(const Http::Request& request, Time timeout)
{
    // First make sure that the request is valid -- add missing mandatory fields
    Request toSend(request);
    if (!toSend.hasField("From"))
    {
        toSend.setField("From", "user@sfml-dev.org");
    }
    if (!toSend.hasField("User-Agent"))
    {
        toSend.setField("User-Agent", "libsfml-network/3.x");
    }
    if (!toSend.hasField("Host"))
    {
        toSend.setField("Host", m_impl->hostName);
    }
    if (!toSend.hasField("Content-Length"))
    {
        std::ostringstream out;
        out << toSend.m_impl->body.size();
        toSend.setField("Content-Length", out.str());
    }
    if ((toSend.m_impl->method == Request::Method::Post) && !toSend.hasField("Content-Type"))
    {
        toSend.setField("Content-Type", "application/x-www-form-urlencoded");
    }
    if ((toSend.m_impl->majorVersion * 10 + toSend.m_impl->minorVersion >= 11) && !toSend.hasField("Connection"))
    {
        toSend.setField("Connection", "close");
    }

    // Prepare the response
    Response received;

    // Connect the socket to the host
    if (m_impl->connection.connect(m_impl->host.value(), m_impl->port, timeout) == Socket::Status::Done)
    {
        // Convert the request to string and send it through the connected socket
        const std::string requestStr = toSend.prepare();

        if (!requestStr.empty())
        {
            // Send it through the socket
            if (m_impl->connection.send(requestStr.c_str(), requestStr.size()) == Socket::Status::Done)
            {
                // Wait for the server's response
                std::string receivedStr;
                std::size_t size = 0;
                char        buffer[1024];
                while (m_impl->connection.receive(buffer, sizeof(buffer), size) == Socket::Status::Done)
                {
                    receivedStr.append(buffer, buffer + size);
                }

                // Build the Response object from the received data
                received.parse(receivedStr);
            }
        }

        // Close the connection
        m_impl->connection.disconnect();
    }

    return received;
}

} // namespace sf
