#pragma once

#include <string>

namespace text_manip {

bool is_text_empty(const std::string &input_text);
bool is_text_delimited(const std::string &input_text);

struct Parts {
    std::string head;
    std::string original_text;
    std::string modified_text;
    std::string tail;
};

Parts unpack_text_into_parts(const std::string &input_text);
std::string pack_parts_into_text(const Parts &parts);

} // namespace text_manip
