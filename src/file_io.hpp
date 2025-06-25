#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace file_io {

class FileIO {
private:
    bool is_delimited_ = false;
    std::string core_;
    std::string head_;
    std::string tail_;
    std::optional<std::string> text_;

public:
    void load_input_text(const std::filesystem::path &filename);
    void dump_output_text(const std::filesystem::path &filename);
    std::string get_text();
    void set_text(const std::string &text);
};

} // namespace file_io
