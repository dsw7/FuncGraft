#pragma once

#include <string>

namespace utils {
std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
std::string get_user_api_key();
} // namespace utils
