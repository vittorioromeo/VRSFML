////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Http.hpp"

#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/FromChars.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"
#include "ZancleBase/String.hpp"
#include "ZancleBase/StringView.hpp"


namespace
{
////////////////////////////////////////////////////////////
/// Request a URL
///
////////////////////////////////////////////////////////////
void requestUrl(const zb::StringView& url, int redirectsRemaining)
{
    // Split the URL up into host and resource parts
    const auto resourcePos = url.find('/', url.find("://") + 3);

    const auto resource = (resourcePos != zb::StringView::nPos) ? url.substrByPosLen(resourcePos)
                                                                      : zb::StringView("/");

    const auto host = url.substrByPosLen(0, resourcePos);

    const auto portPos = host.find(':', 6);

    const auto port = (portPos != zb::StringView::nPos) ? host.substrByPosLen(portPos + 1)
                                                              : zb::StringView("0");

    // Create a new HTTP client
    za::Http http;

    unsigned short portNum = 0;

    const auto [ptr, ec] = zb::fromChars(port.data(), port.data() + port.size(), portNum);

    if (ec != zb::FromCharsError::None)
    {
        zb::printLn("Invalid port number in URL: {}", port);
        return;
    }

    http.setHost(host.substrByPosLen(0, portPos).to<zb::String>(), portNum);

    // Prepare a request to get the resource
    const za::Http::Request request(resource.to<zb::String>());

    // Send the request
    const za::Http::Response response = http.sendRequest(request);

    // Check the numeric status code and display the result
    const auto statusNum = static_cast<int>(response.getStatus());
    zb::printLn("Server responded with HTTP status {}", statusNum);

    // Output body if its content type is text-based and not compressed
    if (response.getField("Content-Type").find("text") == 0)
    {
        if (const auto encoding = response.getField("Content-Encoding"); encoding.empty())
        {
            zb::printLn("{}", response.getBody());
        }
        else
        {
            zb::printLn("{} compressed body content, length: {}", encoding, response.getBody().size());
        }
    }

    // Follow redirections (HTTP status codes 301 to 308)
    static constexpr auto movedPermanently  = 301;
    static constexpr auto permanentRedirect = 308;

    if (statusNum >= movedPermanently && statusNum <= permanentRedirect)
    {
        if (redirectsRemaining == 0)
        {
            zb::printLn("Maximum number of redirects reached");
            return;
        }

        if (auto nextUrl = response.getField("Location"); !nextUrl.empty())
        {
            if ((nextUrl.find("http://") != 0) && (nextUrl.find("https://") != 0))
                nextUrl = host + nextUrl;

            zb::printLn("Following redirect to {}", nextUrl);
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
    zb::String url;

    do
    {
        zb::print("Type the complete URL of the webpage to request: ");
        (void)zb::scnStdinInto(url);
    } while ((url.find("http://") != 0) && (url.find("https://") != 0));

    zb::printLn("\nRequesting {}", url);

    // Request the URL stopping at a maximum of 16 redirects
    requestUrl(url, 16);

    // Wait until the user presses 'enter' key
    zb::printLn("Press enter to exit...");
    zb::scnStdinIgnoreLine();
    zb::scnStdinIgnoreLine();
}
