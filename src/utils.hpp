#pragma once

#include <string>

namespace utils {

unsigned short get_term_width();
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);

} // namespace utils
