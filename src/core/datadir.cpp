#include "datadir.hpp"

#include <cstdlib>
#include <fmt/core.h>
#include <stdexcept>

namespace {

std::filesystem::path get_home_dir_()
{
    const char *home_dir = std::getenv("HOME");

    if (not home_dir) {
        throw std::runtime_error("Could not locate user home directory!");
    }

    return std::filesystem::path(home_dir);
}

std::filesystem::path get_project_dir_()
{
    static std::filesystem::path proj_dir = get_home_dir_() / ".funcgraft";

    if (not std::filesystem::exists(proj_dir)) {
        throw std::runtime_error(fmt::format("Could not locate '{}'", proj_dir.string()));
    }

    return proj_dir;
}

} // namespace

namespace core {
namespace datadir {

std::filesystem::path get_config_file()
{
    static std::filesystem::path config_file = get_project_dir_() / "funcgraft.toml";

    if (not std::filesystem::exists(config_file)) {
        throw std::runtime_error("Could not locate FuncGraft configuration file!");
    }

    return config_file;
}

} // namespace datadir
} // namespace core
