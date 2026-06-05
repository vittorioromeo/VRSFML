////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "Zancle/Network/Ftp.hpp"

#include "Zancle/Network/IpAddress.hpp"
#include "Zancle/Network/IpAddressUtils.hpp"

#include "Zancle/Fmt/Fmt.hpp"
#include "Zancle/Fmt/FmtNumeric.hpp"
#include "Zancle/Vocabulary/Optional.hpp"
#include "Zancle/Scn/ScnStdin.hpp"
#include "Zancle/Scn/ScnString.hpp"


////////////////////////////////////////////////////////////
/// `Fmt` ADL hook for FTP response types. Lives in `namespace za`
/// so ADL finds it for the nested `za::Ftp::Response*` types.
////////////////////////////////////////////////////////////
namespace za
{
[[nodiscard]] inline za::FmtResult fmtArg(za::FmtSink& sink, const Ftp::Response& response, const za::FmtSpec&)
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
    za::Optional<za::IpAddress> address;
    do
    {
        za::print("Enter the FTP server address: ");

        za::String addressStr;
        (void)za::scnStdinInto(addressStr);
        address = za::IpAddressUtils::resolve(addressStr);
    } while (!address.hasValue());

    // Connect to the server
    za::Ftp                 server;
    const za::Ftp::Response connectResponse = server.connect(address.value());
    za::printLn("{}", connectResponse);
    if (!connectResponse.isOk())
        return 1;

    // Ask for user name and password
    za::String user;
    za::String password;
    za::print("User name: ");
    (void)za::scnStdinInto(user);
    za::print("Password: ");
    (void)za::scnStdinInto(password);

    // Login to the server
    const za::Ftp::Response loginResponse = server.login(user, password);
    za::printLn("{}", loginResponse);
    if (!loginResponse.isOk())
        return 1;

    // Main menu
    int choice = 0;
    do
    {
        // Main FTP menu
        za::printLn(
            "{}Choose an action:\n1. Print working directory\n2. Print contents of working directory\n3. Change "
            "directory\n4. Create directory\n5. Delete directory\n6. Rename file\n7. Remove file\n8. Download file\n9. "
            "Upload file\n0. Disconnect",
            '\n');

        za::print("Your choice: ");
        (void)za::scnStdinInto(choice);
        za::printLn("");

        switch (choice)
        {
            default:
            {
                // Wrong choice
                za::printLn("Invalid choice!");

                za::scnStdinIgnoreLine();
                break;
            }

            case 1:
            {
                // Print the current server directory
                const za::Ftp::DirectoryResponse response = server.getWorkingDirectory();
                za::printLn("{}{}Current directory is {}", response, '\n', response.getDirectory().to<za::String>());
                break;
            }

            case 2:
            {
                // Print the contents of the current server directory
                const za::Ftp::ListingResponse response = server.getDirectoryListing();
                za::printLn("{}", response);
                for (const za::String& name : response.getListing())
                    za::printLn("{}", name);
                break;
            }

            case 3:
            {
                // Change the current directory
                za::String directory;
                za::print("Choose a directory: ");
                (void)za::scnStdinInto(directory);
                za::printLn("{}", server.changeDirectory(directory));
                break;
            }

            case 4:
            {
                // Create a new directory
                za::String directory;
                za::print("Name of the directory to create: ");
                (void)za::scnStdinInto(directory);
                za::printLn("{}", server.createDirectory(directory));
                break;
            }

            case 5:
            {
                // Remove an existing directory
                za::String directory;
                za::print("Name of the directory to remove: ");
                (void)za::scnStdinInto(directory);
                za::printLn("{}", server.deleteDirectory(directory));
                break;
            }

            case 6:
            {
                // Rename a file
                za::String source;
                za::String destination;
                za::print("Name of the file to rename: ");
                (void)za::scnStdinInto(source);
                za::print("New name: ");
                (void)za::scnStdinInto(destination);
                za::printLn("{}", server.renameFile(source, destination));
                break;
            }

            case 7:
            {
                // Remove an existing directory
                za::String filename;
                za::print("Name of the file to remove: ");
                (void)za::scnStdinInto(filename);
                za::printLn("{}", server.deleteFile(filename));
                break;
            }

            case 8:
            {
                // Download a file from server
                za::String filename;
                za::String directory;
                za::print("Filename of the file to download (relative to current directory): ");
                (void)za::scnStdinInto(filename);
                za::print("Directory to download the file to: ");
                (void)za::scnStdinInto(directory);
                za::printLn("{}", server.download(filename, directory));
                break;
            }

            case 9:
            {
                // Upload a file to server
                za::String filename;
                za::String directory;
                za::print("Path of the file to upload (absolute or relative to working directory): ");
                (void)za::scnStdinInto(filename);
                za::print("Directory to upload the file to (relative to current directory): ");
                (void)za::scnStdinInto(directory);
                za::printLn("{}", server.upload(filename, directory));
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
    za::printLn("Disconnecting from server...\n{}", server.disconnect());

    // Wait until the user presses 'enter' key
    za::printLn("Press enter to exit...");
    za::scnStdinIgnoreLine();
    za::scnStdinIgnoreLine();
}
