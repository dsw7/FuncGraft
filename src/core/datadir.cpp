#include "datadir.hpp"

#include <cstdlib>
#include <fmt/core.h>
#include <stdexcept>
#include <string>

namespace core {
namespace datadir {

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

} // namespace datadir
} // namespace core
