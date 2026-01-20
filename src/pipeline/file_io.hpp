#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace file_io {
std::string import_file_to_edit(const fs::path &filename);
void export_edited_file(const std::string &code, const fs::path &filename);
void export_edited_file_with_prompt(const std::string &code, const fs::path &filename);
} // namespace file_io
