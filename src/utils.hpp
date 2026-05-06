#pragma once

#include <filesystem>
#include <string>

namespace utils {

unsigned short get_term_width();
void print_separator();
std::string read_from_file(const std::filesystem::path &filename);
void write_to_file(const std::string &filename, const std::string &text);

} // namespace utils
