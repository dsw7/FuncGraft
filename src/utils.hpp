#pragma once

#include <string>

namespace utils {
void print_separator();
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
} // namespace utils
