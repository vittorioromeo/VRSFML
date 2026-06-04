////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Ftp.hpp"
#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"
#include "ZancleBase/Fmt/Fmt.hpp"
#include "ZancleBase/Fmt/FmtNumeric.hpp"
#include "ZancleBase/Optional.hpp"
#include "ZancleBase/Scn/ScnStdin.hpp"
#include "ZancleBase/Scn/ScnString.hpp"


////////////////////////////////////////////////////////////
/// `Fmt` ADL hook for FTP response types. Lives in `namespace za`
/// so ADL finds it for the nested `za::Ftp::Response*` types.
////////////////////////////////////////////////////////////
namespace za
{
[[nodiscard]] inline zb::FmtResult fmtArg(zb::FmtSink& sink, const Ftp::Response& response, const zb::FmtSpec&)
{
    return sink.fmt("{}{}", static_cast<int>(response.getStatus()), response.getMessage());
}
} // namespace za


////////////////////////////////////////////////////////////
/// Main
///
////////////////////////////////////////////////////////////
int main()
{
    // Choose the server address
    zb::Optional<za::IpAddress> address;
    do
    {
        zb::print("Enter the FTP server address: ");

        zb::String addressStr;
        (void)zb::scnStdinInto(addressStr);
        address = za::IpAddressUtils::resolve(addressStr);
    } while (!address.hasValue());

    // Connect to the server
    za::Ftp                 server;
    const za::Ftp::Response connectResponse = server.connect(address.value());
    zb::printLn("{}", connectResponse);
    if (!connectResponse.isOk())
        return 1;

    // Ask for user name and password
    zb::String user;
    zb::String password;
    zb::print("User name: ");
    (void)zb::scnStdinInto(user);
    zb::print("Password: ");
    (void)zb::scnStdinInto(password);

    // Login to the server
    const za::Ftp::Response loginResponse = server.login(user, password);
    zb::printLn("{}", loginResponse);
    if (!loginResponse.isOk())
        return 1;

    // Main menu
    int choice = 0;
    do
    {
        // Main FTP menu
        zb::printLn(
            "{}Choose an action:\n1. Print working directory\n2. Print contents of working directory\n3. Change "
            "directory\n4. Create directory\n5. Delete directory\n6. Rename file\n7. Remove file\n8. Download file\n9. "
            "Upload file\n0. Disconnect",
            '\n');

        zb::print("Your choice: ");
        (void)zb::scnStdinInto(choice);
        zb::printLn("");

        switch (choice)
        {
            default:
            {
                // Wrong choice
                zb::printLn("Invalid choice!");

                zb::scnStdinIgnoreLine();
                break;
            }

            case 1:
            {
                // Print the current server directory
                const za::Ftp::DirectoryResponse response = server.getWorkingDirectory();
                zb::printLn("{}{}Current directory is {}", response, '\n', response.getDirectory().to<zb::String>());
                break;
            }

            case 2:
            {
                // Print the contents of the current server directory
                const za::Ftp::ListingResponse response = server.getDirectoryListing();
                zb::printLn("{}", response);
                for (const zb::String& name : response.getListing())
                    zb::printLn("{}", name);
                break;
            }

            case 3:
            {
                // Change the current directory
                zb::String directory;
                zb::print("Choose a directory: ");
                (void)zb::scnStdinInto(directory);
                zb::printLn("{}", server.changeDirectory(directory));
                break;
            }

            case 4:
            {
                // Create a new directory
                zb::String directory;
                zb::print("Name of the directory to create: ");
                (void)zb::scnStdinInto(directory);
                zb::printLn("{}", server.createDirectory(directory));
                break;
            }

            case 5:
            {
                // Remove an existing directory
                zb::String directory;
                zb::print("Name of the directory to remove: ");
                (void)zb::scnStdinInto(directory);
                zb::printLn("{}", server.deleteDirectory(directory));
                break;
            }

            case 6:
            {
                // Rename a file
                zb::String source;
                zb::String destination;
                zb::print("Name of the file to rename: ");
                (void)zb::scnStdinInto(source);
                zb::print("New name: ");
                (void)zb::scnStdinInto(destination);
                zb::printLn("{}", server.renameFile(source, destination));
                break;
            }

            case 7:
            {
                // Remove an existing directory
                zb::String filename;
                zb::print("Name of the file to remove: ");
                (void)zb::scnStdinInto(filename);
                zb::printLn("{}", server.deleteFile(filename));
                break;
            }

            case 8:
            {
                // Download a file from server
                zb::String filename;
                zb::String directory;
                zb::print("Filename of the file to download (relative to current directory): ");
                (void)zb::scnStdinInto(filename);
                zb::print("Directory to download the file to: ");
                (void)zb::scnStdinInto(directory);
                zb::printLn("{}", server.download(filename, directory));
                break;
            }

            case 9:
            {
                // Upload a file to server
                zb::String filename;
                zb::String directory;
                zb::print("Path of the file to upload (absolute or relative to working directory): ");
                (void)zb::scnStdinInto(filename);
                zb::print("Directory to upload the file to (relative to current directory): ");
                (void)zb::scnStdinInto(directory);
                zb::printLn("{}", server.upload(filename, directory));
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
    zb::printLn("Disconnecting from server...\n{}", server.disconnect());

    // Wait until the user presses 'enter' key
    zb::printLn("Press enter to exit...");
    zb::scnStdinIgnoreLine();
    zb::scnStdinIgnoreLine();
}
