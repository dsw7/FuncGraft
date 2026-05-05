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

} // namespace file_io
} // namespace core
