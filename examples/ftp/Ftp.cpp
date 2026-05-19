////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Ftp.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Fmt/Fmt.hpp"
#include "SFML/Base/Fmt/FmtNumeric.hpp"
#include "SFML/Base/Optional.hpp"


////////////////////////////////////////////////////////////
/// `Fmt` ADL hook for FTP response types. Lives in `namespace sf`
/// so ADL finds it for the nested `sf::Ftp::Response*` types.
////////////////////////////////////////////////////////////
namespace sf
{
inline void fmtArg(sf::base::FmtSink& sink, const Ftp::Response& response, const sf::base::FmtSpec&)
{
    sink.fmt("{}{}", static_cast<int>(response.getStatus()), response.getMessage());
}
} // namespace sf


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose the server address
    sf::base::Optional<sf::IpAddress> address;
    do
    {
        sf::base::print("Enter the FTP server address: ");

        sf::base::String addressStr;
        sf::cIn() >> addressStr;
        address = sf::IpAddressUtils::resolve(addressStr);
    } while (!address.hasValue());

    // Connect to the server
    sf::Ftp                 server;
    const sf::Ftp::Response connectResponse = server.connect(address.value());
    sf::base::printLn("{}", connectResponse);
    if (!connectResponse.isOk())
        return 1;

    // Ask for user name and password
    sf::base::String user;
    sf::base::String password;
    sf::base::print("User name: ");
    sf::cIn() >> user;
    sf::base::print("Password: ");
    sf::cIn() >> password;

    // Login to the server
    const sf::Ftp::Response loginResponse = server.login(user, password);
    sf::base::printLn("{}", loginResponse);
    if (!loginResponse.isOk())
        return 1;

    // Main menu
    int choice = 0;
    do
    {
        // Main FTP menu
        sf::base::printLn(
            "{}Choose an action:\n1. Print working directory\n2. Print contents of working directory\n3. Change "
            "directory\n4. Create directory\n5. Delete directory\n6. Rename file\n7. Remove file\n8. Download file\n9. "
            "Upload file\n0. Disconnect",
            '\n');

        sf::base::print("Your choice: ");
        sf::cIn() >> choice;
        sf::base::printLn("");

        switch (choice)
        {
            default:
            {
                // Wrong choice
                sf::base::printLn("Invalid choice!");
                sf::cIn().clear();
                sf::cIn().ignore(10'000, '\n');
                break;
            }

            case 1:
            {
                // Print the current server directory
                const sf::Ftp::DirectoryResponse response = server.getWorkingDirectory();
                sf::base::printLn("{}{}Current directory is {}",
                                  response,
                                  '\n',
                                  response.getDirectory().to<sf::base::String>());
                break;
            }

            case 2:
            {
                // Print the contents of the current server directory
                const sf::Ftp::ListingResponse response = server.getDirectoryListing();
                sf::base::printLn("{}", response);
                for (const sf::base::String& name : response.getListing())
                    sf::base::printLn("{}", name);
                break;
            }

            case 3:
            {
                // Change the current directory
                sf::base::String directory;
                sf::base::print("Choose a directory: ");
                sf::cIn() >> directory;
                sf::base::printLn("{}", server.changeDirectory(directory));
                break;
            }

            case 4:
            {
                // Create a new directory
                sf::base::String directory;
                sf::base::print("Name of the directory to create: ");
                sf::cIn() >> directory;
                sf::base::printLn("{}", server.createDirectory(directory));
                break;
            }

            case 5:
            {
                // Remove an existing directory
                sf::base::String directory;
                sf::base::print("Name of the directory to remove: ");
                sf::cIn() >> directory;
                sf::base::printLn("{}", server.deleteDirectory(directory));
                break;
            }

            case 6:
            {
                // Rename a file
                sf::base::String source;
                sf::base::String destination;
                sf::base::print("Name of the file to rename: ");
                sf::cIn() >> source;
                sf::base::print("New name: ");
                sf::cIn() >> destination;
                sf::base::printLn("{}", server.renameFile(source, destination));
                break;
            }

            case 7:
            {
                // Remove an existing directory
                sf::base::String filename;
                sf::base::print("Name of the file to remove: ");
                sf::cIn() >> filename;
                sf::base::printLn("{}", server.deleteFile(filename));
                break;
            }

            case 8:
            {
                // Download a file from server
                sf::base::String filename;
                sf::base::String directory;
                sf::base::print("Filename of the file to download (relative to current directory): ");
                sf::cIn() >> filename;
                sf::base::print("Directory to download the file to: ");
                sf::cIn() >> directory;
                sf::base::printLn("{}", server.download(filename, directory));
                break;
            }

            case 9:
            {
                // Upload a file to server
                sf::base::String filename;
                sf::base::String directory;
                sf::base::print("Path of the file to upload (absolute or relative to working directory): ");
                sf::cIn() >> filename;
                sf::base::print("Directory to upload the file to (relative to current directory): ");
                sf::cIn() >> directory;
                sf::base::printLn("{}", server.upload(filename, directory));
                break;
            }

            case 0:
            {
                // Disconnect
                break;
            }
        }

    } while (choice != 0);

    // Disconnect from the server
    sf::base::printLn("Disconnecting from server...\n{}", server.disconnect());

    // Wait until the user presses 'enter' key
    sf::base::printLn("Press enter to exit...");
    sf::cIn().ignore(10'000, '\n');
    sf::cIn().ignore(10'000, '\n');
}
