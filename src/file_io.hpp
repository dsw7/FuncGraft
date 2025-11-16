#pragma once

#include <filesystem>
#include <string>

namespace file_io {

std::string read_input_text(const std::filesystem::path &filename);
bool is_text_delimited(const std::string &input_text);

struct Parts {
    std::string head;
    std::string core;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &input_text);
std::string pack_parts_into_text(const Parts &parts);
void write_output_text(const std::filesystem::path &filename, const std::string &output_text);

} // namespace file_io
