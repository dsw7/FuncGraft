#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace core {
namespace file_io {

class FileToEdit {
public:
    FileToEdit(const std::filesystem::path &filename);
    std::string get_file_content();
    void set_file_content(const std::string &content);
    void export_content(const std::filesystem::path &filename);

private:
    std::string content_;
};

void export_edited_file(const std::string &code, const fs::path &filename);
void export_edited_file_with_prompt(const std::string &code, const fs::path &filename);
} // namespace file_io
} // namespace core
