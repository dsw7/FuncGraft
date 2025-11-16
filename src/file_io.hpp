#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace file_io {

std::string load_input_text(const std::filesystem::path &filename);
bool is_text_delimited(const std::string &input_text);

struct Parts {
    std::string head;
    std::string core;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &input_text);
std::string pack_parts_into_text(const Parts &parts);

class FileIO {
private:
    bool is_delimited_ = false;
    std::optional<std::string> core_;
    std::optional<std::string> head_;
    std::optional<std::string> tail_;
    std::optional<std::string> text_;

public:
    void load_input_text_from_file(const std::filesystem::path &filename);
    std::string get_text();
    void set_text(const std::string &text);
    std::string dump_output_text_to_string();
    void dump_output_text_to_file(const std::filesystem::path &filename);
};

} // namespace file_io
