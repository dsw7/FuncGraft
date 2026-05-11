#include "utils.hpp"

#include <algorithm>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

std::string build_separator_(const unsigned short length)
{
    std::string s;

    for (short i = 0; i < length; i++) {
        s += "\u2500";
    }

    return s;
}

} // namespace

namespace utils {

unsigned short get_term_width()
{
    static struct winsize window_size;
    window_size.ws_col = 0;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == 0) {
        return window_size.ws_col;
    }

    return 20;
}

void print_separator()
{
    static unsigned short width = get_term_width();
    static std::string separator = build_separator_(width);

    fmt::print(fg(fmt::color::gray), "{}\n", separator);
}

void print_right_aligned_text(const std::string &message)
{
    static unsigned short width = get_term_width();
    static unsigned short r_pad = 2;

    fmt::print(fg(fmt::color::gray), "{:>{}}\n", message, width - r_pad);
}

std::string read_from_file(const std::filesystem::path &filename)
{
    if (not std::filesystem::exists(filename)) {
        throw std::runtime_error(fmt::format("File '{}' does not exist!", filename.string()));
    }

    if (not std::filesystem::is_regular_file(filename)) {
        throw std::runtime_error(fmt::format("Input '{}' is not a file!", filename.string()));
    }

    std::ifstream file(filename);

    if (not file.is_open()) {
        throw std::runtime_error(fmt::format("Unable to open '{}' for reading", filename.string()));
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

std::string seconds_to_hhmmss(const double seconds)
{
    int h = static_cast<int>(seconds) / 3600;
    int m = (static_cast<int>(seconds) % 3600) / 60;
    int s = static_cast<int>(seconds) % 60;

    std::string result;

    if (h > 0) {
        result += std::to_string(h) + "h";
        if (m > 0 || s > 0) {
            result += " ";
        }
    }

    if (m > 0) {
        result += std::to_string(m) + "m";
        if (s > 0) {
            result += " ";
        }
    }

    if (s > 0 || result.empty()) {
        result += std::to_string(s) + "s";
    }

    return result;
}

bool is_text_empty(const std::string &text)
{
    if (text.empty()) {
        return true;
    }

    return std::all_of(text.begin(), text.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    });
}

} // namespace utils
