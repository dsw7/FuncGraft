#pragma once

#include <filesystem>
#include <string>
#include <variant>

namespace core {
namespace code {

class DelimitedCode {
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

class CodeToEdit {
public:
    CodeToEdit(const std::string &original_code);
    std::string get_file_content();
    void set_file_content(const std::string &content);
    bool is_delimited();
    void export_content_to_file(const std::filesystem::path &filename);

private:
    std::variant<std::string, DelimitedCode> code_;
};

} // namespace code
} // namespace core
