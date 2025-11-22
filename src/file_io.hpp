#pragma once

#include <filesystem>
#include <string>

namespace file_io {
std::string import_file(const std::filesystem::path &filename);
void export_file(const std::string &code, const std::filesystem::path &filename);
void export_file_with_prompt(const std::string &code, const std::filesystem::path &filename);
} // namespace file_io
