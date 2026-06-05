////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Http.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"
#include "Zancle/String/FromChars.hpp"
#include "Zancle/Scn/ScnStdin.hpp"
#include "Zancle/Scn/ScnString.hpp"
#include "Zancle/String/String.hpp"
#include "Zancle/String/StringView.hpp"


namespace
{
////////////////////////////////////////////////////////////
/// Request a URL
///
////////////////////////////////////////////////////////////
void requestUrl(const za::StringView& url, int redirectsRemaining)
{
    // Split the URL up into host and resource parts
    const auto resourcePos = url.find('/', url.find("://") + 3);

    const auto resource = (resourcePos != za::StringView::nPos) ? url.substrByPosLen(resourcePos) : za::StringView("/");

    const auto host = url.substrByPosLen(0, resourcePos);

    const auto portPos = host.find(':', 6);

    const auto port = (portPos != za::StringView::nPos) ? host.substrByPosLen(portPos + 1) : za::StringView("0");

    // Create a new HTTP client
    za::Http http;

    unsigned short portNum = 0;

    const auto [ptr, ec] = za::fromChars(port.data(), port.data() + port.size(), portNum);

    if (ec != za::FromCharsError::None)
    {
        za::printLn("Invalid port number in URL: {}", port);
        return;
    }

    http.setHost(host.substrByPosLen(0, portPos).to<za::String>(), portNum);

    // Prepare a request to get the resource
    const za::Http::Request request(resource.to<za::String>());

    // Send the request
    const za::Http::Response response = http.sendRequest(request);

    // Check the numeric status code and display the result
    const auto statusNum = static_cast<int>(response.getStatus());
    za::printLn("Server responded with HTTP status {}", statusNum);

    // Output body if its content type is text-based and not compressed
    if (response.getField("Content-Type").find("text") == 0)
    {
        if (const auto encoding = response.getField("Content-Encoding"); encoding.empty())
        {
            za::printLn("{}", response.getBody());
        }
        else
        {
            za::printLn("{} compressed body content, length: {}", encoding, response.getBody().size());
        }
    }

    // Follow redirections (HTTP status codes 301 to 308)
    static constexpr auto movedPermanently  = 301;
    static constexpr auto permanentRedirect = 308;

    if (statusNum >= movedPermanently && statusNum <= permanentRedirect)
    {
        if (redirectsRemaining == 0)
        {
            za::printLn("Maximum number of redirects reached");
            return;
        }

        if (auto nextUrl = response.getField("Location"); !nextUrl.empty())
        {
            if ((nextUrl.find("http://") != 0) && (nextUrl.find("https://") != 0))
                nextUrl = host + nextUrl;

            za::printLn("Following redirect to {}", nextUrl);
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
    za::String url;

    do
    {
        za::print("Type the complete URL of the webpage to request: ");
        (void)za::scnStdinInto(url);
    } while ((url.find("http://") != 0) && (url.find("https://") != 0));

    za::printLn("\nRequesting {}", url);

    // Request the URL stopping at a maximum of 16 redirects
    requestUrl(url, 16);

    // Wait until the user presses 'enter' key
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
    za::scnStdinIgnoreLine();
}
