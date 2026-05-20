////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Http.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/FromChars.hpp"
#include "SFML/Base/Scn/ScnStdin.hpp"
#include "SFML/Base/Scn/ScnString.hpp"
#include "SFML/Base/String.hpp"
#include "SFML/Base/StringView.hpp"


namespace
{
////////////////////////////////////////////////////////////
/// Request a URL
///
////////////////////////////////////////////////////////////
void requestUrl(const sf::base::StringView& url, int redirectsRemaining)
{
    // Split the URL up into host and resource parts
    const auto resourcePos = url.find('/', url.find("://") + 3);

    const auto resource = (resourcePos != sf::base::StringView::nPos) ? url.substrByPosLen(resourcePos)
                                                                      : sf::base::StringView("/");

    const auto host = url.substrByPosLen(0, resourcePos);

    const auto portPos = host.find(':', 6);

    const auto port = (portPos != sf::base::StringView::nPos) ? host.substrByPosLen(portPos + 1)
                                                              : sf::base::StringView("0");

    // Create a new HTTP client
    sf::Http http;

    unsigned short portNum = 0;

    const auto [ptr, ec] = sf::base::fromChars(port.data(), port.data() + port.size(), portNum);

    if (ec != sf::base::FromCharsError::None)
    {
        sf::base::printLn("Invalid port number in URL: {}", port);
        return;
    }

    http.setHost(host.substrByPosLen(0, portPos).to<sf::base::String>(), portNum);

    // Prepare a request to get the resource
    const sf::Http::Request request(resource.to<sf::base::String>());

    // Send the request
    const sf::Http::Response response = http.sendRequest(request);

    // Check the numeric status code and display the result
    const auto statusNum = static_cast<int>(response.getStatus());
    sf::base::printLn("Server responded with HTTP status {}", statusNum);

    // Output body if its content type is text-based and not compressed
    if (response.getField("Content-Type").find("text") == 0)
    {
        if (const auto encoding = response.getField("Content-Encoding"); encoding.empty())
        {
            sf::base::printLn("{}", response.getBody());
        }
        else
        {
            sf::base::printLn("{} compressed body content, length: {}", encoding, response.getBody().size());
        }
    }

    // Follow redirections (HTTP status codes 301 to 308)
    static constexpr auto movedPermanently  = 301;
    static constexpr auto permanentRedirect = 308;

    if (statusNum >= movedPermanently && statusNum <= permanentRedirect)
    {
        if (redirectsRemaining == 0)
        {
            sf::base::printLn("Maximum number of redirects reached");
            return;
        }

        if (auto nextUrl = response.getField("Location"); !nextUrl.empty())
        {
            if ((nextUrl.find("http://") != 0) && (nextUrl.find("https://") != 0))
                nextUrl = host + nextUrl;

            sf::base::printLn("Following redirect to {}", nextUrl);
            requestUrl(nextUrl, redirectsRemaining - 1);
        }
    }
}
} // namespace


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main()
{
    // Ask for the complete webpage URL
    sf::base::String url;

    do
    {
        sf::base::print("Type the complete URL of the webpage to request: ");
        (void)sf::base::scnStdinInto(url);
    } while ((url.find("http://") != 0) && (url.find("https://") != 0));

    sf::base::printLn("\nRequesting {}", url);

    // Request the URL stopping at a maximum of 16 redirects
    requestUrl(url, 16);

    // Wait until the user presses 'enter' key
    sf::base::printLn("Press enter to exit...");
    sf::base::scnStdinIgnoreLine();
    sf::base::scnStdinIgnoreLine();
}
