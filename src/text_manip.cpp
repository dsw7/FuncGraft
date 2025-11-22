#include "text_manip.hpp"

#include "utils.hpp"

#include <algorithm>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>

namespace {

const std::string DELIMITER_LINE_ = "@@@\n";
const std::size_t SIZE_DELIM_LINE_ = DELIMITER_LINE_.size();

const std::string MARKER_ORIGINAL_ = "<<<<<<< Original code\n";
const std::string MARKER_SPLIT_ = "=======\n";
const std::string MARKER_MODIFIED_ = ">>>>>>> Modified code\n";

struct Positions {
    std::size_t pos_start_1 = 0;
    std::size_t pos_end_1 = 0;
    std::size_t pos_start_2 = 0;
    std::size_t pos_end_2 = 0;
};

Positions get_delimiter_positions(const std::string &text)
{
    std::vector<std::size_t> indices;
    std::size_t pos = text.find(DELIMITER_LINE_);

    while (pos != std::string::npos) {
        indices.push_back(pos);
        pos = text.find(DELIMITER_LINE_, pos + SIZE_DELIM_LINE_);
    }

    int num_indices = indices.size();

    if (num_indices == 1) {
        throw std::runtime_error("No matching closing delimiter line");
    } else if (num_indices != 2) {
        throw std::runtime_error("The number of delimiter lines must be exactly 2");
    }

    Positions positions;
    positions.pos_start_1 = indices[0];
    positions.pos_end_1 = indices[0] + SIZE_DELIM_LINE_;
    positions.pos_start_2 = indices[1];
    positions.pos_end_2 = indices[1] + SIZE_DELIM_LINE_;
    return positions;
}

} // namespace

namespace text_manip {

bool is_text_empty(const std::string &input_text)
{
    if (input_text.empty()) {
        return false;
    }

    return std::all_of(input_text.begin(), input_text.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c));
    });
}

bool is_text_delimited(const std::string &input_text)
{
    return input_text.find(DELIMITER_LINE_) != std::string::npos;
}

Parts unpack_text_into_parts(const std::string &input_text)
{
    /*
     * Unpack text of form:
     * -> "aaaa\n@@@\nbbbb\n@@@\ncccc\n"
     * Then return "bbbb\n" for processing
     *
     * To:
     * head: aaaa\n
     * original_text: bbbb\n
     * tail: cccc\n
     *
     * So we can operate on the "core" only
     */
    const Positions positions = get_delimiter_positions(input_text);

    Parts parts;
    parts.head = input_text.substr(0, positions.pos_start_1);
    parts.original_text = input_text.substr(positions.pos_end_1, positions.pos_start_2 - positions.pos_end_1);
    parts.tail = input_text.substr(positions.pos_end_2);
    return parts;
}

std::string pack_parts_into_text(const Parts &parts)
{
    std::string modified_text = parts.modified_text;

    if (not modified_text.empty() and modified_text.back() != '\n') {
        modified_text += '\n';
    }

#ifndef TESTING_ENABLED
    return fmt::format(
        "{}{}{}{}{}{}{}",
        parts.head,
        MARKER_ORIGINAL_,
        parts.original_text,
        MARKER_SPLIT_,
        modified_text,
        MARKER_MODIFIED_,
        parts.tail);
#else
    return fmt::format(
        "{}{}{}",
        parts.head,
        modified_text,
        parts.tail);
#endif
}

} // namespace text_manip
