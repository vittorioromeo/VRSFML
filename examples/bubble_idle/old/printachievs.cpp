
std::vector<std::string> apinames;
    std::vector<std::string> displaynames;
    std::vector<std::string> descriptions;

    int idx = 0;
    for (const auto& [name, description, secret] : achievementData)
    {
        apinames.emplace_back("ACH_" + std::to_string(idx));
        displaynames.emplace_back(name);
        descriptions.emplace_back(description);

        std::cout << "ACH_" << idx << "\t" << name << "\t" << description << "\n";

        ++idx;
    }

    std::cout << "const apinames = [\n";
    for (const auto& name : apinames)
        std::cout << "`" << name << "`, \n";
    std::cout << "];\n\n";

    std::cout << "const displaynames = [\n";
    for (const auto& name : displaynames)
        std::cout << "`" << name << "`, \n";
    std::cout << "];\n\n";

    std::cout << "const descriptions = [\n";
    for (const auto& name : descriptions)
        std::cout << "`" << name << "`, \n";
    std::cout << "];\n\n";
