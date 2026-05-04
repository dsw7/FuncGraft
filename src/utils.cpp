#include "utils.hpp"

#include <cstdlib>
#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace utils {

std::string read_from_file(const std::string &filename)
{
    std::ifstream file(filename);

    if (not file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open '{}' for reading", filename));
    }

    std::stringstream buffer;

    buffer << file.rdbuf();
    file.close();

    const std::string text = buffer.str();
    return text;
}

void write_to_file(const std::string &filename, const std::string &text)
{
    std::ofstream file(filename);

    if (not file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open '{}' for writing", filename));
    }

    file << text;
    file.close();
}

std::filesystem::path get_project_data_dir()
{
    const char *home_dir = std::getenv("HOME");

    if (not home_dir) {
        throw std::runtime_error("Could not locate user home directory!");
    }

    const std::string proj_dir = std::string(home_dir) + "/.funcgraft";

    if (not std::filesystem::exists(proj_dir)) {
        throw std::runtime_error(fmt::format("Could not locate '{}'", proj_dir));
    }

    return std::filesystem::path(proj_dir);
}

} // namespace utils
