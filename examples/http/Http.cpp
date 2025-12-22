////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Http.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/FromChars.hpp"
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
        sf::cOut() << "Invalid port number in URL: " << port << '\n' << sf::endL;
        return;
    }

    http.setHost(host.substrByPosLen(0, portPos).to<sf::base::String>(), portNum);

    // Prepare a request to get the resource
    const sf::Http::Request request(resource.to<sf::base::String>());

    // Send the request
    const sf::Http::Response response = http.sendRequest(request);

    // Check the numeric status code and display the result
    const auto statusNum = static_cast<int>(response.getStatus());
    sf::cOut() << "Server responded with HTTP status " << statusNum << '\n' << sf::endL;

    // Output body if its content type is text-based and not compressed
    if (response.getField("Content-Type").toStringView().find("text") == 0)
    {
        if (const auto encoding = response.getField("Content-Encoding"); encoding.empty())
        {
            sf::cOut() << response.getBody() << sf::endL;
        }
        else
        {
            sf::cOut() << encoding << " compressed body content, length: " << response.getBody().size() << '\n'
                       << sf::endL;
        }
    }

    // Follow redirections (HTTP status codes 301 to 308)
    static constexpr auto movedPermanently  = 301;
    static constexpr auto permanentRedirect = 308;

    if (statusNum >= movedPermanently && statusNum <= permanentRedirect)
    {
        if (redirectsRemaining == 0)
        {
            sf::cOut() << "Maximum number of redirects reached" << sf::endL;
            return;
        }

        if (auto nextUrl = response.getField("Location"); !nextUrl.empty())
        {
            if ((nextUrl.toStringView().find("http://") != 0) && (nextUrl.toStringView().find("https://") != 0))
                nextUrl = host + nextUrl;

            sf::cOut() << "Following redirect to " << nextUrl << '\n' << sf::endL;
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
        sf::cOut() << "Type the complete URL of the webpage to request: ";
        sf::cIn() >> url;
    } while ((url.toStringView().find("http://") != 0) && (url.toStringView().find("https://") != 0));

    sf::cOut() << "\nRequesting " << url << '\n' << sf::endL;

    // Request the URL stopping at a maximum of 16 redirects
    requestUrl(url, 16);

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
    sf::cIn().ignore(10'000, '\n');
}
