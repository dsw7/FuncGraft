#include "export_file.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <iostream>

namespace export_file {

void export_file(
    const std::string &code,
    const std::filesystem::path &filename)
{
    utils::write_to_file(filename, code);
}

void export_file_with_prompt(const std::string &code, const std::filesystem::path &filename)
{
    char choice = 'n';

    while (true) {
        fmt::print("Overwrite file? [y/n]: ");
        choice = std::cin.get();

        if (choice == 'y' or choice == 'n') {
            break;
        } else {
            fmt::print("Invalid choice. Input either 'y' or 'n'!\n");
        }
    }

    if (choice == 'y') {
        export_file(filename, code);
    }
}

} // namespace export_file
