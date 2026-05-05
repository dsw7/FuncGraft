#pragma once

#include <filesystem>
#include <string>

namespace core {
namespace file_io {

class DelimitedContent {
public:
    void unpack_content_into_parts(const std::string &content);
    std::string get_core_original();
    void set_core_modified(const std::string &modified_core);
    std::string pack_parts_into_content();

private:
    std::string head_;
    std::string core_original_;
    std::string core_modified_;
    std::string tail_;
};

class FileToEdit {
public:
    FileToEdit(const std::filesystem::path &filename);
    std::string get_file_content();
    void set_file_content(const std::string &content);
    void export_content(const std::filesystem::path &filename);

private:
    std::string content_;
    bool is_delimited_ = false;
    DelimitedContent delim_content_;
};

} // namespace file_io
} // namespace core
