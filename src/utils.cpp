#include "utils.hpp"

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

} // namespace utils
