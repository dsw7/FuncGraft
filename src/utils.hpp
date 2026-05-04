#pragma once

#include <filesystem>
#include <string>

namespace utils {

std::string read_from_file(const std::string &filename);
void write_to_file(const std::string &filename, const std::string &text);
std::filesystem::path get_project_data_dir();

} // namespace utils
