#pragma once

#include <string>

namespace import_file {

bool is_text_delimited(const std::string &input_text);

struct Parts {
    std::string head;
    std::string original_text;
    std::string modified_text;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &input_text);
std::string pack_parts_into_text(const Parts &parts);

} // namespace import_file
