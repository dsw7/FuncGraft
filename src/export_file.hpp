#pragma once

#include <filesystem>
#include <string>

namespace export_file {
void export_file(const std::string &code, const std::filesystem::path &filename);
void export_file_with_prompt(const std::string &code, const std::filesystem::path &filename);
} // namespace export_file
