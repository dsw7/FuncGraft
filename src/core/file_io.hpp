#pragma once

#include <filesystem>
#include <string>

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
    bool is_delimited_ = false;
};

} // namespace file_io
} // namespace core
