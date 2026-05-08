#pragma once

#include <string>
#include <variant>

namespace core {
namespace code {

class DelimitedCode {
public:
    void unpack_content_into_parts(const std::string &content);
    std::string get_core_original() const;
    void set_core_modified(const std::string &modified_core);
    std::string pack_parts_into_content() const;

private:
    std::string head_;
    std::string core_original_;
    std::string core_modified_;
    std::string tail_;
};

class CodeToEdit {
public:
    CodeToEdit(const std::string &original_code);
    std::string get_original_code() const;
    void overwrite_original_code(const std::string &modified_code);
    bool is_delimited();
    std::string get_modified_code() const;

private:
    std::variant<std::string, DelimitedCode> code_;
};

} // namespace code
} // namespace core
