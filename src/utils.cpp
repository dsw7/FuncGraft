#include "utils.hpp"

#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

unsigned short get_terminal_columns()
{
    static struct winsize window_size;
    window_size.ws_col = 0;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == 0) {
        return window_size.ws_col;
    }

    return 20;
}

} // namespace

namespace utils {

void print_separator()
{
    static unsigned short columns = get_terminal_columns();
    static std::string separator = std::string(columns, '-');

    fmt::print("{}\n", separator);
}

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

nlohmann::json parse_json(const std::string &response)
{
    nlohmann::json json;

    try {
        json = nlohmann::json::parse(response);
    } catch (const nlohmann::json::parse_error &e) {
        throw std::runtime_error(fmt::format("Failed to parse response: {}", e.what()));
    }

    return json;
}

} // namespace utils
