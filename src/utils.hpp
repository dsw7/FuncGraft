#pragma once

#include <filesystem>
#include <string>

namespace utils {

unsigned short get_term_width();
void print_separator();
void print_right_aligned_text(const std::string &message);
std::string read_from_file(const std::filesystem::path &filename);
void write_to_file(const std::string &filename, const std::string &text);
std::string seconds_to_hhmmss(const double seconds);

} // namespace utils
