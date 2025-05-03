////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Network/Ftp.hpp"

#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/IpAddressUtils.hpp"

#include "SFML/System/IO.hpp"

#include "SFML/Base/Optional.hpp"

#include <string>

#include <cstdlib>


namespace
{
////////////////////////////////////////////////////////////
/// Print a FTP response into a standard output stream
///
////////////////////////////////////////////////////////////
sf::IOStreamOutput& operator<<(sf::IOStreamOutput& stream, const sf::Ftp::Response& response)
{
    return stream << static_cast<int>(response.getStatus()) << response.getMessage();
}

sf::IOStreamOutput& operator<<(sf::IOStreamOutput& stream, const sf::Ftp::DirectoryResponse& response)
{
    return stream << static_cast<int>(response.getStatus()) << response.getMessage();
}

sf::IOStreamOutput& operator<<(sf::IOStreamOutput& stream, const sf::Ftp::ListingResponse& response)
{
    return stream << static_cast<int>(response.getStatus()) << response.getMessage();
}

} // namespace


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
        sf::cOut() << "Enter the FTP server address: ";

        std::string addressStr;
        sf::cIn() >> addressStr;
        address = sf::IpAddressUtils::resolve(addressStr);
    } while (!address.hasValue());

    // Connect to the server
    sf::Ftp                 server;
    const sf::Ftp::Response connectResponse = server.connect(address.value());
    sf::cOut() << connectResponse << sf::endL;
    if (!connectResponse.isOk())
        return EXIT_FAILURE;

    // Ask for user name and password
    std::string user;
    std::string password;
    sf::cOut() << "User name: ";
    sf::cIn() >> user;
    sf::cOut() << "Password: ";
    sf::cIn() >> password;

    // Login to the server
    const sf::Ftp::Response loginResponse = server.login(user, password);
    sf::cOut() << loginResponse << sf::endL;
    if (!loginResponse.isOk())
        return EXIT_FAILURE;

    // Main menu
    int choice = 0;
    do
    {
        // Main FTP menu
        sf::cOut() << '\n'
                   << "Choose an action:\n"
                   << "1. Print working directory\n"
                   << "2. Print contents of working directory\n"
                   << "3. Change directory\n"
                   << "4. Create directory\n"
                   << "5. Delete directory\n"
                   << "6. Rename file\n"
                   << "7. Remove file\n"
                   << "8. Download file\n"
                   << "9. Upload file\n"
                   << "0. Disconnect\n"
                   << sf::endL;

        sf::cOut() << "Your choice: ";
        sf::cIn() >> choice;
        sf::cOut() << sf::endL;

        switch (choice)
        {
            default:
            {
                // Wrong choice
                sf::cOut() << "Invalid choice!" << sf::endL;
                sf::cIn().clear();
                sf::cIn().ignore(10'000, '\n');
                break;
            }

            case 1:
            {
                // Print the current server directory
                const sf::Ftp::DirectoryResponse response = server.getWorkingDirectory();
                sf::cOut() << response << '\n'
                           << "Current directory is " << response.getDirectory().to<std::string>() << sf::endL;
                break;
            }

            case 2:
            {
                // Print the contents of the current server directory
                const sf::Ftp::ListingResponse response = server.getDirectoryListing();
                sf::cOut() << response << '\n';
                for (const std::string& name : response.getListing())
                    sf::cOut() << name << '\n';
                sf::cOut().flush();
                break;
            }

            case 3:
            {
                // Change the current directory
                std::string directory;
                sf::cOut() << "Choose a directory: ";
                sf::cIn() >> directory;
                sf::cOut() << server.changeDirectory(directory) << sf::endL;
                break;
            }

            case 4:
            {
                // Create a new directory
                std::string directory;
                sf::cOut() << "Name of the directory to create: ";
                sf::cIn() >> directory;
                sf::cOut() << server.createDirectory(directory) << sf::endL;
                break;
            }

            case 5:
            {
                // Remove an existing directory
                std::string directory;
                sf::cOut() << "Name of the directory to remove: ";
                sf::cIn() >> directory;
                sf::cOut() << server.deleteDirectory(directory) << sf::endL;
                break;
            }

            case 6:
            {
                // Rename a file
                std::string source;
                std::string destination;
                sf::cOut() << "Name of the file to rename: ";
                sf::cIn() >> source;
                sf::cOut() << "New name: ";
                sf::cIn() >> destination;
                sf::cOut() << server.renameFile(source, destination) << sf::endL;
                break;
            }

            case 7:
            {
                // Remove an existing directory
                std::string filename;
                sf::cOut() << "Name of the file to remove: ";
                sf::cIn() >> filename;
                sf::cOut() << server.deleteFile(filename) << sf::endL;
                break;
            }

            case 8:
            {
                // Download a file from server
                std::string filename;
                std::string directory;
                sf::cOut() << "Filename of the file to download (relative to current directory): ";
                sf::cIn() >> filename;
                sf::cOut() << "Directory to download the file to: ";
                sf::cIn() >> directory;
                sf::cOut() << server.download(filename, directory) << sf::endL;
                break;
            }

            case 9:
            {
                // Upload a file to server
                std::string filename;
                std::string directory;
                sf::cOut() << "Path of the file to upload (absolute or relative to working directory): ";
                sf::cIn() >> filename;
                sf::cOut() << "Directory to upload the file to (relative to current directory): ";
                sf::cIn() >> directory;
                sf::cOut() << server.upload(filename, directory) << sf::endL;
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
    sf::cOut() << "Disconnecting from server...\n" << server.disconnect() << '\n';

    // Wait until the user presses 'enter' key
    sf::cOut() << "Press enter to exit..." << sf::endL;
    sf::cIn().ignore(10'000, '\n');
    sf::cIn().ignore(10'000, '\n');
}
