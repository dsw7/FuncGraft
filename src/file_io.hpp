#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace file_io {
std::string import_file(const fs::path &filename);
void export_file(const std::string &code, const fs::path &filename);
void export_file_with_prompt(const std::string &code, const fs::path &filename);
} // namespace file_io
