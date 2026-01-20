#include "file_io.hpp"

#include "utils.hpp"

#include <fmt/core.h>
#include <iostream>
#include <stdexcept>

namespace pipeline {

std::string import_file_to_edit(const fs::path &filename)
{
    if (not fs::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not fs::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    return utils::read_from_file(filename);
}

void export_edited_file(const std::string &code, const fs::path &filename)
{
    utils::write_to_file(filename, code);
}

void export_edited_file_with_prompt(const std::string &code, const fs::path &filename)
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
        export_edited_file(code, filename);
    }
}

} // namespace pipeline
